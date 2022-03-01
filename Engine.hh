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

class Engine
{
public:
	Engine();

	struct Tile
	{
		Tile(PieceName piece, size_t id) : piece(piece), playerID(id) {}

		PieceName piece;
		size_t playerID;
	};

	Tile at(size_t x, size_t y);
	size_t getCurrentTurn() { return currentPlayer; }
	Vector2 <size_t> getBoardSize() { return mainBoard.size; }

	void move(const Vector2 <size_t>& from, const Vector2 <size_t>& to);

	void legalMoves(Vector2 <size_t> position,
					const std::function <void(Vector2 <size_t>, MoveType)>& callback);

private:
	struct Player
	{
		bool kingMoved = false;
		bool kingThreatened = false;
		bool rookMoved[2] { false, false };

		Vector2 <size_t> kingPosition;
		Vector2 <int> inverseDirection;
		Vector2 <int> pawnDirection;

		Vector2 <size_t> pawnSpawnStart;
		Vector2 <size_t> pawnSpawnEnd;
	};

	struct Board
	{
		bool occupied(const Vector2 <size_t>& position);
		bool isInside(const Vector2 <size_t>& position);

		Tile& at(const Vector2 <size_t>& position);
		std::vector <std::vector <Tile>> data;

		Vector2 <size_t> size;
	};

	void createPlayer(Vector2 <size_t> kingPosition, Vector2 <size_t> middle);
	void flagThreatenedKings(Board& board);
	bool leadsToCheck(Board& board, Vector2 <size_t> from, Vector2 <size_t> to);
	bool canCastle(Board& board, Player& player, Vector2 <size_t>& position, bool queenSide);

	void legalMoves(Board& board, Vector2 <size_t> position, bool protectKing,
					const std::function <void(Vector2 <size_t>, MoveType)>& callback);

	void move(Board& board, const Vector2 <size_t>& from, Vector2 <size_t> to);

	std::vector <Player> players;
	std::vector <Board> boardBuffer;

	size_t currentPlayer = 0;
	Board mainBoard;
};
 
#endif
