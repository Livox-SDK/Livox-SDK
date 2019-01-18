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

#include "thread_base.h"

namespace livox {
static void *APR_THREAD_FUNC ClassThreadHelperFunc(apr_thread_t *thd, void *data) {
  ThreadBase *caller = static_cast<ThreadBase *>(data);
  if (caller == NULL) {
    return NULL;
  }
  caller->ThreadFunc();
  apr_thread_exit(thd, APR_SUCCESS);
  return NULL;
}

ThreadBase::ThreadBase() : thread_(NULL), quit_(false), pool_(NULL) {}

bool ThreadBase::Start() {
  quit_ = false;
  apr_threadattr_t *thread_attr = NULL;
  apr_status_t rv = APR_SUCCESS;
  rv = apr_threadattr_create(&thread_attr, pool_);
  if (rv != APR_SUCCESS) {
    return false;
  }
  rv = apr_thread_create(&thread_, thread_attr, ClassThreadHelperFunc, this, pool_);
  return rv == APR_SUCCESS;
}

void ThreadBase::Join() {
  apr_status_t rv = APR_SUCCESS;
  if (thread_) {
    apr_thread_join(&rv, thread_);
    thread_ = NULL;
  }
}

bool ThreadBase::Init() {
  return apr_pool_create(&pool_, NULL) == APR_SUCCESS;
}

void ThreadBase::Uninit() {
  if (pool_) {
    apr_pool_destroy(pool_);
    pool_ = NULL;
  }
}

}  // namespace livox
