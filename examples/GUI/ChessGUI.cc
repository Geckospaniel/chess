#include "ChessGUI.hh"
#include "BitImage.hh"

ChessGUI::ChessGUI() : e(12, 12)
{
	addWindow(2, 2);
	capFPS(30);

	Vec2s boardSize = e.getBoardSize();
	size_t centerLeft = boardSize.x / 2 - 1;

	Vec2s kingPos1(centerLeft, 0);
	Vec2s kingPos2(centerLeft, boardSize.y - 1);
	Vec2s kingPos3(0, boardSize.y / 2 - 1);
	Vec2s kingPos4(boardSize.x - 1, centerLeft);

	Vec2s middle(centerLeft, boardSize.y / 2);
	e.addPlayer(kingPos1, middle, false);
	e.addPlayer(kingPos2, middle, false);
	e.addPlayer(kingPos3, middle, false);
	e.addPlayer(kingPos4, middle, false);
}

ChessGUI::~ChessGUI() {}

void ChessGUI::onRender()
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

			Chess::Tile current = e.at(x, y);
			uint64_t value;	
			
			switch(current.piece)
			{
				case Chess::PieceName::Pawn: value = 13542615986929664UL; break;
				case Chess::PieceName::King: value = 877951994586394672UL; break;
				case Chess::PieceName::Queen: value = 227185009270878220UL; break;
				case Chess::PieceName::Bishop: value = 9039977631055872UL; break;
				case Chess::PieceName::Knight: value = 15657040604704768UL; break;
				case Chess::PieceName::Rook: value = 9288468072107520UL; break;

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

	e.getChecks([this, &tileSize](Vec2s pos)
	{
		window(WindowID::Main).setColor(255, 255, 0);

		Vec2 origin = tileSize * pos.as <float> ();
		window(WindowID::Main).drawBox(origin, tileSize, false, 1);
		window(WindowID::Main).drawLine(origin, origin + tileSize);
		window(WindowID::Main).drawLine(origin + Vec2(tileSize.x, 0.0f), origin + Vec2(0.0f, tileSize.y));

	});

	for(auto& cache : cachedMoves)
	{
		switch(cache.second)
		{
			case Chess::MoveType::Capture: window(WindowID::Main).setColor(255, 0, 0); break;
			case Chess::MoveType::Move: window(WindowID::Main).setColor(0, 255, 0); break;
		}

		Vec2 origin = tileSize * cache.first.as <float> ();
		window(WindowID::Main).drawBox(origin, tileSize, false, 1);
		window(WindowID::Main).drawLine(origin, origin + tileSize);
		window(WindowID::Main).drawLine(origin + Vec2(tileSize.x, 0.0f), origin + Vec2(0.0f, tileSize.y));
	}

	window(WindowID::Main).render();
}

void ChessGUI::cacheMoves()
{
	cachedMoves.clear();
	e.legalMoves(selected, [this](Vector2 <size_t> pos, Chess::MoveType type)
	{
		cachedMoves.push_back(std::make_pair(pos, type));
	});
}

void ChessGUI::onMouseClick(bool left, bool right)
{
	if(left)
	{
		Vec2 tileSize = Vec2(1.0f, 1.0f) / e.getBoardSize().as <float> ();
		Vector2 <size_t> newSelection = (window(WindowID::Main).getMouse() / tileSize).as <size_t> ();

		size_t oldTurn = e.getCurrentTurn();

		for(auto& cache : cachedMoves)
		{
			if(cache.first == newSelection)
			{
				//	FIXME for some reason this condition equals true multiple times occasionally
				e.move(selected, newSelection);
				cacheMoves();
				break;
			}
		}

		if(e.getCurrentTurn() != oldTurn)
			return;

		Chess::Tile current = e.at(newSelection.x, newSelection.y);

		if(current.piece != Chess::PieceName::None && e.getCurrentTurn() == current.playerID)
		{
			selected = newSelection;
			cacheMoves();
		}
	}
}
