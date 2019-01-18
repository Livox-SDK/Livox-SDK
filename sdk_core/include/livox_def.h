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

#ifndef LIVOX_DEF_H_
#define LIVOX_DEF_H_

#include <stdint.h>

#define kMaxLidarCount 32

/** Device type. */
typedef enum {
  kDeviceTypeHub = 0,     /**< Livox Hub. */
  kDeviceTypeLidarMid40 = 1, /**< Mid-40. */
  kDeviceTypeLidarTele = 2, /**< Tele. */
  kDeviceTypeLidarHorizon = 3  /**< Horizon. */
} DeviceType;

/** Lidar state. */
typedef enum {
  kLidarStateInit = 0,        /**< Initialization state. */
  kLidarStateNormal = 1,      /**< Normal work state. */
  kLidarStatePowerSaving = 2, /**< Power-saving state. */
  kLidarStateStandBy = 3,     /**< Standby state. */
  kLidarStateError = 4,       /**< Error state. */
  kLidarStateUnknown = 5
} LidarState;

/** Lidar mode. */
typedef enum {
  kLidarModeNormal = 1,      /**< Normal mode. */
  kLidarModePowerSaving = 2, /**< Power-saving mode. */
  kLidarModeStandby = 3      /**< Standby mode. */
} LidarMode;

/** Lidar feature. */
typedef enum {
  kLidarFeatureNone = 0,   /**< No feature. */
  kLidarFeatureRainFog = 1 /**< Rain and fog feature. */
} LidarFeature;

/** Function return value definition. */
typedef enum {
  kStatusSuccess = 0,        /**< Success. */
  kStatusFailure = 1,        /**< Failure. */
  kStatusNotConnected = 2,   /**< Requested device is not connected. */
  kStatusNotSupported = 3,   /**< Operation is not supported on this device. */
  kStatusTimeout = 4,        /**< Operation timeouts. */
  kStatusNotEnoughMemory = 5 /**< No enough memory. */
} ResponseStatus;

/** Device update type, indicating the change of device connection or working state. */
typedef enum {
  kEventConnect = 0,    /**< Device is connected. */
  kEventDisconnect = 1, /**< Device is removed. */
  kEventStateChange = 2 /**< Device working state changes or an error occurs. */
} DeviceEvent;

/** Timestamp sync mode define. */
typedef enum {
  kTimestampTypeNoSync = 0, /**< No sync signal mode. */
  kTimestampTypePtp = 1,    /**< 1588v2.0 PTP sync mode. */
  kTimestampTypeRsvd = 2,   /**< Reserved use. */
  kTimestampTypePpsGps = 3, /**< pps+gps sync mode. */
  kTimestampTypePps = 4,    /**< pps only sync mode. */
  kTimestampTypeUnknown = 5 /**< Unknown mode. */
} TimestampType;

#pragma pack(1)

#define kBroadcastCodeSize 16

/** Cartesian coordinate format. */
typedef struct {
  int32_t x;            /**< X axis, Unit:mm */
  int32_t y;            /**< Y axis, Unit:mm */
  int32_t z;            /**< Z axis, Unit:mm */
  uint8_t reflectivity; /**< Reflectivity */
} LivoxRawPoint;

/** standard point cloud format */
typedef struct {
  float x;              /**< X axis, Unit:m */
  float y;              /**< X axis, Unit:m */
  float z;              /**< X axis, Unit:m */
  uint8_t reflectivity; /**< Reflectivity */
} LivoxPoint;

/** Point cloud packet. */
typedef struct {
  uint8_t version;        /**< Packet protocol version. */
  uint8_t slot;           /**< Slot number used for connecting LiDAR. */
  uint8_t id;             /**< LiDAR id. */
  uint8_t rsvd;           /**< Reserved. */
  uint32_t err_code;      /**< Device error status indicator information. */
  uint8_t timestamp_type; /**< Timestamp type. */
  /** Point cloud coordinate format, 1 for spherical coordinate data, 0 for cartesian coordinate data. */
  uint8_t data_type;
  uint8_t timestamp[8]; /**< Nanosecond or UTC format timestamp. */
  uint8_t data[1];      /**< Point cloud data. */
} LivoxEthPacket;

/** Information of LiDAR work state. */
typedef union {
  uint32_t status_code; /**< LiDAR work state status code. */
  uint32_t progress;   /**< LiDAR work state switching progress. */
} StatusUnion;

/** Information of the connected LiDAR or hub. */
typedef struct {
  char broadcast_code[kBroadcastCodeSize]; /**< Device broadcast code, null-terminated string, 15 characters at most. */
  uint8_t handle;                          /**< Device handle. */
  uint8_t slot;                            /**< Slot number used for connecting LiDAR. */
  uint8_t id;                              /**< LiDAR id. */
  uint32_t type;                           /**< Device type, refer to \ref DeviceType. */
  uint16_t data_port;                      /**< Point cloud data UDP port. */
  uint16_t cmd_port;                       /**< Control command UDP port. */
  char ip[16];                             /**< IP address. */
  LidarState state;                        /**< LiDAR state. */
  LidarFeature feature;                    /**< LiDAR feature. */
  StatusUnion status;                      /** LiDAR work state status. */
} DeviceInfo;

