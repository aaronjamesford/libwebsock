#include <string>
#include <map>
#include <iostream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "md5/md5.h"
#include "WebSocket.h"
#include "Handshake.h"

namespace libwebsock
{

	WebSocket::WebSocket( boost::asio::io_service& io_service, int port )
		: _io_service( io_service ), _port( port ), _maxBytes( 2048 )
	{
		_current_id = 0;
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
					// u->sock->close( );
					
					std::cout << "Request from client " << u->uid << " to close the connection." << std::endl;
					
					_disconnect( u );
					delete[ ] request;
					
					return;
				}
			}
			else
			{// lets shake hands, mr client
				std::cout << "Handshaking with client " << u->uid << std::endl;
				_handshake( u, std::string( request, bytes_transferred ) );
			}
		
			// request = new char[ _maxBytes ];
			u->sock->async_receive( boost::asio::buffer( request, _maxBytes ), boost::bind( &WebSocket::_async_read, this, u, request, _1, _2 ) );
		}
		else if( error == boost::asio::error::eof )
		{
			delete[ ] request;
			
			std::cout << "Client " << u->uid << " closed the connection." << std::endl;
			// u->sock->close( );
			_disconnect( u );
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
			u->uid = _current_id++;
			u->handshaken = false;
			
			std::cout << "New connection, giving id of " << u->uid << std::endl;
			
			// _users.push_back( *u );
			_users[ u->uid ] = *u;
			
			char* buf = new char[ _maxBytes ];
			sock->async_receive( boost::asio::buffer( buf, _maxBytes ), boost::bind( &WebSocket::_async_read, this, u, buf, _1, _2 ) );
			
			sock_ptr s( new tcp::socket( _io_service ) );
			_server_sock->async_accept( *s, boost::bind( &WebSocket::_async_accept, this, s, boost::asio::placeholders::error ) );
		}
	}

	void WebSocket::_handshake( usr_ptr u, std::string header )
	{
		Handshake h;
		
		if( h.processHandshake( header ) )
		{
			_async_send( u, h.getHandshake( ) );
			
			u->handshaken = true;
		}
		else
		{
			std::cout << "Handshake fail with client " << u->uid << std::endl;
		}
	}
	
	void WebSocket::_async_broadcast( const std::string& message )
	{
		std::map< int, User >::iterator user = _users.begin( );
		while( user != _users.end( ) )
		{
			_async_send( usr_ptr( new User( user->second ) ), message );
			
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
			
			u->sock->async_send( boost::asio::buffer( (unsigned char*)r->c_str( ), r->length( ) ), boost::bind( &WebSocket::_async_sent, this, r, _1, _2 ) );
		}
	}
	
	void WebSocket::_async_sent( str_ptr sent, const boost::system::error_code& error, size_t bytes_transferred )
	{
		if( error )
		{
			std::cout << "Error sending to client: " << error.message( ) << "\n\n\n";
		}
	}
	
	void WebSocket::_pad( std::string& message )
	{
		message = std::string( char( 0x00 ) + message + char( 0xFF ) );
	}
	
	void WebSocket::broadcast( std::string message )
	{
		_async_broadcast( message );
	}
	
	void WebSocket::_disconnect( usr_ptr u )
	{
		std::cout << "Disconnection with client " << u->uid << std::endl;
		u->sock->close( );
		_users.erase( u->uid );
	}
	
	ResponseType WebSocket::process( std::string& request, std::string& response )
	{
		response = request;
		
		return RESPOND;
	}
}