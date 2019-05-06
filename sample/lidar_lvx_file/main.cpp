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

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <apr_general.h>
#include <apr_getopt.h>
#include <algorithm>
#include <string.h>
#include "lvx_file.h"

DeviceItem devices[kMaxLidarCount];
LvxFileHandle lvx_file_handler;
std::list<LvxBasePackDetail> point_packet_list;
std::vector<std::string> broadcast_code_rev;
std::condition_variable condition_variable;
std::mutex mtx;
int lvx_file_save_time = 10;
bool is_finish_extrinsic_parameter = false;
bool is_read_extrinsic_from_xml = false;

#define FRAME_RATE 20

/** Connect all the broadcast device in default and connect specific device when use program options or broadcast_code_list is not empty. */
std::vector<std::string> broadcast_code_list = {
  //"000000000000002",
  //"000000000000003",
  //"000000000000004"
};

/** Receiving point cloud data from Livox LiDAR. */
void GetLidarData(uint8_t handle, LivoxEthPacket *data, uint32_t data_num, void *client_data) {
  if (data) {
    if (handle < broadcast_code_list.size() && is_finish_extrinsic_parameter) {
      std::lock_guard<std::mutex> lock(mtx);
      LvxBasePackDetail packet;
      packet.device_index = handle;
      lvx_file_handler.BasePointsHandle(data, packet);
      lvx_file_handler.CalcExtrinsicPoints(packet);
      point_packet_list.push_back(packet);

      if (point_packet_list.size() % (50 * broadcast_code_list.size()) == 0) {
        condition_variable.notify_one();
      }
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

/** Callback function of get LiDARs' extrinsic parameter. */
void OnGetLidarExtrinsicParameter(uint8_t status, uint8_t handle, LidarGetExtrinsicParameterResponse *response, void *data) {
  if (status == kStatusSuccess) {
    if (response != 0) {
      printf("OnGetLidarExtrinsicParameter statue %d handle %d response %d \n", status, handle, response->ret_code);
      std::lock_guard<std::mutex> lock(mtx);
      LvxDeviceInfo lidar_info;
      strncpy((char *)lidar_info.lidar_broadcast_code, devices[handle].info.broadcast_code, kBroadcastCodeSize);
      memset(lidar_info.hub_broadcast_code, 0, kBroadcastCodeSize);
      lidar_info.device_index = handle;
      lidar_info.device_type = devices[handle].info.type;
      lidar_info.pitch = response->pitch;
      lidar_info.roll = response->roll;
      lidar_info.yaw = response->yaw;
      lidar_info.x = static_cast<float>(response->x / 1000.0);
      lidar_info.y = static_cast<float>(response->y / 1000.0);
      lidar_info.z = static_cast<float>(response->z / 1000.0);
      lvx_file_handler.AddDeviceInfo(lidar_info);
      if (lvx_file_handler.GetDeviceInfoListSize() == broadcast_code_list.size()) {
        is_finish_extrinsic_parameter = true;
        condition_variable.notify_one();
      }
    }
  }
  else if (status == kStatusTimeout) {
    printf("GetLidarExtrinsicParameter timeout! \n");
  }
}

/** Get LiDARs' extrinsic parameter from file named "extrinsic.xml". */
void LidarGetExtrinsicFromXml(uint8_t handle) {
  LvxDeviceInfo lidar_info;
  ParseExtrinsicXml(devices[handle], lidar_info);
  lvx_file_handler.AddDeviceInfo(lidar_info);
  if (lvx_file_handler.GetDeviceInfoListSize() == broadcast_code_list.size()) {
    is_finish_extrinsic_parameter = true;
    condition_variable.notify_one();
  }
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
  if (info == nullptr) {
    return;
  }
  printf("OnDeviceChange broadcast code %s update type %d\n", info->broadcast_code, type);
  uint8_t handle = info->handle;
  if (handle >= kMaxLidarCount) {
    return;
  }
  if (type == kEventConnect) {
    QueryDeviceInformation(handle, OnDeviceInformation, nullptr);
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
      if (devices[handle].info.type != kDeviceTypeHub) {
        if (!is_read_extrinsic_from_xml) {
          LidarGetExtrinsicParameter(handle, OnGetLidarExtrinsicParameter, nullptr);
        }
        else {
          LidarGetExtrinsicFromXml(handle);
        }
        LidarStartSampling(handle, OnSampleCallback, nullptr);
        devices[handle].device_state = kDeviceStateSampling;
      }
    }
  }
}

/** Callback function when broadcast message received.
 * You need to add listening device broadcast code and set the point cloud data callback in this function.
 */
void OnDeviceBroadcast(const BroadcastDeviceInfo *info) {
  if (info == nullptr) {
    return;
  }

  printf("Receive Broadcast Code %s\n", info->broadcast_code);

  if (broadcast_code_list.size() > 0) {
    bool found = false;
    uint8_t i = 0;
    for (i = 0; i < broadcast_code_list.size(); ++i) {
      if (strncmp(info->broadcast_code, broadcast_code_list[i].c_str(), kBroadcastCodeSize) == 0) {
        found = true;
        break;
      }
    }
    if (!found) {
      return;
    }
  }
  else {
    if ((broadcast_code_rev.size() == 0) || (std::find(broadcast_code_rev.begin(), broadcast_code_rev.end(), info->broadcast_code) == broadcast_code_rev.end()))
      broadcast_code_rev.push_back(info->broadcast_code);
    return;
  }

  bool result = false;
  uint8_t handle = 0;
  result = AddLidarToConnect(info->broadcast_code, &handle);
  if (result == kStatusSuccess) {
    /** Set the point cloud data for a specific Livox LiDAR. */
    SetDataCallback(handle, GetLidarData, nullptr);
    devices[handle].handle = handle;
    devices[handle].device_state = kDeviceStateDisconnect;
  }
}

/** Set the program options.
* You can input the registered device broadcast code and decide whether to save the log file.
*/
int SetProgramOption(int argc, const char *argv[]) {
  apr_status_t rv;
  apr_pool_t *mp = nullptr;
  static const apr_getopt_option_t opt_option[] = {
    /** Long-option, short-option, has-arg flag, description */
    { "code", 'c', 1, "Register device broadcast code" },     
    { "log", 'l', 0, "Save the log file" },    
    { "time", 't', 1, "Time to save point cloud to the lvx file" },
    { "param", 'p', 0, "Get the extrinsic parameter from extrinsic.xml file" },
    { "help", 'h', 0, "Show help" },    
    { nullptr, 0, 0, nullptr },
  };
  apr_getopt_t *opt = nullptr;
  int optch = 0;
  const char *optarg = nullptr;

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
      int i = 0;
      broadcast_code_list.clear();
      while (sn_list) {
        broadcast_code_list.push_back(sn_list);
        sn_list = strtok(nullptr, "&");
        i++;
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
    case 't': {
      printf("Time to save point cloud to the lvx file:%s.\n", optarg);
      lvx_file_save_time = atoi(optarg);
      break;
    }
    case 'p': {
      printf("Get the extrinsic parameter from extrinsic.xml file.\n");
      is_read_extrinsic_from_xml = true;
      break;
    }
    case 'h': {
      printf(
        " [-c] Register device broadcast code\n"
        " [-l] Save the log file\n"
        " [-t] Time to save point cloud to the lvx file\n"
        " [-p] Get the extrinsic parameter from extrinsic.xml file\n"
        " [-h] Show help\n"
      );
      is_help = true;
      break;
    }
    }
  }
  if (rv != APR_EOF) {
    printf("Invalid options.\n");
  }

  apr_pool_destroy(mp);
  mp = nullptr;
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
  Sleep(2000);
#else
  sleep(2);
#endif

  if (broadcast_code_rev.size() != 0)
    broadcast_code_list = broadcast_code_rev;

  {
    std::unique_lock<std::mutex> lock(mtx);
    condition_variable.wait(lock);
  }

  printf("Start initialize lvx file.\n");
  if (!lvx_file_handler.InitLvxFile()) {
    Uninit();
    return -1;
  }
  lvx_file_handler.InitLvxFileHeader();

  int i = 0;
  for (i = 0; i < lvx_file_save_time * FRAME_RATE; ++i) {
    std::list<LvxBasePackDetail> point_packet_list_temp;
    {
      std::unique_lock<std::mutex> lock(mtx);
      condition_variable.wait(lock);
      point_packet_list_temp.swap(point_packet_list);
    }

    printf("Finish save %d frame to lvx file.\n", i);
    lvx_file_handler.SaveFrameToLvxFile(point_packet_list_temp);
  }

  lvx_file_handler.CloseLvxFile();

  for (i = 0; i < kMaxLidarCount; ++i) {
    if (devices[i].device_state == kDeviceStateSampling) {
/** Stop the sampling of Livox LiDAR. */
      LidarStopSampling(devices[i].handle, OnStopSampleCallback, nullptr);
    }
  }

/** Uninitialize Livox-SDK. */
  Uninit();
}
