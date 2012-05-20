/*  LibMary - C++ library for high-performance network servers
    Copyright (C) 2011 Dmitry Shatrov

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef __LIBMARY__ASYNC_OUTPUT_STREAM__H__
#define __LIBMARY__ASYNC_OUTPUT_STREAM__H__


#include <libmary/types.h>

#ifndef LIBMARY_PLATFORM_WIN32
#include <sys/uio.h>
#endif

#include <libmary/exception.h>


namespace M {

class AsyncOutputStream
{
public:
    virtual mt_throws AsyncIoResult write (ConstMemory const &mem,
					   Size              *ret_nwritten) = 0;

    virtual mt_throws AsyncIoResult writev (struct iovec *iovs,
					    Count         num_iovs,
					    Size         *ret_nwritten);

    virtual ~AsyncOutputStream () {}
};

}


#endif /* __LIBMARY__ASYNC_OUTPUT_STREAM__H__ */

