#include <string>
#include <map>
#include <iostream>
#include <cstdio>

#include <boost/algorithm/string/trim.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "md5/md5.h"
#include "WebSocket.h"
#include "Handshake.h"

#include "Frame_08.h"

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
				size_t start = 0;
				size_t offset = 0;
				
				while( offset < bytes_transferred )
				offset += u->ftype->unpackFrame( (unsigned char*)(request + offset), bytes_transferred - offset );
				if( u->ftype->disconnect( ) )
				{
					_disconnect( u );
					delete[ ] request;
					
					return;
				}
				else
				{
					std::cout << "Data: " << u->ftype->data( ) << std::endl;
					
					std::string response;
					std::string req = u->ftype->data( );
					
					size_t b;
					switch( process( req, response ) )
					{
					case RESPOND:
						unsigned char* frame;
						b = u->ftype->packFrame( response, frame, false );
						_async_send( u, std::string( (char*)frame, b ) );
						delete[ ] frame;
					
						break;
					case BROADCAST:
						_async_broadcast( response );
						break;
					default:
						break;
					}
				}
				
				// std::cout << "Frames in request: " << framecount << std::endl;
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
			_users[ u->uid ] = u;
			
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
			u->ftype = frame_ptr( h.getFrameType( ) );
		}
		else
		{
			std::cout << "Handshake fail with client " << u->uid << std::endl;
		}
	}
	
	void WebSocket::_async_broadcast( const std::string& message )
	{
		std::map< int, usr_ptr >::iterator user = _users.begin( );
		while( user != _users.end( ) )
		{
			std::cout << "broadcasting to user: " << user->first << std::endl;
			
			unsigned char* frame;
			
			size_t size = user->second->ftype->packFrame( message, frame, true );
			_async_send( user->second, std::string( (char*)frame, size ) );
			
			delete[ ] frame;
			
			++user;
		}
	}
	
	void WebSocket::_async_send( usr_ptr u, std::string resp )
	{
		if( u->sock->is_open( ) )
		{
			str_ptr r( new std::string( resp ) );
			
			boost::asio::async_write( *(u->sock), boost::asio::buffer( (unsigned char*)r->c_str( ), r->length( ) ), boost::bind( &WebSocket::_async_sent, this, r, _1, _2 ) );
		}
	}
	
	void WebSocket::_async_sent( str_ptr sent, const boost::system::error_code& error, size_t bytes_transferred )
	{
		if( error )
		{
			std::cout << "Error sending to client: " << error.message( ) << "\n\n\n";
		}
		else
		{
			std::cout << "Sent " << bytes_transferred << " bytes\n";
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