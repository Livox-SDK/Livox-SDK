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

#include "livox_sdk.h"
#include "apr_general.h"
#include "command_handler/command_handler.h"
#include "data_handler/data_handler.h"
#include "base/logging.h"
#include "device_manager.h"
using namespace livox;
IOThread *g_thread = NULL;
static bool is_initialized = false;

void GetLivoxSdkVersion(LivoxSdkVersion *version) {
  if (version != NULL) {
    version->major = LIVOX_SDK_MAJOR_VERSION;
    version->minor = LIVOX_SDK_MINOR_VERSION;
    version->patch = LIVOX_SDK_PATCH_VERSION;
  }
}

bool Init() {
  if (is_initialized) {
    return false;
  }

  bool result = false;
  do {
    InitLogger();

    if (apr_initialize() != APR_SUCCESS) {
      result = false;
      break;
    }

    g_thread = new IOThread();
    g_thread->Init();

    if (device_discovery().Init() == false) {
      result = false;
      break;
    }

    if (device_manager().Init() == false) {
      result = false;
      break;
    }

    if (command_handler().Init(g_thread->loop()) == false) {
      result = false;
      break;
    }
    if (data_handler().Init() == false) {
      result = false;
      break;
    }

    result = true;
  } while (0);

  if (result == false) {
    apr_terminate();
  }

  is_initialized = result;
  return result;
}

void Uninit() {
  if (!is_initialized) {
    return;
  }
  if (g_thread) {
    g_thread->Quit();
    g_thread->Join();
  }

  device_discovery().Uninit();
  command_handler().Uninit();
  data_handler().Uninit();
  device_manager().Uninit();

  if (g_thread) {
    g_thread->Uninit();
    delete g_thread;
    g_thread = NULL;
  }
  
  UninitLogger();
  apr_terminate();
  is_initialized = false;
}

bool Start() {
  if (g_thread && device_discovery().Start(g_thread->loop()) && g_thread->Start()) {
    return true;
  }
  return false;
}

void SaveLoggerFile() {
  is_save_log_file = true;
}