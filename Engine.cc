#include "Engine.hh"
#include <SDL2/SDL_log.h>

#include <cmath>

const char* name(PieceName name)
{
	const char* n;
	switch(name)
	{
		case PieceName::Pawn: n = "Pawn"; break;
		case PieceName::Bishop: n = "Bishop"; break;
		case PieceName::Knight: n = "Knight"; break;
		case PieceName::Rook: n = "Rook"; break;
		case PieceName::King: n = "King"; break;
		case PieceName::Queen: n = "Queen"; break;

		default: return "";
	}

	return n;
}

Engine::Engine()
{
	mainBoard.size.x = 8;
	mainBoard.size.y = 8;
	mainBoard.data.resize(mainBoard.size.x * mainBoard.size.y, {PieceName::None, 0});

	//	Left-side tile from the center
	size_t centerLeft = mainBoard.size.x / 2 - 1;

	Vec2s kingPos1(centerLeft, 0);
	Vec2s kingPos2(centerLeft, mainBoard.size.y - 1);
	Vec2s kingPos3(0, mainBoard.size.y / 2 - 1);
	Vec2s kingPos4(mainBoard.size.x - 1, mainBoard.size.y / 2 - 1);

	Vec2s middle(centerLeft, mainBoard.size.y / 2);

	createPlayer(kingPos1, middle);
	createPlayer(kingPos2, middle);
	//createPlayer(kingPos3, middle);
	//createPlayer(kingPos4, middle);
}

void Engine::createPlayer(Vec2s kingPosition, Vec2s middle)
{
	size_t id = players.size();
	players.push_back({});
	Player& player = players.back();

	//	Calculate a direction vector from the king to the middle
	Vec2i sub = middle.as <int> () - kingPosition.as <int> ();
	float angle = atan2(sub.y, sub.x);
	player.pawnDirection = Vec2i(round(cos(angle)), round(sin(angle)));

	//	Invert the direction vector
	player.inverseDirection = Vec2i(player.pawnDirection.y, player.pawnDirection.x);

	/*	Because we use pawnDirection and inverseDirection to place the pieces, the
	 *	pieces will be mirrored relative to other player's pieces. If x or y
	 *	is negative, making them positive will fix the mirroring issue */
	if(player.inverseDirection.x < 0) player.inverseDirection.x = -player.inverseDirection.x;
	else if(player.inverseDirection.y < 0) player.inverseDirection.y = -player.inverseDirection.y;

	//	Define a line where the pawns spawn. This is used to determine if they can move 2 spaces
	player.pawnSpawnStart = kingPosition + player.pawnDirection + (player.inverseDirection * -3);
	player.pawnSpawnEnd = player.pawnSpawnStart + (player.inverseDirection * 7);

	//	Pawns
	for(size_t x = 0; x < 8; x++)
		mainBoard.at(player.pawnSpawnStart + (player.inverseDirection * x)) = Tile(PieceName::Pawn, id);

	//	King
	mainBoard.at(kingPosition) = Tile(PieceName::King, id);
	player.kingPosition = kingPosition;

	//	Queen
	//mainBoard.at(kingPosition + player.inverseDirection) = Tile(PieceName::Queen, id);

	//	Rooks, Bishops and Knights
	for(int i = 3; i <= 3; i++)
	{
		//	Because of the way the enum is ordered, we can initialize these pieces in a loop
		PieceName piece = static_cast <PieceName> (static_cast <int> (PieceName::Pawn) + i);

		mainBoard.at(kingPosition + (player.inverseDirection * (1 + i))) = Tile(piece, id);
		mainBoard.at(kingPosition + (player.inverseDirection * (-i))) = Tile(piece, id);
	}
}

Engine::Tile Engine::at(size_t x, size_t y)
{
	//	TODO validate position
	return mainBoard.at(Vec2s(x, y));
}

void Engine::move(const Vec2s& from, const Vec2s& to)
{
	move(mainBoard, from, to);
}

