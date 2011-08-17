#include <string>
#include <boost/algorithm/string/trim.hpp>

#include "Handshake.h"
#include "md5/md5.h"

namespace libwebsock
{
	Handshake::Handshake( )
	{
		/*
		 -- Empty implementation
		*/
	}
	
	bool Handshake::processHandshake( std::string& header )
	{
		boost::algorithm::trim( header );
		
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
		std::string response = "";
		//response += char( 0x00 );
		response += "HTTP/1.1 101 WebSocket Protocol Handshake\r\n";
		response += "Upgrade: WebSocket\r\nConnection: Upgrade\r\n";
		response += "Sec-WebSocket-Origin: " + _origin + "\r\n";
		response += "Sec-WebSocket-Location: ws://" + _host + _path + "\r\n";
		// response += "Sec-WebSocket-Protocol: " + _protocol + "\r\n";
		response += 0x0D;
		response += 0x0A;
		response += secret;
		
		_handshake = response;
		
		// place holder
		return true;
	}
	
	std::string Handshake::getHandshake( )
	{
		return _handshake;
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

	std::string Handshake::_genSecret( )
	{
		int k1 = _extractKey( _key1 );
		int k2 = _extractKey( _key2 );
		
		return md5( _getBigEndRep( k1 ) + _getBigEndRep( k2 ) + _l8b, true );
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