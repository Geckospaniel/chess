#include "Player.hh"

bool Player::move(Vec2s to)
{
	//	Forbid other players from moving pieces owned by this player
	if(game.at(movesFrom.x, movesFrom.y).playerID != playerID)
		return false;

	//	Forbid moving pieces when it's not this player's turn
	if(game.getCurrentTurn() != playerID)
		return false;

	//	Is the given move found in the cache
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

	//	We want to prevent checks only on pieces owned by this player
	bool protectKing = game.at(from.x, from.y).playerID == playerID;

	movesFrom = from;
	game.legalMoves(from, [this, &str](Vec2s pos, Chess::MoveType t)
	{
		str << ' ' << pos.x << ' ' << pos.y << ' ' << static_cast <size_t> (t);
		moves.push_back(std::make_pair(pos, t));
	}, protectKing);

	return str;
}

std::ostringstream Player::getView()
{
	std::ostringstream str;
	str << "view 0 ";

	//	Perform black magic to tell the player in which angle to look at the board
	//	FIXME Players on the sides are flipped
	str << (player->pawnDirection.y > 0 ? game.getBoardSize().y - 1 : 0);
	str << ' ' << (player->pawnDirection.y == 0);

	return str;
}
