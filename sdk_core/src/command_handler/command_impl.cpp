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

#include <livox_sdk.h>

#include "command_handler.h"
#include "command_impl.h"
#include "data_handler/data_handler.h"
#include "device_manager.h"
#include "livox_def.h"
#include "livox_sdk.h"

using std::pair;
using std::vector;
using namespace livox;

void SetDeviceStateUpdateCallback(DeviceStateUpdateCallback cb) {
  device_manager().SetDeviceConnectedCallback(cb);
}

void SetBroadcastCallback(DeviceBroadcastCallback cb) {
  device_manager().SetDeviceBroadcastCallback(cb);
}

uint8_t AddHubToConnect(const char *broadcast_code, uint8_t *handle) {
  bool result = device_manager().AddListeningDevice(broadcast_code, kDeviceModeHub, *handle);
  if (result) {
    return kStatusSuccess;
  } else {
    return kStatusFailure;
  }
}

uint8_t AddLidarToConnect(const char *broadcast_code, uint8_t *handle) {
  bool result = device_manager().AddListeningDevice(broadcast_code, kDeviceModeLidar, *handle);
  if (result) {
    return kStatusSuccess;
  } else {
    return kStatusFailure;
  }
}

uint8_t GetConnectedDevices(DeviceInfo *devices, uint8_t *size) {
  if (devices == NULL || size == NULL) {
    return kStatusFailure;
  }
  vector<DeviceInfo> device;
  device_manager().GetConnectedDevices(device);
  uint16_t final_size = (*size >= device.size()) ? device.size() : *size;
  for (int i = 0; i < final_size; i++) {
    devices[i] = device[i];
  }
  *size = final_size;
  return kStatusSuccess;
}

void SetDataCallback(uint8_t handle, DataCallback cb, void *client_data) {
  data_handler().AddDataListener(handle, cb, client_data);
}

