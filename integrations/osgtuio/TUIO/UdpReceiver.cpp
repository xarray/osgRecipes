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

#include "UdpReceiver.h"

using namespace TUIO;
using namespace osc;

#ifndef WIN32
static void* ClientThreadFunc( void* obj )
#else
static DWORD WINAPI ClientThreadFunc( LPVOID obj )
#endif
{
	static_cast<UdpReceiver*>(obj)->socket->Run();
	return 0;
};

UdpReceiver::UdpReceiver(int port)
: socket      (NULL)
, thread      (NULL)
, locked      (false)
{
	try {
		socket = new UdpListeningReceiveSocket(IpEndpointName( IpEndpointName::ANY_ADDRESS, port ), this );
	} catch (std::exception &e) { 
		std::cerr << "could not bind to UDP port " << port << std::endl;
		socket = NULL;
	}
	
	if (socket!=NULL) {
		if (!socket->IsBound()) {
			delete socket;
			socket = NULL;
		} else std::cout << "listening to TUIO/UDP messages on port " << port << std::endl;
	}
}

UdpReceiver::~UdpReceiver() {	
	delete socket;
}

void UdpReceiver::connect(bool lk) {
	
	if (connected) return;
	if (socket==NULL) return;
	locked = lk;
	
	if (!locked) {
#ifndef WIN32
		pthread_create(&thread , NULL, ClientThreadFunc, this);
#else
		DWORD threadId;
		thread = CreateThread( 0, 0, ClientThreadFunc, this, 0, &threadId );
#endif
	} else socket->Run();
	
	connected = true;
}

void UdpReceiver::disconnect() {
	
	if (!connected) return;
	if (socket==NULL) {
		connected = false;
		locked = false;
		return;
	}
	socket->Break();
	
	if (!locked) {
#ifdef WIN32
		if( thread ) CloseHandle( thread );
#endif
		thread = 0;
	} else locked = false;

	connected = false;
}