/** The information of broadcast device. */
typedef struct {
  char broadcast_code[kBroadcastCodeSize]; /**< Device broadcast code, null-terminated string, 15 characters at most. */
  uint8_t dev_type;                        /**< Device type, refer to \ref DeviceType. */
  uint16_t reserved;                           /**< Reserved. */
} BroadcastDeviceInfo;

/** The information of LiDAR units that are connected to the Livox Hub. */
typedef struct {
  char broadcast_code[kBroadcastCodeSize]; /**< Device broadcast code, null-terminated string, 15 characters at most. */
  uint8_t dev_type;                        /**< Device type, refer to \ref DeviceType. */
  uint8_t version[4];                      /**< Firmware version. */
  uint8_t slot;                            /**< Slot number used for connecting LiDAR units. */
  uint8_t id;                              /**< Device id. */
} ConnectedLidarInfo;

/** LiDAR mode configuration information. */
typedef struct {
  char broadcast_code[kBroadcastCodeSize]; /**< Device broadcast code, null-terminated string, 15 characters at most. */
  uint8_t state;                           /**< LiDAR state. */
} LidarModeRequestItem;

typedef struct {
  uint8_t ret_code;                        /**< Return code. */
  char broadcast_code[kBroadcastCodeSize]; /**< Device broadcast code. */
} ReturnCode;

typedef struct {
  char broadcast_code[kBroadcastCodeSize]; /**< Device broadcast code. */
} DeviceBroadcastCode;

typedef struct {
  char broadcast_code[kBroadcastCodeSize]; /**< Device broadcast code. */
  uint8_t feature;                         /**< Close or open the rain and fog feature. */
} RainFogSuppressRequestItem;

/** LiDAR configuration information. */
typedef struct {
  char broadcast_code[kBroadcastCodeSize]; /**< Device broadcast code. */
  float roll;                              /**< Roll angle, unit: degree. */
  float pitch;                             /**< Pitch angle, unit: degree. */
  float yaw;                               /**< Yaw angle, unit: degree. */
  int32_t x;                               /**< X translation, unit: mm. */
  int32_t y;                               /**< Y translation, unit: mm. */
  int32_t z;                               /**< Z translation, unit: mm. */
} ExtrinsicParameterRequestItem;

/** LiDAR extrinsic parameters. */
typedef struct {
  uint8_t ret_code;                        /**< Return code. */
  char broadcast_code[kBroadcastCodeSize]; /**< Broadcast code. */
  float roll;                              /**< Roll angle, unit: degree. */
  float pitch;                             /**< Pitch angle, unit: degree. */
  float yaw;                               /**< Yaw angle, unit: degree. */
  int32_t x;                               /**< X translation, unit: mm. */
  int32_t y;                               /**< Y translation, unit: mm. */
  int32_t z;                               /**< Z translation, unit: mm. */
} ExtrinsicParameterResponseItem;

typedef struct {
  char broadcast_code[kBroadcastCodeSize]; /**< Broadcast code. */
  uint8_t state;                           /**< LiDAR state. */
  uint8_t feature;                         /**< LiDAR feature. */
  StatusUnion error_union;                  /** LiDAR work state. */
} LidarStateItem;

/**
 * The request body for the command of handshake.
 */
typedef struct {
  uint32_t ip_addr;   /**< IP address of the device. */
  uint16_t data_port; /**< UDP port of the data connection. */
  uint16_t cmd_port;  /**< UDP port of the command connection. */
} HandshakeRequest;

/**
 * The response body of querying device information.
 */
typedef struct {
  uint8_t ret_code;            /**< Return code. */
  uint8_t firmware_version[4]; /**< Firmware version. */
} DeviceInformationResponse;

/**
 * The response body of getting device error status.
 */
typedef struct {
  uint32_t error_code; /**< Error code. */
} ErrorMessage;

/**
 * The request body of the command for setting device's IP mode.
 */
typedef struct {
  uint8_t ip_mode;  /**< IP address mode: 0 for dynamic IP address, 1 for static IP address.  */
  uint32_t ip_addr; /**< IP address. */
} SetDeviceIPModeRequest;

/**
 * The response body of getting device's IP mode.
 */
typedef struct {
  uint8_t ret_code; /**< Return code. */
  uint8_t ip_mode;  /**< IP address mode: 0 for dynamic IP address, 1 for static IP address.  */
  uint32_t ip_addr; /**< IP address. */
} GetDeviceIPModeResponse;

