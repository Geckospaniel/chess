#include "Server.hh"

#include <sstream>

Server::Server() : game(12, 8)
{
	server.set_access_channels(websocketpp::log::alevel::all);
	server.clear_access_channels(websocketpp::log::alevel::frame_payload);
	server.init_asio();

	server.set_message_handler([this](websocketpp::connection_hdl conn, message_ptr msg)
	{
		auto client = server.get_con_from_hdl(conn);
		std::ostringstream addr;
		addr << client->get_raw_socket().remote_endpoint();

		//	TODO add a rate limiter

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

				//	Cache legal moves and store them to a stringstream
				std::ostringstream ss = player->second.getLegalMoves(legalFrom);
				client->send(ss.str(), msg->get_opcode());
			}

			else if(cmd == "move")
			{
				Vec2s moveTo;
				received >> moveTo.x;
				received >> moveTo.y;

				//	Can a move happen?
				MoveResult result = players.find(conn)->second.move(moveTo);

				//	Move happened
				if(result == MoveResult::Moved)
				{
					//	Inform the player that the given move happened
					server.send(conn, "move", msg->get_opcode());

					std::ostringstream tileData = getTileData();
					std::ostringstream checks;

					checks << "check";
					game.getChecks([&checks](Vec2s pos) { checks << ' ' << pos.x << ' ' << pos.y; });

					//	Send each player new tile and check data
					for(auto& player : players)
					{
						server.send(player.first, tileData.str(), msg->get_opcode());
						server.send(player.first, checks.str(), msg->get_opcode());
					}
				}

				//	Move happened and it led to a promotion
				else if(result == MoveResult::Promotion)
				{
					waitForPromotion = true;
					std::ostringstream promotion;
					promotion << "promote " << game.getPromotion().x << ' ' << game.getPromotion().y;
					server.send(conn, promotion.str(), msg->get_opcode());
				}
			}

			else if(cmd == "promote")
			{
				Vec2s promotionAt = game.getPromotion();

				//	Is a promotion possible and is the correct player trying to promote
				if(!waitForPromotion ||
					players.find(conn)->second.playerID != game.at(promotionAt.x, promotionAt.y).playerID)
				{
					//	TODO Punish the player for trying to promote when it's not possible >:)
					std::cout << "ILLEGAL PROMOTION\n";
				}

				else
				{
					size_t newPiece;
					received >> newPiece;
					std::cout << "PROMOTE TO " << newPiece << "\n";

					//	Promote the piece to whatever the player said
					game.promote(static_cast <Chess::PieceName> (newPiece));
					waitForPromotion = false;

					std::ostringstream tileData = getTileData();
					std::ostringstream checks;

					checks << "check";
					game.getChecks([&checks](Vec2s pos) { checks << ' ' << pos.x << ' ' << pos.y; });

					//	Send each player new tile and check data
					for(auto& player : players)
					{
						server.send(player.first, tileData.str(), msg->get_opcode());
						server.send(player.first, checks.str(), msg->get_opcode());
					}
				}
			}

			//	TODO forbid multiple "new" messages
			else if(cmd == "new")
			{
				std::ostringstream str;
				Vec2s boardSize = game.getBoardSize();

				//	Tell whoever connected how large the board is
				str << "size " << boardSize.x << ' ' << boardSize.y;
				server.send(conn, str.str(), msg->get_opcode());

				//	Tell the player their ID
				str = std::ostringstream(std::string());
				str << "id ";
				str << players.size();
				server.send(conn, str.str(), msg->get_opcode());

				//	TODO have a player limit as a variable
				if(players.size() < 2)
				{
					//	Find the middle point of the board
					size_t centerLeft = boardSize.x / 2 - 1;
					Vec2s middle(centerLeft, boardSize.y / 2);

					//	King positions for the players
					Vec2s positions[]
					{
						Vec2s(centerLeft, 0),
						Vec2s(centerLeft, boardSize.y - 1),
						Vec2s(0, centerLeft),
						Vec2s(boardSize.x - 1, centerLeft)
					};
				
					//	Add a new player
					const Chess::Player& p = game.addPlayer(positions[players.size()], middle, false);
					players.emplace(conn, Player(game, &p, players.size()));

					//	Tell the player who they should look at the board
					str = players.find(conn)->second.getView();
					server.send(conn, str.str(), msg->get_opcode());

					//	Inform each player about the new pieces on the board
					str = getTileData();
					for(auto& player : players)
						server.send(player.first, str.str(), msg->get_opcode());

					//	TODO Wait for all players to connect before starting the game
				}

				else
				{
					//	Send data about the tiles to new spectators
					players.emplace(conn, Player(game, nullptr, players.size()));
					str = getTileData();
					server.send(conn, str.str(), msg->get_opcode());

					//	Send check information to new spectators
					std::ostringstream checks;
					checks << "check";
					game.getChecks([&checks](Vec2s pos) { checks << ' ' << pos.x << ' ' << pos.y; });
					server.send(conn, checks.str(), msg->get_opcode());
				}
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
	Vec2s boardSize = game.getBoardSize();
	std::ostringstream str;
	str << "tile";

	for(size_t x = 0; x < boardSize.x; x++)
	{
		for(size_t y = 0; y < boardSize.y; y++)
			str << ' ' << static_cast <size_t> (game.at(x, y).piece) << ' ' << game.at(x, y).playerID;
	}

	return str;
}
