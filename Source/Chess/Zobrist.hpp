#pragma once

#include "Core.hpp"
#include "Position.hpp"
#include <random>

namespace Chess
{

class Zobrist
{
public:
	Zobrist() = default;
	static void InitRandomNumbers();
	static U64 Hash(const Position& position);

	static U64 SideToMove;
	static U64 Pieces[6][2][64];
	static U64 Castling[4];
	static U64 EnPassant[8];
private:
};

}