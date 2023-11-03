#include <iostream>
#include "Chess/MoveGenerator.hpp"
#include "Chess/Core.hpp"
#include "Chess/PreCalculated.hpp"
#include "Chess/Position.hpp"
#include "Dom/Dom.hpp"
#include "Chess/Perft.hpp"
#include "Engine/Engine.hpp"
#include <emscripten/emscripten.h>
#include "Book/Book.hpp"
#include "Chess/Zobrist.hpp"

extern "C"
{
#include "Chess/magicmoves/magicmoves.h"
}

using namespace Chess;
int marked = -1;
MoveList moveList{};
PositionStack stack{};
bool ai = true;
Book::OpeningBook book;

class AddToMoveList
{
public:
	// static void OnFindMove(const Move& move)
	// {
	// 	moveList.push_back(move);
	// }
	template<Piece::Color C, bool EP, U8 DEPTH, bool OC>
	static inline void Node(PositionStack& stack, const Move& move)
	{
		constexpr U8 depth = DEPTH - 1;
		MoveGenerator::EnumerateMoves<AddToMoveList, C, EP, depth, OC>(stack, move);
	}
	static inline void LeafNode(PositionStack& stack, const Move& move)
	{
		moveList.push_back(move);
	}
};

void aiMove(void*)
{
	Move bookMove{};
	if(stack.size() < Book::NUM_MOVES_IN_BOOK)
	{
		bookMove = book.LookUpPosition(stack.top());
		if(Move::Valid(bookMove))
		{
			std::cout << "Playing from Book\n";
			MakeMove(stack, bookMove);
			DOM::DisplayPosition(stack.top());
			moveList.clear();
			MoveGenerator::EnumerateMoves<AddToMoveList, 1>(stack);
			return;
		}
	}
	std::cout << "Calculating best move\n";
	Engine::bestMove = Move::NoMove;
	Timer t("Test");
	t.Start();
	Engine::Evaluation eval = Engine::Search<6, 0>(stack, Engine::NegativeInfinity, Engine::Infinity);
	t.End();
	const Move& bestMove = Engine::bestMove;
	if(!Move::Valid(bestMove))
	{
		std::cout << "no ai moves" << std::endl;
		return;
	}
	std::cout << "EVAL: " << eval << std::endl;
	MakeMove(stack, bestMove);
	DOM::DisplayPosition(stack.top());
	moveList.clear();
	MoveGenerator::EnumerateMoves<AddToMoveList, 1>(stack);
	std::cout << "Positions searched: " << Engine::numPositionsSearched << std::endl;
}

// #define DBG_BOOK

int main()
{
	initmagicmoves();
	PreCalculateMoveData();
	Zobrist::InitRandomNumbers();

	book.LoadGameDatabase("game_database/lichess_elite_2019-05.pgn");

	#ifdef DBG_BOOK

	// DOM::OnKeyDown = [](U16 keyCode){
	// 	DOM::UnMarkAll();
	// 	DOM::DisplayPosition(book.tempStack.top());
	// 	std::cout << Zobrist::Hash(book.tempStack.top()) << std::endl;
	// 	std::cout << book.tempStack.top().zobrist << std::endl;
	// 	PopPosition(book.tempStack);
	// };

	return 0;

	#else

	DOM::OnClickSquare = [](U8 squareIndex){
		if(marked != -1 && HasMove(moveList, marked, squareIndex)) {
			MakeMove(stack, FindMove(moveList, marked, squareIndex));
			DOM::DisplayPosition(stack.top());
			moveList.clear();
			MoveGenerator::EnumerateMoves<AddToMoveList, 1>(stack);
			// if(moveList.size() == 0) std::cout << "No More Moves" << std::endl;
			marked = -1;

			if(ai)
				emscripten_async_call(aiMove, nullptr, 25);

			// DOM::UnMarkAll();

			// VisualizeBitBoard(1ULL << bestMove.startSquare);
			// VisualizeBitBoard(1ULL << bestMove.endSquare);
		}
		else if(HasPieceAt(stack.top(), squareIndex) && (marked != squareIndex))
		{
			marked = squareIndex;
		}
		else
		{
			marked = -1;
		}

		DOM::UnMarkAll();
		if(marked != -1)
		{
			VisualizeMoves(moveList, marked);
		}
	};

	DOM::OnKeyDown = [](U16 keyCode){
		// std::cout << keyCode << std::endl;
		if(keyCode == 81)
		{
			PopPosition(stack);
			DOM::DisplayPosition(stack.top());
			moveList.clear();
			MoveGenerator::EnumerateMoves<AddToMoveList, 1>(stack);
		}
		if(keyCode == 65)
		{
			Timer t("Test");
			t.Start();
			Engine::Evaluation eval = Engine::Search<4, 0>(stack, Engine::NegativeInfinity, Engine::Infinity);
			t.End();
			const Move& bestMove = Engine::bestMove;
			std::cout << "EVAL: " << eval << std::endl;
			DOM::MarkSquare(bestMove.startSquare);
			DOM::MarkSquare(bestMove.endSquare);
			std::cout << "Positions searched: " << Engine::numPositionsSearched << std::endl;
		}
		if(keyCode == 87)
		{
			VisualizeBitBoard(stack.top().colorBB[stack.top().turn]);
		}
		if(keyCode == 84)
			ai = !ai;
		if(keyCode == 91)
		{
			// std::cout << Zobrist::Hash(stack.top()) << std::endl;
			// std::cout << stack.top().zobrist << std::endl;

			const auto bookMove = book.LookUpPosition(stack.top());
			if(Move::Valid(bookMove))
			{
				DOM::UnMarkAll();
				DOM::MarkSquare(bookMove.startSquare, true);
				DOM::MarkSquare(bookMove.endSquare);
			}
			else
			{
				std::cout << "position not found in book\n";
			}
		}
	};

	LoadFEN(stack, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	DOM::DisplayPosition(stack.top());
	MoveGenerator::EnumerateMoves<AddToMoveList, 1>(stack);

	// emscripten_async_call(aiMove, nullptr, 25);

	// RunPerftTest<8>("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", "Perft");
	// RunPerftTest<6>("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", "Perft");

	return 0;
	#endif
}