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

#include "network_util.h"
#ifdef WIN32
#include <boost/scoped_array.hpp>
#include <Winsock2.h>
#include <iphlpapi.h>
#include <string>
#pragma comment(lib,"iphlpapi.lib")
#pragma comment(lib,"ws2_32.lib")
#else
#include <ifaddrs.h>
#endif

namespace livox {
namespace util {
apr_socket_t *CreateBindSocket(uint16_t port, apr_pool_t *mem_pool, bool reuse_port, bool nonblock) {
  apr_sockaddr_t *sa = NULL;
  apr_socket_t *s = NULL;
  apr_status_t rv = APR_SUCCESS;



  rv = apr_sockaddr_info_get(&sa, NULL, APR_INET, port, 0, mem_pool);
  if (rv != APR_SUCCESS) {
    return NULL;
  }

  rv = apr_socket_create(&s, sa->family, SOCK_DGRAM, APR_PROTO_UDP, mem_pool);
  if (rv != APR_SUCCESS) {
    return NULL;
  }

  if (reuse_port) {
    rv = apr_socket_opt_set(s, APR_SO_REUSEADDR, 1);
    if (rv != APR_SUCCESS) {
      apr_socket_close(s);
      s = NULL;
      return NULL;
    }
  }

  rv = apr_socket_bind(s, sa);
  if (rv != APR_SUCCESS) {
    apr_socket_close(s);
    s = NULL;
    return NULL;
  }

  if (nonblock) {
    apr_socket_opt_set(s, APR_SO_NONBLOCK, 1);
    apr_socket_timeout_set(s, 0);
  }
  return s;
}


#ifdef WIN32

bool GetAdapterState(const IP_ADAPTER_INFO *pAdapter)
{
  if(pAdapter == NULL) {
    return false;
  }
  MIB_IFROW info;
  memset(&info ,0 ,sizeof(MIB_IFROW));
  info.dwIndex = pAdapter->Index;
  if (GetIfEntry(&info) != NOERROR) {
    return false;
  }
  if (info.dwOperStatus == IF_OPER_STATUS_NON_OPERATIONAL
    || info.dwOperStatus == IF_OPER_STATUS_UNREACHABLE
    || info.dwOperStatus == IF_OPER_STATUS_DISCONNECTED
    || info.dwOperStatus == IF_OPER_STATUS_CONNECTING)
    return false;
  return true;
}

bool FindLocalIp(const struct sockaddr_in &client_addr, uint32_t &local_ip) {
  bool found = false;
  ULONG ulOutbufLen = sizeof(IP_ADAPTER_INFO);
  boost::scoped_array<uint8_t> pAdapterInfo(new uint8_t[ulOutbufLen]);
  DWORD dlRetVal = GetAdaptersInfo(reinterpret_cast<IP_ADAPTER_INFO *>(pAdapterInfo.get()), &ulOutbufLen);
  if (dlRetVal == ERROR_BUFFER_OVERFLOW) {
    pAdapterInfo.reset(new uint8_t[ulOutbufLen]);
    dlRetVal = GetAdaptersInfo(reinterpret_cast<IP_ADAPTER_INFO *>(pAdapterInfo.get()), &ulOutbufLen);
  }

  IP_ADAPTER_INFO *pAdapter = reinterpret_cast<IP_ADAPTER_INFO *>(pAdapterInfo.get());
  if (NO_ERROR == dlRetVal && pAdapter != NULL) {
    while (pAdapter != NULL) {
      if (GetAdapterState(pAdapter)) {
        std::string str_ip = pAdapter->IpAddressList.IpAddress.String;
        std::string str_mask = pAdapter->IpAddressList.IpMask.String;
        ULONG host_ip = inet_addr(const_cast<char *>(str_ip.c_str()));
        ULONG host_mask = inet_addr(const_cast<char *>(str_mask.c_str()));

        if ((host_ip & host_mask) ==
          (client_addr.sin_addr.S_un.S_addr & host_mask)) {
          local_ip = host_ip;
          found = true;
          break;
        }
      }
      pAdapter = pAdapter->Next;
    }
  }
  return found;
}
#else
bool FindLocalIp(const struct sockaddr_in &client_addr, uint32_t &local_ip) {
  struct ifaddrs *if_addrs = NULL, *addrs = NULL;
  if (getifaddrs(&if_addrs) == -1) {
    return false;
  }
  addrs = if_addrs;
  bool found = false;
  while (if_addrs != NULL) {
    if ((if_addrs->ifa_addr != NULL) && (if_addrs->ifa_addr->sa_family == AF_INET))  // check it is IP4
    {
      // is a connected IP4 Address
      struct sockaddr_in *ifu_localaddr = (struct sockaddr_in *)if_addrs->ifa_addr;
      struct sockaddr_in *ifu_netmask = (struct sockaddr_in *)if_addrs->ifa_netmask;
      if (ifu_localaddr->sin_addr.s_addr != htonl(INADDR_ANY)) {
        if ((ifu_localaddr->sin_addr.s_addr & ifu_netmask->sin_addr.s_addr) ==
            (client_addr.sin_addr.s_addr & ifu_netmask->sin_addr.s_addr)) {
          local_ip = ifu_localaddr->sin_addr.s_addr;
          found = true;
          break;
        }
      }
    }
    if_addrs = if_addrs->ifa_next;
  }

  if (addrs) {
    freeifaddrs(addrs);
  }
  return found;
}
#endif
}  // namespace util
}  // namespace livox
