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


HOST=arm-linux


if [ ! -n "$1" ]; then
	echo "please specify C Compiler!"
	exit 1
else
	CC=$1
	echo $CC
fi

apr_dir=$(cd `dirname $0`; pwd)
pushd ${apr_dir}/apr-1.6.5
echo ${apr_dir}

make clean


if [ ! -f "include/apr.h"]; then
	rm Makefile
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

cat Makefile.in|sed 's?$(LINK_PROG) $(OBJECTS_gen_test_char) $(ALL_LIBS)?gcc -Wall -O2 tools/gen_test_char.c -s -o tools/gen_test_char?' >> temp

mv Makefile.in Makefile.in.bak
mv temp Makefile.in

./buildconf
./configure CC=${CC} --host=${HOST} --prefix=${apr_dir} ac_cv_file__dev_zero="yes" ac_cv_func_setpgrp_void="yes" apr_cv_tcp_nodelay_with_cork="yes" ac_cv_sizeof_struct_iovec="8" apr_cv_mutex_recursive="yes" apr_cv_mutex_robust_shared="no" apr_cv_process_shared_works="yes" 


make
make install

rm -f Makefile.in
mv Makefile.in.bak Makefile.in

popd
