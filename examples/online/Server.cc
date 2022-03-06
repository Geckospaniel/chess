#include "Server.hh"

#include <sstream>

Server::Server()
{
	server.set_access_channels(websocketpp::log::alevel::all);
	server.clear_access_channels(websocketpp::log::alevel::frame_payload);
	server.init_asio();

	server.set_message_handler([this](websocketpp::connection_hdl conn, message_ptr msg)
	{
		auto client = server.get_con_from_hdl(conn);
		std::ostringstream addr;
		addr << client->get_raw_socket().remote_endpoint();

		std::cout << "addr is " << addr.str() << '\n';

		try
		{
			std::stringstream received(msg->get_payload());

			std::string cmd;
			received >> cmd;

			if(cmd == "kill")
			{
				server.stop_listening();
				server.stop();
			}

			else if(cmd == "legal")
			{
				Vec2s legalFrom;
				received >> legalFrom.x;
				received >> legalFrom.y;

				auto player = players.find(conn);
				std::ostringstream ss = player->second.getLegalMoves(legalFrom);
				client->send(ss.str(), msg->get_opcode());
			}

			else if(cmd == "move")
			{
				Vec2s moveTo;
				received >> moveTo.x;
				received >> moveTo.y;

				auto player = players.find(conn);
				if(player->second.move(moveTo))
				{
					server.send(conn, "move", msg->get_opcode());
					std::ostringstream tileData = getTileData();

					for(auto& player : players)
						server.send(player.first, tileData.str(), msg->get_opcode());
				}
			}

			else if(cmd == "tile")
			{
				std::ostringstream tileData = getTileData();
				server.send(conn, tileData.str(), msg->get_opcode());
			}

			else if(cmd == "new")
			{
				players.emplace(conn, Player(e, players.size()));

				std::stringstream str;
				str << "size " << e.getBoardSize().x << ' ' << e.getBoardSize().y;
				server.send(conn, str.str(), msg->get_opcode());
			}

			else server.send(conn, "invalid", msg->get_opcode());
		}

		catch (websocketpp::exception const & e)
		{
			std::cout << "Echo failed because: "
					  << "(" << e.what() << ")" << std::endl;
		}
	});

	server.set_reuse_addr(true);
	server.listen(9002);
	server.start_accept();
	server.run();
}

std::ostringstream Server::getTileData()
{
	Vec2s boardSize = e.getBoardSize();
	std::ostringstream str;
	str << "tile";

	for(size_t x = 0; x < boardSize.x; x++)
	{
		for(size_t y = 0; y < boardSize.y; y++)
			str << ' ' << static_cast <size_t> (e.at(x, y).piece) << ' ' << e.at(x, y).playerID;
	}

	return str;
}
