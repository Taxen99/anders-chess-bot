#include "Protocol.hpp"
#include "AndersChessBot/Chess/Core.hpp"
#include "AndersChessBot/Chess/Position.hpp"
#include "AndersChessBot/Chess/Zobrist.hpp"
#include "AndersChessBot/Engine/Engine.hpp"
#include "AndersChessBot/Protocol/AddToMoveList.hpp"
#include "AndersChessBot/Protocol/State.hpp"
#include <cmath>

namespace Chess::Protocol {

bool Init() noexcept {
  try {
    initmagicmoves();
    PreCalculateMoveData();
    Zobrist::InitRandomNumbers();
    return true;
  } catch (...) {
    return false;
  }
}

bool SetupBoard(const char *FEN) noexcept {
  try {
    LoadFEN(State::GetInstance().PositionStack, FEN);
    return true;
  } catch (...) {
    return false;
  }
}
void PrintBoard() noexcept {
  try {
    std::cout << State::GetInstance().PositionStack.top().ToString()
              << std::endl;
  } catch (...) {
  }
  return;
}
bool MakeMove(Chess::Move move) noexcept {
  try {
    MakeMove(State::GetInstance().PositionStack, move);
    return true;
  } catch (...) {
    return false;
  }
}
MoveList QueryMoves() noexcept {
  try {
    AddToMoveList::moveList.clear();
    Chess::MoveGenerator::EnumerateMoves<AddToMoveList, 1>(
        State::GetInstance().PositionStack);
    return AddToMoveList::moveList;
  } catch (...) {
    return {};
  }
}
bool PopulateOpeningBook(const char *data, size_t len) noexcept {
  try {
    State &state = State::GetInstance();
    if (state.OpeningBook.Initialized()) {
      return false;
    }
    state.OpeningBook.LoadGamaDatabase(data, len);
    return true;
  } catch (...) {
    return false;
  }
}
static std::pair<Move, double> BestMoveAndEval() {
  State &state = State::GetInstance();
  std::cout << "Calculating best move\n";
  Engine::bestMove = Chess::Move::NoMove;
  Chess::Timer t("Test");
  std::cout << "FIX DEPTH IS ALWAYS 6" << std::endl;
  t.Start();
  Chess::Engine::Evaluation eval = Chess::Engine::Search<6, 0>(
      state.PositionStack, Chess::Engine::NegativeInfinity,
      Chess::Engine::Infinity);
  t.End();
  return {Chess::Engine::bestMove, eval};
}
double Evaluate() noexcept {
  try {
    const auto [bestMove, eval] = BestMoveAndEval();
    return eval;
  } catch (...) {
    return std::nan("");
  }
}
Move BestMove(int depth, bool playFromBook) noexcept {
  try {
    State &state = State::GetInstance();
    if (playFromBook) {
      if (state.PositionStack.size() < Chess::Book::NUM_MOVES_IN_BOOK) {
        Chess::Move bookMove =
            state.OpeningBook.LookUpPosition(state.PositionStack.top());
        if (Chess::Move::Valid(bookMove)) {
          std::cout << "Playing from Book\n";
          return bookMove;
        }
      }
    }
    const auto [bestMove, eval] = BestMoveAndEval();
    std::cout << "eval = " << eval << std::endl;
    if (!Chess::Move::Valid(bestMove)) {
      std::cout << "no ai moves" << std::endl;
      return Move::NoMove;
    };
    return bestMove;
  } catch (...) {
    return Move::NoMove;
  }
}

} // namespace Chess::Protocol