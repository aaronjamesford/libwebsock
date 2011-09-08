#include <string>

#include <boost/algorithm/string/trim.hpp>

#include <iostream>
#include <sstream>

#include <cstdio>

#include <openssl/sha.h>

#include "Handshake.h"
#include "md5/md5.h"

#include "Frame.h"
#include "Frame_00.h"
#include "Frame_08.h"

#include "utils.h"

namespace libwebsock
{
	const std::string Handshake::_magic = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
	
	Handshake::Handshake( )
	{
		/*
		 -- Empty implementation
		*/
	}
	
	bool Handshake::processHandshake( std::string& header )
	{
		// boost::algorithm::trim( header );
		
		// the request - i.e. "GET /uobnfsjldf HTTP1.1/1" or whatever the fuck it is
		_req = header.substr( 0, header.find( '\n' ) );
		
		_path = _getPath( _req );
		boost::algorithm::trim( _path );
		
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
		
		// get the protocol version
		std::stringstream ss_version( boost::algorithm::trim_copy( _getField( header, "Sec-WebSocket-Version" ) ) );
		if( ss_version )
		{
			ss_version >> _version;
		}
		else
		{
			_version = 0;
		}
		
		// TODO : validate all the fields gathered so far
		
		// construct the response
		// probably needs some more validation as some of these are optional and some may depend on static values
		std::string response = "";
		
		
		response += "HTTP/1.1 101 ";
		response += (_version >= 6) ? "Switching Protocols" : "WebSocket Protocol Handshake";
		response += "\r\n";
		
		response += "Upgrade: websocket\r\nConnection: Upgrade\r\n";
		// response += "Sec-WebSocket-Origin: " + _origin + "\r\n";
		// response += "Sec-WebSocket-Location: ws://" + _host + _path + "\r\n";
		// response += "Sec-WebSocket-Protocol: " + _protocol + "\r\n";
		response += (_version >= 6) ? _genAccept( header ) + "\r\n\r\n" : _genSecret( header );
		
		// std::cout << "Handshake version: " << _version;
		
		
		_handshake = response;
		
		// place holder
		return true;
	}
	
	std::string Handshake::getHandshake( )
	{
		return _handshake;
	}
	
	Frame* Handshake::getFrameType( )
	{
		if( _version >= 6 )
		{
			return new Frame_08( );
		}
		
		return new Frame_00( );
	}
	
	std::string Handshake::_getField( const std::string& header, const std::string& field )
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
	
	std::string Handshake::_getPath( const std::string& line )
	{
		size_t start = line.find( ' ' );
		if( start != std::string::npos )
		{
			size_t end = line.find( ' ', start + 1 );
			if( end != std::string::npos )
			{
				return line.substr( start + 1, (end - start) - 1 );
			}
		}
		
		return "";
	}
	
	std::string Handshake::_genAccept( const std::string& header )
	{
		_key = _getField( header, "Sec-WebSocket-Key" );
		boost::algorithm::trim( _key );
		
		std::string tosha( _key + _magic );
		unsigned char shad[ 20 ];
		SHA1( (unsigned char*)tosha.c_str( ), tosha.length( ), shad );
		
		std::string secret( base64( shad, 20 ) );
		std::cout << "Secret: " << secret << std::endl;
		
		return "Sec-WebSocket-Accept: " + boost::algorithm::trim_copy( secret );
	}

	std::string Handshake::_genSecret( const std::string& header )
	{
		// get key 1
		_key1 = _getField( header, "Sec-WebSocket-Key1" );
		boost::algorithm::trim( _key1 );
		
		// get key2
		_key2 = _getField( header, "Sec-WebSocket-Key2" );
		boost::algorithm::trim( _key2 );
		
		// the last 8 bytes is easy peasy
		_l8b = header.substr( header.length( ) - 8 );
		
		int k1 = _extractKey( _key1 );
		int k2 = _extractKey( _key2 );
		
		std::string k( _getBigEndRep( k1 ) + _getBigEndRep( k2 ) + _l8b );
		
		std::string response = "";
		response += 0x0D;
		response += 0x0A;
		response += md5( k , true );
		
		return response;
	}

	int Handshake::_extractKey( const std::string& token )
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

	std::string Handshake::_getBigEndRep( int x )
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
}