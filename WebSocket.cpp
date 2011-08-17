#include <string>
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
		
	}
	
	void WebSocket::start( )
	{
		_server_sock = accept_ptr( new tcp::acceptor( _io_service ) );
		tcp::endpoint ep( tcp::v4( ), _port );
		
		// open the socket, set it to non blocking and bind the socket
		_server_sock->open( ep.protocol( ) );
		// server_sock.set_option( boost::asio::socket_base::non_blocking_io( true ) );
		_server_sock->bind( ep );
		_server_sock->listen( );
		
		_accept( );
	}
	
	void WebSocket::_handleAccept( sock_ptr sock, const boost::system::error_code& error )
	{
		std::cout << "Errr" << std::endl;
		if( !error )
		{
			std::cout << "connection :)\n\n\n\n";
			// create a new user and add it to the list
			User u;
			// tcp::socket::non_blocking_io command( true );
			// sock->io_control( command );
			u.sock = sock;
			_users.push_back( u );
		}
		
		while( true )
		{
			_read( );
		}
	}
	
	void WebSocket::_read( )
	{
		// read shit, determine validity, process
		std::vector< User >::iterator it = _users.begin( );
		while( it != _users.end( ) )
		{
			if( it->sock->available( ) > 0 )
			{
				// std::cout << "Hello" << std::endl;
				boost::system::error_code error;
				// std::string req;
				char* req = new char[ _maxBytes ];
				size_t bytes = it->sock->receive( boost::asio::buffer( req, _maxBytes ), 0, error );
				// size_t bytes = boost::asio::read( *(it->sock), boost::asio::buffer( req, _maxBytes ) );
				if( error == boost::asio::error::eof )
				{
					std::cout << "eof" << std::endl;
					it->sock->close( );
					it = _users.erase( it );
				}
				else if( !error && bytes > 0 )
				{
					// req[ bytes ] = 0x00;
					// std::string r( req + 1, bytes - 2 );
					// std::cout << "Recieved: \n" << r << std::endl << std::endl;
					if( it->handshaken )
					{ // process the actual request
						if( req[ 0 ] == 0x00 )
						{
							std::string s ( req + 1, bytes - 1 );
							std::cout << s << std::endl;
							_process( *it, s.substr( 0, s.length( ) - 1 ) );
						}
						else
						{
							_process( *it, std::string( req ) );
						}
					}
					else
					{ // needs to handshake
						_handshake( *it, req );
					}
				}
				else if( error )
				{
					std::cout << error.message( ) << std::endl;
					throw error;
				}
				
				delete[] req;
			}
			
			++it;
		}
	}
	
	void WebSocket::_accept( )
	{
			// accept the socket, if it actually accepted, add the socket to the list
			std::cout << "Accepting..." << std::endl;
			sock_ptr sock( new tcp::socket( _io_service ) );
			_server_sock->async_accept( *sock, boost::bind( &WebSocket::_handleAccept, this, sock, _1 ) );
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

	void WebSocket::_process( User& u, std::string req )
	{
		req = char( 0x00 ) + req + char( 0xFF );
		_send( u.sock, req );
	}
	
	void WebSocket::_send( sock_ptr sock, const std::string& resp )
	{
		// resp.insert( resp.begin( ), '\0' );
		sock->send( boost::asio::buffer( resp, resp.length( ) ) );
		
		std::cout << "SENT" << std::endl;
	}
}