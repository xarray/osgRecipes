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

#ifndef INCLUDED_DEVRECEIVER_H
#define INCLUDED_DEVRECEIVER_H

#include "OscReceiver.h"
#define MAX_DEV_SIZE 65536

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

namespace TUIO {
	
	/**
	 * The DevReceiver provides the OscReceiver functionality for the TUIO device transport method 
	 *
	 * @author Martin Kaltenbrunner
	 * @version 1.5
	 */ 
	class LIBDECL DevReceiver: public OscReceiver {
				
	public:
		
		/**
		 * This constructor creates a DevReceiver instance listening to the provided TUIO device 
		 *
		 * @param  no  the number of the TUIO device to listen to, defaults to 0
		 */
		DevReceiver (int dev_no=0);

		/**
		 * The destructor is doing nothing in particular. 
		 */
		virtual ~DevReceiver();
		
		/**
		 * The DevReceiver connects and starts receiving TUIO messages from device
		 *
		 * @param  lock  running in the background if set to false (default)
		 */
		void connect(bool lock=false);
		
		/**
		 * The DevReceiver disconnects and stops receiving TUIO messages from device
		 */
		void disconnect();

#ifndef WIN32
		int dev_socket;
		std::list<int> dev_client_list;
		char dev_name[16];
#endif
		
	private:

#ifndef WIN32
		pthread_t server_thread;
#endif	
		
		bool locked;
	};
};
#endif /* INCLUDED_DevReceiver_H */
