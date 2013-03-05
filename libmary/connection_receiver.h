/*  LibMary - C++ library for high-performance network servers
    Copyright (C) 2011-2013 Dmitry Shatrov

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


#ifndef LIBMARY__CONNECTION_RECEIVER__H__
#define LIBMARY__CONNECTION_RECEIVER__H__


#include <libmary/receiver.h>
#include <libmary/async_input_stream.h>
#include <libmary/code_referenced.h>


namespace M {


// TODO Rename to AsyncReceiver. It now depends on AsyncInputStream, not on Connection.

// Synchronized externally by the associated AsyncInputStream object.
class ConnectionReceiver : public Receiver,
			   public DependentCodeReferenced
{
private:
    DeferredProcessor::Task unblock_input_task;
    DeferredProcessor::Registration deferred_reg;

    mt_const AsyncInputStream *conn;

    mt_const Byte *recv_buf;
    mt_const Size const recv_buf_len;

#ifdef LIBMARY_WIN32_IOCP
    mt_sync_domain (conn_input_frontend) Overlapped overlapped;
    mt_sync_domain (conn_input_frontend) bool overlapped_in_progress;
#endif

    mt_sync_domain (conn_input_frontend) Size recv_buf_pos;
    mt_sync_domain (conn_input_frontend) Size recv_accepted_pos;

    mt_sync_domain (conn_input_frontend) bool error_reported;

    mt_sync_domain (conn_input_frontend) void doProcessInput ();

  mt_iface (AsyncInputStream::InputFrontend)
    static AsyncInputStream::InputFrontend const conn_input_frontend;

#ifdef LIBMARY_WIN32_IOCP
    static void inputComplete (Overlapped *overlapped,
                               Size        bytes_transferred,
                               void       *_self);
#else
    static void processInput (void *_self);

    static void processError (Exception *exc_,
			      void      *_self);
#endif
  mt_iface_end

    static bool unblockInputTask (void *_self);

public:
  mt_iface (Receiver)
    void unblockInput ();
  mt_iface_end

    mt_const void init (AsyncInputStream  * const mt_nonnull conn,
                        DeferredProcessor * const mt_nonnull deferred_processor)
    {
        deferred_reg.setDeferredProcessor (deferred_processor);

	this->conn = conn;
	conn->setInputFrontend (
                CbDesc<AsyncInputStream::InputFrontend> (&conn_input_frontend, this, getCoderefContainer()));
    }

    ConnectionReceiver (Object * const coderef_container);

    ~ConnectionReceiver ();
};

}


#endif /* LIBMARY__CONNECTION_RECEIVER__H__ */

