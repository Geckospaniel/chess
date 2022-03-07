#ifndef PLAYER_HEADER
#define PLAYER_HEADER

#include "../../chess/Game.hh"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Player
{
public:
	Player(Chess::Game& game, size_t playerID)
		: game(game), playerID(playerID)
	{
		std::cout << "Added player " << playerID << '\n';
	}

	bool move(Vec2s to);
	std::ostringstream getLegalMoves(Vec2s from);

private:
	Chess::Game& game;
	size_t playerID;

	std::vector <std::pair <Vec2s, Chess::MoveType>> moves;
	Vec2s movesFrom;
};

#endif
