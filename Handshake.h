#ifndef HANDSHAKE_H
#define HANDSHAKE_H

#include <string>

#include "Logger.h"

namespace libwebsock
{
	class Frame;
	class Handshake
	{
	public:
		Handshake( );
		bool processHandshake( std::string& header );
		std::string getHandshake( );
	
		Frame* getFrameType( );
	
		inline void log( const std::string& m ) { _logger->log( m ); }
	private:
		std::string _handshake;
	
		int _version;				// Sec-WebSocket-Version
	
		std::string _path;
		std::string _req;
		std::string _origin;
		std::string _host;
		std::string _protocol;
	
		// protocol version 00 specific
		std::string _key1;
		std::string _key2;
		std::string _l8b;
	
		// protocol version 06 specific
		std::string _key;
		const static std::string _magic;
	
		std::string _getField( const std::string& header, const std::string& field );
		std::string _getPath( const std::string& line );
	
		// which one of these gets called is dependant on the version
		std::string _genAccept( const std::string& header );
		std::string _genSecret( const std::string& header );
		
		int _extractKey( const std::string& token );
		std::string _getBigEndRep( int x );
		
		Logger* _logger;
	};
}

#endif