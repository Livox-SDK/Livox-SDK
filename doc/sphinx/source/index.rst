.. Didar documentation master file, created by
   sphinx-quickstart on Mon Dec 10 15:38:07 2018.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to the Livox LiDAR SDK API Reference
#############################################

.. toctree::
   :maxdepth: 2
   :numbered:
   :caption: Contents:

Basic Types and Functions
#############################

.. doxygenenum:: DeviceType
   :project: didar

.. doxygenenum:: LidarState
   :project: didar

.. doxygenenum:: LidarFeature
   :project: didar

.. doxygenenum:: ResponseStatus
   :project: didar

.. doxygenenum:: DeviceEvent
   :project: didar

.. doxygenenum:: TimestampType
   :project: didar

.. doxygenstruct:: DeviceInfo
   :project: didar
   :members:
   :undoc-members:

.. doxygenunion:: StatusUnion
  :project: didar

.. doxygenfunction:: Init
   :project: didar

.. doxygenfunction:: Start
   :project: didar

.. doxygenfunction:: Uninit
   :project: didar

.. doxygenstruct:: BroadcastDeviceInfo
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: DeviceBroadcastCallback
  :project: didar

.. doxygenfunction:: SetBroadcastCallback
   :project: didar

.. doxygentypedef:: DeviceStateUpdateCallback
   :project: didar

.. doxygenfunction:: SetDeviceStateUpdateCallback
   :project: didar

.. doxygenfunction:: AddHubToConnect
   :project: didar

.. doxygenfunction:: AddLidarToConnect
   :project: didar

.. doxygenfunction:: GetConnectedDevices
   :project: didar

General Functions
######################

Query Device Information
==========================

.. doxygenstruct:: DeviceInformationResponse
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: DeviceInformationCallback
   :project: didar

.. doxygenfunction:: QueryDeviceInformation
   :project: didar

Receive Point Cloud Data
==========================

.. doxygenstruct:: LivoxEthPacket
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: DataCallback
   :project: didar

.. doxygenfunction:: SetDataCallback
   :project: didar

.. doxygenfunction:: HubGetLidarHandle
   :project: didar

Set Coordinate System
============================

.. doxygenfunction:: SetCartesianCoordinate
   :project: didar

.. doxygenfunction:: SetSphericalCoordinate
   :project: didar

Error Message From Device
=======================================

.. doxygenstruct:: ErrorMessage
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: ErrorMessageCallback
   :project: didar

.. doxygenfunction:: SetErrorMessageCallback
   :project: didar

Configure Static/Dynamic IP
=======================================

.. doxygenstruct:: SetDeviceIPModeRequest
   :project: didar
   :members:
   :undoc-members:

.. doxygenfunction:: SetStaticDynamicIP
   :project: didar

.. doxygenstruct:: GetDeviceIPModeResponse
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: GetDeviceIPInformationCallback
   :project: didar

.. doxygenfunction:: GetDeviceIPInformation
   :project: didar

Livox Hub Functions
######################

Query Connected LiDAR Unit Information
=======================================

.. doxygenstruct:: ConnectedLidarInfo
   :project: didar
   :members:
   :undoc-members:

.. doxygenstruct:: HubQueryLidarInformationResponse
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: HubQueryLidarInformationCallback
   :project: didar

.. doxygenfunction:: HubQueryLidarInformation
   :project: didar

Configure Lidars Mode
==========================================

.. doxygenstruct:: HubSetModeResponse
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: HubSetModeCallback
   :project: didar

.. doxygenstruct:: HubSetModeRequest
   :project: didar
   :members:
   :undoc-members:

.. doxygenstruct:: LidarModeRequestItem
   :project: didar
   :members:
   :undoc-members:

.. doxygenfunction:: HubSetMode
   :project: didar

.. doxygenstruct:: LidarStateItem
   :project: didar
   :members:
   :undoc-members:

.. doxygenstruct:: HubQueryLidarStatusResponse
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: HubQueryLidarStatusCallback
   :project: didar

.. doxygenfunction:: HubQueryLidarStatus
   :project: didar

Sampling Control
=================

.. doxygentypedef:: CommonCommandCallback
   :project: didar

.. doxygenfunction:: HubStartSampling
   :project: didar

.. doxygenfunction:: HubStopSampling
   :project: didar

Slot Power Control
========================

.. doxygenstruct:: HubControlSlotPowerRequest
   :project: didar
   :members:
   :undoc-members:

.. doxygenfunction:: HubControlSlotPower
   :project: didar

Configure Livox Hub Extrinsic Parameters
========================================

.. doxygenstruct:: HubSetExtrinsicParameterResponse
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: HubSetExtrinsicParameterCallback
   :project: didar

.. doxygenstruct:: HubSetExtrinsicParameterRequest
   :project: didar
   :members:
   :undoc-members:

.. doxygenstruct:: ExtrinsicParameterRequestItem
   :project: didar
   :members:
   :undoc-members:

.. doxygenfunction:: HubSetExtrinsicParameter
   :project: didar

.. doxygenstruct:: HubGetExtrinsicParameterRequest
   :project: didar
   :members:
   :undoc-members:

.. doxygenstruct:: DeviceBroadcastCode
   :project: didar
   :members:
   :undoc-members:

.. doxygenstruct:: HubGetExtrinsicParameterResponse
   :project: didar
   :members:
   :undoc-members:

.. doxygenstruct:: ExtrinsicParameterResponseItem
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: HubGetExtrinsicParameterCallback
   :project: didar

.. doxygenfunction:: HubGetExtrinsicParameter
   :project: didar

Enable Hub Calculating Extrinsic Parameters
========================================================

.. doxygenfunction:: HubExtrinsicParameterCalculation
   :project: didar

Enable or Disable The Rain/Fog Suppression
========================================================

.. doxygenstruct:: RainFogSuppressRequestItem
   :project: didar
   :members:
   :undoc-members:

.. doxygenstruct:: HubRainFogSuppressRequest
   :project: didar
   :members:
   :undoc-members:

.. doxygenstruct:: HubRainFogSuppressResponse
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: HubRainFogSuppressCallback
   :project: didar

.. doxygenfunction:: HubRainFogSuppress
   :project: didar

LiDAR Functions
#####################

Configure LiDAR Mode
==============================

.. doxygenenum:: LidarMode
   :project: didar

.. doxygenfunction:: LidarSetMode
   :project: didar

Sample Control
=================

.. doxygenfunction:: LidarStartSampling
   :project: didar

.. doxygenfunction:: LidarStopSampling
   :project: didar

Configure LiDAR Extrinsic Parameters
====================================
.. doxygenstruct:: LidarSetExtrinsicParameterRequest
   :project: didar
   :members:
   :undoc-members:

.. doxygenfunction:: LidarSetExtrinsicParameter
   :project: didar

.. doxygenstruct:: LidarGetExtrinsicParameterResponse
   :project: didar
   :members:
   :undoc-members:

.. doxygentypedef:: LidarGetExtrinsicParameterCallback
   :project: didar

.. doxygenfunction:: LidarGetExtrinsicParameter
   :project: didar

Enable and Disable the Rain/Fog Suppression
============================================

.. doxygenfunction:: LidarRainFogSuppress
   :project: didar