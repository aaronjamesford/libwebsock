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
		
		// set to reuse address
		tcp::acceptor::reuse_address reuse( true );
		_server_sock->set_option( reuse );
		
		_server_sock->bind( ep );
		_server_sock->listen( );
		
		sock_ptr s( new tcp::socket( _io_service ) );
		_server_sock->async_accept( *s, boost::bind( &WebSocket::_async_accept, this, s, boost::asio::placeholders::error ) );
	}
	
	void WebSocket::_async_read( usr_ptr u, char* request, const boost::system::error_code& error, size_t bytes_transferred )
	{
		if( !error )
		{
			if( u->handshaken )
			{ // bitch has handshaken. Lets process him :)
				if( request[ 0 ] == 0x00 && (unsigned char)request[ bytes_transferred - 1] == (unsigned char)0xFF )
				{
					// trim the string
					std::string req( request + 1, bytes_transferred - 2 );
					std::string response;
					
					// process and take appropiate action
					switch( process( req, response ) )
					{
					case NO_RESPOND:
						break;
					case RESPOND:
						_async_send( u, response );
						break;
					case BROADCAST:
						_async_broadcast( response );
						break;
					default:
						break;
					}
				}
				else if( (unsigned char)request[ 0 ] == (unsigned char)0xFF && request[ 1 ] == 0x00 )
				{ // client has usked us to close connection
					u->sock->close( );
					delete[ ] request;
					
					return;
				}
			}
			else
			{// lets shake hands, mr client
				_handshake( *u, std::string( request ) );
			}
		
			// request = new char[ _maxBytes ];
			u->sock->async_receive( boost::asio::buffer( request, _maxBytes ), boost::bind( &WebSocket::_async_read, this, u, request, _1, _2 ) );
		}
		else if( error == boost::asio::error::eof )
		{
			delete[ ] request;
			
			u->sock->close( );
		}
		else
		{
			delete[ ] request;
			std::cout << "Error: " << error.message( ) << std::endl;
		}
	}
	
	void WebSocket::_async_accept( sock_ptr sock, const boost::system::error_code& error )
	{
		if( !error )
		{
			// create a new user and add it to the list
			usr_ptr u( new User( ) );
			u->sock = sock;
			
			boost::mutex::scoped_lock ulock( _userMutex );
			_users.push_back( *u );
			ulock.unlock( );
			
			char* buf = new char[ _maxBytes ];
			sock->async_receive( boost::asio::buffer( buf, _maxBytes ), boost::bind( &WebSocket::_async_read, this, u, buf, _1, _2 ) );
			
			sock_ptr s( new tcp::socket( _io_service ) );
			_server_sock->async_accept( *s, boost::bind( &WebSocket::_async_accept, this, s, boost::asio::placeholders::error ) );
		}
	}

	void WebSocket::_handshake( User& u, std::string header )
	{
		Handshake h;
		
		if( h.processHandshake( header ) )
		{
			_async_send( usr_ptr( new User( u ) ), h.getHandshake( ) );
			
			u.handshaken = true;
		}
	}
	
	void WebSocket::_async_broadcast( const std::string& message )
	{
		std::vector< User >::iterator user = _users.begin( );
		while( user < _users.end( ) )
		{
			_async_send( usr_ptr( new User( *user ) ), message );
			
			++user;
		}
	}
	
	void WebSocket::_async_send( usr_ptr u, std::string resp )
	{
		if( u->handshaken )
		{
			_pad( resp );
		}
		
		if( u->sock->is_open( ) )
		{
			str_ptr r( new std::string( resp ) );
			
			u->sock->async_send( boost::asio::buffer( *r, r->length( ) ), boost::bind( &WebSocket::_async_sent, this, r, _1, _2 ) );
		}
	}
	
	void WebSocket::_async_sent( str_ptr sent, const boost::system::error_code& error, size_t bytes_transferred )
	{
		/*
		 -- Empty Implementation
		*/
	}
	
	void WebSocket::_pad( std::string& message )
	{
		message = std::string( char( 0x00 ) + message + char( 0xFF ) );
	}
	
	void WebSocket::broadcast( std::string message )
	{
		_async_broadcast( message );
	}
	
	ResponseType WebSocket::process( std::string& request, std::string& response )
	{
		response = request;
		
		return RESPOND;
	}
}