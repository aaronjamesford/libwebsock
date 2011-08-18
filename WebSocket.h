#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace libwebsock
{

	using boost::asio::ip::tcp;
	typedef boost::shared_ptr< tcp::socket > sock_ptr;
	typedef boost::shared_ptr< tcp::acceptor > accept_ptr;
	typedef boost::shared_ptr< boost::mutex > mutex_ptr;
	typedef boost::shared_ptr< std::string > str_ptr;
	
	struct User
	{
		// socket for receiving / sending
		sock_ptr sock;
		
		// to provide status on the handshake
		bool handshaken;
		
		// mutex for exclusive access to the resource
		mutex_ptr mut;
		
		User( )
		{
			handshaken = false;
			mut = mutex_ptr( new boost::mutex( ) );
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
		boost::asio::io_service& _io_service;
		accept_ptr _server_sock;
		
		int _port;
		int _maxBytes;
	
		std::vector< User > _users;
		boost::mutex  _userMutex;
	
		void _read( User& u );
		void _async_read( User& u, char* request, const boost::system::error_code& error, size_t bytes_transferred );
	
		void _accept( );
		void _async_accept( sock_ptr sock, const boost::system::error_code& error );
	
		void _handshake( User& u, std::string header );
		
		virtual ResponseType _process( std::string& request, std::string& response );
		
		void _broadcast( const std::string& message );
		void _send( User& u, const std::string& resp );
		void _async_send( User& u, std::string resp );
		void _async_sent( str_ptr sent, const boost::system::error_code& error, size_t bytes_transferred );
	};

}

#endif