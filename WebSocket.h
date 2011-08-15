#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <string>
// #include <boost/asio.hpp>

// using boost::asio::ip::tcp;

class WebSocket
{
public:
	WebSocket();
	WebSocket( int port );
	~WebSocket( );
		
	void start( );
private:
	int _port;
	
	std::string _req;
	std::string _origin;
	std::string _host;
	std::string _protocol;
	std::string _key1;
	std::string _key2;
	std::string _l8b;
	std::string _path;
	
	void _handshake( const std::string& header );
	std::string _getField( const std::string& header, const std::string& field );
	std::string _genSecret( );
	int _extractKey( const std::string& token );
	std::string _getBigEndRep( int x );
};

#endif