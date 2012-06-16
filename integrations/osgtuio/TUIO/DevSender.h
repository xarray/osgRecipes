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

#ifndef INCLUDED_DEVSENDER_H
#define INCLUDED_DEVSENDER_H

#include "OscSender.h"

#include <cstdio>
#ifndef WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif

#include <list>
#define MAX_DEV_SIZE 65536

namespace TUIO {
	
	/**
	 * The DevSender implements the devive socket transport method for OSC
	 *
	 * @author Martin Kaltenbrunner
	 * @version 1.5
	 */ 
	class LIBDECL DevSender : public OscSender {
				
	public:
		
		/**
		 * This constructor creates a DevSender that sends through the provided /dev/tuio# device number
		 *
		 * @param  port  the TUIO device number, defaults to 0
		 */
		DevSender(int dev_no=0);	
		
		/**
		 * The destructor closes the socket. 
		 */
		~DevSender();
		
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

		
#ifndef WIN32
		int dev_socket;
		std::list<int> dev_client_list;
		char dev_name[16];
#endif

		bool connected;
	private:
		char data_size[4];
		

#ifndef WIN32
		pthread_t server_thread;
#endif
		
	};
}
#endif /* INCLUDED_DevSender_H */
