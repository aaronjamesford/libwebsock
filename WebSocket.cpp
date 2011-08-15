#include <string>
#include <iostream>

#include <boost/algorithm/string/trim.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "md5/md5.h"
#include "WebSocket.h"

namespace WebSocket
{

	WebSocket::WebSocket( boost::asio::io_service& io_service )
	 : _io_service( io_service ), _maxBytes( 2048 )
	{
		
	}
	
	void WebSocket::start( )
	{
		tcp::acceptor server_sock( _io_service );
		tcp::endpoint ep( tcp::v4( ), _port );
		
		// open the socket, set it to non blocking and bind the socket
		server_sock.open( ep.protocol( ) );
		// server_sock.set_option( boost::asio::socket_base::non_blocking_io( true ) );
		server_sock.bind( ep );
				
		while( true )
		{
			// accept the socket, if it actually accepted, add the socket to the list
			sock_ptr sock( new tcp::socket( _io_service ) );
			server_sock.async_accept( *sock, boost::bind( &WebSocket::_handleAccept, this, sock, _1 ) );
			_io_service.run( );
			
			// read shit, determine validity, process
			std::vector< User >::iterator it = _users.begin( );
			while( it != _users.end( ) )
			{
				boost::system::error_code error;
				std::string req;
				it->sock->receive( boost::asio::buffer( req, _maxBytes ), 0, error );
				if( error == boost::asio::error::eof )
				{
					it->sock->close( );
					it = _users.erase( it );
				}
				else
				{
					if( it->handshaken )
					{ // process the actual request
						process( *it, req );
					}
					else
					{ // needs to handshake
						_handshake( *it, req );
					}
				}
				
				++it;
			}
		}
	}
	
	void WebSocket::_handleAccept( sock_ptr sock, const boost::system::error_code& error )
	{
		if( !error )
		{
			// create a new user and add it to the list
			User u;
			tcp::socket::non_blocking_io command( true );
			sock->io_control( command );
			u.sock = sock;
			_users.push_back( u );
		}
	}

	void WebSocket::_handshake( User& u, const std::string& header )
	{
		// the request - i.e. "GET /uobnfsjldf HTTP1.1/1" or whatever the fuck it is
		_req = header.substr( 0, header.find( '\n' ) );
		
		// the Host
		_host = _getField( header, "Host" );
		boost::algorithm::trim( _host );
		
		// the origin
		_origin = _getField( header, "Origin" );
		boost::algorithm::trim( _origin );
		
		// protocol
		_protocol = _getField( header, "Sec-WebSocket-Protocol" );
		boost::algorithm::trim( _protocol );
		
		// TODO : Probably add some more
		
		// get key 1
		_key1 = _getField( header, "Sec-WebSocket-Key1" );
		boost::algorithm::trim( _key1 );
		
		// get key2
		_key2 = _getField( header, "Sec-WebSocket-Key2" );
		boost::algorithm::trim( _key2 );
		
		// the last 8 bytes is easy peasy
		_l8b = header.substr( header.length( ) - 8 );
		
		// TODO : validate all the fields gathered so far
		
		// get the secret :)
		std::string secret = _genSecret( );
		
		// construct the response
		// probably needs some more validation as some of these are optional and some may depend on static values
		std::string response = "HTTP/1.1 101 WebSocket Protocol Handshake\r\n";
		response += "Upgrade: WebSocket\r\nConnection: Upgrade\r\n";
		response += "Sec-WebSocket-Origin: " + _origin + "\r\n";
		response += "Sec-WebSocket-Location: ws://" + _host + _path + "\r\n";
		response += "Sec-WebSocket-Protocol: " + _protocol + "\r\n";
		response += 0x0D;
		response += 0x0A;
		response += secret;
		
		// std::cout << response << std::endl;
		
		_send( u.sock, response );
		
		u.handshaken = true;
	}

	std::string WebSocket::_getField( const std::string& header, const std::string& field )
	{
		size_t start = header.find( field );
		size_t colon;
		size_t newline;
		
		if( start != std::string::npos )
		{
			colon = header.find( ':', start );
			if( colon != std::string::npos )
			{
				newline = header.find_first_of( "\r\n", colon );
				if( newline != std::string::npos )
				{
					return header.substr( colon + 1, (newline - colon) - 1 );
				}
			}
		}
		
		return "";
	}

	std::string WebSocket::_genSecret( )
	{
		int k1 = _extractKey( _key1 );
		int k2 = _extractKey( _key2 );
		
		return md5( _getBigEndRep( k1 ) + _getBigEndRep( k2 ) + _l8b, true );
	}

	int WebSocket::_extractKey( const std::string& token )
	{
		unsigned int spaceCount = 0;
		unsigned int res = 0;
		
		std::string::const_iterator it = token.begin( );
		while( it != token.end( ) )
		{
			if( *it == ' ' )
			{
				++spaceCount;
			}
			else
			{
				int c = *it - '0';
				if( c >= 0 && c <= 9 )
				{
					res = res * 10;
					res = res + c;
				}
			}
			
			++it;
		}
		
		if( spaceCount == 0 )
		{ // error time
			return -1;
		}
		
		return res / spaceCount;
	}

	std::string WebSocket::_getBigEndRep( int x )
	{
		int test = 1;
		char* endian = (char*)&test;
		
		if( endian[ 0 ] )
		{ // little endian
			x = (x>>24) | 
				((x<<8) & 0x00FF0000) |
				((x>>8) & 0x0000FF00) |
				(x<<24);
		}
		
		unsigned char* r = (unsigned char*)&x;
		std::string res = "";
		for( int i = 0; i < 4; i++ )
		{
			res += r[ i ];
		}
		
		return res;
	}

	void WebSocket::_process( User& u, const std::string& req )
	{
		_send( u.sock, req );
	}
	
	void WebSocket::_send( sock_ptr sock, const std::string& resp )
	{
		sock->send( boost::asio::buffer( resp, _maxBytes ) );
	}
}