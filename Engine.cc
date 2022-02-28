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
	mainBoard.size.x = 12;
	mainBoard.size.y = 12;
	mainBoard.data.resize(mainBoard.size.x, std::vector <Tile> (mainBoard.size.y, { PieceName::None, 0 }));

	//	Left-side tile from the center
	size_t centerLeft = mainBoard.size.x / 2 - 1;

	Vector2 <size_t> kingPos1(centerLeft, 0);
	Vector2 <size_t> kingPos2(centerLeft, mainBoard.size.y - 1);
	Vector2 <size_t> kingPos3(0, mainBoard.size.y / 2 - 1);
	Vector2 <size_t> kingPos4(mainBoard.size.x - 1, mainBoard.size.y / 2 - 1);

	Vector2 <size_t> middle(centerLeft, mainBoard.size.y / 2);

	createPlayer(kingPos1, middle);
	createPlayer(kingPos2, middle);
	createPlayer(kingPos3, middle);
	createPlayer(kingPos4, middle);
}

void Engine::createPlayer(Vector2 <size_t> kingPosition, Vector2 <size_t> middle)
{
	size_t id = players.size();
	players.push_back({});
	Player& player = players.back();

	//	Calculate a direction vector from the king to the middle
	Vector2 <int> sub = middle.as <int> () - kingPosition.as <int> ();
	float angle = atan2(sub.y, sub.x);
	player.pawnDirection = Vector2 <int> (round(cos(angle)), round(sin(angle)));

	//	Invert the direction vector
	player.inverseDirection = Vector2 <int> (player.pawnDirection.y, player.pawnDirection.x);

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
	mainBoard.at(kingPosition + player.inverseDirection) = Tile(PieceName::Queen, id);

	//	Rooks, Bishops and Knights
	for(int i = 1; i <= 3; i++)
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
	return mainBoard.data[x][y];
}

void Engine::move(const Vector2 <size_t>& from, const Vector2 <size_t>& to)
{
	move(mainBoard, from, to);
}

void Engine::move(Board& board, const Vector2 <size_t>& from, const Vector2 <size_t>& to)
{
	board.data[to.x][to.y] = board.data[from.x][from.y];
	board.data[from.x][from.y].piece = PieceName::None;

	//	Cache the king position
	if(board.data[to.x][to.y].piece == PieceName::King)
		players[currentPlayer].kingPosition = to;

	//	Since basically any move can trigger a check, check for those checks
	flagThreatenedKings(board);	

	//	Update the turn
	if(++currentPlayer >= players.size())
		currentPlayer = 0;
}

void Engine::showMoves(Vector2 <size_t> position,
						const std::function <void(Vector2 <size_t>, MoveType)>& callback)
{
	//	Reveal checked kings
	for(size_t i = 0; i < players.size(); i++)
	{
		if(players[i].kingThreatened)
			callback(players[i].kingPosition, MoveType::Check);
	}

	showMoves(mainBoard, position, true, callback);
}

