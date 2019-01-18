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

#ifndef LIVOX_LIDAR_DATA_HANDLER_H_
#define LIVOX_LIDAR_DATA_HANDLER_H_

#include <boost/shared_ptr.hpp>
#include <boost/smart_ptr/scoped_array.hpp>
#include <boost/thread/mutex.hpp>
#include "data_handler.h"
#include "device_manager.h"

namespace livox {

class LidarDataHandlerImpl : public DataHandlerImpl {
 public:
  LidarDataHandlerImpl(DataHandler *handler, apr_pool_t *mem_pool) : DataHandlerImpl(handler), mem_pool_(mem_pool) {}
  ~LidarDataHandlerImpl() { Uninit(); }
  bool Init();
  void Uninit();
  bool AddDevice(const DeviceInfo &info);
  void RemoveDevice(uint8_t handle);
  void OnData(apr_socket_t *sock, void *client_data);

 private:
  typedef struct {
    apr_socket_t *sock;
    boost::shared_ptr<IOThread> thread;
    uint16_t handle;
  } DeviceItem;
  std::list<DeviceItem> devices_;

  boost::array<boost::scoped_array<char>, kMaxConnectedDeviceNum> data_buffers_;
  apr_pool_t *mem_pool_;
  boost::mutex mutex_;
};

}  // namespace livox

#endif  // LIVOX_LIDAR_DATA_HANDLER_H_
