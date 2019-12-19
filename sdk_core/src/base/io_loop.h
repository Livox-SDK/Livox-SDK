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

#ifndef LIVOX_IO_LOOP_H_
#define LIVOX_IO_LOOP_H_

#include <boost/function.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>
#include <utility>
#include <vector>
#include "apr_general.h"
#include "apr_network_io.h"
#include "apr_poll.h"
#include "apr_thread_proc.h"
#include "command_callback.h"
#include "noncopyable.h"
#include "thread_base.h"
#include "util.h"

namespace livox {

class IOLoop : public noncopyable {
 public:
  class IOLoopDelegate {
   public:
    virtual void OnData(apr_socket_t *, void *) {}
    virtual void OnTimer(apr_time_t) {}
    virtual void OnWake() {}
  };

  typedef boost::function<void(void)> IOLoopTask;

 public:
  explicit IOLoop(apr_pool_t *mem_pool, bool enable_timer = true, bool enable_wake = true)
      : pollset_(NULL), mem_pool_(mem_pool), last_timeout_(0), enable_timer_(enable_timer), enable_wake_(enable_wake){};

  bool Init();
  void Uninit();
  void AddDelegate(apr_socket_t *sock, IOLoopDelegate *delegate, void *data = NULL);
  void RemoveDelegate(apr_socket_t *sock, IOLoopDelegate *delegate);
  void RemoveDelegateSync(apr_socket_t *sock);
  void Loop();
  bool Wakeup();
  void PostTask(const IOLoopTask &task);
  void SetThreadId (apr_os_thread_t thread_id) { thread_id_ = thread_id; }
  apr_os_thread_t GetThreadId () { return thread_id_; }

 private:
  void AddDelegateAsync(apr_socket_t *sock, IOLoopDelegate *delegate, void *data);
  void RemoveDelegateAsync(apr_socket_t *sock);

 private:
  static const apr_uint32_t kMaxPollCount = 48;
  typedef std::pair<IOLoopDelegate *, void *> ClientData;
  typedef boost::unordered_map<apr_socket_t *, ClientData *> DelegatesType;
  DelegatesType delegates_;
  apr_pollset_t *pollset_;
  apr_pool_t *mem_pool_;
  apr_time_t last_timeout_;
  boost::mutex mutex_;
  bool enable_timer_;
  bool enable_wake_;
  std::vector<IOLoopTask> pending_tasks_;
  apr_os_thread_t thread_id_;
};

}  // namespace livox

#endif  // LIVOX_IO_LOOP_H_