void Engine::showMoves(Board& board, Vector2 <size_t> position, bool protectKing,
						const std::function <void(Vector2 <size_t>, MoveType)>& callback)
{
	auto show = [this, &board, &callback, protectKing]
	(Vector2 <size_t> from, Vector2 <size_t> to, MoveType m)
	{
		//	If there's a check that should not happen, don't reveal it
		if(protectKing && leadsToCheck(board, from, to))
		{
			SDL_Log("Moving '%s' from (%lu, %lu) to (%lu, %lu) causes a check", name(board.at(from).piece), from.x, from.y, to.x, to.y);
			return;
		}

		callback(to, m);
	};

	Tile t = board.data[position.x][position.y];

	//	Some large number that's way larger than the board
	size_t steps = 1000;

	//	Which way can the piece go
	bool slant = false;
	bool straight = false;

	//	TODO validate position
	switch(board.data[position.x][position.y].piece)
	{
		case PieceName::Pawn:
		{
			//	Determine whether the pawn can move 1 or 2 steps
			steps = 1 + (position >= players[t.playerID].pawnSpawnStart && position <= players[t.playerID].pawnSpawnEnd);

			Vector2 <int>& direction = players[t.playerID].pawnDirection;
			Vector2 <size_t> current = position;

			for(size_t i = 0; i < steps && board.isInside(position); i++)
			{
				//	Captures can be done on the origin tile
				if(i == 0)
				{
					Vector2 <size_t> sides[2]
					{
						current + direction + players[t.playerID].inverseDirection,
						current + direction - players[t.playerID].inverseDirection
					};

					//	If a capture can be made, reveal it
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
		case PieceName::King: steps = 1; slant = true; straight = true; break;

		case PieceName::Knight:
		{
			//	Hardcoded knight movements
			const Vector2 <size_t> moves[]
			{
				position + Vector2 <size_t> (-1, -2), position + Vector2 <size_t> (1, -2),
				position + Vector2 <size_t> (2, -1), position + Vector2 <size_t> (2, 1),
				position + Vector2 <size_t> (1, 2), position + Vector2 <size_t> (-1, 2),
				position + Vector2 <size_t> (-2, -1), position + Vector2 <size_t> (-2, 1),
				position + Vector2 <size_t> (-1, 2)
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

	const Vector2 <size_t> directions[]
	{
		//	Straight
		Vector2 <size_t> (-1, 0),
		Vector2 <size_t> (0, -1),
		Vector2 <size_t> (0, 1),
		Vector2 <size_t> (1, 0),

		//	Slant
		Vector2 <size_t> (-1, -1),
		Vector2 <size_t> (1, -1),
		Vector2 <size_t> (-1, 1),
		Vector2 <size_t> (1, 1)
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
		Vector2 <size_t> current = position;
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

bool Engine::leadsToCheck(Board& board, Vector2 <size_t> from, Vector2 <size_t> to)
{
	//	Save the old state
	Tile oldFromTile = board.at(from);
	Tile oldToTile = board.at(to);

	//	Save the old check information
	std::vector <bool> oldThreatens(players.size());
	for(size_t i = 0; i < players.size(); i++)
		oldThreatens[i] = players[i].kingThreatened;

	//	Perform a fake move
	board.at(to) = board.at(from);
	board.at(from).piece = PieceName::None;

	//	Check if any kings are threatened
	flagThreatenedKings(board);
	bool result = players[currentPlayer].kingThreatened;

	//	Reset the old state
	board.data[from.x][from.y] = oldFromTile;
	board.data[to.x][to.y] = oldToTile;

	//	Reset the old threatens
	for(size_t i = 0; i < players.size(); i++)
		players[i].kingThreatened = oldThreatens[i];

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
			size_t playerID = board.data[x][y].playerID;

			//	Ignore tiles without a piece
			if(board.data[x][y].piece == PieceName::None)
				continue;

			//	Get every move that the piece in this tile can make
			showMoves(board, Vector2 <size_t> (x, y), false,
			[this, &board, playerID](Vector2 <size_t> pos, MoveType type)
			{
				Tile t = board.data[pos.x][pos.y];

				//	If the colors match or there's no capture, the move is irrelevant
				if(t.playerID == playerID || type != MoveType::Capture)
					return;

				//	Is the target piece a king
				if(t.piece == PieceName::King)
					players[t.playerID].kingThreatened = true;
			});
		}
	}
}

Engine::Tile& Engine::Board::at(const Vector2 <size_t>& position)
{
	return data[position.x][position.y];
}

bool Engine::Board::occupied(const Vector2 <size_t>& position)
{
	return data[position.x][position.y].piece != PieceName::None;
}

bool Engine::Board::isInside(const Vector2 <size_t>& position)
{
	return	position >= Vector2 <size_t> () && 
			position < size;
}
