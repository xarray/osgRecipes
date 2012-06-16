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

#include "OscReceiver.h"

using namespace TUIO;
using namespace osc;

void OscReceiver::ProcessMessage( const ReceivedMessage& msg, const IpEndpointName& remoteEndpoint) {
	for (std::list<TuioClient*>::iterator client=clientList.begin(); client!= clientList.end(); client++)
		(*client)->processOSC(msg);
}
void OscReceiver::ProcessBundle( const ReceivedBundle& b, const IpEndpointName& remoteEndpoint) {
	
	try {
		for( ReceivedBundle::const_iterator i = b.ElementsBegin(); i != b.ElementsEnd(); ++i ){
			if( i->IsBundle() )
				ProcessBundle( ReceivedBundle(*i), remoteEndpoint);
			else
				ProcessMessage( ReceivedMessage(*i), remoteEndpoint);
		}
	} catch (MalformedBundleException& e) {
		std::cerr << "malformed OSC bundle: " << e.what() << std::endl;
	}
	
}

void OscReceiver::ProcessPacket( const char *data, int size, const IpEndpointName& remoteEndpoint ) {
	try {
		ReceivedPacket p( data, size );
		if(p.IsBundle()) ProcessBundle( ReceivedBundle(p), remoteEndpoint);
		else ProcessMessage( ReceivedMessage(p), remoteEndpoint);
	} catch (MalformedBundleException& e) {
		std::cerr << "malformed OSC bundle: " << e.what() << std::endl;
	}
}

bool OscReceiver::isConnected() {
	return connected;
}

void OscReceiver::addTuioClient(TuioClient *client) {
	clientList.push_back(client);
}

