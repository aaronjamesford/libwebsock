#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <string>
#include <vector>
#include <queue>

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace libwebsock
{

	using boost::asio::ip::tcp;
	typedef boost::shared_ptr< tcp::socket > sock_ptr;
	typedef boost::shared_ptr< tcp::acceptor > accept_ptr;
	typedef boost::shared_ptr< boost::mutex > mutex_ptr;
	
	struct User
	{
		// mutex for exclusive access to the resource
		mutex_ptr mutex;
		
		// socket for sneding/recieving... obviously
		sock_ptr sock;
		
		// this queue is for the messages that have to be sent to the User
		std::queue< std::string > q;
		
		// to determing whether the user has handshaken
		bool handshaken;
		
		User( )
		{
			handshaken = false;
			mutex = mutex_ptr( new boost::mutex( ) );
		}
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
		std::vector< User >::iterator _userIt;
		boost::mutex  _userMutex;
	
		void _read( User& u );
		void _accept( );
		void _asyncSend( );
	
		void _handshake( User& u, std::string header );
		
		void _process( User& u, std::string req );
		
		void _send( sock_ptr sock, const std::string& resp );
		void _send( User& u, const std::string& resp );
	};

}

#endif