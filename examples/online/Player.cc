#include "Player.hh"

MoveResult Player::move(Vec2s to)
{
	//	Forbid other players from moving pieces owned by this player
	if(game.at(movesFrom.x, movesFrom.y).playerID != playerID)
		return MoveResult::NoMove;

	//	Forbid moving pieces when it's not this player's turn
	if(game.getCurrentTurn() != playerID)
		return MoveResult::NoMove;

	//	Is the given move found in the cache
	for(auto& move : moves)
	{
		if(to == move.first)
		{
			if(!game.move(movesFrom, to))
				return MoveResult::Promotion;

			return MoveResult::Moved;
		}
	}

	return MoveResult::NoMove;
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
	str << "view";

	bool atBottom = player->pawnDirection.y < 0 || player->pawnDirection.x < 0;
	
	//	TODO fine tuning to view angles

	//	If this player is at the bottom in board-space, the king should be on the right
	str << ' ' << (atBottom ? 0 : game.getBoardSize().x - 1);

	//	Always have the player's pieces on the bottom on the screen
	str << ' ' << (atBottom ? 0 : game.getBoardSize().y - 1);
	str << ' ' << (player->pawnDirection.y == 0);

	return str;
}
