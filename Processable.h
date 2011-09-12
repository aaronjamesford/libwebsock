#ifndef PROCESSABLE_H
#define PROCESSABLE_H

#include "Logger.h"

namespace libwebsock
{
	enum Response
	{
		NO_RESPOND, RESPOND, BROADCAST
	};
	
	class Processable
	{
	public:
		virtual int process( std::string& in, std::string& out )
		{
			out = in;
			return RESPOND;
		}
	protected:
		inline static void log( const std::string& message ) { Logger::getInstance( )->log( message ); }
	};
}

#endif
