#include "Perft.hpp"

namespace Chess {

U64 IncrementMoveTotal::count = 0;

// static U64 __Perft(PositionStack& stack, U8 depth)
// {
// 	MoveList moveList{};
// 	U64 nMoves = 0, totalMoves = 0;

// 	MoveGenerator::GenerateMoves(moveList, stack.top());
// 	nMoves = moveList.size();

// 	if(depth == 1)
// 		return nMoves;

// 	for(const Move& move : moveList)
// 	{
// 		MakeMove(stack, move);
// 		totalMoves += __Perft(stack, depth - 1);
// 		PopPosition(stack);
// 	}

// 	return totalMoves;
// }

// static U64 _Perft(PositionStack& stack, U8 depth)
// {
// 	MoveList moveList{};
// 	U64 nMoves = 0, totalMoves = 0;

// 	MoveGenerator::GenerateMoves(moveList, stack.top());
// 	nMoves = moveList.size();

// 	if(depth == 1)
// 		return nMoves;

// 	for(const Move& move : moveList)
// 	{
// 		MakeMove(stack, move);
// 		const U64 pMoves = __Perft(stack, depth - 1);
// 		PopPosition(stack);
// 		// char startFile = (move.startSquare & 7) + 97;
// 		// char startRank = (move.startSquare >> 3) + 49;
// 		// char endFile = (move.endSquare & 7) + 97;
// 		// char endRank = (move.endSquare >> 3) + 49;
// 		// std::cout << startFile << startRank << endFile << endRank <<
// ": " << pMoves << std::endl; 		totalMoves += pMoves;
// 	}

// 	return totalMoves;
// }

} // namespace Chess