/*
 TUIO C++ Library - part of the reacTIVision project
 http://reactivision.sourceforge.net/
 
 Copyright (c) 2005-2009 Martin Kaltenbrunner <martin@tuio.org>
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef INCLUDED_TCPSENDER_H
#define INCLUDED_TCPSENDER_H

#include "OscSender.h"

#ifdef WIN32
#include <winsock.h>
#include <io.h>
typedef int socklen_t;
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <list>
#define MAX_TCP_SIZE 65536

namespace TUIO {
	
	/**
	 * The TcpSender implements the TCP transport method for OSC
	 *
	 * @author Martin Kaltenbrunner
	 * @version 1.5
	 */ 
	class LIBDECL TcpSender : public OscSender {
				
	public:

		/**
		 * The default constructor creates a TcpSender that sends to the default TUIO port 3333 on localhost
		 */
		TcpSender();
		
		/**
		 * This constructor creates a TcpSender that sends to the provided port on the the given host
		 *
		 * @param  host  the receiving host name
		 * @param  port  the outgoing TUIO TCP port number
		 */
		TcpSender(const char *host, int port);		

		/**
		 * This constructor creates a TcpSender that listens to the provided port
		 *
		 * @param  port	the incoming TUIO TCP port number
		 */
		TcpSender(int port);	
		
		/**
		 * The destructor closes the socket. 
		 */
		~TcpSender();
		
		/**
		 * This method delivers the provided OSC data
		 *
		 * @param *bundle  the OSC stream to deliver
		 * @return true if the data was delivered successfully
		 */
		
		bool sendOscPacket (osc::OutboundPacketStream *bundle);

		/**
		 * This method returns the connection state
		 *
		 * @return true if the connection is alive
		 */
		bool isConnected ();

		int port_no;
		
#ifdef WIN32
		SOCKET tcp_socket;
		std::list<SOCKET> tcp_client_list;
#else
		int tcp_socket;
		std::list<int> tcp_client_list;
#endif
		bool connected;
	private:
		char data_size[4];
		char data_buffer[MAX_TCP_SIZE+4];
		

#ifdef WIN32
		HANDLE server_thread;
		DWORD ServerThreadId;
#else
		pthread_t server_thread;
#endif
		
	};
}
#endif /* INCLUDED_TCPSENDER_H */
