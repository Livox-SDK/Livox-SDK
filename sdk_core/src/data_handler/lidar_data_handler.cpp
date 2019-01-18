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

#include "lidar_data_handler.h"
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/locks.hpp>
#include "base/network_util.h"

using boost::lock_guard;
using boost::mutex;
using boost::shared_ptr;
using std::list;

namespace livox {

bool LidarDataHandlerImpl::Init() {
  return true;
}

void LidarDataHandlerImpl::Uninit() {
  for (list<DeviceItem>::iterator ite = devices_.begin(); ite != devices_.end(); ++ite) {
    DeviceItem &item = *ite;
    if (item.thread) {
      if (item.thread->loop()) {
        item.thread->loop()->RemoveDelegate(item.sock, this);
      }
      item.thread->Quit();
      item.thread->Join();
      item.thread->Uninit();
    }

    if (item.sock) {
      apr_socket_close(item.sock);
    }
  }

  devices_.clear();
}

bool LidarDataHandlerImpl::AddDevice(const DeviceInfo &info) {
  apr_socket_t *sock = util::CreateBindSocket(info.data_port, mem_pool_);
  if (sock == NULL) {
    return false;
  }

  shared_ptr<IOThread> thread = boost::make_shared<IOThread>();
  thread->Init(false, false);
  thread->loop()->AddDelegate(sock, this, reinterpret_cast<void *>(info.handle));
  {
    lock_guard<mutex> lock(mutex_);
    DeviceItem item = {sock, thread, info.handle};
    devices_.push_back(item);
  }
  return thread->Start();
}

void LidarDataHandlerImpl::RemoveDevice(uint8_t handle) {
  DeviceItem item;
  bool found = false;
  {
    lock_guard<mutex> lock(mutex_);
    for (list<DeviceItem>::iterator ite = devices_.begin(); ite != devices_.end(); ++ite) {
      if (ite->handle == handle) {
        item = *ite;
        found = true;
        devices_.erase(ite);
        break;
      }
    }
  }

  if (found) {
    item.thread->loop()->RemoveDelegate(item.sock, this);
    item.thread->Quit();
    item.thread->Join();
    item.thread->Uninit();
    if (item.sock) {
      apr_socket_close(item.sock);
    }
  }
}

void LidarDataHandlerImpl::OnData(apr_socket_t *sock, void *client_data) {
  uint8_t handle = static_cast<uint8_t>(reinterpret_cast<uintptr_t>(client_data));
  if (handle > data_buffers_.size()) {
    return;
  }
  boost::scoped_array<char> &buf = data_buffers_[handle];
  if (buf.get() == NULL) {
    buf.reset(new char[kMaxBufferSize]);
  }

  apr_sockaddr_t addr;
  apr_size_t size = kMaxBufferSize;
  if (APR_SUCCESS == apr_socket_recvfrom(&addr, sock, 0, buf.get(), &size)) {
    if (handler_) {
      handler_->OnDataCallback(handle, buf.get(), size);
    }
  }
}

}  // namespace livox
