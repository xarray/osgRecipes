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

#ifndef INCLUDED_UDPSENDER_H
#define INCLUDED_UDPSENDER_H

#include "OscSender.h"
#include "ip/UdpSocket.h"

#define IP_MTU_SIZE 1500
#define MAX_UDP_SIZE 4096
#define MIN_UDP_SIZE 576

namespace TUIO {
	
	/**
	 * The UdpSender implements the UDP transport method for OSC
	 *
	 * @author Martin Kaltenbrunner
	 * @version 1.5
	 */ 
	class LIBDECL UdpSender : public OscSender {
				
	public:

		/**
		 * The default constructor creates a UdpSender that sends to the default UDP port 3333 on localhost
		 * using the maximum packet size of 65536 bytes for single packets on the loopback device
		 */
		UdpSender();
		
		/**
		 * This constructor creates a UdpSender that sends to the provided port on the the given host
		 * using the default MTU size of 1500 bytes to deliver unfragmented UDP packets on a LAN
		 *
		 * @param  host  the receiving host name
		 * @param  port  the outgoing UDP port number
		 */
		
		UdpSender(const char *host, int port);		
		/**
		 * This constructor creates a UdpSender that sends to the provided port on the the given host
		 * the UDP packet size can be set to a value between 576 and 65536 bytes
		 *
		 * @param  host  the receiving host name
		 * @param  port  the outgoing UDP port number
		 * @param  size  the maximum UDP packet size
		 */
		UdpSender(const char *host, int port, int size);

		/**
		 * The destructor closes the socket. 
		 */
		~UdpSender();
		
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
		
	private:
		UdpTransmitSocket *socket;
	};
}
#endif /* INCLUDED_UDPSENDER_H */
