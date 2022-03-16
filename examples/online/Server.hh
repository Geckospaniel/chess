#ifndef SERVER_HEADER
#define SERVER_HEADER

#include "Room.hh"
#include "optionparser/OptionParser.hh"

#include <unordered_map>
#include <string>

class Server
{
public:
	Server(OptionParser& opt);

private:
	Room* findRoom(Connection& conn);

	std::unordered_map <std::string, Room> rooms;
	Websocket server;
};

#endif
