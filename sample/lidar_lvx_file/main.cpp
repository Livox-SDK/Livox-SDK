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

#include <apr_general.h>
#include <apr_getopt.h>
#include <algorithm>
#include <string.h>
#include "lvx_file.h"

DeviceItem devices[kMaxLidarCount];
LvxFileHandle lvx_file_handler;
std::list<LvxBasePackDetail> point_packet_list;
std::vector<std::string> broadcast_code_rev;
std::condition_variable lidar_arrive_condition;
std::condition_variable extrinsic_condition;
std::condition_variable point_pack_condition;
std::mutex mtx;
int lvx_file_save_time = 10;
bool is_finish_extrinsic_parameter = false;
bool is_read_extrinsic_from_xml = false;
uint8_t connected_lidar_count = 0;

#define FRAME_RATE 20


using namespace std::chrono;

/** Connect all the broadcast device in default and connect specific device when use program options or broadcast_code_list is not empty. */
std::vector<std::string> broadcast_code_list = {
  //"000000000000001"
  //"000000000000002",
  //"000000000000003",
  //"000000000000004"
};

/** Receiving error message from Livox Lidar. */
void OnLidarErrorStatusCallback(livox_status status, uint8_t handle, ErrorMessage *message) {
  static uint32_t error_message_count = 0;
  if (message != NULL) {
    ++error_message_count;
    if (0 == (error_message_count % 100)) {
      printf("handle: %u\n", handle);
      printf("temp_status : %u\n", message->lidar_error_code.temp_status);
      printf("volt_status : %u\n", message->lidar_error_code.volt_status);
      printf("motor_status : %u\n", message->lidar_error_code.motor_status);
      printf("dirty_warn : %u\n", message->lidar_error_code.dirty_warn);
      printf("firmware_err : %u\n", message->lidar_error_code.firmware_err);
      printf("pps_status : %u\n", message->lidar_error_code.device_status);
      printf("fan_status : %u\n", message->lidar_error_code.fan_status);
      printf("self_heating : %u\n", message->lidar_error_code.self_heating);
      printf("ptp_status : %u\n", message->lidar_error_code.ptp_status);
      printf("time_sync_status : %u\n", message->lidar_error_code.time_sync_status);
      printf("system_status : %u\n", message->lidar_error_code.system_status);
    }
  }
}

/** Receiving point cloud data from Livox LiDAR. */
void GetLidarData(uint8_t handle, LivoxEthPacket *data, uint32_t data_num, void *client_data) {
  if (data) {
    if (handle < connected_lidar_count && is_finish_extrinsic_parameter) {
      std::unique_lock<std::mutex> lock(mtx);
      LvxBasePackDetail packet;
      packet.device_index = handle;
      lvx_file_handler.BasePointsHandle(data, packet);
      point_packet_list.push_back(packet);
    }
  }
}

/** Callback function of starting sampling. */
void OnSampleCallback(livox_status status, uint8_t handle, uint8_t response, void *data) {
  printf("OnSampleCallback statues %d handle %d response %d \n", status, handle, response);
  if (status == kStatusSuccess) {
    if (response != 0) {
      devices[handle].device_state = kDeviceStateConnect;
    }
  } else if (status == kStatusTimeout) {
    devices[handle].device_state = kDeviceStateConnect;
  }
}

/** Callback function of stopping sampling. */
void OnStopSampleCallback(livox_status status, uint8_t handle, uint8_t response, void *data) {
}

