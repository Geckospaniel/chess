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
	Player(Chess::Game& game, const Chess::Player* player, size_t playerID)
		: game(game), player(player), playerID(playerID)
	{
		std::cout << "Added player " << playerID << '\n';
	}

	bool move(Vec2s to);
	std::ostringstream getLegalMoves(Vec2s from);
	std::ostringstream getView();

private:
	Chess::Game& game;
	const Chess::Player* player;
	size_t playerID;

	std::vector <std::pair <Vec2s, Chess::MoveType>> moves;
	Vec2s movesFrom;
};

#endif
