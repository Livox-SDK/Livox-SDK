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
#include <ifaddrs.h>
namespace livox {
namespace util {
apr_socket_t *CreateBindSocket(uint16_t port, apr_pool_t *mem_pool, bool nonblock) {
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

bool FindLocalIP(const struct sockaddr_in &client_addr, uint32_t &local_ip) {
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

}  // namespace util
}  // namespace livox
