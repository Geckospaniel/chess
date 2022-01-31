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
	for(auto& x : mainBoard.data)
	{
		for(auto& y : x)
			y.piece = PieceName::None;
	}

	size_t centerLeft = boardWidth / 2 - 1;

	for(size_t y = 1; y < boardHeight; y+=boardHeight - 3)
	{
		Color color = y != 1 ? Color::White : Color::Black;
		size_t back = y != 1 ? y + 1 : y - 1;

		//	Pawns
		for(size_t x = 0; x < boardWidth; x++)
		{
			mainBoard.data[x][y].color = color;
			mainBoard.data[x][y].piece = PieceName::Pawn;
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

	if(board.data[to.x][to.y].piece == PieceName::King)
		board.kingPosition[static_cast <size_t> (currentTurn)] = to;

	for(auto& threatened : board.kingThreatened)
		threatened = false;

	//	Ugly brute force to check if some piece can capture a king
	for(size_t x = 0; x < boardWidth; x++)
	{
		for(size_t y = 0; y < boardHeight; y++)
		{
			Color c = board.data[x][y].color;
			if(board.data[x][y].piece == PieceName::None)
				continue;

			SDL_Log("Testing %s '%s' at %lu %lu", board.data[x][y].color == Color::White ? "White" : "Black", name(board.data[x][y].piece), x, y);

			showMoves(board, Vector2 <size_t> (x, y), [this, &board, c](Vector2 <size_t> pos, MoveType type)
			{
				Tile t = board.data[pos.x][pos.y];

				if(c == t.color || type != MoveType::Capture)
					return;

				SDL_Log("Can capture %s '%s' at %lu %lu", t.color == Color::White ? "White" : "Black", name(t.piece), pos.x, pos.y);

				if(t.piece == PieceName::King)
					board.kingThreatened[static_cast <size_t> (t.color)] = true;
			});
		}
	}

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

	showMoves(mainBoard, position, callback);
}

void Engine::showMoves(Board& board, Vector2 <size_t> position,
						const std::function <void(Vector2 <size_t>, MoveType)>& callback)
{
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
			size_t middle = boardHeight / 2 - (t.color != Color::White);

			//	Determine whether the pawn can move 1 or 2 steps
			steps = 1 + (t.color == Color::White ? (position.y - 2 >= middle) : (position.y + 2 <= middle));
			int move = t.color == Color::White ? -1 : 1;

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

					for(auto& side : sides)
					{
						if(board.isInside(side) && board.occupied(side) && t.color != board.data[side.x][side.y].color)
							callback(side, MoveType::Capture);
					}
				}

				position.y += move;
				if(!board.isInside(position) || board.occupied(position))
					break;

				callback(position, MoveType::Move);
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
				position + Vector2 <size_t >(-1, -2), position + Vector2 <size_t >(1, -2),
				position + Vector2 <size_t >(2, -1), position + Vector2 <size_t >(2, 1),
				position + Vector2 <size_t >(1, 2), position + Vector2 <size_t >(-1, 2),
				position + Vector2 <size_t >(-2, -1), position + Vector2 <size_t >(-2, 1),
				position + Vector2 <size_t >(-1, 2)
			};

			for(auto& move : moves)
			{
				if(board.isInside(move))
				{
					if(board.occupied(move))
					{
						if(board.data[move.x][move.y].color != t.color)
							callback(move, MoveType::Capture);

						else continue;
					}

					else callback(move, MoveType::Move);
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
				if(board.data[current.x][current.y].color != t.color)
				{
					callback(current, MoveType::Capture);
					break;
				}

				else break;
			}

			callback(current, MoveType::Move);
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
			position < Vector2 <size_t> (Engine::boardWidth, Engine::boardHeight);
}
