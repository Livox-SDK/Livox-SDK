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

#ifndef LIVOX_SDK_H_
#define LIVOX_SDK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>
#include "livox_def.h"

/**
 * Initialize the SDK.
 * @return true if successfully initialized, otherwise false.
 */
bool Init();

/**
 * Start the device scanning routine which runs on a separate thread.
 * @return true if successfully started, otherwise false.
 */
bool Start();

/**
 * Uninitialize the SDK.
 */
void Uninit();

/**
 * @c SetBroadcastCallback response callback function.
 * @param info information of the broadcast device, becomes invalid after the function returns.
 */
typedef void (*DeviceBroadcastCallback)(const BroadcastDeviceInfo *info);

/**
 * Set the callback of listening device broadcast message. When broadcast message is received from Livox Hub/LiDAR, cb
 * is called.
 * @param cb callback for device broadcast.
 */
void SetBroadcastCallback(DeviceBroadcastCallback cb);

/**
 * @c SetDeviceStateUpdateCallback response callback function.
 * @param device  information of the connected device.
 * @param type    the update type that indicates connection/disconnection of the device or change of working state.
 */
typedef void (*DeviceStateUpdateCallback)(const DeviceInfo *device, DeviceEvent type);

/**
 * @brief Add a callback for device connection or working state changing event.
 * @note Livox SDK supports two hardware connection modes. 1: Directly connecting to the LiDAR device; 2. Connecting to
 * the LiDAR device(s) via the Livox Hub. In the first mode, connection/disconnection of every LiDAR unit is reported by
 * this callback. In the second mode, only connection/disconnection of the Livox Hub is reported by this callback. If
 * you want to get information of the LiDAR unit(s) connected to hub, see \ref HubQueryLidarInformation.
 * @note 3 conditions can trigger this callback:
 *         1. Connection and disconnection of device.
 *         2. A change of device working state.
 *         3. An error occurs.
 * @param cb callback for device connection/disconnection.
 */
void SetDeviceStateUpdateCallback(DeviceStateUpdateCallback cb);

/**
 * Add a broadcast code to the connecting list and only devices with broadcast code in this list will be connected. The
 * broadcast code is unique for every device.
 * @param broadcast_code device's broadcast code.
 * @param handle device handle. For Livox Hub, the handle is always 31; for LiDAR units connected to the Livox Hub,
 * the corresponding handle is (slot-1)*3+id-1.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t AddHubToConnect(const char *broadcast_code, uint8_t *handle);

/**
 * Add a broadcast code to the connecting list and only devices with broadcast code in this list will be connected. The
 * broadcast code is unique for every device.
 * @param broadcast_code device's broadcast code.
 * @param handle device handle. The handle is the same as the order calling AddLidarToConnect starting from 0.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t AddLidarToConnect(const char *broadcast_code, uint8_t *handle);

/**
 * Get all connected devices' information.
 * @param devices list of connected devices' information.
 * @param size    number of devices connected.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t GetConnectedDevices(DeviceInfo *devices, uint8_t *size);

/**
 * Function type of callback that queries device's information.
 * @param status   kStatusSuccess on successful return, kStatusTimeout on timeout, see \ref ResponseStatus for other
 * error code.
 * @param handle   device handle.
 * @param response response from the device.
 * @param client_data user data associated with the command.
 */
typedef void (*DeviceInformationCallback)(uint8_t status,
                                          uint8_t handle,
                                          DeviceInformationResponse *response,
                                          void *client_data);

/**
 * Command to query device's information.
 * @param  handle        device handle.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t QueryDeviceInformation(uint8_t handle, DeviceInformationCallback cb, void *client_data);

/**
 * Callback function for receiving point cloud data.
 * @param handle   device handle.
 * @param data     device's data.
 * @param data_num number of points in data.
 */
typedef void (*DataCallback)(uint8_t handle, LivoxEthPacket *data, uint32_t data_num);

/**
 * Set the callback to receive point cloud data. Only one callback is supported for a specific device. Set the point
 * cloud data callback before beginning sampling.
 * @param cb callback to receive point cloud data.
 */
void SetDataCallback(uint8_t handle, DataCallback cb);

/**
 * Function type of callback with 1 byte of response.
 * @param status      kStatusSuccess on successful return, kStatusTimeout on timeout, see \ref ResponseStatus for other
 * error code.
 * @param handle      device handle.
 * @param response    response from the device.
 * @param client_data user data associated with the command.
 */
typedef void (*CommonCommandCallback)(uint8_t status, uint8_t handle, uint8_t response, void *client_data);

