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
	Vec2 tileSize = Vec2(1.0f, 1.0f) / Vec2(e.boardWidth, e.boardHeight);
	window(WindowID::Main).clear(199, 172, 143);

	window(WindowID::Main).setColor(110, 79, 47);
	bool dark = false;

	Vec2 pieceSize = tileSize / 2;

	for(size_t x = 0; x < e.boardWidth; x++, dark = !dark)
	{
		for(size_t y = 0; y < e.boardHeight; y++, dark = !dark)
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

			unsigned char color = (current.color == Color::White) * 255;
			window(WindowID::Main).setColor(color, color, color);
			BitImage::render(window(WindowID::Main), tileSize * Vec2(x, y) + (pieceSize / 2), pieceSize, value);
		}
	}

	e.showMoves(selected, [this, &tileSize](Vector2 <size_t> pos, MoveType type)
	{
		switch(type)
		{
			case MoveType::Capture: window(WindowID::Main).setColor(255, 0, 0); break;
			case MoveType::Move: window(WindowID::Main).setColor(0, 255, 0); break;
			case MoveType::Check: window(WindowID::Main).setColor(255, 255, 0); break;
		}

		Vec2 origin = tileSize * pos.as <float> ();
		window(WindowID::Main).drawBox(origin, tileSize, false, 1);
		window(WindowID::Main).drawLine(origin, origin + tileSize);
		window(WindowID::Main).drawLine(origin + Vec2(tileSize.x, 0.0f), origin + Vec2(0.0f, tileSize.y));
	});

	window(WindowID::Main).render();
}

void Chess::onMouseClick(bool left, bool right)
{
	if(left)
	{
		Vec2 tileSize = Vec2(1.0f, 1.0f) / Vec2(e.boardWidth, e.boardHeight);
		Vec2 mouse = window(WindowID::Main).getMouse() / tileSize;
		Vector2 <size_t> newSelection = mouse.as <size_t> ();

		Engine::Tile current = e.at(newSelection.x, newSelection.y);
		if(current.piece != PieceName::None && e.getCurrentTurn() == current.color)
			selected = newSelection;

		else
		{
			e.showMoves(selected, [this, &newSelection](Vector2 <size_t> pos, MoveType type)
			{
				if(pos == newSelection && type != MoveType::Check)
					e.move(selected, newSelection);
			});
		}
	}
}
