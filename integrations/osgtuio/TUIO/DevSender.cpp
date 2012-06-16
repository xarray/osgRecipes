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


#include "DevSender.h"

using namespace TUIO;

#ifndef  WIN32
static void* ClientThreadFunc( void* obj )
#else
static DWORD WINAPI ClientThreadFunc( LPVOID obj )
#endif
{
	DevSender *sender = static_cast<DevSender*>(obj);
	char buf[16];
	
#ifdef WIN32
	SOCKET client = sender->dev_client_list.back();
#else
	int client = sender->dev_client_list.back();
#endif
	
	int connected = 1;
	while (connected) {
		connected = recv(client, buf, sizeof(buf),0);
	}
	
	std::cout << "TUIO/DEV client disconnected from " << sender->dev_name << std::endl;
	sender->dev_client_list.remove(client);
	if (sender->dev_client_list.size()==0) sender->connected=false;
	//std::cout << sender->dev_client_list.size() << " clients left"<< std::endl;	
	
	return 0;
};

#ifndef  WIN32
static void* ServerThreadFunc( void* obj )
#else
static DWORD WINAPI ServerThreadFunc( LPVOID obj )
#endif
{
	DevSender *sender = static_cast<DevSender*>(obj);
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	
	std::cerr << "TUIO/DEV socket created in file " << sender->dev_name << std::endl;
	while (sender->dev_socket) {
#ifdef WIN32
		SOCKET client = -1;
#else
		int client = -1;
#endif
		client = accept(sender->dev_socket, (struct sockaddr*)&client_addr, &len);
		
		if (client>0) {
			std::cout << "TUIO/DEV client connected to socket " << sender->dev_name << std::endl;
			sender->dev_client_list.push_back(client);
			sender->connected=true;
			//std::cout << sender->dev_client_list.size() << " clients connected"<< std::endl;	
			
#ifndef WIN32
			pthread_t client_thread;
			pthread_create(&client_thread , NULL, ClientThreadFunc,obj);
#else
			DWORD ClientThreadId;
			HANDLE client_thread = CreateThread( 0, 0, ClientThreadFunc, obj, 0, &ClientThreadId );
#endif
		}
	}
	
	return 0;
};

DevSender::DevSender(int dev_no)
	:connected (false) 
{
	local = true;
	buffer_size = MAX_DEV_SIZE;
	
	dev_socket = socket( AF_UNIX, SOCK_STREAM, 0 );
	if (dev_socket < 0) {
		std::cerr << "could not create TUIO socket" << std::endl;
		return;
	}
	
	struct sockaddr_un dev_server;
	memset( &dev_server, 0, sizeof (dev_server));
	
	sprintf(dev_name, "tuio%d",dev_no);
	while (access (dev_name, F_OK )==0) {
		dev_no++;
		sprintf(dev_name, "tuio%d",dev_no);
	}
	
	dev_server.sun_family = AF_UNIX;
	strcpy(dev_server.sun_path, dev_name);

	unlink(dev_name);
	int ret = bind(dev_socket,(struct sockaddr*)&dev_server,sizeof(dev_server));
	if (ret < 0) {
		std::cerr << "could not create TUIO/DEV socket " << dev_name << std::endl;
		close(dev_socket);
		return;
	}
	
	ret =  listen(dev_socket, 1);
	if (ret < 0) {
		std::cerr << "could not start listening on TUIO/DEV socket" << std::endl;
		close(dev_socket);
		return;
	}
	
#ifndef WIN32
	pthread_create(&server_thread , NULL, ServerThreadFunc, this);
#else
	DWORD threadId;
	server_thread = CreateThread( 0, 0, ServerThreadFunc, this, 0, &threadId );
#endif

}

DevSender::~DevSender() {
#ifndef WIN32
	for (std::list<int>::iterator client = dev_client_list.begin(); client!=dev_client_list.end(); client++) {
		close((*client));
	}
	close(dev_socket);
	if (remove(dev_name) != 0)
		std::cerr <<  "could not remove TUIO/DEV socket " << dev_name << std::endl;
	server_thread = 0;
#endif		
}


bool DevSender::isConnected() { 
	return connected;
}

bool DevSender::sendOscPacket (osc::OutboundPacketStream *bundle) {
	if (!connected) return false; 
	if ( bundle->Size() > buffer_size ) return false;
	if ( bundle->Size() == 0 ) return false;
	
#ifdef OSC_HOST_LITTLE_ENDIAN             
	data_size[0] = bundle->Size()>>24;
	data_size[1] = (bundle->Size()>>16) & 255;
	data_size[2] = (bundle->Size()>>8) & 255;
	data_size[3] = (bundle->Size()) & 255;
#else
	*((int32_t*)data_size) = bundle->Size();
#endif

#ifndef WIN32	
	for (std::list<int>::iterator client = dev_client_list.begin(); client!=dev_client_list.end(); client++) {
		send((*client),data_size, 4,0);
		send((*client), bundle->Data(), bundle->Size(),0);
	}
#endif
	
	return true;
}
