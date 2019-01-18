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

#include "util.h"
#include <stdio.h>
#include <stdlib.h>
using std::string;
namespace livox {

string PrintAPRStatus(apr_status_t s) {
  char buf[128];
  apr_strerror(s, buf, sizeof(buf));
  return buf;
}
string PrintAPRTime(apr_time_t t) {
  char cbuf[45 + 1];
  apr_size_t sz = 45;
  apr_time_exp_t xt;
  apr_time_exp_gmt(&xt, t);
  int milli = t % 1000;
  apr_strftime(cbuf, &sz, 45, " %T.", &xt);
  string s(cbuf);

  char buffer[33];
  sprintf(buffer, "%d", milli);

  s += buffer;
  return s;
}

}  // namespace livox
