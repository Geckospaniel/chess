#include "Room.hh"

void Room::handleMessage(Connection& conn, std::string& cmd, std::stringstream& args)
{
	if(cmd == "legal")
	{
		Vec2s legalFrom;
		args >> legalFrom.x;
		args >> legalFrom.y;

		auto user = users.find(conn);

		//	Cache legal moves and store them to a stringstream
		std::ostringstream ss = user->second.getLegalMoves(legalFrom);
		server.send(conn, ss.str(), websocketpp::frame::opcode::text);
	}

	else if(cmd == "move")
	{
		Vec2s moveTo;
		args >> moveTo.x;
		args >> moveTo.y;

		//	Can a move happen?
		MoveResult result = users.find(conn)->second.move(moveTo);

		//	Move happened
		if(result == MoveResult::Moved)
		{
			//	Inform the user that the given move happened
			server.send(conn, "move", websocketpp::frame::opcode::text);

			std::ostringstream tileData = getTileData();
			std::ostringstream checks;

			checks << "check";
			game.getChecks([&checks](Vec2s pos) { checks << ' ' << pos.x << ' ' << pos.y; });

			//	Send each user new tile and check data
			for(auto& user : users)
			{
				server.send(user.first, tileData.str(), websocketpp::frame::opcode::text);
				server.send(user.first, checks.str(), websocketpp::frame::opcode::text);
			}
		}

		//	Move happened and it led to a promotion
		else if(result == MoveResult::Promotion)
		{
			waitForPromotion = true;
			std::ostringstream promotion;
			promotion << "promote " << game.getPromotion().x << ' ' << game.getPromotion().y;
			server.send(conn, promotion.str(), websocketpp::frame::opcode::text);
		}
	}

	else if(cmd == "promote")
	{
		Vec2s promotionAt = game.getPromotion();

		//	Is a promotion possible and is the correct user trying to promote
		if(!waitForPromotion ||
			users.find(conn)->second.playerID != game.at(promotionAt.x, promotionAt.y).playerID)
		{
			//	TODO Punish the user for trying to promote when it's not possible >:)
			std::cout << "ILLEGAL PROMOTION\n";
		}

		else
		{
			size_t newPiece;
			args >> newPiece;
			std::cout << "PROMOTE TO " << newPiece << "\n";

			//	Promote the piece to whatever the user said
			game.promote(static_cast <Chess::PieceName> (newPiece));
			waitForPromotion = false;

			std::ostringstream tileData = getTileData();
			std::ostringstream checks;

			checks << "check";
			game.getChecks([&checks](Vec2s pos) { checks << ' ' << pos.x << ' ' << pos.y; });

			//	Send each user new tile and check data
			for(auto& user : users)
			{
				server.send(user.first, tileData.str(), websocketpp::frame::opcode::text);
				server.send(user.first, checks.str(), websocketpp::frame::opcode::text);
			}
		}
	}

	else server.send(conn, "invalid", websocketpp::frame::opcode::text);
}

bool Room::connectionHere(Connection& conn)
{
	return users.find(conn) != users.end();
}

void Room::addConnection(Connection& conn)
{
	std::ostringstream str;
	Vec2s boardSize = game.getBoardSize();

	//	Tell whoever connected how large the board is
	str << "size " << boardSize.x << ' ' << boardSize.y;
	server.send(conn, str.str(), websocketpp::frame::opcode::text);

	//	Tell the player their ID
	str = std::ostringstream(std::string());
	str << "id ";
	str << users.size();
	server.send(conn, str.str(), websocketpp::frame::opcode::text);

	//	TODO have the player limit as a variable
	if(users.size() < 2)
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
		const Chess::Player& p = game.addPlayer(positions[users.size()], middle, false);
		Player& newUser = users.emplace(conn, Player(game, &p, users.size())).first->second;

		//	Tell the player who they should look at the board
		str = newUser.getView();
		server.send(conn, str.str(), websocketpp::frame::opcode::text);

		//	Inform each player about the new pieces on the board
		str = getTileData();
		for(auto& user : users)
			server.send(user.first, str.str(), websocketpp::frame::opcode::text);

		//	TODO Wait for all players to connect before starting the game
	}

	else
	{
		//	Send data about the tiles to new spectators
		users.emplace(conn, Player(game, nullptr, users.size()));
		str = getTileData();
		server.send(conn, str.str(), websocketpp::frame::opcode::text);

		//	Send check information to new spectators
		std::ostringstream checks;
		checks << "check";
		game.getChecks([&checks](Vec2s pos) { checks << ' ' << pos.x << ' ' << pos.y; });
		server.send(conn, checks.str(), websocketpp::frame::opcode::text);
	}
}

std::ostringstream Room::getTileData()
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
