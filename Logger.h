#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace libwebsock
{
	class Logger
	{
	public:
		static Logger* getInstance( );
		void log( const std::string& message );
	private:
		static Logger* _inst;
	
		Logger( ) { }
	};
}

#endif
