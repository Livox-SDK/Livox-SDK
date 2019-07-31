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

/** Livox LiDAR data source, data from hub */

#ifndef COAT_LDS_HUB_H_
#define COAT_LDS_HUB_H_

#include <memory>
#include <vector>

#include "livox_def.h"
#include "livox_sdk.h"

typedef enum {
  kConnectStateOff = 0,
  kConnectStateOn = 1,
  kConnectStateSampling = 2,
} LidarConnectState;

typedef struct {
  uint8_t handle;
  LidarConnectState connect_state;
  DeviceInfo info;
} LidarDevice;

/**
 * LiDAR data source, data from hub.
 */
class LdsHub {
 public:
  static LdsHub& GetInstance() {
    static LdsHub lds_hub;
    return lds_hub;
  }

  int InitLdsHub(std::vector<std::string>& broadcast_code_strs);
  int DeInitLdsHub(void);

 private:
  LdsHub();
  LdsHub(const LdsHub&) = delete;
  ~LdsHub();
  LdsHub& operator=(const LdsHub&) = delete;

  /** LiDAR data receive callback */
  static void GetLidarDataCb(uint8_t hub_handle, LivoxEthPacket *data,\
                           uint32_t data_num, void *client_data);
  static void OnDeviceBroadcast(const BroadcastDeviceInfo *info);
  static void OnDeviceChange(const DeviceInfo *info, DeviceEvent type);
  static void StartSampleCb(uint8_t status, uint8_t handle, uint8_t response, void *clent_data);
  static void StopSampleCb(uint8_t status, uint8_t handle, uint8_t response, void *clent_data);
  static void HubQueryLidarInfoCb(uint8_t status, uint8_t handle, \
                                  HubQueryLidarInformationResponse *response, void *client_data);

  int AddBroadcastCodeToWhitelist(const char* broadcast_code);
  void AddLocalBroadcastCode(void);
  bool FindInWhitelist(const char* broadcast_code);
  void UpdateHubLidarinfo(void);

  void EnableAutoConnectMode(void) { auto_connect_mode_ = true; }
  void DisableAutoConnectMode(void) { auto_connect_mode_ = false; }
  bool IsAutoConnectMode(void) { return auto_connect_mode_; }

  bool auto_connect_mode_;
  uint32_t whitelist_count_;
  volatile bool is_initialized_;
  char broadcast_code_whitelist_[kMaxLidarCount][kBroadcastCodeSize];

  uint32_t lidar_count_;
  LidarDevice lidars_[kMaxLidarCount];
  LidarDevice hub_;
  uint32_t receive_packet_count_;

};

#endif
