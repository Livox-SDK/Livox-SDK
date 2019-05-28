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

#include <time.h>
#include <cmath>
#include "lvx_file.h"

#define WRITE_BUFFER_LEN 1024 * 1024
#define MAGIC_CODE       (0xac0ea767)
#define PACK_POINT_NUM   100
#define M_PI             3.14159265358979323846

LvxFileHandle::LvxFileHandle() : cur_frame_index_(0), cur_offset_(0) {
}

bool LvxFileHandle::InitLvxFile() {
  time_t curtime = time(nullptr);
  char filename[30] = { 0 };

  tm* local_time = localtime(&curtime);
  sprintf(filename, "%d-%02d-%02d_%02d-%02d-%02d.lvx", local_time->tm_year + 1900,
                                                       local_time->tm_mon + 1,
                                                       local_time->tm_mday,
                                                       local_time->tm_hour,
                                                       local_time->tm_min,
                                                       local_time->tm_sec);

  lvx_file_.open(filename, std::ios::out | std::ios::binary);

  if (!lvx_file_.is_open()) {
    return false;
  }
  return true;
}

void LvxFileHandle::InitLvxFileHeader() {
  LvxFileHeader lvx_file_header = { 0 };
  std::unique_ptr<char[]> write_buffer(new char[WRITE_BUFFER_LEN]);
  std::string signature = "livox_tech";
  memcpy(lvx_file_header.signature, signature.c_str(), signature.size());
  
  lvx_file_header.version[0] = 1;
  lvx_file_header.version[1] = 0;
  lvx_file_header.version[2] = 0;
  lvx_file_header.version[3] = 0;

  lvx_file_header.magic_code = MAGIC_CODE;

  memcpy(write_buffer.get() + cur_offset_, (void *)&lvx_file_header, sizeof(LvxFileHeader));
  cur_offset_ += sizeof(LvxFileHeader);

  uint8_t device_count = static_cast<uint8_t>(device_info_list_.size());
  memcpy(write_buffer.get() + cur_offset_, (void *)&device_count, sizeof(uint8_t));
  cur_offset_ += sizeof(uint8_t);
  
  for (int i = 0; i < device_count; i++) {
    memcpy(write_buffer.get() + cur_offset_, (void *)&device_info_list_[i], sizeof(LvxDeviceInfo));
    cur_offset_ += sizeof(LvxDeviceInfo);
  }

  lvx_file_.write((char *)write_buffer.get(), cur_offset_);
}

void LvxFileHandle::SaveFrameToLvxFile(std::list<LvxBasePackDetail> &point_packet_list_temp) {
  uint64_t cur_pos = 0;
  FrameHeader frame_header = { 0 };
  std::unique_ptr<char[]> write_buffer(new char[WRITE_BUFFER_LEN]);

  int pack_num = point_packet_list_temp.size();
  frame_header.current_offset = cur_offset_;
  frame_header.next_offset = cur_offset_ + (int64_t)pack_num * sizeof(LvxBasePackDetail) + sizeof(FrameHeader);
  frame_header.package_count = pack_num;
  frame_header.frame_index = cur_frame_index_;

  memcpy(write_buffer.get() + cur_pos, (void*)&frame_header, sizeof(FrameHeader));
  cur_pos += sizeof(FrameHeader);

  auto iter = point_packet_list_temp.begin();
  for (; iter != point_packet_list_temp.end(); iter++) {
    if (cur_pos + sizeof(LvxBasePackDetail) >= WRITE_BUFFER_LEN) {
      lvx_file_.write((char*)write_buffer.get(), cur_pos);
      cur_pos = 0;
      memcpy(write_buffer.get() + cur_pos, (void*)&(*iter), sizeof(LvxBasePackDetail));
      cur_pos += sizeof(LvxBasePackDetail);
    }
    else {
      memcpy(write_buffer.get() + cur_pos, (void*)&(*iter), sizeof(LvxBasePackDetail));
      cur_pos += sizeof(LvxBasePackDetail);
    }
  }
  lvx_file_.write((char*)write_buffer.get(), cur_pos);

  cur_offset_ = frame_header.next_offset;
  cur_frame_index_++;
}

void LvxFileHandle::CloseLvxFile() {
  if (lvx_file_.is_open())
    lvx_file_.close();
}

void LvxFileHandle::BasePointsHandle(LivoxEthPacket *data, LvxBasePackDetail &packet) {
  packet.version = data->version;
  packet.port_id = data->slot;
  packet.lidar_index = data->id;
  packet.rsvd = data->rsvd;
  packet.error_code = data->err_code;
  packet.timestamp_type = data->timestamp_type;
  packet.data_type = data->data_type;
  memcpy(packet.timestamp, data->timestamp, 8 * sizeof(uint8_t));

  if (packet.data_type == 0) {
    LivoxRawPoint tmp[PACK_POINT_NUM];
    memcpy(tmp, (void *)data->data, PACK_POINT_NUM * sizeof(LivoxRawPoint));
    for (int i = 0; i < PACK_POINT_NUM; i++) {
      packet.point[i].x = static_cast<float>(tmp[i].x / 1000.0);
      packet.point[i].y = static_cast<float>(tmp[i].y / 1000.0);
      packet.point[i].z = static_cast<float>(tmp[i].z / 1000.0);
      packet.point[i].reflectivity = tmp[i].reflectivity;
    }
  }
  else if (packet.data_type == 1) {
    LivoxSpherPoint tmp[PACK_POINT_NUM];
    memcpy(tmp, (void *)data->data, PACK_POINT_NUM * sizeof(LivoxSpherPoint));
    for (int i = 0; i < PACK_POINT_NUM; i++) {
      packet.point[i].x = static_cast<float>(tmp[i].depth / 1000.0);
      packet.point[i].y = static_cast<float>(tmp[i].theta);
      packet.point[i].z = static_cast<float>(tmp[i].phi);
      packet.point[i].reflectivity = tmp[i].reflectivity;
      LivoxPoint temp = { packet.point[i].x * std::sin(packet.point[i].y) * std::cos(packet.point[i].z), packet.point[i].x * std::sin(packet.point[i].y) * std::sin(packet.point[i].z), packet.point[i].x * std::cos(packet.point[i].y) };
      packet.point[i] = temp;
    }
  }
}
