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
uint32_t data_recveive_count[kMaxLidarCount];

char *broadcast_code_list[] = {
  "000000000000002",
  "000000000000003",
  "000000000000004"
};

#define BROADCAST_CODE_LIST_SIZE    (sizeof(broadcast_code_list) / sizeof(intptr_t))

/** Receiving point cloud data from Livox LiDAR. */
void GetLidarData(uint8_t handle, LivoxEthPacket *data, uint32_t data_num) {
  if (data) {
    data_recveive_count[handle] += data_num;
    if (data_recveive_count[handle] % 10000 == 0) {
      printf("receive packet count %d %d\n", handle, data_recveive_count[handle]);

	  /** Parsing the timestamp and the point cloud data. */
	  uint64_t cur_timestamp = *((uint64_t *)(data->timestamp));
	  LivoxRawPoint *p_point_data = (LivoxRawPoint *)data->data;
    }
  }
}

/** Callback function of starting sampling. */
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

/** Callback function of stopping sampling. */
void OnStopSampleCallback(uint8_t status, uint8_t handle, uint8_t response, void *data) {
}

/** Query the firmware version of Livox LiDAR. */
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

/** Callback function of changing of device state. */
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
    QueryDeviceInformation(handle, OnDeviceInformation, NULL);
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

/** Callback function when broadcast message received.
 * You need to add listening device broadcast code and set the point cloud data callback in this function.
 */
void OnDeviceBroadcast(const BroadcastDeviceInfo *info) {
  if (info == NULL) {
    return;
  }

  printf("Receive Broadcast Code %s\n", info->broadcast_code);
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

  bool result = false;
  uint8_t handle = 0;
  result = AddLidarToConnect(info->broadcast_code, &handle);
  if (result == kStatusSuccess) {
    /** Set the point cloud data for a specific Livox LiDAR. */
    SetDataCallback(handle, GetLidarData);
    devices[handle].handle = handle;
    devices[handle].device_state = kDeviceStateDisconnect;
  }
}

int main(int argc, const char *argv[]) {
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
  memset(data_recveive_count, 0, sizeof(data_recveive_count));

/** Set the callback function receiving broadcast message from Livox LiDAR. */
  SetBroadcastCallback(OnDeviceBroadcast);

/** Set the callback function called when device state change,
 * which means connection/disconnection and changing of LiDAR state.
 */
  SetDeviceStateUpdateCallback(OnDeviceChange);

/** Start the device discovering routine. */
  if (!Start()) {
    Uninit();
    return -1;
  }
  printf("Start discovering device.\n");

#ifdef WIN32
  Sleep(30000);
#else
  sleep(30);
#endif

  int i = 0;
  for (i = 0; i < kMaxLidarCount; ++i) {
    if (devices[i].device_state == kDeviceStateSampling) {
/** Stop the sampling of Livox LiDAR. */
      LidarStopSampling(devices[i].handle, OnStopSampleCallback, NULL);
    }
  }

/** Uninitialize Livox-SDK. */
  Uninit();
}
