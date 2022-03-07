#include "Player.hh"

bool Player::move(Vec2s to)
{
	if(game.at(movesFrom.x, movesFrom.y).playerID != playerID)
		return false;

	if(game.getCurrentTurn() != playerID)
		return false;

	for(auto& move : moves)
	{
		if(to == move.first)
		{
			game.move(movesFrom, to);
			return true;
		}
	}

	return false;
}

std::ostringstream Player::getLegalMoves(Vec2s from)
{
	Vec2s boardSize = game.getBoardSize();
	moves.clear();

	std::cout << "Legal from " << from.x << ' ' << from.y << '\n';

	std::ostringstream str;
	str << "legal";

	if(from.x >= boardSize.x || from.y >= boardSize.y)
		return str;

	movesFrom = from;
	game.legalMoves(from, [this, &str](Vec2s pos, Chess::MoveType t)
	{
		str << ' ' << pos.x << ' ' << pos.y << ' ' << static_cast <size_t> (t);
		moves.push_back(std::make_pair(pos, t));
	});

	return str;
}
