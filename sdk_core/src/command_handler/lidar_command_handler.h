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

#ifndef LIVOX_LIDAR_COMMAND_HANDLER_H_
#define LIVOX_LIDAR_COMMAND_HANDLER_H_
#include <boost/smart_ptr.hpp>
#include "command_handler.h"

namespace livox {

class LidarCommandHandlerImpl : public CommandHandlerImpl {
 public:
  LidarCommandHandlerImpl(CommandHandler *handler, apr_pool_t *pool, IOLoop *loop)
      : CommandHandlerImpl(handler), mem_pool_(pool), loop_(loop) {}

  void Uninit();

  bool AddDevice(const DeviceInfo &info);
  bool RemoveDevice(uint8_t handle);
  livox_status SendCommand(uint8_t handle, const Command &command);

 private:
  typedef struct {
    boost::shared_ptr<CommandChannel> channel;
    DeviceInfo info;
  } DeviceItem;
  apr_pool_t *mem_pool_;
  std::list<DeviceItem> devices_;
  IOLoop *loop_;
};

}  // namespace livox

#endif  // LIVOX_LIDAR_COMMAND_HANDLER_H_