void Engine::move(Board& board, const Vec2s& from, Vec2s to)
{
	/*	TODO maybe move() could take a move type as a parameter
	 *	so that we won't have do checks with positions */

	//	Handle en passant
	if(board.at(from).piece == PieceName::Pawn)
	{
		Vec2i sub = from.as <int> () - to;

		//	En passant was made if the target isn't a piece and movement is slant
		if(board.at(to).piece == PieceName::None && sub.x != 0 && sub.y != 0)
		{
			//	Passing the same coordinate to move will make it empty
			Vec2s enemyPosition = from + players[currentPlayer].pawnDirection;
			move(board, enemyPosition, enemyPosition);
		}
	}

	//	Update the king position and handle castling
	else if(board.at(from).piece == PieceName::King)
	{
		players[currentPlayer].kingPosition = to;
		players[currentPlayer].kingMoved = true;

		//	Handle the king castling
		if(players[currentPlayer].kingCanCastle)
		{
			bool kingSide = to <= from;

			//	Get a direction vector pointing towards the rook
			Vec2i shift = players[currentPlayer].inverseDirection * (kingSide ? -1 : +1);

			//	Make sure that the user actually selected a castling position
			if(board.at(to + shift * (1 + !kingSide)).piece == PieceName::Rook)
			{
				//	Where is the rook
				Vec2s rookPosition = from + shift * (3 + !kingSide);

				//	Move the king and the rook
				to = from + shift * 2;
				move(board, rookPosition, to - shift);
			}
		}

		players[currentPlayer].kingCanCastle = false;
	}

	//	Check if rooks have moved
	else if(board.at(from).piece == PieceName::Rook)
	{
		Player& player = players[currentPlayer];

		//	Has the kingside rook moved
		if(from == player.pawnSpawnStart - player.pawnDirection)
			player.rookMoved[0] = true;

		//	Has the queenside rook moved
		else if(from == player.pawnSpawnEnd - player.pawnDirection)
			player.rookMoved[1] = true;
	}

	board.at(to) = board.at(from);
	board.at(from).piece = PieceName::None;

	//	Since basically any move can trigger a check, check for those checks
	flagThreatenedKings(board);	

	//	Add this move to the history
	moveHistory.emplace_back(from, to, board.at(to));

	//	Update the turn
	if(++currentPlayer >= players.size())
		currentPlayer = 0;
}

void Engine::legalMoves(Vec2s position, const std::function <void(Vec2s, MoveType)>& callback)
{
	//	Reveal checked kings
	for(size_t i = 0; i < players.size(); i++)
	{
		if(players[i].kingThreatened)
			callback(players[i].kingPosition, MoveType::Check);
	}

	legalMoves(mainBoard, position, true, callback);
}

