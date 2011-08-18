#include <string>
#include <iostream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include "md5/md5.h"
#include "WebSocket.h"
#include "Handshake.h"

namespace libwebsock
{

	WebSocket::WebSocket( boost::asio::io_service& io_service, int port )
		: _io_service( io_service ), _port( port ), _maxBytes( 2048 )
	{
		
	}
	
	void WebSocket::start( )
	{
		_server_sock = accept_ptr( new tcp::acceptor( _io_service ) );
		tcp::endpoint ep( tcp::v4( ), _port );
		
		// open the socket, set it to non blocking and bind the socket
		_server_sock->open( ep.protocol( ) );
		tcp::acceptor::reuse_address reuse( true );
		_server_sock->set_option( reuse );
		// server_sock.set_option( boost::asio::socket_base::non_blocking_io( true ) );
		_server_sock->bind( ep );
		_server_sock->listen( );
		
		while( _server_sock->is_open( ) )
		{
			_accept( );
		}
	}
	
	void WebSocket::_read( User& u )
	{
		std::string disconnect( 2, 0x00 );
		disconnect[ 1 ] = 0xFF;
		
		bool socketOpen = true;
		boost::mutex::scoped_lock userLock( *(u.mut), boost::defer_lock );
		// userLock.unlock( );
		
		while( socketOpen )
		{
			userLock.lock( );
			
			socketOpen = u.sock->is_open( );
			
			if( socketOpen )
			{
				// try reading, process the read shit blah blah blah
				boost::system::error_code error;
				// std::string req;
				char* req = new char[ _maxBytes ];
				size_t bytes = u.sock->receive( boost::asio::buffer( req, _maxBytes ), 0, error );
				// size_t bytes = boost::asio::read( *(it->sock), boost::asio::buffer( req, _maxBytes ) );
				if( error == boost::asio::error::eof )
				{
					std::cout << "eof" << std::endl;
					u.sock->close( );
				}
				else if( !error && bytes > 0 )
				{
					if( u.handshaken )
					{ // process the actual request
						if( req[ 0 ] == 0x00 && (unsigned char)req[ bytes - 1] == (unsigned char)0xFF )
						{ // a real request
							std::string request( req + 1, bytes - 2 );
							std::string response;
							switch( _process( request, response ) )
							{
							case NO_RESPOND:
								break;
							case RESPOND:
								_send( u.sock, char( 0x00 ) + response + char( 0xFF ) );
								break;
							case BROADCAST:
								// add call to _broadcast
								break;
							default:
								break;
							}
							/* if( _process( request, response ) )
							{
								_send( u.sock, char( 0x00 ) + response + char( 0xFF ) );
							} */
						}
						else if( (unsigned char)req[ 0 ] == (unsigned char)0xFF && req[ 1 ] == 0x00 )
						{ // Request to close the connection
							u.sock->close( );
						}
					}
					else
					{ // needs to handshake
						_handshake( u, req );
					}
				}
			}
			
			userLock.unlock( );
		}
	}
	
	void WebSocket::_accept( )
	{
		// accept the socket, if it actually accepted, add the socket to the list
		sock_ptr sock( new tcp::socket( _io_service ) );
		boost::system::error_code error;
		_server_sock->accept( *sock, error );
		if( !error )
		{
			// create a new user and add it to the list
			User u;
			u.sock = sock;
			
			boost::mutex::scoped_lock ulock( _userMutex );
			_users.push_back( u );
			ulock.unlock( );
			
			// start new thread
			boost::thread( boost::bind( &WebSocket::_read, this, u ) );
		}
	}

	void WebSocket::_handshake( User& u, std::string header )
	{
		Handshake h;
		
		if( h.processHandshake( header ) )
		{
			_send( u.sock, h.getHandshake( ) );
			
			u.handshaken = true;
		}
	}
	
	ResponseType WebSocket::_process( std::string& request, std::string& response )
	{
		response = request;
		
		return RESPOND;
	}
	
	void WebSocket::_send( sock_ptr sock, const std::string& resp )
	{
		// resp.insert( resp.begin( ), '\0' );
		sock->send( boost::asio::buffer( resp, resp.length( ) ) );
	}
}