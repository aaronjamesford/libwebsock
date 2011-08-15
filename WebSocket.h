#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <string>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

namespace WebSocket
{

	using boost::asio::ip::tcp;
	typedef boost::shared_ptr< tcp::socket* > sock_ptr;
	typedef boost::shared_ptr< tcp::acceptor* > accept_ptr;

	class WebSocket
	{
	public:
		WebSocket( boost::asio::io_service& io_service );
		WebSocket( int port );
		~WebSocket( );
			
		void start( );
	private:
		boost::asio::io_service& _io_service;
		
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

}

#endif