void Engine::legalMoves(Board& board, Vec2s position, bool protectKing,
						const std::function <void(Vec2s, MoveType)>& callback)
{
	auto show = [this, &board, &callback, protectKing]
	(Vec2s from, Vec2s to, MoveType m)
	{
		//	If there's a check that should not happen, don't reveal it
		if(protectKing && leadsToCheck(board, from, to))
		{
			SDL_Log("Moving '%s' from (%lu, %lu) to (%lu, %lu) causes a check", name(board.at(from).piece), from.x, from.y, to.x, to.y);
			return;
		}

		callback(to, m);
	};

	Tile t = board.at(position);

	//	Some large number that's way larger than the board
	size_t steps = 1000;

	//	Which way can the piece go
	bool slant = false;
	bool straight = false;

	//	TODO validate position
	switch(t.piece)
	{
		case PieceName::Pawn:
		{
			//	Determine whether the pawn can move 1 or 2 steps
			steps = 1 + (position >= players[t.playerID].pawnSpawnStart &&
						 position <= players[t.playerID].pawnSpawnEnd);

			Vec2i& direction = players[t.playerID].pawnDirection;
			Vec2s current = position;

			for(size_t i = 0; i < steps && board.isInside(position); i++)
			{
				//	Captures can be done on the origin tile
				if(i == 0)
				{
					Player& player = players[t.playerID];

					Vec2s sides[2]
					{
						current + direction + players[t.playerID].inverseDirection,
						current + direction - players[t.playerID].inverseDirection
					};

					//	En passante shouldn't be checked by flagThreatenedKings()
					if(protectKing)
					{
						//	Calculate the distance from the pawn spawn row
						Vec2s spawnDiff = (current - player.pawnSpawnStart) * player.pawnDirection;

						//	En passante could be possible if this pawn is on the 5th rank
						if(spawnDiff.x == 3 || spawnDiff.y == 3)
						{
							const Vec2i directions[]
							{
								Vec2i(+1, +0),
								Vec2i(+0, -1),
								Vec2i(-1, +0),
								Vec2i(+0, +1),
							};

							for(auto& dir : directions)
							{
								Vec2s capturePosition = current;

								//	If the direction isn't the opposite or we can en passante, reveal the capture
								if(dir != player.pawnDirection * -1 && canEnPassante(board, player, capturePosition, dir))
									show(current, capturePosition, MoveType::Capture);
							}
						}
					}

					//	If a normal capture can be made, reveal it
					for(auto& side : sides)
					{
						if(board.isInside(side) && board.occupied(side) && t.playerID != board.at(side).playerID)
							show(position, side, MoveType::Capture);
					}
				}

				//	Move the pawn and make sure that it could actually move
				current += direction;
				if(!board.isInside(current.as <size_t> ()) || board.occupied(current.as <size_t> ()))
					break;

				//	Reveal the pawn movement
				show(position, current.as <size_t> (), MoveType::Move);
			}

			return;
		}

		case PieceName::Rook: straight = true; break;
		case PieceName::Bishop: slant = true; break;
		case PieceName::Queen: slant = true; straight = true; break;

		case PieceName::King:
		{
			steps = 1;
			slant = true;
			straight = true;

			//	Try castling if king isn't being threated and it hasn't moved
			if(!players[t.playerID].kingThreatened && protectKing && !players[t.playerID].kingMoved)
			{
				Vec2s queenSide = position;
				Vec2s kingSide = position;

				//	Can the king castle on kingside
				if(canCastle(board, players[t.playerID], kingSide, false))
					show(position, kingSide, MoveType::Move);

				//	Can the king castle on queenside
				if(canCastle(board, players[t.playerID], queenSide, true))
					show(position, queenSide, MoveType::Move);
			}

			break;
		}

		case PieceName::Knight:
		{
			//	Hardcoded knight movements
			const Vec2s moves[]
			{
				position + Vec2s(-1, -2), position + Vec2s(1, -2),
				position + Vec2s(2, -1), position + Vec2s(2, 1),
				position + Vec2s(1, 2), position + Vec2s(-1, 2),
				position + Vec2s(-2, -1), position + Vec2s(-2, 1),
				position + Vec2s(-1, 2)
			};

			//	Loop through each possible knight move
			for(auto& move : moves)
			{
				if(board.isInside(move))
				{
					if(board.occupied(move))
					{
						//	Reveal a capture if the knight can make one
						if(board.at(move).playerID != t.playerID)
							show(position, move, MoveType::Capture);

						else continue;
					}

					//	Reveal the knight movement
					else show(position, move, MoveType::Move);
				}
			}

			return;
		}

		case PieceName::None: return;
	}

	const Vec2s directions[]
	{
		//	Straight
		Vec2s(-1, 0),
		Vec2s(0, -1),
		Vec2s(0, 1),
		Vec2s(1, 0),

		//	Slant
		Vec2s(-1, -1),
		Vec2s(1, -1),
		Vec2s(-1, 1),
		Vec2s(1, 1)
	};

	size_t limit = 8;
	size_t i = 0;

	/*	Depending on whether straight or slant movement is needed,
	 *	adjust the limit and offset to the appropriate values so that
	 *	both or either are used in the loop below */
	if(straight && slant);
	else if(straight) limit = 4;
	else if(slant) i = 4;
	else limit = 0;

	for(; i < limit; i++)
	{
		Vec2s current = position;
		for(size_t j = 0; j < steps; j++)
		{
			current += directions[i];

			if(!board.isInside(current))
				break;

			if(board.occupied(current))
			{
				//	If a capture is available reveal it
				if(board.at(current).playerID != t.playerID)
				{
					show(position, current, MoveType::Capture);
					break;
				}

				else break;
			}

			//	Reveal the movement
			show(position, current, MoveType::Move);
		}
	}
}

