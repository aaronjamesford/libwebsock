#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

namespace WebSocket
{

	using boost::asio::ip::tcp;
	typedef boost::shared_ptr< tcp::socket > sock_ptr;
	typedef boost::shared_ptr< tcp::acceptor > accept_ptr;
	
	namespace
	{
		struct User
		{
			sock_ptr sock;
			bool handshaken;
			
			User( )
			{
				handshaken = false;
			}
		};
	}

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
		int _maxBytes;
	
		std::vector< User > _users;
		
		std::string _req;
		std::string _origin;
		std::string _host;
		std::string _protocol;
		std::string _key1;
		std::string _key2;
		std::string _l8b;
		std::string _path;
		
		void _handleAccept( sock_ptr sock, const boost::system::error_code& error );
	
		void _handshake( User& u, const std::string& header );
		std::string _getField( const std::string& header, const std::string& field );
		std::string _genSecret( );
		int _extractKey( const std::string& token );
		std::string _getBigEndRep( int x );
		
		void _process( User& u, const std::string& req );
		
		void _send( sock_ptr sock, const std::string& resp );
	};

}

#endif