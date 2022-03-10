#ifndef PLAYER_HEADER
#define PLAYER_HEADER

#include "../../chess/Game.hh"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

enum class MoveResult
{
	Moved,
	NoMove,
	Promotion
};

class Player
{
public:
	Player(Chess::Game& game, const Chess::Player* player, size_t playerID)
		: playerID(playerID), game(game), player(player)
	{
		std::cout << "Added player " << playerID << '\n';
	}

	MoveResult move(Vec2s to);
	std::ostringstream getLegalMoves(Vec2s from);
	std::ostringstream getView();

	const size_t playerID;

private:
	Chess::Game& game;
	const Chess::Player* player;

	std::vector <std::pair <Vec2s, Chess::MoveType>> moves;
	Vec2s movesFrom;
};

#endif
