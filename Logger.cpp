#include "Logger.h"

#include <string>
#include <ctime>

#include <iostream>

namespace libwebsock
{
	Logger* Logger::_inst = 0;
	
	Logger* Logger::getInstance( )
	{
		if( !_inst )
		{
			_inst = new Logger( );
		}
		
		return _inst;
	}
	
	void Logger::log( const std::string& message )
	{
		time_t currenttime;
		
		time( &currenttime );
		
		char* str_time = ctime( &currenttime );
		std::string returnless_time( str_time );
		returnless_time = returnless_time.substr( 0, returnless_time.length( ) - 1 );
		
		std::cout << "[" << returnless_time << "] " << message << std::endl;
	}
}
