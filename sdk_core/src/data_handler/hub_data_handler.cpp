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

#include "hub_data_handler.h"
#include <base/logging.h>
#include "base/network_util.h"

namespace livox {

bool HubDataHandlerImpl::Init() {
  if (thread_->Init(false, false)) {
    return thread_->Start();
  }
  return false;
}

void HubDataHandlerImpl::Uninit() {
  if (thread_) {
    thread_->Quit();
    thread_->Join();
    thread_->Uninit();
    thread_.reset(NULL);
  }

  if (sock_) {
    apr_socket_close(sock_);
    sock_ = NULL;
  }
  is_valid_ = false;
  mem_pool_ = NULL;
}

bool HubDataHandlerImpl::AddDevice(const DeviceInfo &info) {
  if (is_valid_) {
    return false;
  }
  is_valid_ = true;
  hub_info_ = info;

  sock_ = util::CreateBindSocket(info.data_port, mem_pool_);
  if (sock_ == NULL) {
    is_valid_ = false;
    return false;
  }

  thread_->loop()->AddDelegate(sock_, this);
  return true;
}

void HubDataHandlerImpl::OnData(apr_socket_t *, void *) {
  apr_sockaddr_t addr;
  if (buf_.size() < kMaxBufferSize) {
    buf_.resize(kMaxBufferSize);
  }
  apr_size_t size = kMaxBufferSize;
  apr_socket_recvfrom(&addr, sock_, 0, &buf_[0], &size);

  if (handler_) {
    handler_->OnDataCallback(hub_info_.handle, &buf_[0], size);
  }
}

void HubDataHandlerImpl::RemoveDevice(uint8_t handle) {
  if (handle == hub_info_.handle) {
    thread_->loop()->RemoveDelegate(sock_, this);
    apr_socket_close(sock_);
    sock_ = NULL;
    is_valid_ = false;
  }
}

}  // namespace livox
