#include "Engine.hh"
#include <SDL2/SDL_log.h>

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
	mainBoard.data.resize(mainBoard.size.x, std::vector <Tile> (mainBoard.size.y, { PieceName::None }));

	//	Left-side tile from the center
	size_t centerLeft = mainBoard.size.x / 2 - 1;

	for(size_t y = 1; y < mainBoard.size.y; y += mainBoard.size.y - 3)
	{
		Color color = y != 1 ? Color::White : Color::Black;
		size_t back = y != 1 ? y + 1 : y - 1;

		//	Pawns
		for(size_t x = 0; x < 8; x++)
		{
			//	Because there's only 8 pawns, use centerLeft as an offset
			mainBoard.data[centerLeft - 3 + x][y].color = color;
			mainBoard.data[centerLeft - 3 + x][y].piece = PieceName::Pawn;
		}

		//	King
		mainBoard.data[centerLeft][back].color = color;
		mainBoard.data[centerLeft][back].piece = PieceName::King;
		mainBoard.kingPosition[static_cast <size_t>	(color)] = Vector2 <size_t> (centerLeft, back);

		//	Queen
		mainBoard.data[centerLeft + 1][back].color = color;
		mainBoard.data[centerLeft + 1][back].piece = PieceName::Queen;

		//	Rooks, Bishops and Knights
		for(int i = 1; i <= 3; i++)
		{
			PieceName piece = static_cast <PieceName> (static_cast <int> (PieceName::Pawn) + i);

			mainBoard.data[centerLeft - i][back].color = color;
			mainBoard.data[centerLeft - i][back].piece = piece;

			mainBoard.data[centerLeft + 1 + i][back].color = color;
			mainBoard.data[centerLeft + 1 + i][back].piece = piece;
		}
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

	SDL_Log("Moved %s '%s' to %lu %lu", board.data[to.x][to.y].color == Color::White ? "White" : "Black", name(board.data[to.x][to.y].piece), to.x, to.y);

	//	Cache the king position
	if(board.data[to.x][to.y].piece == PieceName::King)
		board.kingPosition[static_cast <size_t> (currentTurn)] = to;

	//	Since basically any move can trigger a check, check for those checks
	flagThreatenedKings(board);	

	//	Update the turn
	switch(currentTurn)
	{
		case Color::Black: currentTurn = Color::White; break;
		case Color::White: currentTurn = Color::Black; break;
	}
}

void Engine::showMoves(Vector2 <size_t> position,
						const std::function <void(Vector2 <size_t>, MoveType)>& callback)
{
	for(size_t i = 0; i < 2; i++)
	{
		if(mainBoard.kingThreatened[i])
			callback(mainBoard.kingPosition[i], MoveType::Check);
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
			SDL_Log("Moving '%s' from (%lu, %lu) to (%lu, %lu) causes a check", name(board.data[from.x][from.y].piece), from.x, from.y, to.x, to.y);
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
			//	Get the middle point
			size_t middle = board.size.y / 2 - (t.color != Color::White);

			//	Determine whether the pawn can move 1 or 2 steps
			//	TODO this is a bit overcomplicated since you could just check if the pawn hasn't moved
			steps = 1 + (t.color == Color::White ? (position.y - 2 >= middle) : (position.y + 2 <= middle));
			int move = t.color == Color::White ? -1 : 1;

			Vector2 <size_t> old = position;
			for(size_t i = 0; i < steps && board.isInside(position); i++)
			{
				//	Captures can be done on the origin tile
				if(i == 0)
				{
					Vector2 <size_t> sides[2]
					{
						position + Vector2 <size_t> (-1, move),
						position + Vector2 <size_t> (1, move)
					};

					//	If a capture can be made, reveal it
					for(auto& side : sides)
					{
						if(board.isInside(side) && board.occupied(side) && t.color != board.data[side.x][side.y].color)
							show(position, side, MoveType::Capture);
					}
				}

				//	Move the pawn and make sure that it could actually move
				position.y += move;
				if(!board.isInside(position) || board.occupied(position))
					break;

				//	Reveal the pawn movement
				show(old, position, MoveType::Move);
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
						if(board.data[move.x][move.y].color != t.color)
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
				if(board.data[current.x][current.y].color != t.color)
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
	Tile oldFromTile = board.data[from.x][from.y];
	Tile oldToTile = board.data[to.x][to.y];
	bool oldThreatens[2] { board.kingThreatened[0], board.kingThreatened[1] };

	//	Perform a fake move
	board.data[to.x][to.y] = board.data[from.x][from.y];
	board.data[from.x][from.y].piece = PieceName::None;

	//	Check if any kings are threatened
	flagThreatenedKings(board);
	bool result = board.kingThreatened[static_cast <size_t> (currentTurn)];

	//	Reset the old state
	board.data[from.x][from.y] = oldFromTile;
	board.data[to.x][to.y] = oldToTile;

	//	Reset the old threatens
	for(size_t i = 0; i < 2; i++)
		board.kingThreatened[i] = oldThreatens[i];

	//	Is the king of the current turn threatened
	return result;
}

void Engine::flagThreatenedKings(Board& board)
{
	for(auto& threatened : board.kingThreatened)
		threatened = false;

	//	Ugly brute force to check if some piece can capture a king
	for(size_t x = 0; x < board.size.x; x++)
	{
		for(size_t y = 0; y < board.size.y; y++)
		{
			Color c = board.data[x][y].color;

			//	Ignore tiles without a piece
			if(board.data[x][y].piece == PieceName::None)
				continue;

			//	Get every move that the piece in this tile can make
			showMoves(board, Vector2 <size_t> (x, y), false,
			[this, &board, c](Vector2 <size_t> pos, MoveType type)
			{
				Tile t = board.data[pos.x][pos.y];

				//	If the colors match or there's no capture, the move is irrelevant
				if(c == t.color || type != MoveType::Capture)
					return;

				//	Is the target piece a king
				if(t.piece == PieceName::King)
					board.kingThreatened[static_cast <size_t> (t.color)] = true;
			});
		}
	}
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
