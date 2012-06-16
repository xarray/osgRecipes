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

#include "DevReceiver.h"

using namespace TUIO;
using namespace osc;

// workaround for connect method name conflict
int dev_connect(int socket, const struct sockaddr *address, socklen_t address_len) {
		return connect(socket, address, address_len);
}

#ifndef  WIN32
static void* ClientThreadFunc( void* obj )
#else
static DWORD WINAPI ClientThreadFunc( LPVOID obj )
#endif
{
	DevReceiver *sender = static_cast<DevReceiver*>(obj);
	char data_buffer[MAX_DEV_SIZE+4];
	char data_size[4];
	
#ifdef WIN32
	SOCKET client = sender->dev_client_list.back();
#else
	int client = sender->dev_client_list.back();
#endif
	
	int bytes = 1;
	while (bytes) {
		bytes = recv(client, data_buffer, sizeof(data_buffer),0);

		if (bytes>=4) {
			memcpy(&data_size[0],&data_buffer[0], 4);
#ifdef OSC_HOST_LITTLE_ENDIAN             
			int32_t temp = *((int32_t*)data_size);
			data_size[0] =  temp>>24;
			data_size[1] = (temp>>16) & 255;
			data_size[2] = (temp>>8) & 255;
			data_size[3] = (temp) & 255;
#endif
			int32_t size = *((int32_t*)data_size);
			if (size+4==bytes) {
				sender->ProcessPacket(&data_buffer[4],(int)size,IpEndpointName());
			}
		}
	}
	
	sender->dev_client_list.remove(client);
	std::cout << "closed TUIO/DEV socket " << sender->dev_name << std::endl;

	//if (sender->dev_client_list.size()==0) sender->connected=false;
	//std::cout << sender->dev_client_list.size() << " clients left"<< std::endl;	
	
	return 0;
};

DevReceiver::DevReceiver(int dev_no)
: dev_socket	(-1)
, server_thread	(NULL)
, locked		(false)
{
	
	dev_socket = socket( AF_UNIX, SOCK_STREAM, 0 );
	if (dev_socket < 0) {
		std::cerr << "could not create TUIO/DEV socket" << std::endl;
		return;
	}
	
	struct sockaddr_un dev_server;
	memset( &dev_server, 0, sizeof (dev_server));
	
	sprintf(dev_name, "tuio%d",dev_no);
	dev_server.sun_family = AF_UNIX;
	strcpy(dev_server.sun_path, dev_name);
	
	int ret = dev_connect(dev_socket,(struct sockaddr*)&dev_server,sizeof(dev_server));
	if (ret<0) {
#ifndef WIN32
		std::cerr << "could not connect to TUIO/DEV socket " << dev_name << std::endl;
		close(dev_socket);
		dev_socket=-1;
#endif	
		return;
	} else {
		dev_client_list.push_back(dev_socket);
		std::cout << "reading messages from TUIO/DEV socket " << dev_name << std::endl;

	}
	
}

DevReceiver::~DevReceiver() {	
}

void DevReceiver::connect(bool lk) {
	
	if (connected) return;
	if (dev_socket<0) return;
	locked = lk;
	
	if (!locked) {
#ifndef WIN32
		pthread_create(&server_thread , NULL, ClientThreadFunc, this);
#endif
	} else ClientThreadFunc(this);		
	connected = true;
}

void DevReceiver::disconnect() {
	
	if (!connected) return;
	if (dev_socket<0) return;

#ifndef WIN32
	for (std::list<int>::iterator client =dev_client_list.begin(); client!=dev_client_list.end(); client++)
		close((*client));
	close(dev_socket);
	dev_socket=-1;
	server_thread = 0;
#endif	
	
	dev_client_list.clear();
	if (!locked) {
		server_thread = 0;
	} else locked = false;

	connected = false;
}


