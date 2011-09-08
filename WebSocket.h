#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <string>
#include <vector>
#include <map>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>

#include "Logger.h"

namespace libwebsock
{
	class Frame;

	using boost::asio::ip::tcp;
	typedef boost::shared_ptr< tcp::socket > sock_ptr;
	typedef boost::shared_ptr< tcp::acceptor > accept_ptr;
	typedef boost::shared_ptr< std::string > str_ptr;
	typedef boost::shared_ptr< class User > usr_ptr;
	typedef boost::shared_ptr< Frame > frame_ptr;
	
	struct User
	{
		// socket for receiving / sending
		sock_ptr sock;
		// to provide status on the handshake
		bool handshaken;
		int uid;
		frame_ptr ftype;
		
		User( )
		{
			handshaken = false;
		}
	};
	
	enum ResponseType
	{
		NO_RESPOND, RESPOND, BROADCAST
	};

	class WebSocket
	{
	public:
		WebSocket( boost::asio::io_service& io_service, int port );
		~WebSocket( ) { };
			
		void start( );
	protected:
		virtual ResponseType process( std::string& request, std::string& response );
		void broadcast( std::string message );
		
		inline void log( const std::string& message ) { _logger->log( message ); }
	private:
		boost::asio::io_service& _io_service;
		accept_ptr _server_sock;
		
		int _port;
		int _maxBytes;
	
		std::map< int, usr_ptr > _users;
		int _current_id;
		// std::vector< User > _users;
	
		void _async_read( usr_ptr u, char* request, const boost::system::error_code& error, size_t bytes_transferred );
	
		void _async_accept( sock_ptr sock, const boost::system::error_code& error );
	
		void _handshake( usr_ptr u, std::string header );
		
		void _async_broadcast( const std::string& message );
		void _async_send( usr_ptr u, std::string resp );
		void _async_sent( str_ptr sent, const boost::system::error_code& error, size_t bytes_transferred );
	
		void _pad( std::string& message );
	
		void _disconnect( usr_ptr u );
		
		Logger* _logger;
	};

}

#endif
