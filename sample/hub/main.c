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
#include "livox_sdk.h"

typedef enum {
  kDeviceStateDisconnect = 0,
  kDeviceStateConnect = 1,
  kDeviceStateSampling = 2,
} DeviceState;

typedef struct {
  uint8_t handle;
  DeviceState device_state;
  DeviceInfo info;
} DeviceItem;

DeviceItem devices[kMaxLidarCount];

#define BROADCAST_CODE_LIST_SIZE  1
/** Connect all the broadcast device. */
bool is_connect_all_broadcast_device = true;
char broadcast_code_list[BROADCAST_CODE_LIST_SIZE][kBroadcastCodeSize];

/** Connect the broadcast device in list, please input the broadcast code. */
/*bool is_connect_all_broadcast_device = false;
char broadcast_code_list[BROADCAST_CODE_LIST_SIZE][kBroadcastCodeSize] = {
  "00000000000001"
};*/

void GetLidarData(uint8_t handle, LivoxEthPacket *data, uint32_t data_num, void *client_data) {
  static uint32_t receive_packet_count = 0;
  if (data) {
    ++receive_packet_count;
    if (0 == (receive_packet_count % 10000)) {
      printf("receive packet count %d %d\n", data->id, receive_packet_count);

      /** Parsing the timestamp and the point cloud data. */
      uint64_t cur_timestamp = *((uint64_t *)(data->timestamp));
      LivoxRawPoint *p_point_data = (LivoxRawPoint *)data->data;
    }
    printf("receive packet from %d \n", (uint16_t) HubGetLidarHandle(data->slot, data->id));
  }
}

void OnSampleCallback(uint8_t status, uint8_t handle, uint8_t response, void *data) {
  printf("OnSampleCallback statue %d handle %d response %d \n", status, handle, response);
  if (status == kStatusSuccess) {
    if (response != 0) {
      devices[handle].device_state = kDeviceStateConnect;
    }
  } else if (status == kStatusTimeout) {
    devices[handle].device_state = kDeviceStateConnect;
  }
}

void OnStopSampleCallback(uint8_t status, uint8_t handle, uint8_t response, void *data) {

}

void OnDeviceInformation(uint8_t status, uint8_t handle, DeviceInformationResponse *ack, void *data) {
  if (status != kStatusSuccess) {
    printf("Device Query Informations Failed %d\n", status);
  }
  if (ack) {
    printf("firm ver: %d.%d.%d.%d\n",
           ack->firmware_version[0],
           ack->firmware_version[1],
           ack->firmware_version[2],
           ack->firmware_version[3]);
  }
}

void OnHubLidarInfo(uint8_t status, uint8_t handle, HubQueryLidarInformationResponse *response, void *client_data) {
  if (status != kStatusSuccess) {
    printf("Device Query Informations Failed %d\n", status);
  }
  if (response) {
    int i = 0;
    for (i = 0; i < response->count; ++i) {
      printf("Hub Lidar Info broadcast code %s id %d slot %d \n ",
             response->device_info_list[i].broadcast_code,
             response->device_info_list[i].id,
             response->device_info_list[i].slot);
    }
  }
}

