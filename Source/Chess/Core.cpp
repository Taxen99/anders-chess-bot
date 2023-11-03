#include "Core.hpp"
#include "../Dom/Dom.hpp"

namespace Chess
{

void VisualizeBitBoard(const U64 bitboard)
{
	for(int i = 0; i < 64; i++)
	{
		if(bitboard & (1ULL << i)) DOM::MarkSquare(i, true);
	}
}

void VisualizeMoves(const MoveList moveList, U8 pieceIndex)
{
	for(auto move : moveList)
	{
		if(move.startSquare == pieceIndex) DOM::MarkSquare(move.endSquare);
	}
}

const Piece Piece::NoPiece = { Piece::None, Piece::Black };
const Move Move::NoMove = { 200, 200 };

}