#ifndef PLAYER_HEADER
#define PLAYER_HEADER

#include "../../Engine.hh"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class Player
{
public:
	Player(Engine& engine, size_t playerID)
		: e(engine), playerID(playerID)
	{
		std::cout << "Added player " << playerID << '\n';
	}

	bool move(Vec2s to);
	std::ostringstream getLegalMoves(Vec2s from);

private:
	Engine& e;
	size_t playerID;

	std::vector <std::pair <Vec2s, MoveType>> moves;
	Vec2s movesFrom;
};

#endif
