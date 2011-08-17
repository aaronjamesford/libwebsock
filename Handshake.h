#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include <string>

namespace libwebsock
{
	class Handshake
	{
	public:
		Handshake( );
		bool processHandshake( std::string& header );
		std::string getHandshake( );
	private:
		std::string _handshake;
	
		std::string _req;
		std::string _origin;
		std::string _host;
		std::string _protocol;
		std::string _key1;
		std::string _key2;
		std::string _l8b;
		std::string _path;
	
		std::string _getField( const std::string& header, const std::string& field );
		std::string _getPath( const std::string& line );
		std::string _genSecret( );
		int _extractKey( const std::string& token );
		std::string _getBigEndRep( int x );
	};
}

#endif