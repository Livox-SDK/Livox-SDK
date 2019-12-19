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

#include "device_manager.h"
#include <boost/atomic.hpp>
#include <boost/thread/lock_guard.hpp>
#include <boost/thread/locks.hpp>
#include <vector>
#include "base/logging.h"
#include "command_handler/command_handler.h"
#include "command_handler/command_impl.h"
#include "data_handler/data_handler.h"

using boost::lock_guard;
using boost::mutex;
using std::string;
using std::vector;

namespace livox {

inline bool IsHub(uint8_t mode) {
  return mode == kDeviceTypeHub;
}

inline bool IsLidar(uint8_t mode) {
  return mode == kDeviceTypeLidarTele || mode == kDeviceTypeLidarMid40 || mode == kDeviceTypeLidarHorizon;
}

bool DeviceManager::Init() {
  apr_status_t rv = apr_pool_create(&mem_pool_, NULL);
  if (rv != APR_SUCCESS) {
    return false;
  }

  return true;
}

void DeviceManager::Uninit() {
  broadcast_cb_ = NULL;
  connected_cb_ = NULL;
  device_mode_ = kDeviceModeNone;

  lock_guard<mutex> lock(mutex_);
  for (DeviceContainer::iterator ite = devices_.begin(); ite != devices_.end(); ++ite) {
    ite->clear();
  }

  if (mem_pool_) {
    apr_pool_destroy(mem_pool_);
    mem_pool_ = NULL;
  }
}

bool DeviceManager::AddDevice(const DeviceInfo &device) {
  if (device_mode_ == kDeviceModeNone) {
    if (IsHub(device.type)) {
      device_mode_ = kDeviceModeHub;
    } else if (IsLidar(device.type)) {
      device_mode_ = kDeviceModeLidar;
    }
  }

  {
    lock_guard<mutex> lock(mutex_);
    if (device.handle < devices_.size()) {
      DetailDeviceInfo &info = devices_[device.handle];
      info.connected = true;
      info.info = device;
    }
  }
  return true;
}

void DeviceManager::RemoveDevice(uint8_t handle) {
  lock_guard<mutex> lock(mutex_);
  if (device_mode_ == kDeviceModeHub) {
    for (DeviceContainer::iterator ite = devices_.begin(); ite != devices_.end(); ++ite) {
      ite->connected = false;
    }
  } else if (device_mode_ == kDeviceModeLidar) {
    if (handle < devices_.size()) {
      devices_[handle].connected = false;
    }
  }
  LOG_INFO(" Device {} removed ", (uint16_t)handle);
}

void DeviceManager::BroadcastDevices(const BroadcastDeviceInfo *info) {
  if (broadcast_cb_) {
    broadcast_cb_(info);
  }
}

void DeviceManager::UpdateDevices(const DeviceInfo &device, DeviceEvent type) {
  if (device_mode_ == kDeviceModeLidar && connected_cb_) {
    connected_cb_(&device, type);
  }

  if (device_mode_ == kDeviceModeHub) {
    if (type == kEventHubConnectionChange) {
      LOG_DEBUG("Send Query lidars command");
      command_handler().SendCommand(kHubDefaultHandle,
                                    kCommandSetHub,
                                    kCommandIDHubQueryLidarInformation,
                                    NULL,
                                    0,
                                    MakeCommandCallback<DeviceManager, HubQueryLidarInformationResponse>(
                                        this, &DeviceManager::HubLidarInfomationCallback));
    } else {
      if (connected_cb_) {
        connected_cb_(&device, type);
      }
    }
  }
}

void DeviceManager::HubLidarInfomationCallback(livox_status status, uint8_t, HubQueryLidarInformationResponse *response) {
  if (status == kStatusSuccess) {
    {
      lock_guard<mutex> lock(mutex_);
      for (DeviceContainer::iterator ite = devices_.begin(); ite != devices_.end(); ++ite) {
        if (ite->info.handle != kHubDefaultHandle) {
          ite->connected = false;
        }
      }
      for (int i = 0; i < response->count; i++) {
        std::size_t index = (response->device_info_list[i].slot - 1) * 3 + response->device_info_list[i].id - 1;
        if (index < devices_.size()) {
          DetailDeviceInfo &info = devices_[index];
          info.connected = true;
          strncpy(
              info.info.broadcast_code, response->device_info_list[i].broadcast_code, sizeof(info.info.broadcast_code));
          info.info.handle = index;
        }
      }
    }
    if (connected_cb_) {
      connected_cb_(&(devices_[kHubDefaultHandle].info), kEventHubConnectionChange);
    }
  } else {
    LOG_ERROR("Failed to query lidars information connected to hub.");
  }
}

void DeviceManager::GetConnectedDevices(vector<DeviceInfo> &devices) {
  lock_guard<mutex> lock(mutex_);
  for (DeviceContainer::iterator ite = devices_.begin(); ite != devices_.end(); ++ite) {
    if (ite->connected == true) {
      devices.push_back(ite->info);
    }
  }
}

bool DeviceManager::AddListeningDevice(const string &broadcast_code, DeviceMode mode, uint8_t &handle) {
  lock_guard<mutex> lock(mutex_);
  if (mode == kDeviceModeHub) {
    handle = kHubDefaultHandle;
    devices_[kHubDefaultHandle].connected = false;
    strncpy(devices_[kHubDefaultHandle].info.broadcast_code,
            broadcast_code.c_str(),
            sizeof(devices_[kHubDefaultHandle].info.broadcast_code));
    devices_[kHubDefaultHandle].info.handle = kHubDefaultHandle;
    return true;
  }

  for (DeviceContainer::iterator ite = devices_.begin(); ite != devices_.end(); ++ite) {
    if (strlen(ite->info.broadcast_code) == 0) {
      handle = ite - devices_.begin();
      ite->connected = false;
      strncpy(ite->info.broadcast_code, broadcast_code.c_str(), sizeof(ite->info.broadcast_code));
      ite->info.handle = handle;
      return true;
    } else if (strncmp(ite->info.broadcast_code, broadcast_code.c_str(), sizeof(ite->info.broadcast_code)) == 0) {
      handle = ite->info.handle;
      return true;
    }
  }
  return false;
}

bool DeviceManager::FindDevice(uint8_t handle, DeviceInfo &info) {
  lock_guard<mutex> lock(mutex_);
  if (handle >= devices_.size()) {
    return false;
  }
  info = devices_[handle].info;
  return true;
}

bool DeviceManager::FindDevice(const string &broadcast_code, DeviceInfo &info) {
  lock_guard<mutex> lock(mutex_);
  for (DeviceContainer::iterator ite = devices_.begin(); ite != devices_.end(); ++ite) {
    if (ite->info.broadcast_code == broadcast_code) {
      info = ite->info;
      return true;
    }
  }
  return false;
}

bool DeviceManager::IsDeviceConnected(uint8_t handle) {
  lock_guard<mutex> lock(mutex_);
  if (handle >= devices_.size()) {
    return false;
  }
  return devices_[handle].connected;
}

void DeviceManager::UpdateDeviceState(uint8_t handle, const HeartbeatResponse &response) {
  if (handle >= devices_.size()) {
    return;
  }
  bool update = false;
  DeviceInfo &info = devices_[handle].info;
  if (info.state != response.state) {
    LOG_INFO(" Update State to {}, device connect {}", (uint16_t)response.state, devices_[handle].connected);
    info.state = static_cast<LidarState>(response.state);
    update = true;
  }
  if (info.feature != response.feature) {
    LOG_INFO(" Update feature to {}, device connect {}", (uint16_t)response.feature, devices_[handle].connected);
    info.feature = static_cast<LidarFeature>(response.feature);
    update = true;
  }
  if (response.state == kLidarStateInit) {
    if (info.status.progress != response.error_union.progress) {
      LOG_INFO(
          " Update progress {}, device connect {}", (uint16_t)response.error_union.progress, devices_[handle].connected);
      info.status.progress = response.error_union.progress;
      update = true;
    }
  } else {
    if (info.status.status_code.error_code != response.error_union.status_code.error_code) {
      info.status.status_code.error_code = response.error_union.status_code.error_code;
      update = true;
    }
  }
 
  if (devices_[handle].connected && update == true) {
    if (connected_cb_) {
      connected_cb_(&info, kEventStateChange);
    }
  }
}

bool DeviceManager::IsLidarMid40(uint8_t handle) {
  DeviceInfo lidar_info;
  bool found = device_manager().FindDevice(handle, lidar_info);
  if ( found && lidar_info.type == kDeviceTypeLidarMid40) {
    return true;
  }
  return false;
}

DeviceManager &device_manager() {
  static DeviceManager lidar_manager;
  return lidar_manager;
}

void DeviceFound(const DeviceInfo &lidar_data) {
  device_manager().AddDevice(lidar_data);
  command_handler().AddDevice(lidar_data);
  data_handler().AddDevice(lidar_data);
  if (device_manager().device_mode() == kDeviceModeHub) {
    device_manager().UpdateDevices(lidar_data, kEventHubConnectionChange);
  } else {
    device_manager().UpdateDevices(lidar_data, kEventConnect);
  }
}

void DeviceRemove(uint8_t handle, DeviceEvent device_event) {
  DeviceInfo info;
  bool found = device_manager().FindDevice(handle, info);
  device_manager().RemoveDevice(handle);
  command_handler().RemoveDevice(handle);
  data_handler().RemoveDevice(handle);
  if (found) {
    device_manager().UpdateDevices(info, device_event);
  }
}

}  // namespace livox
