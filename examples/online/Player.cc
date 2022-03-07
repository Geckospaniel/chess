#include "Player.hh"

bool Player::move(Vec2s to)
{
	if(e.at(movesFrom.x, movesFrom.y).playerID != playerID)
		return false;

	if(e.getCurrentTurn() != playerID)
		return false;

	for(auto& move : moves)
	{
		if(to == move.first)
		{
			e.move(movesFrom, to);
			return true;
		}
	}

	return false;
}

std::ostringstream Player::getLegalMoves(Vec2s from)
{
	Vec2s boardSize = e.getBoardSize();
	moves.clear();

	std::cout << "Legal from " << from.x << ' ' << from.y << '\n';

	std::ostringstream str;
	str << "legal";

	if(from.x >= boardSize.x || from.y >= boardSize.y)
		return str;

	movesFrom = from;
	e.legalMoves(from, [this, &str](Vec2s pos, MoveType t)
	{
		str << ' ' << pos.x << ' ' << pos.y << ' ' << static_cast <size_t> (t);
		moves.push_back(std::make_pair(pos, t));
	});

	return str;
}
