#include "Book/Book.hpp"
#include "Chess/Core.hpp"
#include "Chess/MoveGenerator.hpp"
#include "Chess/Perft.hpp"
#include "Chess/Position.hpp"
#include "Chess/PreCalculated.hpp"
#include "Chess/Zobrist.hpp"
#include "Engine/Engine.hpp"
#include "Glue/Glue.hpp"
#include <emscripten/emscripten.h>
#include <iostream>

extern "C" {
#include "Chess/magicmoves/magicmoves.h"
}

Chess::Book::OpeningBook book;
struct Game {
  Chess::PositionStack stack{};
};
Game game;
Chess::MoveList moveList{};

class AddToMoveList {
public:
  template <Chess::Piece::Color C, bool EP, Chess::U8 DEPTH, bool OC>
  static inline void Node(Chess::PositionStack &stack,
                          const Chess::Move &move) {
    constexpr Chess::U8 depth = DEPTH - 1;
    Chess::MoveGenerator::EnumerateMoves<AddToMoveList, C, EP, depth, OC>(stack,
                                                                          move);
  }
  static inline void LeafNode(Chess::PositionStack &stack,
                              const Chess::Move &move) {
    moveList.push_back(move);
  }
};

int main() {
  initmagicmoves();
  Chess::PreCalculateMoveData();
  Chess::Zobrist::InitRandomNumbers();

  book.LoadGameDatabase("game_database/lichess_elite_2019-05.pgn");

  Glue::Callbacks::StartGame = []() {
    game = Game{Chess::PositionStack{}};
    Chess::LoadFEN(game.stack,
                   "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  };

  Glue::Callbacks::MakeMove = [](Glue::Callbacks::Move move) {
    moveList.clear();
    Chess::MoveGenerator::EnumerateMoves<AddToMoveList, 1>(game.stack);
    const auto real_move =
        Chess::FindMove(moveList, move.start_square, move.end_square);
    Chess::MakeMove(game.stack, real_move);
  };

  Glue::Callbacks::Think =
      [](Glue::Callbacks::ThinkArgs args) -> Glue::Callbacks::ThinkResult {
    Chess::Move bookMove{};
    if (game.stack.size() < Chess::Book::NUM_MOVES_IN_BOOK) {
      bookMove = book.LookUpPosition(game.stack.top());
      if (Chess::Move::Valid(bookMove)) {
        std::cout << "Playing from Book\n";
        return Glue::Callbacks::ThinkResult{
            .best_move =
                Glue::Callbacks::Move{
                    bookMove.startSquare,
                    bookMove.endSquare,
                },
            .eval = NAN,
        };
      }
    }
    std::cout << "Calculating best move\n";
    Chess::Engine::bestMove = Chess::Move::NoMove;
    Chess::Timer t("Test");
    std::cout << "FIX DEPTH IS ALWAYS 6" << std::endl;
    t.Start();
    Chess::Engine::Evaluation eval = Chess::Engine::Search<6, 0>(
        game.stack, Chess::Engine::NegativeInfinity, Chess::Engine::Infinity);
    t.End();
    const Chess::Move &bestMove = Chess::Engine::bestMove;
    if (!Chess::Move::Valid(bestMove)) {
      std::cout << "no ai moves" << std::endl;
      return Glue::Callbacks::ThinkResult{
          .best_move =
              Glue::Callbacks::Move{
                  -1,
                  -1,
              },
          .eval = (double)eval,
      };
    }
    return Glue::Callbacks::ThinkResult{
        .best_move =
            Glue::Callbacks::Move{
                bestMove.startSquare,
                bestMove.endSquare,
            },
        .eval = (double)eval,
    };
  };
  Glue::Callbacks::ValidMoves = [](int start_square) -> std::vector<int> {
    std::cout << "start_square = " << start_square << std::endl;
    moveList.clear();
    Chess::MoveGenerator::EnumerateMoves<AddToMoveList, 1>(game.stack);
    std::vector<int> moves{};
    for (const Chess::Move &move : moveList) {
      if (move.startSquare == start_square) {
        moves.push_back(move.endSquare);
        std::cout << "move.endSquare = " << (Chess::U64)move.endSquare
                  << std::endl;
      }
    };
    return moves;
  };
  Glue::Callbacks::QueryBoard = []() -> std::array<int, 64> {
    std::array<int, 64> pieces{};
    int i = 0;
    for (const Chess::Piece piece : game.stack.top().pieces) {
      pieces[i++] =
          (piece.color == Chess::Piece::Color::Black ? 16 : 8) + piece.type + 1;
    }
    return pieces;
  };
}

// using namespace Chess;
// int marked = -1;
// MoveList moveList{};
// PositionStack stack{};
// bool ai = true;
// Book::OpeningBook book;

// class AddToMoveList
// {
// public:
// 	// static void OnFindMove(const Move& move)
// 	// {
// 	// 	moveList.push_back(move);
// 	// }
// 	template<Piece::Color C, bool EP, U8 DEPTH, bool OC>
// 	static inline void Node(PositionStack& stack, const Move& move)
// 	{
// 		constexpr U8 depth = DEPTH - 1;
// 		MoveGenerator::EnumerateMoves<AddToMoveList, C, EP, depth,
// OC>(stack, move);
// 	}
// 	static inline void LeafNode(PositionStack& stack, const Move& move)
// 	{
// 		moveList.push_back(move);
// 	}
// };

// void aiMove(void*)
// {
// 	Move bookMove{};
// 	if(stack.size() < Book::NUM_MOVES_IN_BOOK)
// 	{
// 		bookMove = book.LookUpPosition(stack.top());
// 		if(Move::Valid(bookMove))
// 		{
// 			std::cout << "Playing from Book\n";
// 			MakeMove(stack, bookMove);
// 			DOM::DisplayPosition(stack.top());
// 			moveList.clear();
// 			MoveGenerator::EnumerateMoves<AddToMoveList, 1>(stack);
// 			return;
// 		}
// 	}
// 	std::cout << "Calculating best move\n";
// 	Engine::bestMove = Move::NoMove;
// 	Timer t("Test");
// 	t.Start();
// 	Engine::Evaluation eval = Engine::Search<6, 0>(stack,
// Engine::NegativeInfinity, Engine::Infinity); 	t.End(); 	const Move&
// bestMove = Engine::bestMove; 	if(!Move::Valid(bestMove))
// 	{
// 		std::cout << "no ai moves" << std::endl;
// 		return;
// 	}
// 	std::cout << "EVAL: " << eval << std::endl;
// 	MakeMove(stack, bestMove);
// 	DOM::DisplayPosition(stack.top());
// 	moveList.clear();
// 	MoveGenerator::EnumerateMoves<AddToMoveList, 1>(stack);
// 	std::cout << "Positions searched: " << Engine::numPositionsSearched <<
// std::endl;
// }

// // #define DBG_BOOK

// int main()
// {
// 	initmagicmoves();
// 	PreCalculateMoveData();
// 	Zobrist::InitRandomNumbers();

// 	book.LoadGameDatabase("game_database/lichess_elite_2019-05.pgn");

// 	#ifdef DBG_BOOK

// 	// DOM::OnKeyDown = [](U16 keyCode){
// 	// 	DOM::UnMarkAll();
// 	// 	DOM::DisplayPosition(book.tempStack.top());
// 	// 	std::cout << Zobrist::Hash(book.tempStack.top()) << std::endl;
// 	// 	std::cout << book.tempStack.top().zobrist << std::endl;
// 	// 	PopPosition(book.tempStack);
// 	// };

// 	return 0;

// 	#else

// 	DOM::OnClickSquare = [](U8 squareIndex){
// 		if(marked != -1 && HasMove(moveList, marked, squareIndex)) {
// 			MakeMove(stack, FindMove(moveList, marked,
// squareIndex)); 			DOM::DisplayPosition(stack.top());
// moveList.clear(); 			MoveGenerator::EnumerateMoves<AddToMoveList, 1>(stack);
// 			// if(moveList.size() == 0) std::cout << "No More Moves"
// << std::endl; 			marked = -1;

// 			if(ai)
// 				emscripten_async_call(aiMove, nullptr, 25);

// 			// DOM::UnMarkAll();

// 			// VisualizeBitBoard(1ULL << bestMove.startSquare);
// 			// VisualizeBitBoard(1ULL << bestMove.endSquare);
// 		}
// 		else if(HasPieceAt(stack.top(), squareIndex) && (marked !=
// squareIndex))
// 		{
// 			marked = squareIndex;
// 		}
// 		else
// 		{
// 			marked = -1;
// 		}

// 		DOM::UnMarkAll();
// 		if(marked != -1)
// 		{
// 			VisualizeMoves(moveList, marked);
// 		}
// 	};

// 	DOM::OnKeyDown = [](U16 keyCode){
// 		// std::cout << keyCode << std::endl;
// 		if(keyCode == 81)
// 		{
// 			PopPosition(stack);
// 			DOM::DisplayPosition(stack.top());
// 			moveList.clear();
// 			MoveGenerator::EnumerateMoves<AddToMoveList, 1>(stack);
// 		}
// 		if(keyCode == 65)
// 		{
// 			Timer t("Test");
// 			t.Start();
// 			Engine::Evaluation eval = Engine::Search<4, 0>(stack,
// Engine::NegativeInfinity, Engine::Infinity); 			t.End();
// const Move& bestMove = Engine::bestMove; 			std::cout <<
// "EVAL: " << eval << std::endl; 			DOM::MarkSquare(bestMove.startSquare);
// 			DOM::MarkSquare(bestMove.endSquare);
// 			std::cout << "Positions searched: " <<
// Engine::numPositionsSearched << std::endl;
// 		}
// 		if(keyCode == 87)
// 		{
// 			VisualizeBitBoard(stack.top().colorBB[stack.top().turn]);
// 		}
// 		if(keyCode == 84)
// 			ai = !ai;
// 		if(keyCode == 91)
// 		{
// 			// std::cout << Zobrist::Hash(stack.top()) << std::endl;
// 			// std::cout << stack.top().zobrist << std::endl;

// 			const auto bookMove = book.LookUpPosition(stack.top());
// 			if(Move::Valid(bookMove))
// 			{
// 				DOM::UnMarkAll();
// 				DOM::MarkSquare(bookMove.startSquare, true);
// 				DOM::MarkSquare(bookMove.endSquare);
// 			}
// 			else
// 			{
// 				std::cout << "position not found in book\n";
// 			}
// 		}
// 	};

// 	LoadFEN(stack, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0
// 1"); 	DOM::DisplayPosition(stack.top());
// 	MoveGenerator::EnumerateMoves<AddToMoveList, 1>(stack);

// 	// emscripten_async_call(aiMove, nullptr, 25);

// 	// RunPerftTest<8>("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -", "Perft");
// 	// RunPerftTest<6>("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq -
// 0 1", "Perft");

// 	return 0;
// 	#endif
// }