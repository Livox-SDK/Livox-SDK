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

#ifndef LIVOX_LOGGING_H_
#define LIVOX_LOGGING_H_
#include <iomanip>
#include <iostream>

#define __FILENAME__ (strrchr(__FILE__, '/') ? (strrchr(__FILE__, '/') + 1) : __FILE__)

#define LOG_TRACE std::cout << "TRACE " << __FILENAME__ << ":" << __LINE__ << " " << __FUNCTION__ << " "
#define LOG_DEBUG std::cout << "DEBUG " << __FILENAME__ << ":" << __LINE__ << " " << __FUNCTION__ << " "
#define LOG_INFO std::cout << "INFO " << __FILENAME__ << ":" << __LINE__ << " " << __FUNCTION__ << " "
#define LOG_WARN std::cout << "WARN " << __FILENAME__ << ":" << __LINE__ << " " << __FUNCTION__ << " "
#define LOG_ERROR std::cout << "ERROR " << __FILENAME__ << ":" << __LINE__ << " " << __FUNCTION__ << " "
#define LOG_FATAL std::cout << "FATAL " << __FILENAME__ << ":" << __LINE__ << " " << __FUNCTION__ << " "

#endif  // LIVOX_LOGGING_H_
