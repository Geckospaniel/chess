#include "ChessGUI.hh"
#include "BitImage.hh"

static uint64_t getPieceImage(Chess::PieceName p)
{
	switch(p)
	{
		case Chess::PieceName::Pawn: return 13542615986929664UL;
		case Chess::PieceName::King: return 877951994586394672UL;
		case Chess::PieceName::Queen: return 227185009270878220UL;
		case Chess::PieceName::Bishop: return 9039977631055872UL;
		case Chess::PieceName::Knight: return 15657040604704768UL;
		case Chess::PieceName::Rook: return 9288468072107520UL;

		case Chess::PieceName::None: return 0UL;
	}
}

ChessGUI::ChessGUI() : e(8, 8)
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
	//e.addPlayer(kingPos3, middle, false);
	//e.addPlayer(kingPos4, middle, false);
}

ChessGUI::~ChessGUI() {}

void ChessGUI::onRender()
{
	Vector2 <size_t> boardSize = e.getBoardSize();
	Vec2 tileSize = Vec2(1.0f, 1.0f) / boardSize.as <float> ();

	if(askPromotion)
	{
		window(WindowID::Main).clear(0, 0, 0);
		window(WindowID::Main).setColor(255, 255, 255);

		size_t start = static_cast <size_t> (Chess::PieceName::Bishop);
		size_t end = static_cast <size_t> (Chess::PieceName::Queen);

		for(size_t i = start; i <= end; i++)
		{
			Vec2 offset = tileSize * Vec2(i, i);
			BitImage::render(window(WindowID::Main), offset, tileSize, getPieceImage(static_cast <Chess::PieceName> (i)));
		}

		window(WindowID::Main).render();
		return;
	}

	window(WindowID::Main).clear(199, 172, 143);
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
			uint64_t value = getPieceImage(current.piece);	

			if(value == 0UL)
				continue;
			
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

		//	Handle the promotion screen input
		if(askPromotion)
		{
			size_t start = static_cast <size_t> (Chess::PieceName::Bishop);
			size_t end = static_cast <size_t> (Chess::PieceName::Queen);

			for(size_t i = start; i <= end; i++)
			{
				Vec2 offset = tileSize * Vec2(i, i);
				Vec2 mouse = window(WindowID::Main).getMouse();

				//	Was the mouse clicked inside a piece
				if(mouse >= offset && mouse <= offset + tileSize)
				{
					e.promote(static_cast <Chess::PieceName> (i));
					askPromotion = false;
					break;
				}
			}

			return;
		}

		Vector2 <size_t> newSelection = (window(WindowID::Main).getMouse() / tileSize).as <size_t> ();
		size_t oldTurn = e.getCurrentTurn();

		for(auto& cache : cachedMoves)
		{
			if(cache.first == newSelection)
			{
				if(!e.move(selected, newSelection))
					askPromotion = true;

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