/** Callback function of get LiDARs' extrinsic parameter. */
void OnGetLidarExtrinsicParameter(livox_status status, uint8_t handle, LidarGetExtrinsicParameterResponse *response, void *data) {
  if (status == kStatusSuccess) {
    if (response != 0) {
      printf("OnGetLidarExtrinsicParameter statue %d handle %d response %d \n", status, handle, response->ret_code);
      std::unique_lock<std::mutex> lock(mtx);
      LvxDeviceInfo lidar_info;
      strncpy((char *)lidar_info.lidar_broadcast_code, devices[handle].info.broadcast_code, kBroadcastCodeSize);
      memset(lidar_info.hub_broadcast_code, 0, kBroadcastCodeSize);
      lidar_info.device_index = handle;
      lidar_info.device_type = devices[handle].info.type;
      lidar_info.extrinsic_enable = true;
      lidar_info.pitch = response->pitch;
      lidar_info.roll = response->roll;
      lidar_info.yaw = response->yaw;
      lidar_info.x = static_cast<float>(response->x / 1000.0);
      lidar_info.y = static_cast<float>(response->y / 1000.0);
      lidar_info.z = static_cast<float>(response->z / 1000.0);
      lvx_file_handler.AddDeviceInfo(lidar_info);
      if (lvx_file_handler.GetDeviceInfoListSize() == connected_lidar_count) {
        is_finish_extrinsic_parameter = true;
        extrinsic_condition.notify_one();
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
    extrinsic_condition.notify_one();
  }
}

/** Query the firmware version of Livox LiDAR. */
void OnDeviceInformation(livox_status status, uint8_t handle, DeviceInformationResponse *ack, void *data) {
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

void LidarConnect(const DeviceInfo *info) {
  uint8_t handle = info->handle;
  QueryDeviceInformation(handle, OnDeviceInformation, NULL);
  if (devices[handle].device_state == kDeviceStateDisconnect) {
    devices[handle].device_state = kDeviceStateConnect;
    devices[handle].info = *info;
  }
}

void LidarDisConnect(const DeviceInfo *info) {
  uint8_t handle = info->handle;
  devices[handle].device_state = kDeviceStateDisconnect;
}

void LidarStateChange(const DeviceInfo *info) {
  uint8_t handle = info->handle;
  devices[handle].info = *info;
}

/** Callback function of changing of device state. */
void OnDeviceInfoChange(const DeviceInfo *info, DeviceEvent type) {
  if (info == nullptr) {
    return;
  }
  printf("OnDeviceChange broadcast code %s update type %d\n", info->broadcast_code, type);
  uint8_t handle = info->handle;
  if (handle >= kMaxLidarCount) {
    return;
  }

  if (type == kEventConnect) {
    LidarConnect(info);
    printf("[WARNING] Lidar sn: [%s] Connect!!!\n", info->broadcast_code);
  } else if (type == kEventDisconnect) {
    LidarDisConnect(info);
    printf("[WARNING] Lidar sn: [%s] Disconnect!!!\n", info->broadcast_code);
  } else if (type == kEventStateChange) {
    LidarStateChange(info);
    printf("[WARNING] Lidar sn: [%s] StateChange!!!\n", info->broadcast_code);
  }

  if (devices[handle].device_state == kDeviceStateConnect) {
    printf("Device Working State %d\n", devices[handle].info.state);
    if (devices[handle].info.state == kLidarStateInit) {
      printf("Device State Change Progress %u\n", devices[handle].info.status.progress);
    } else {
      printf("Device State Error Code 0X%08x\n", devices[handle].info.status.status_code.error_code);
    }
    printf("Device feature %d\n", devices[handle].info.feature);
    SetErrorMessageCallback(handle, OnLidarErrorStatusCallback);
    if (devices[handle].info.state == kLidarStateNormal) {
      if (!is_read_extrinsic_from_xml) {
        LidarGetExtrinsicParameter(handle, OnGetLidarExtrinsicParameter, nullptr);
      } else {
        LidarGetExtrinsicFromXml(handle);
      }
      LidarStartSampling(handle, OnSampleCallback, nullptr);
      devices[handle].device_state = kDeviceStateSampling;
    }
  }
}

void OnDeviceInfoChangeExtra(int param, const DeviceInfo *info, DeviceEvent type) {
  // ... Do stuff with the extra param
}


/** Callback function when broadcast message received.
 * You need to add listening device broadcast code and set the point cloud data callback in this function.
 */
void OnDeviceBroadcast(const BroadcastDeviceInfo *info) {
  if (info == nullptr || info->dev_type == kDeviceTypeHub) {
    return;
  }

  printf("Receive Broadcast Code %s\n", info->broadcast_code);
  if ((broadcast_code_rev.size() == 0) ||
      (std::find(broadcast_code_rev.begin(), broadcast_code_rev.end(), info->broadcast_code) == broadcast_code_rev.end())) {
    broadcast_code_rev.push_back(info->broadcast_code);
    lidar_arrive_condition.notify_one();
  }
}

struct OptBcast{
  int a;
  int b;
};

void OnDeviceBroadcastExtra(OptBcast o, const BroadcastDeviceInfo *info) {
 // ... Do stuff with the extra param
}

/** Wait until no new device arriving in 2 second. */
void WaitForDevicesReady( ) {
  bool device_ready = false;
  seconds wait_time = seconds(2);
  steady_clock::time_point last_time = steady_clock::now();
  while (!device_ready) {
    std::unique_lock<std::mutex> lock(mtx);
    lidar_arrive_condition.wait_for(lock,wait_time);
    if ((steady_clock::now() - last_time + milliseconds(50)) >= wait_time) {
      device_ready = true;
    } else {
      last_time = steady_clock::now();
    }
  }
}

void WaitForExtrinsicParameter() {
  std::unique_lock<std::mutex> lock(mtx);
  extrinsic_condition.wait(lock);
}

void AddDevicesToConnect() {
  if (broadcast_code_rev.size() == 0)
    return;

  for (int i = 0; i < broadcast_code_rev.size(); ++i) {
    if ((broadcast_code_list.size() != 0) &&
        (std::find(broadcast_code_list.begin(), broadcast_code_list.end(), broadcast_code_rev[i]) == broadcast_code_list.end())) {
      continue;
    }
    uint8_t handle = 0;
    bool result = AddLidarToConnect(broadcast_code_rev[i].c_str(), &handle);
    if (result == kStatusSuccess) {
      /** Set the point cloud data for a specific Livox LiDAR. */
      SetDataCallback(handle, GetLidarData, nullptr);
      devices[handle].handle = handle;
      devices[handle].device_state = kDeviceStateDisconnect;
      connected_lidar_count++;
    }
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
  using namespace std::placeholders;


/** Set the callback function receiving broadcast message from Livox LiDAR. */
  OptBcast bcastopts;
  std::function<void(const BroadcastDeviceInfo *info)> cb_bcast(std::bind(OnDeviceBroadcastExtra, bcastopts, _1));
  SetBroadcastCallback(cb_bcast);

/** Set the callback function called when device state change,
 * which means connection/disconnection and changing of LiDAR state.
 */
 std::function<void(const DeviceInfo *, DeviceEvent)> cb_device_state(std::bind(OnDeviceInfoChangeExtra, 1, _1, _2));
 SetDeviceStateUpdateCallback(cb_device_state);

/** Start the device discovering routine. */
  if (!Start()) {
    Uninit();
    return -1;
  }
  printf("Start discovering device.\n");

  WaitForDevicesReady();

  AddDevicesToConnect();

  if (connected_lidar_count == 0) {
    printf("No device will be connected.\n");
    Uninit();
    return -1;
  }

  WaitForExtrinsicParameter();

  printf("Start initialize lvx file.\n");
  if (!lvx_file_handler.InitLvxFile()) {
    Uninit();
    return -1;
  }

  lvx_file_handler.InitLvxFileHeader();

  int i = 0;
  steady_clock::time_point last_time = steady_clock::now();
  for (i = 0; i < lvx_file_save_time * FRAME_RATE; ++i) {
    std::list<LvxBasePackDetail> point_packet_list_temp;
    {
      std::unique_lock<std::mutex> lock(mtx);
      point_pack_condition.wait_for(lock, milliseconds(kDefaultFrameDurationTime) - (steady_clock::now() - last_time));
      last_time = steady_clock::now();
      point_packet_list_temp.swap(point_packet_list);
    }
    if(point_packet_list_temp.empty()) {
      printf("Point cloud packet is empty.\n");
      break;
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
