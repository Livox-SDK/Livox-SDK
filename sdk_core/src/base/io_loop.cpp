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

#include "io_loop.h"
#include <boost/bind.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/locks.hpp>
#include <iostream>
#include "logging.h"

using boost::lock_guard;
using boost::mutex;
using std::vector;

namespace livox {

bool IOLoop::Init() {
  if (mem_pool_ == NULL) {
    return false;
  }

#ifdef WIN32
  apr_status_t rv =
      apr_pollset_create(&pollset_, kMaxPollCount, mem_pool_, APR_POLLSET_WAKEABLE);
#else
  apr_status_t rv =
      apr_pollset_create(&pollset_, kMaxPollCount, mem_pool_, APR_POLLSET_THREADSAFE | APR_POLLSET_WAKEABLE);
#endif

  if (rv != APR_SUCCESS) {
    LOG_ERROR(PrintAPRStatus(rv));
    return false;
  }

  return true;
}

void IOLoop::Uninit() {
  if (pollset_) {
    apr_pollset_destroy(pollset_);
    pollset_ = NULL;
  }

  for (DelegatesType::const_iterator ite = delegates_.begin(); ite != delegates_.end(); ++ite) {
    ClientData *data = ite->second;
    if (data) {
      delete data;
    }
  }

  delegates_.clear();
  mem_pool_ = NULL;
}

void IOLoop::AddDelegate(apr_socket_t *sock, IOLoop::IOLoopDelegate *delegate, void *data) {
  PostTask(boost::bind(&IOLoop::AddDelegateAsync, this, sock, delegate, data));
}

void IOLoop::RemoveDelegate(apr_socket_t *sock, IOLoopDelegate *) {
  PostTask(boost::bind(&IOLoop::RemoveDelegateAsync, this, sock));
}

void IOLoop::RemoveDelegateSync(apr_socket_t *sock) {
  assert(apr_os_thread_equal(this->thread_id_, apr_os_thread_current()));
  RemoveDelegateAsync(sock);
}

void IOLoop::Loop() {
  apr_int32_t num = 0;
  const apr_pollfd_t *ret_pfd = NULL;
  apr_status_t rv = apr_pollset_poll(pollset_, apr_time_from_msec(50), &num, &ret_pfd);

  if (rv == APR_SUCCESS) {
    for (int i = 0; i < num; i++) {
      ClientData *data = static_cast<ClientData *>(ret_pfd[i].client_data);
      if (data) {
        IOLoopDelegate *delegate = data->first;
        if (delegate) {
          delegate->OnData(ret_pfd->desc.s, data->second);
        }
      }
    }
  }

  if (enable_wake_ && APR_STATUS_IS_EINTR(rv)) {
    DelegatesType delegates = delegates_;
    for (DelegatesType::const_iterator ite = delegates.begin(); ite != delegates.end(); ++ite) {
      ClientData *data = ite->second;
      if (data) {
        IOLoopDelegate *delegate = data->first;
        if (delegate) {
          delegate->OnWake();
        }
      }
    }
  }

  if (enable_timer_) {
    apr_time_t t = apr_time_now();
    if (last_timeout_ == 0 || t - last_timeout_ > apr_time_from_msec(50)) {
      last_timeout_ = t;
      // copy delegates_ in case delegate is removed in callback.
      DelegatesType delegates = delegates_;
      for (DelegatesType::const_iterator ite = delegates.begin(); ite != delegates.end(); ++ite) {
        ClientData *data = ite->second;
        if (data) {
          IOLoopDelegate *delegate = data->first;
          if (delegate) {
            delegate->OnTimer(t);
          }
        }
      }
    }
  }

  vector<IOLoopTask> tasks;
  {
    lock_guard<mutex> lock(mutex_);
    tasks.swap(pending_tasks_);
  }

  for (vector<IOLoopTask>::iterator ite = tasks.begin(); ite != tasks.end(); ++ite) {
    IOLoopTask &task = *ite;
    task();
  }

  if (rv != APR_SUCCESS && !APR_STATUS_IS_TIMEUP(rv) && !APR_STATUS_IS_EINTR(rv)) {
    LOG_ERROR(PrintAPRStatus(rv));
  }
}

bool IOLoop::Wakeup() {
  apr_status_t rv = apr_pollset_wakeup(pollset_);
  if (rv != APR_SUCCESS) {
    LOG_ERROR(PrintAPRStatus(rv));
    return false;
  }
  return true;
}

void IOLoop::PostTask(const IOLoopTask &task) {
  {
    lock_guard<mutex> lock(mutex_);
    pending_tasks_.push_back(task);
  }
  Wakeup();
}

void IOLoop::AddDelegateAsync(apr_socket_t *sock, IOLoop::IOLoopDelegate *delegate, void *data) {
  ClientData *p = new ClientData(delegate, data);
  apr_pollfd_t pfd = {mem_pool_, APR_POLL_SOCKET, APR_POLLIN, 0, {NULL}, p};
  pfd.desc.s = sock;
  apr_pollset_add(pollset_, &pfd);
  delegates_[sock] = p;
}

void IOLoop::RemoveDelegateAsync(apr_socket_t *sock) {
  apr_pollfd_t pfd = {mem_pool_, APR_POLL_SOCKET, 0, 0, {NULL}, NULL};
  pfd.desc.s = sock;
  apr_pollset_remove(pollset_, &pfd);

  DelegatesType::iterator ite = delegates_.find(sock);
  if (ite != delegates_.end()) {
    ClientData *p = ite->second;
    if (p) {
      delete p;
    }
    delegates_.erase(ite);
  }
}

}  // namespace livox
