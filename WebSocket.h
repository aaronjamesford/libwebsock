#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <string>

class WebSocket
{
public:
	WebSocket( int port );
	~WebSocket( );
private:
	int _port;

	std::string _host;
	std::string _key1;
	std::string _key2;
	std::string _l8b;

	void _handshake( const std::string& header );
	std::string _genSecret( );
	int _extractKey( const std::string& token );
	std::string _getBigEndRep( int x );
};

#endif