bool Engine::leadsToCheck(Board& board, Vec2s from, Vec2s to)
{
	//	Save the old state
	Tile oldFromTile = board.at(from);
	Tile oldToTile = board.at(to);

	//	FIXME this is rather stupid because it copies unnecessary things
	//	Save the player states
	std::vector <Player> oldPlayerStates = players;

	//	Perform a fake move
	board.at(to) = board.at(from);
	board.at(from).piece = PieceName::None;

	//	Check if any kings are threatened
	flagThreatenedKings(board);
	bool result = players[currentPlayer].kingThreatened;

	//	Reset the old state
	board.at(from) = oldFromTile;
	board.at(to) = oldToTile;

	//	Reset the old player states
	players = oldPlayerStates;

	//	Is the king of the current turn threatened
	return result;
}

void Engine::flagThreatenedKings(Board& board)
{
	//	Reset the check states
	for(auto& player : players)
		player.kingThreatened = false;

	//	Ugly brute force to check if some piece can capture a king
	for(size_t x = 0; x < board.size.x; x++)
	{
		for(size_t y = 0; y < board.size.y; y++)
		{
			Tile& originTile = board.at(Vec2s(x, y));

			//	Ignore tiles without a piece
			if(originTile.piece == PieceName::None)
				continue;

			//	Get every move that the piece in this tile can make
			legalMoves(board, Vec2s(x, y), false,
			[this, &board, &originTile](Vec2s pos, MoveType type)
			{
				Tile t = board.at(pos);

				//	If the colors match or there's no capture, the move is irrelevant
				if(t.playerID == originTile.playerID || type != MoveType::Capture)
					return;

				//	Is the target piece a king
				if(t.piece == PieceName::King)
					players[t.playerID].kingThreatened = true;
			});
		}
	}
}

bool Engine::canCastle(Board& board, Player& player, Vec2s& position, bool queenSide)
{
	//	If the rook on the given side has moved, no castling can happen
	if(player.rookMoved[queenSide])
	{
		player.kingCanCastle = false;
		return false;
	}

	//	How many steps until the king reaches the rook
	size_t steps = 2;

	//	Is the king going left or right
	int multiplier = queenSide ? +1 : -1;
	Vec2s originalPosition = position;

	/*	We need to check if any enemy piece intercepts the castling
	 *	path. Let's implement a spaghetti solution and simulate
	 *	the king going towards the rook. If any checks
	 *	happen, the king cannot castle */
	for(size_t i = 0; i < steps; i++)
	{
		//	Move the fake king
		position += (player.inverseDirection * multiplier);

		//	Forbid castling when some piece blocks or intercepts it
		if(	(i < steps - 1 && board.occupied(position)) ||
			(leadsToCheck(board, originalPosition, position)))
		{
			player.kingCanCastle = false;
			return false;
		}
	}

	player.kingCanCastle = true;
	return true;
}

bool Engine::canEnPassante(Board& board, Player& player, Vec2s& position, Vec2i direction)
{
	//	Is the adjacent position inside the board
	Vec2s adjacentPosition = position + direction;
	if(!board.isInside(adjacentPosition))
		return false;

	Tile& t = board.at(adjacentPosition);

	//	If the adjacent piece isn't a pawn or not an enemy, en passante can't happen
	if(	t.piece != PieceName::Pawn ||
		t.playerID == static_cast <size_t> (&player - &players[0]))
	{
		return false;
	}

	//	The position where the enemy should have moved from on their last turn
	position = adjacentPosition - (players[t.playerID].pawnDirection * 2);

	for(size_t i = moveHistory.size() - 1; i < moveHistory.size(); i--)
	{
		if(moveHistory[i].change.playerID == t.playerID)
		{
			//	The enemy player did a double step on their last turn
			if(moveHistory[i].from == position && moveHistory[i].to == adjacentPosition)
			{
				position = position + players[t.playerID].pawnDirection;
				return true;
			}

			/*	If the double step didn't happen on the last turn of
			 *	the given player, en passant can't happen */
			break;
		}
	}

	return false;
}

Engine::Tile& Engine::Board::at(const Vec2s& position)
{
	return data[size.x * position.y + position.x];
}

bool Engine::Board::occupied(const Vec2s& position)
{
	return at(position).piece != PieceName::None;
}

bool Engine::Board::isInside(const Vec2s& position)
{
	return	position >= Vec2s () && position < size;
}
