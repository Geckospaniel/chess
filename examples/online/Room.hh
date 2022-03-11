#ifndef ROOM_HEADER
#define ROOM_HEADER

#include "../../chess/Game.hh"
#include "Player.hh"

#include <websocketpp/server.hpp>
#include <websocketpp/config/asio_no_tls.hpp>

#include <sstream>
#include <map>

typedef websocketpp::server<websocketpp::config::asio> Websocket;
typedef websocketpp::connection_hdl Connection;
typedef Websocket::message_ptr Message;

class Room
{
public:
	Room(Websocket& server) : server(server), game(12, 8)
	{
	}

	void handleMessage(Connection& conn, std::string& cmd, std::stringstream& args);

	bool connectionHere(Connection& conn);
	void addConnection(Connection& conn);

private:
	std::ostringstream getTileData();
	Websocket& server;
	Chess::Game game;

	bool waitForPromotion = false;
    std::map <Connection, Player, std::owner_less <Connection>> users;
};

#endif