/**
 * The body of heartbeat response.
 */
typedef struct {
  uint8_t ret_code;       /**< Return code. */
  uint8_t state;          /**< Working state. */
  uint8_t feature;        /**< LiDAR feature. */
  StatusUnion error_union; /**< LiDAR work state. */
} HeartbeatResponse;

/**
 * The request body for the command of setting LiDAR's parameters.
 */
typedef struct {
  float roll;  /**< Roll angle, unit: degree. */
  float pitch; /**< Pitch angle, unit: degree. */
  float yaw;   /**< Yaw angle, unit: degree. */
  int32_t x;   /**< X translation, unit: mm. */
  int32_t y;   /**< Y translation, unit: mm. */
  int32_t z;   /**< Z translation, unit: mm. */
} LidarSetExtrinsicParameterRequest;

/**
 * The response body of getting LiDAR's parameters.
 */
typedef struct {
  uint8_t ret_code;
  float roll;  /**< Roll angle, unit: degree. */
  float pitch; /**< Pitch angle, unit: degree. */
  float yaw;   /**< Yaw angle, unit: degree. */
  int32_t x;   /**< X translation, unit: mm. */
  int32_t y;   /**< Y translation, unit: mm. */
  int32_t z;   /**< Z translation, unit: mm. */
} LidarGetExtrinsicParameterResponse;

/**
 * The response body of querying the information of LiDAR units connected to the Livox Hub.
 */
typedef struct {
  uint8_t ret_code;                       /**< Return code from device. */
  uint8_t count;                          /**< Count of device_info_list. */
  ConnectedLidarInfo device_info_list[1]; /**< Connected lidars information. */
} HubQueryLidarInformationResponse;

/**
 * The request body of setting LiDAR units working mode.
 */
typedef struct {
  uint8_t count;                  /**< Number of LiDAR units to set. */
  LidarModeRequestItem config_list[1]; /**< LiDAR mode configuration list. */
} HubSetModeRequest;

/**
 * The response of setting LiDAR units working mode.
 */
typedef struct {
  uint8_t ret_code;             /**< Return code from device. */
  uint8_t count;                /**< Count of ret_state_list. */
  ReturnCode ret_state_list[1]; /**< Return status list. */
} HubSetModeResponse;

/**
 * The request body of toggling the power supply of the slot.
 */
typedef struct {
  uint8_t slot;  /**< Slot of the hub. */
  uint8_t state; /**< Status of toggling the power supply. */
} HubControlSlotPowerRequest;

/**
 * The request body of setting the Livox Hub's parameters.
 */
typedef struct {
  uint8_t count;                 /**< Count of cfg_param_list. */
  ExtrinsicParameterRequestItem parameter_list[1]; /**< Configuration parameter list. */
} HubSetExtrinsicParameterRequest;

/**
 * The response body of setting the Livox Hub's parameters.
 */
typedef struct {
  uint8_t ret_code;            /**< Return code. */
  uint8_t count;               /**< Count of ret_code_list. */
  ReturnCode ret_code_list[1]; /**< Return code */
} HubSetExtrinsicParameterResponse;

/**
 * The request body of getting the Livox Hub's parameters.
 */
typedef struct {
  uint8_t count;                    /**< Count of code_list. */
  DeviceBroadcastCode code_list[1]; /**< Broadcast code list. */
} HubGetExtrinsicParameterRequest;

/**
 * The response body of getting the Livox Hub's parameters.
 */
typedef struct {
  uint8_t ret_code;        /**< Return code. */
  uint8_t count;           /**< Number of LiDAR units connected to the Livox Hub. */
  ExtrinsicParameterResponseItem parameter_list[1]; /**< Postion parameters of connected LiDAR unit(s). */
} HubGetExtrinsicParameterResponse;

/**
 * The response body of getting sub LiDAR's state.
 */
typedef struct {
  uint8_t ret_code;             /**< Return code. */
  uint8_t count;                /**< Number of LiDAR connected to the Livox Hub. */
  LidarStateItem state_list[1]; /**< Device information of connected LiDAR units. */
} HubQueryLidarStatusResponse;

/**
 * The request body of toggling the LiDAR units rain and fog mode.
 */
typedef struct {
  uint8_t count;                                   /**< Number of LiDAR units connected to the Livox Hub. */
  RainFogSuppressRequestItem lidar_cfg_list[1]; /**< Command data of connected LiDAR units. */
} HubRainFogSuppressRequest;

/**
 * The response body of toggling the LiDAR units rain and fog mode.
 */
typedef struct {
  uint8_t ret_code;             /**< Return code. */
  uint8_t count;                /**< Count of ret_state_list. */
  ReturnCode ret_state_list[1]; /**< Return code */
} HubRainFogSuppressResponse;

#pragma pack()

#endif  // LIVOX_DEF_H_