/**
 * Start hub sampling.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t HubStartSampling(CommonCommandCallback cb, void *client_data);

/**
 * Stop the Livox Hub's sampling.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t HubStopSampling(CommonCommandCallback cb, void *client_data);

/**
 * Get the LiDAR unit handle used in the Livox Hub data callback function from slot and id.
 * @param  slot   Livox Hub's slot.
 * @param  id     Livox Hub's id.
 * @return LiDAR unit handle.
 */
uint8_t HubGetLidarHandle(uint8_t slot, uint8_t id);

/**
 * Command to change point cloud coordinate system to cartesian coordinate.
 * @param  handle        device handle.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t SetCartesianCoordinate(uint8_t handle, CommonCommandCallback cb, void *client_data);

/**
 * Change point cloud coordinate system to spherical coordinate.
 * @param  handle        device handle.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t SetSphericalCoordinate(uint8_t handle, CommonCommandCallback cb, void *client_data);

/**
 * Callback of the error status message.
 * @param handle      device handle.
 * @param response    response from the device.
 */
typedef void (*ErrorMessageCallback)(uint8_t handle, ErrorMessage *message);

/**
 * Add error status callback for the device.
 * @param  handle        device handle.
 * @param  cb            callback for the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t SetErrorMessageCallback(uint8_t handle, ErrorMessageCallback cb);

/**
 * Set device's IP mode.
 * @param  handle        device handle.
 * @param  req           request sent to device.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t SetStaticDynamicIP(uint8_t handle,
                           SetDeviceIPModeRequest *req,
                           CommonCommandCallback cb,
                           void *client_data);

/**
 * Callback function that gets device's IP information.
 * @param status      kStatusSuccess on successful return, kStatusTimeout on timeout, see \ref ResponseStatus for other
 * error code.
 * @param handle      device handle.
 * @param response    response from the device.
 * @param client_data user data associated with the command.
 */
typedef void (*GetDeviceIPInformationCallback)(uint8_t status,
                                        uint8_t handle,
                                        GetDeviceIPModeResponse *response,
                                        void *client_data);

/**
 * Get device's IP mode.
 * @param  handle        device handle.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t GetDeviceIPInformation(uint8_t handle, GetDeviceIPInformationCallback cb, void *client_data);

/**
 * @c HubQueryLidarInformation response callback function.
 * @param status      kStatusSuccess on successful return, kStatusTimeout on timeout, see \ref ResponseStatus for other
 * error code.
 * @param handle      device handle.
 * @param response    response from the device.
 * @param client_data user data associated with the command.
 */
typedef void (*HubQueryLidarInformationCallback)(uint8_t status,
                                                 uint8_t handle,
                                                 HubQueryLidarInformationResponse *response,
                                                 void *client_data);

/**
 * Query the information of LiDARs connected to the hub.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t HubQueryLidarInformation(HubQueryLidarInformationCallback cb, void *client_data);

/**
 * @c HubSetMode response callback function.
 * @param status      kStatusSuccess on successful return, kStatusTimeout on timeout, see \ref ResponseStatus for other
 * error code.
 * @param handle      device handle.
 * @param response    response from the device.
 * @param client_data user data associated with the command.
 */
typedef void (*HubSetModeCallback)(uint8_t status, uint8_t handle, HubSetModeResponse *response, void *client_data);

/**
 * Set the mode of LiDAR unit connected to the Livox Hub.
 * @param  req           mode configuration of LiDAR units.
 * @param  length        length of req.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t HubSetMode(HubSetModeRequest *req, uint16_t length, HubSetModeCallback cb, void *client_data);

/**
 * @c HubQueryLidarStatus response callback function.
 * @param status      kStatusSuccess on successful return, kStatusTimeout on timeout, see \ref ResponseStatus for other
 * error code.
 * @param handle      device handle.
 * @param response    response from the device.
 * @param client_data user data associated with the command.
 */
typedef void (*HubQueryLidarStatusCallback)(uint8_t status, uint8_t handle, HubQueryLidarStatusResponse *response, void *client_data);

/**
 * Get the state of LiDAR units connected to the Livox Hub.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t HubQueryLidarStatus(HubQueryLidarStatusCallback cb, void *client_data);

/**
 * Toggle the power supply of designated slots.
 * @param  req           request whether to enable or disable the power of designated slots.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */

uint8_t HubControlSlotPower(HubControlSlotPowerRequest *req, CommonCommandCallback cb, void *client_data);

/**
 * @c HubSetExtrinsicParameter response callback function.
 * @param status      kStatusSuccess on successful return, kStatusTimeout on timeout, see \ref ResponseStatus for other
 * error code.
 * @param handle      device handle.
 * @param response    response from the device.
 * @param client_data user data associated with the command.
 */
