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

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <string.h>
#include <apr_general.h>
#include <apr_getopt.h>
#include "lds_lidar.h"

/** Cmdline input broadcast code */
static std::vector<std::string> cmdline_broadcast_code;

/** Set the program options.
* You can input the registered device broadcast code and decide whether to save the log file.
*/
static int SetProgramOption(int argc, const char *argv[]) {
  apr_status_t rv;
  apr_pool_t *mp = NULL;
  static const apr_getopt_option_t opt_option[] = {
    /** Long-option, short-option, has-arg flag, description */
    { "code", 'c', 1, "Register device broadcast code" },
    { "log", 'l', 0, "Save the log file" },
    { "help", 'h', 0, "Show help" },
    { NULL, 0, 0, NULL },
  };
  apr_getopt_t *opt = NULL;
  int optch = 0;
  const char *optarg = NULL;

  if (apr_initialize() != APR_SUCCESS) {
    return -1;
  }

  if (apr_pool_create(&mp, NULL) != APR_SUCCESS) {
    return -1;
  }

  rv = apr_getopt_init(&opt, mp, argc, argv);
  if (rv != APR_SUCCESS) {
    printf("Program options initialization failed.\n");
    return -1;
  }

  /** Parse the all options based on opt_option[] */
  bool is_help = false;
  while ((rv = apr_getopt_long(opt, opt_option, &optch, &optarg)) == APR_SUCCESS) {
    switch (optch) {
      case 'c': {
        printf("Register broadcast code: %s\n", optarg);
        char *sn_list = (char *)malloc(sizeof(char)*(strlen(optarg) + 1));
        strncpy(sn_list, optarg, sizeof(char)*(strlen(optarg) + 1));
        char *sn_list_head = sn_list;
        sn_list = strtok(sn_list, "&");
        cmdline_broadcast_code.clear();
        while (sn_list) {
          cmdline_broadcast_code.push_back(sn_list);
          sn_list = strtok(nullptr, "&");
        }
        free(sn_list_head);
        sn_list_head = nullptr;
        break;
      }
      case 'l': {
        printf("Save the log file.\n");
        SaveLoggerFile();
        break;
      }
      case 'h': {
        printf(
          " [-c] Register device broadcast code\n"
          " [-l] Save the log file\n"
          " [-h] Show help\n"
        );
        is_help = true;
        break;
      }
      default: {
        break;
      }
    }
  }

  if (rv != APR_EOF) {
    printf("Invalid options.\n");
  }

  apr_pool_destroy(mp);
  mp = NULL;
  apr_terminate();
  if (is_help)
    return 1;

  return 0;
}

int main(int argc, const char *argv[]) {
/** Set the program options. */
  if (SetProgramOption(argc, argv)) {
    return 0;
  }

  LdsLidar& read_lidar = LdsLidar::GetInstance();

  int ret = read_lidar.InitLdsLidar(cmdline_broadcast_code);
  if (!ret) {
    printf("Init lds lidar success!\n");
  } else {
    printf("Init lds lidar fail!\n");
  }

  printf("Start discovering device.\n");

#ifdef WIN32
  Sleep(100000);
#else
  sleep(100);
#endif

  read_lidar.DeInitLdsLidar();
  printf("Livox lidar demo end!\n");

}
