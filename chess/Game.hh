#ifndef CHESS_GAME_HEADER
#define CHESS_GAME_HEADER

#include "../Vector2.hh"
#include "Player.hh"

#include <functional>
#include <cstddef>
#include <vector>

namespace Chess
{

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
	Capture
};

struct Tile
{
	Tile(PieceName piece, size_t id) : piece(piece), playerID(id) {}

	PieceName piece;
	size_t playerID;
};

class Game
{
public:
	Game(size_t boardWidth, size_t boardHeight);
	const Player& addPlayer(const Vec2s& kingPosition, const Vec2s& middle, bool isBot);

	Tile at(size_t x, size_t y);
	size_t getCurrentTurn() { return currentPlayer; }
	Vec2s getBoardSize() { return mainBoard.size; }

	void move(const Vec2s& from, const Vec2s& to);

	void getChecks(const std::function <void(Vec2s)>& callback);
	void legalMoves(Vec2s position, const std::function <void(Vec2s, MoveType)>& callback,
					bool protectKing = true);

private:
	struct Board
	{
		bool occupied(const Vec2s& position);
		bool isInside(const Vec2s& position);

		Tile& at(const Vec2s& position);
		std::vector <Tile> data;

		Vec2s size;
	};

	struct HistoryEntry
	{
		HistoryEntry(Vec2s from, Vec2s to, Tile change)
			: from(from), to(to), change(change) {}

		Vec2s from;
		Vec2s to;

		Tile change;
	};

	void createPlayer(Vec2s kingPosition, Vec2s middle);
	void flagThreatenedKings(Board& board, bool countLegalMoves);
	bool leadsToCheck(Board& board, Vec2s from, Vec2s to);

	bool canCastle(Board& board, Player& player, Vec2s& position, bool queenSide);
	bool canEnPassante(Board& board, Player& player, Vec2s& position, Vec2i direction);

	void legalMoves(Board& board, Vec2s position, bool protectKing,
					const std::function <void(Vec2s, MoveType)>& callback);

	void move(Board& board, const Vec2s& from, Vec2s to);

	std::vector <Player> players;
	std::vector <HistoryEntry> moveHistory;

	size_t currentPlayer = 0;
	Board mainBoard;
};
 
}

#endif
