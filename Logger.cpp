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
		
		std::cout << "[" << ctime( &currenttime ) << "] " << message << std::endl;
	}
}