uint8_t DeviceSampleControl(uint8_t handle, bool enable, CommonCommandCallback cb, void *data) {
  uint8_t req = enable;
  bool result = command_handler().SendCommand(handle,
                                              kCommandSetGeneral,
                                              kCommandIDGeneralControlSample,
                                              &req,
                                              sizeof(req),
                                              MakeCommandCallback<uint8_t>(cb, data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t LidarStartSampling(uint8_t handle, CommonCommandCallback cb, void *data) {
  if (device_manager().device_mode() != kDeviceModeLidar) {
    return kStatusNotSupported;
  }
  return DeviceSampleControl(handle, true, cb, data);
}

uint8_t LidarStopSampling(uint8_t handle, CommonCommandCallback cb, void *data) {
  if (device_manager().device_mode() != kDeviceModeLidar) {
    return kStatusNotSupported;
  }
  return DeviceSampleControl(handle, false, cb, data);
}

uint8_t HubStartSampling(CommonCommandCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  return DeviceSampleControl(kHubDefaultHandle, true, cb, client_data);
}

uint8_t HubStopSampling(CommonCommandCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  return DeviceSampleControl(kHubDefaultHandle, false, cb, client_data);
}

uint8_t HubGetLidarHandle(uint8_t slot, uint8_t id) {
  return (slot - 1) * 3 + id - 1;
}

uint8_t QueryDeviceInformation(uint8_t handle, DeviceInformationCallback cb, void *data) {
  bool result = command_handler().SendCommand(handle,
                                              kCommandSetGeneral,
                                              kCommandIDGeneralDeviceInfo,
                                              NULL,
                                              0,
                                              MakeCommandCallback<DeviceInformationResponse>(cb, data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t SetCartesianCoordinate(uint8_t handle, CommonCommandCallback cb, void *client_data) {
  uint8_t req = 0;
  bool result = command_handler().SendCommand(handle,
                                              kCommandSetGeneral,
                                              kCommandIDGeneralCoordinateSystem,
                                              &req,
                                              sizeof(req),
                                              MakeCommandCallback<uint8_t>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t SetSphericalCoordinate(uint8_t handle, CommonCommandCallback cb, void *client_data) {
  uint8_t req = 1;
  bool result = command_handler().SendCommand(handle,
                                              kCommandSetGeneral,
                                              kCommandIDGeneralCoordinateSystem,
                                              &req,
                                              sizeof(req),
                                              MakeCommandCallback<uint8_t>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t SetErrorMessageCallback(uint8_t handle, ErrorMessageCallback cb) {
  bool result = command_handler().RegisterPush(
      handle, kCommandSetGeneral, kCommandIDGeneralPushAbnormalState, MakeMessageCallback<ErrorMessage>(cb));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t SetStaticDynamicIP(uint8_t handle,
                           SetDeviceIPModeRequest *req,
                           CommonCommandCallback cb,
                           void *client_data) {
  bool result = command_handler().SendCommand(handle,
                                              kCommandSetGeneral,
                                              kCommandIDGeneralConfigureStaticDynamicIP,
                                              (uint8_t *)req,
                                              sizeof(*req),
                                              MakeCommandCallback<uint8_t>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t GetDeviceIPInformation(uint8_t handle, GetDeviceIPInformationCallback cb, void *client_data) {
  bool result = command_handler().SendCommand(handle,
                                              kCommandSetGeneral,
                                              kCommandIDGeneralGetDeviceIPInformation,
                                              NULL,
                                              0,
                                              MakeCommandCallback<GetDeviceIPModeResponse>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t LidarSetMode(uint8_t handle, LidarMode mode, CommonCommandCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeLidar) {
    return kStatusNotSupported;
  }
  uint8_t req = static_cast<uint8_t>(mode);
  bool result = command_handler().SendCommand(handle,
                                              kCommandSetLidar,
                                              kCommandIDLidarSetMode,
                                              &req,
                                              sizeof(req),
                                              MakeCommandCallback<uint8_t>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t LidarSetExtrinsicParameter(uint8_t handle,
                                   LidarSetExtrinsicParameterRequest *req,
                                   CommonCommandCallback cb,
                                   void *client_data) {
  if (device_manager().device_mode() != kDeviceModeLidar) {
    return kStatusNotSupported;
  }
  bool result = command_handler().SendCommand(handle,
                                              kCommandSetLidar,
                                              kCommandIDLidarSetExtrinsicParameter,
                                              (uint8_t *)req,
                                              sizeof(*req),
                                              MakeCommandCallback<uint8_t>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t LidarGetExtrinsicParameter(uint8_t handle, LidarGetExtrinsicParameterCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeLidar) {
    return kStatusNotSupported;
  }
  bool result = command_handler().SendCommand(handle,
                                              kCommandSetLidar,
                                              kCommandIDLidarGetExtrinsicParameter,
                                              NULL,
                                              0,
                                              MakeCommandCallback<LidarGetExtrinsicParameterResponse>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t LidarRainFogSuppress(uint8_t handle, bool enable, CommonCommandCallback cb, void *data) {
  uint8_t req = static_cast<uint8_t>(enable);
  bool result = command_handler().SendCommand(handle,
                                              kCommandSetLidar,
                                              kCommandIDLidarControlRainFogSuppression,
                                              &req,
                                              sizeof(req),
                                              MakeCommandCallback<uint8_t>(cb, data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t HubQueryLidarInformation(HubQueryLidarInformationCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  bool result = command_handler().SendCommand(kHubDefaultHandle,
                                              kCommandSetHub,
                                              kCommandIDHubQueryLidarInformation,
                                              NULL,
                                              0,
                                              MakeCommandCallback<HubQueryLidarInformationResponse>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t HubSetMode(HubSetModeRequest *req, uint16_t length, HubSetModeCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  bool result = command_handler().SendCommand(kHubDefaultHandle,
                                              kCommandSetHub,
                                              kCommandIDHubSetMode,
                                              (uint8_t *)req,
                                              length,
                                              MakeCommandCallback<HubSetModeResponse>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t HubControlSlotPower(HubControlSlotPowerRequest *req, CommonCommandCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  bool result = command_handler().SendCommand(kHubDefaultHandle,
                                              kCommandSetHub,
                                              kCommandIDHubControlSlotPower,
                                              (uint8_t *)req,
                                              sizeof(*req),
                                              MakeCommandCallback<uint8_t>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t HubSetExtrinsicParameter(HubSetExtrinsicParameterRequest *req,
                                 uint16_t length,
                                 HubSetExtrinsicParameterCallback cb,
                                 void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  bool result = command_handler().SendCommand(kHubDefaultHandle,
                                              kCommandSetHub,
                                              kCommandIDHubSetExtrinsicParameter,
                                              (uint8_t *)req,
                                              length,
                                              MakeCommandCallback<HubSetExtrinsicParameterResponse>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t HubGetExtrinsicParameter(HubGetExtrinsicParameterCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  bool result = command_handler().SendCommand(kHubDefaultHandle,
                                              kCommandSetHub,
                                              kCommandIDHubGetExtrinsicParameter,
                                              NULL,
                                              0,
                                              MakeCommandCallback<HubGetExtrinsicParameterResponse>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t HubQueryLidarStatus(HubQueryLidarStatusCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  bool result = command_handler().SendCommand(kHubDefaultHandle,
                                              kCommandSetHub,
                                              kCommandIDHubQueryLidarDeviceStatus,
                                              NULL,
                                              0,
                                              MakeCommandCallback<HubQueryLidarStatusResponse>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t HubExtrinsicParameterCalculation(bool enable, CommonCommandCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  uint8_t req = static_cast<uint8_t>(enable);
  bool result = command_handler().SendCommand(kHubDefaultHandle,
                                              kCommandSetHub,
                                              kCommandIDHubExtrinsicParameterCalculation,
                                              &req,
                                              sizeof(req),
                                              MakeCommandCallback<uint8_t>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t HubRainFogSuppress(HubRainFogSuppressRequest *req,
                           uint16_t length,
                           HubRainFogSuppressCallback cb,
                           void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  bool result = command_handler().SendCommand(kHubDefaultHandle,
                                              kCommandSetHub,
                                              kCommandIDHubRainFogSuppression,
                                              (uint8_t *)req,
                                              length,
                                              MakeCommandCallback<HubRainFogSuppressResponse>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}

uint8_t HubQuerySlotPowerStatus(HubQuerySlotPowerStatusCallback cb, void *client_data) {
  if (device_manager().device_mode() != kDeviceModeHub) {
    return kStatusNotSupported;
  }
  bool result = command_handler().SendCommand(kHubDefaultHandle,
                                              kCommandSetHub,
                                              kCommandIDHubQuerySlotPowerStatus,
                                              NULL,
                                              0,
                                              MakeCommandCallback<HubQuerySlotPowerStatusResponse>(cb, client_data));
  return result ? kStatusSuccess : kStatusFailure;
}