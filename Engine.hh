#ifndef ENGINE_HEADER
#define ENGINE_HEADER

#include "Vector2.hh"

#include <functional>
#include <cstddef>
#include <vector>

enum class PieceName
{
	None,
	Pawn,
	Bishop,
	Knight,
	Rook,
	Queen,
	King
};

enum class MoveType
{
	Move,
	Capture,
	Check
};

enum class Color
{
	White,
	Black
};

class Engine
{
public:
	Engine();

	struct Tile
	{
		PieceName piece;
		Color color;
	};

	Tile at(size_t x, size_t y);
	Color getCurrentTurn() { return currentTurn; }
	Vector2 <size_t> getBoardSize() { return mainBoard.size; }

	void move(const Vector2 <size_t>& from, const Vector2 <size_t>& to);

	void showMoves(Vector2 <size_t> position,
					const std::function <void(Vector2 <size_t>, MoveType)>& callback);

private:
	struct Board
	{
		bool occupied(const Vector2 <size_t>& position);
		bool isInside(const Vector2 <size_t>& position);

		std::vector <std::vector <Tile>> data;
		bool kingThreatened[2] { false, false };

		Vector2 <size_t> kingPosition[2];
		Vector2 <size_t> size;
	};

	void flagThreatenedKings(Board& board);
	bool leadsToCheck(Board& board, Vector2 <size_t> from, Vector2 <size_t> to);

	void showMoves(Board& board, Vector2 <size_t> position, bool protectKing,
					const std::function <void(Vector2 <size_t>, MoveType)>& callback);

	void move(Board& board, const Vector2 <size_t>& from, const Vector2 <size_t>& to);

	Color currentTurn = Color::White;
	std::vector <Board> boardBuffer;
	Board mainBoard;
};
 
#endif
