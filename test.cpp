#include "WebSocket.h"

#include <iostream>
#include <boost/asio.hpp>

int main( int argc, char** argv )
{
	if( argc != 2 )
	{
		std::cerr << "usage: test <port>" << std::endl;
		return -1;
	}
	
	boost::asio::io_service io_service;
	libwebsock::WebSocket ws( io_service, 2738 );
	ws.start( );
	io_service.run( );
	
	return 0;
}