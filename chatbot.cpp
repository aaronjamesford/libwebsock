#include "WebSocket.h"
#include "Processable.h"

class ChatBot : public libwebsock::Processable
{
public:
	int process( std::string& request, std::string& response );
};

int ChatBot::process( std::string& request, std::string& response )
{
	if( request == "hello" )
	{
		response = "Why, hello there!";
	}
	else if( request == "weather" )
	{
		response = "Weather sucks here!";
	}
	else
	{
		response = "I have no idea what you just said, but I'm sure it was interesting";
	}
	
	return libwebsock::RESPOND;
}

int main( )
{
	ChatBot c;
	libwebsock::WebSocket w( 2738 );
	w.start( &c );
}
