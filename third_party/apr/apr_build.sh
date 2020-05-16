#!/bin/bash

################################################################################
# The MIT License (MIT)
#
# Copyright (c) 2019 Livox. All rights reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#################################################################################


apr_dir=$(cd `dirname $0`; pwd)
pushd ${apr_dir}/apr-1.6.5
echo ${apr_dir}

./buildconf
./configure


make clean

if [ ! -f "include/apr.h"]; then
	rm apr-1-config
	rm apr.pc
	rm build/apr_rules.mk
	rm build/pkg/pkginfo
	rm config.log
	rm config.nice
	rm config.status
	rm include/apr.h
	rm include/arch/unix/apr_private.h
	rm libtool
	rm test/Makefile
	rm test/internal/Makefile
fi

if [ ! -d "tools/.libs"]; then
	rm -r tools/.libs
fi

make
make install

popd

if [ ! -d "/usr/local/lib/pkgconfig" ]; then
	mkdir /usr/local/lib/pkgconfig
fi

cp /usr/local/apr/lib/pkgconfig/apr-1.pc /usr/local/lib/pkgconfig/