void OnDeviceChange(const DeviceInfo *info, DeviceEvent type) {
  if (info == NULL) {
    return;
  }
  printf("OnDeviceChange broadcast code %s update type %d\n", info->broadcast_code, type);
  uint8_t handle = info->handle;
  if (handle >= kMaxLidarCount) {
    return;
  }
  if (type == kEventConnect) {
    DeviceInfo *_devices = (DeviceInfo *) malloc(sizeof(DeviceInfo) * kMaxLidarCount);

    uint8_t count = kMaxLidarCount;
    uint8_t status = GetConnectedDevices(_devices, &count);
    if (status == kStatusSuccess) {
      int i = 0;
      for (i = 0; i < count; ++i) {
        uint8_t handle = _devices[i].handle;
        if (handle < kMaxLidarCount) {
          devices[handle].handle = handle;
          devices[handle].info = _devices[i];
          devices[handle].device_state = kDeviceStateConnect;
        }
      }
    }
    if (_devices) {
      free(_devices);
    }
    if (devices[handle].info.type == kDeviceTypeHub) {
      HubQueryLidarInformation(OnHubLidarInfo, NULL);
    }
    if (devices[handle].device_state == kDeviceStateDisconnect) {
      devices[handle].device_state = kDeviceStateConnect;
      devices[handle].info = *info;
    }
  } else if (type == kEventDisconnect) {
    devices[handle].device_state = kDeviceStateDisconnect;
  } else if (type == kEventStateChange) {
    devices[handle].info = *info;
  }

  if (devices[handle].device_state == kDeviceStateConnect) {
    printf("Device State error_code %d\n", devices[handle].info.status.status_code);
    printf("Device State working state %d\n", devices[handle].info.state);
    printf("Device feature %d\n", devices[handle].info.feature);
    if (devices[handle].info.state == kLidarStateNormal) {
      if (devices[handle].info.type == kDeviceTypeHub) {
        HubStartSampling(OnSampleCallback, NULL);
      } else {
        LidarStartSampling(handle, OnSampleCallback, NULL);
      }
      devices[handle].device_state = kDeviceStateSampling;
    }
  }
}

void OnDeviceBroadcast(const BroadcastDeviceInfo *info) {
  if (info == NULL) {
    return;
  }

  printf("Receive Broadcast Code %s\n", info->broadcast_code);

  if (!is_connect_all_broadcast_device) {
    bool found = false;
    int i = 0;
    for (i = 0; i < BROADCAST_CODE_LIST_SIZE; ++i) {
      if (strncmp(info->broadcast_code, broadcast_code_list[i], kBroadcastCodeSize) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      return;
    }
  }

  bool result = false;
  uint8_t handle = 0;
  result = AddHubToConnect(info->broadcast_code, &handle);
  if (result == kStatusSuccess && handle < kMaxLidarCount) {
    SetDataCallback(handle, GetLidarData, NULL);
    devices[handle].handle = handle;
    devices[handle].device_state = kDeviceStateDisconnect;
  }
}

/** Set the program options.
* You can input the registered device broadcast code and decide whether to save the log file.
*/
int SetProgramOption(int argc, const char *argv[]) {
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
    case 'c':
      printf("Register broadcast code: %s\n", optarg);
      strncpy(broadcast_code_list[0], optarg, kBroadcastCodeSize);
      break;
    case 'l':
      printf("Save the log file.\n");
      SaveLoggerFile();
      break;
    case 'h':
      printf(
        " [-c] Register device broadcast code\n"
        " [-l] Save the log file\n"
        " [-h] Show help\n"
      );
      is_help = true;
      break;
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
  if (SetProgramOption(argc, argv))
    return 0;

  printf("Livox SDK initializing.\n");
  /** Initialize Livox-SDK. */
  if (!Init()) {
    return -1;
  }
  printf("Livox SDK has been initialized.\n");

  LivoxSdkVersion _sdkversion;
  GetLivoxSdkVersion(&_sdkversion);
  printf("Livox SDK version %d.%d.%d .\n", _sdkversion.major, _sdkversion.minor, _sdkversion.patch);

  memset(devices, 0, sizeof(devices));
  SetBroadcastCallback(OnDeviceBroadcast);
  SetDeviceStateUpdateCallback(OnDeviceChange);

  if (Start()) {
	printf("Start discovering device.\n");

#ifdef WIN32
    Sleep(20000);
#else
    sleep(20);
#endif
    HubStopSampling(OnStopSampleCallback, NULL);
    printf("stop sample\n");
#ifdef WIN32
    Sleep(10000);
#else
    sleep(10);
#endif
  }

  Uninit();
}
