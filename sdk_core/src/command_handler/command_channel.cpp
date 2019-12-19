//
// The MIT License (MIT)
//
// Copyright (c) 2019 Livox. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#include "command_channel.h"
#include <boost/bind.hpp>
#include "base/logging.h"
#include "base/network_util.h"
#include "command_impl.h"
#include "device_manager.h"
#include "livox_def.h"

using boost::atomic_uint16_t;
using boost::bind;
using std::list;
using std::make_pair;
using std::map;
using std::pair;
using std::string;

namespace livox {

CommandChannel::CommandChannel(apr_port_t port,
                               uint8_t handle,
                               const string &remote_ip,
                               CommandChannelDelegate *cb,
                               apr_pool_t *pool)
    : handle_(handle),
      port_(port),
      sock_(NULL),
      mem_pool_(pool),
      loop_(NULL),
      callback_(cb),
      comm_port_(new CommPort),
      heartbeat_time_(0),
      remote_ip_(remote_ip),
      last_heartbeat_(0) {}

bool CommandChannel::Bind(IOLoop *loop) {
  if (loop == NULL) {
    return false;
  }
  loop_ = loop;
  sock_ = util::CreateBindSocket(port_, mem_pool_);
  if (sock_ == NULL) {
    return false;
  }

  loop_->AddDelegate(sock_, this);
  return true;
}

void CommandChannel::OnData(apr_socket_t *, void *) {
  apr_sockaddr_t addr;
  uint32_t buf_size = 0;
  uint8_t *cache_buf = comm_port_->FetchCacheFreeSpace(&buf_size);
  apr_size_t size = buf_size;
  apr_status_t rv = apr_socket_recvfrom(&addr, sock_, 0, reinterpret_cast<char *>(cache_buf), &size);
  comm_port_->UpdateCacheWrIdx(size);
  if (rv != APR_SUCCESS) {
    LOG_ERROR(PrintAPRStatus(rv));
    return;
  }

  CommPacket packet;
  memset(&packet, 0, sizeof(packet));

  while ((kParseSuccess == comm_port_->ParseCommStream(&packet))) {
    if (packet.packet_type == kCommandTypeAck) {
      uint16_t seq = packet.seq_num;
      if (commands_.find(seq) != commands_.end()) {
        Command command = commands_[seq].first;
        command.packet = packet;
        if (callback_) {
          callback_->OnCommand(handle_, command);
        }
        commands_.erase(seq);
      } else if (packet.cmd_set == kCommandSetGeneral && packet.cmd_code == kCommandIDGeneralHeartbeat) {
        OnHeartbeatAck(packet);
        if (callback_) {
          callback_->OnHeartbeatStateUpdate(handle_, *(reinterpret_cast<HeartbeatResponse *>(packet.data)));
        }
      }
    } else if (packet.packet_type == kCommandTypeMsg) {
      if (callback_) {
        Command command;
        command.packet = packet;
        callback_->OnCommand(handle_, command);
      }
    }
  }
}

void CommandChannel::SendAsync(const Command &command) {
  Command cmd = DeepCopy(command);
  if (loop_) {
    loop_->PostTask(bind(&CommandChannel::Send, this, cmd));
  }
}

void CommandChannel::OnTimer(apr_time_t now) {
  list<Command> timeout_commands;
  map<uint16_t, pair<Command, apr_time_t> >::iterator ite = commands_.begin();
  while (ite != commands_.end()) {
    pair<Command, apr_time_t> &command_pair = ite->second;
    if (now > command_pair.second) {
      timeout_commands.push_back(command_pair.first);
      commands_.erase(ite++);
    } else {
      ++ite;
    }
  }

  for (list<Command>::iterator ite = timeout_commands.begin(); ite != timeout_commands.end(); ++ite) {
    LOG_WARN("Apr time now: {}, Command Timeout: Set {}, Id {}, Seq {}", PrintAPRTime(apr_time_now()), 
        (uint16_t)ite->packet.cmd_set, ite->packet.cmd_code, ite->packet.seq_num);
    if (callback_) {
      ite->packet.packet_type = kCommandTypeAck;
      callback_->OnCommand(handle_, *ite);
    }
  }

  if ((last_heartbeat_ != 0) && (now - last_heartbeat_ > apr_time_from_sec(3))) {
    DeviceDisconnect(handle_);
  } else {
    HeartBeat(now);
  }
}

void CommandChannel::Uninit() {
  if (sock_) {
    apr_os_thread_t thread_id = apr_os_thread_current();
    if (apr_os_thread_equal(loop_->GetThreadId(), thread_id)) {
      loop_->RemoveDelegateSync(sock_);
    } else {
      loop_->RemoveDelegate(sock_, this);
    }
    loop_ = NULL;
    apr_socket_close(sock_);
    sock_ = NULL;
  }
  callback_ = NULL;
  if (comm_port_) {
    comm_port_.reset(NULL);
  }

  commands_.clear();
  last_heartbeat_ = 0;
  heartbeat_time_ = 0;
  remote_ip_ = "";
}

void CommandChannel::HeartBeat(apr_time_t t) {
  if (heartbeat_time_ == 0 || (t - heartbeat_time_) > apr_time_from_msec(kHeartbeatTimer)) {
    heartbeat_time_ = t;

    Command command(handle_,
                    kCommandTypeCmd,
                    kCommandSetGeneral,
                    kCommandIDGeneralHeartbeat,
                    GenerateSeq(),
                    NULL,
                    0,
                    0,
                    boost::shared_ptr<CommandCallback>());
    SendInternal(command);
  }
}

void CommandChannel::SendInternal(const Command &command) {
  apr_pool_t *subpool = NULL;
  apr_pool_create(&subpool, mem_pool_);
  uint8_t *buf = (uint8_t *)apr_palloc(subpool, kMaxCommandBufferSize);
  uint32_t size = 0;
  comm_port_->Pack(buf, kMaxCommandBufferSize, &size, command.packet);
  apr_size_t apr_size = size;
  apr_status_t rv = APR_SUCCESS;
  apr_sockaddr_t *sa = NULL;
  rv = apr_sockaddr_info_get(&sa, remote_ip_.c_str(), APR_INET, 65000, 0, subpool);
  if (rv == APR_SUCCESS) {
    rv = apr_socket_sendto(sock_, sa, 0, (const char *)buf, &apr_size);
  }
  if (rv != APR_SUCCESS) {
    (*command.cb)(kStatusSendFailed, handle_, NULL);
  }
  apr_pool_destroy(subpool);
}

uint16_t CommandChannel::GenerateSeq() {
  static atomic_uint16_t seq(1);
  uint16_t value = seq.load();
  uint16_t desired = 0;
  do {
    if (value == UINT16_MAX) {
      desired = 1;
    } else {
      desired = value + 1;
    }
  } while (!seq.compare_exchange_weak(value, desired));
  return desired;
}

Command CommandChannel::DeepCopy(const Command &cmd) {
  Command result_cmd(cmd);
  if (result_cmd.packet.data != NULL) {
    result_cmd.packet.data = new uint8_t[result_cmd.packet.data_len];
    memcpy(result_cmd.packet.data, cmd.packet.data, result_cmd.packet.data_len);
  }
  return result_cmd;
}

void CommandChannel::OnHeartbeatAck(const CommPacket &) {
  last_heartbeat_ = apr_time_now();
}

void CommandChannel::DeviceDisconnect(uint8_t handle) {
  DeviceRemove(handle,kEventDisconnect);
}

void CommandChannel::Send(const Command &command) {
  LOG_INFO(" Send Command: Set {} Id {} Seq {}", (uint16_t)command.packet.cmd_set, command.packet.cmd_code, command.packet.seq_num);
  SendInternal(command);
  commands_[command.packet.seq_num] = make_pair(command, apr_time_now() + apr_time_from_msec(command.time_out));
  Command &cmd = commands_[command.packet.seq_num].first;
  if (cmd.packet.data != NULL) {
    delete[] cmd.packet.data;
    cmd.packet.data = NULL;
    cmd.packet.data_len = 0;
  }
}
}  // namespace livox
