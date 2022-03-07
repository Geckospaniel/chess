#ifndef CHESS_PLAYER_HEADER
#define CHESS_PLAYER_HEADER

#include "../Vector2.hh"

namespace Chess
{

struct Player
{
	bool kingMoved = false;
	bool kingCanCastle[2] { false, false };
	bool kingThreatened = false;
	bool rookMoved[2] { false, false };

	size_t possibleMoves;

	Vec2s kingPosition;
	Vec2i inverseDirection;
	Vec2i pawnDirection;

	Vec2s pawnSpawnStart;
	Vec2s pawnSpawnEnd;

	//	FIXME technically you could have multiple possible en passantes
	Vec2s enPassanteCapture;
};

}

#endif
