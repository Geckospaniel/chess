#include "Chess.hh"
#include "BitImage.hh"

Chess::Chess()
{
	addWindow(2, 2);
	capFPS(30);
}

Chess::~Chess() {}

void Chess::onRender()
{
	Vector2 <size_t> boardSize = e.getBoardSize();
	Vec2 tileSize = Vec2(1.0f, 1.0f) / boardSize.as <float> ();
	window(WindowID::Main).clear(199, 172, 143);

	window(WindowID::Main).setColor(110, 79, 47);
	bool dark = false;

	Vec2 pieceSize = tileSize / 2;

	for(size_t x = 0; x < boardSize.x; x++, dark = !dark)
	{
		for(size_t y = 0; y < boardSize.y; y++, dark = !dark)
		{
			if(dark)
			{
				window(WindowID::Main).setColor(110, 79, 47);
				window(WindowID::Main).drawBox(tileSize * Vec2(x, y), tileSize, true, 1);
			}

			if(selected.x == x && selected.y == y)
			{
				window(WindowID::Main).setColor(100, 0, 0);
				window(WindowID::Main).drawBox(tileSize * Vec2(x, y), tileSize, true, 1);
			}

			Engine::Tile current = e.at(x, y);
			uint64_t value;	
			
			switch(current.piece)
			{
				case PieceName::Pawn: value = 13542615986929664UL; break;
				case PieceName::King: value = 877951994586394672UL; break;
				case PieceName::Queen: value = 227185009270878220UL; break;
				case PieceName::Bishop: value = 9039977631055872UL; break;
				case PieceName::Knight: value = 15657040604704768UL; break;
				case PieceName::Rook: value = 9288468072107520UL; break;

				default: continue;
			}

			switch(current.playerID)
			{
				case 0: window(WindowID::Main).setColor(255, 255, 255); break;
				case 1: window(WindowID::Main).setColor(0, 0, 0); break;
				case 2: window(WindowID::Main).setColor(0, 255, 255); break;
				case 3: window(WindowID::Main).setColor(255, 0, 255); break;
			}

			BitImage::render(window(WindowID::Main), tileSize * Vec2(x, y) + (pieceSize / 2), pieceSize, value);
		}
	}

	for(auto& cache : cachedMoves)
	{
		switch(cache.second)
		{
			case MoveType::Capture: window(WindowID::Main).setColor(255, 0, 0); break;
			case MoveType::Move: window(WindowID::Main).setColor(0, 255, 0); break;
			case MoveType::Check: window(WindowID::Main).setColor(255, 255, 0); break;
		}

		Vec2 origin = tileSize * cache.first.as <float> ();
		window(WindowID::Main).drawBox(origin, tileSize, false, 1);
		window(WindowID::Main).drawLine(origin, origin + tileSize);
		window(WindowID::Main).drawLine(origin + Vec2(tileSize.x, 0.0f), origin + Vec2(0.0f, tileSize.y));
	}

	window(WindowID::Main).render();
}

void Chess::cacheMoves()
{
	cachedMoves.clear();
	e.legalMoves(selected, [this](Vector2 <size_t> pos, MoveType type)
	{
		cachedMoves.push_back(std::make_pair(pos, type));
	});
}

void Chess::onMouseClick(bool left, bool right)
{
	if(left)
	{
		Vec2 tileSize = Vec2(1.0f, 1.0f) / e.getBoardSize().as <float> ();
		Vector2 <size_t> newSelection = (window(WindowID::Main).getMouse() / tileSize).as <size_t> ();

		size_t oldTurn = e.getCurrentTurn();

		for(auto& cache : cachedMoves)
		{
			if(cache.first == newSelection && cache.second != MoveType::Check)
			{
				//	FIXME for some reason this condition equals true multiple times occasionally
				e.move(selected, newSelection);
				cacheMoves();
				break;
			}
		}

		if(e.getCurrentTurn() != oldTurn)
			return;

		Engine::Tile current = e.at(newSelection.x, newSelection.y);

		if(current.piece != PieceName::None && e.getCurrentTurn() == current.playerID)
		{
			selected = newSelection;
			cacheMoves();
		}
	}
}
