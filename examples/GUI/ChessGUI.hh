#ifndef CHESS_GUI_HEADER
#define CHESS_GUI_HEADER

#include "Application.hh"
#include "../../chess/Game.hh"

#include <vector>
#include <utility>

enum class WindowID
{
	Main
};

class ChessGUI : public Application
{
public:
	ChessGUI();
	~ChessGUI();

	void onRender() override;
	void onMouseClick(bool left, bool right) override;

private:
	Vector2 <size_t> selected;
	Chess::Game e;

	void cacheMoves();
	std::vector <std::pair <Vector2 <size_t>, Chess::MoveType>> cachedMoves;
};

#endif