typedef void (*HubSetExtrinsicParameterCallback)(uint8_t status,
                                                 uint8_t handle,
                                                 HubSetExtrinsicParameterResponse *response,
                                                 void *client_data);

/**
 * Set extrinsic parameters of LiDAR units connected to the Livox Hub.
 * @param  req           the parameters to write.
 * @param  length        the request's length.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t HubSetExtrinsicParameter(HubSetExtrinsicParameterRequest *req,
                                 uint16_t length,
                                 HubSetExtrinsicParameterCallback cb,
                                 void *client_data);

/**
 * @c HubGetExtrinsicParameter response callback function.
 * @param status      kStatusSuccess on successful return, kStatusTimeout on timeout, see \ref ResponseStatus for other
 * error code.
 * @param handle      device handle.
 * @param response    response from the device.
 * @param client_data user data associated with the command.
 */
typedef void (*HubGetExtrinsicParameterCallback)(uint8_t status,
                                                 uint8_t handle,
                                                 HubGetExtrinsicParameterResponse *response,
                                                 void *client_data);

/**
 * Get extrinsic parameters of LiDAR units connected to the Livox Hub.
 * @param  req           the LiDAR units broadcast code list.
 * @param  length        the request's length.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t HubGetExtrinsicParameter(HubGetExtrinsicParameterRequest *req,
                                 uint16_t length,
                                 HubGetExtrinsicParameterCallback cb,
                                 void *client_data);

/**
 * Turn on or off the calculation of extrinsic parameters.
 * @param  enable        the request whether enable or disable calculating the extrinsic parameters.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */

uint8_t HubExtrinsicParameterCalculation(bool enable, CommonCommandCallback cb, void *client_data);

/**
 * @c HubRainFogSuppress response callback function.
 * @param status      kStatusSuccess on successful return, kStatusTimeout on timeout, see \ref ResponseStatus for other
 * error code.
 * @param handle      device handle.
 * @param response    response from the device.
 * @param client_data user data associated with the command.
 */
typedef void (*HubRainFogSuppressCallback)(uint8_t status,
                                           uint8_t handle,
                                           HubRainFogSuppressResponse *response,
                                           void *client_data);

/**
 * Toggling the rain and fog mode for lidars connected to the hub.
 * @param  req           the request whether open or close the rain and fog mode.
 * @param  length        the request's length.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t HubRainFogSuppress(HubRainFogSuppressRequest *req,
                           uint16_t length,
                           HubRainFogSuppressCallback cb,
                           void *client_data);

/**
 * Start LiDAR sampling.
 * @param  handle        device handle.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t LidarStartSampling(uint8_t handle, CommonCommandCallback cb, void *client_data);

/**
 * Stop LiDAR sampling.
 * @param  handle        device handle.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t LidarStopSampling(uint8_t handle, CommonCommandCallback cb, void *client_data);

/**
 * Set LiDAR mode.
 * @note Successful callback function status only means LiDAR successfully starting the changing process of mode.
 * You need to observe the actually change of mode in DeviceStateUpdateCallback function.
 * @param  handle        device handle.
 * @param  mode          the mode to change.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t LidarSetMode(uint8_t handle, LidarMode mode, CommonCommandCallback cb, void *client_data);

/**
 * Set LiDAR extrinsic parameters.
 * @param  handle        device handle.
 * @param  param         the parameters to write.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t LidarSetExtrinsicParameter(uint8_t handle,
                                   LidarSetExtrinsicParameterRequest *req,
                                   CommonCommandCallback cb,
                                   void *client_data);

/**
 * @c LidarGetExtrinsicParameter response callback function.
 * @param status      kStatusSuccess on successful return, kStatusTimeout on timeout, see \ref ResponseStatus for other
 * error code.
 * @param handle      device handle.
 * @param response    response from the device.
 * @param client_data user data associated with the command.
 */
typedef void (*LidarGetExtrinsicParameterCallback)(uint8_t status,
                                          uint8_t handle,
                                          LidarGetExtrinsicParameterResponse *response,
                                          void *client_data);

/**
 * Get LiDAR extrinsic parameters.
 * @param  handle        device handle.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t LidarGetExtrinsicParameter(uint8_t handle, LidarGetExtrinsicParameterCallback cb, void *client_data);

/**
 * Enable and disable the rain/fog suppression.
 * @param  handle        device handle.
 * @param  cb            callback for the command.
 * @param  client_data   user data associated with the command.
 * @return kStatusSuccess on successful return, see \ref ResponseStatus for other error code.
 */
uint8_t LidarRainFogSuppress(uint8_t handle, bool enable, CommonCommandCallback cb, void *client_data);

#ifdef __cplusplus
}
#endif

#endif  // LIVOX_SDK_H_
