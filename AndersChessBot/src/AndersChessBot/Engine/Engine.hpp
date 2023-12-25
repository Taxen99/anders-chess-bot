#pragma once

#include "../Chess/MoveGenerator.hpp"
#include <limits>

namespace Chess::Engine {

class EngineMoveHandler {
public:
  static MoveList *moveList;

  static inline void LeafNode(PositionStack &stack, const Move &move) {
    moveList->push_back(move);
  }
};

using Evaluation = int32_t;

extern Evaluation bestEval;
extern Move bestMove;
extern U64 numPositionsSearched;

constexpr Evaluation Infinity = std::numeric_limits<Evaluation>::max();
constexpr Evaluation NegativeInfinity = -std::numeric_limits<Evaluation>::max();
constexpr Evaluation MateEval = NegativeInfinity + 100000;

Evaluation Evaluate(Chess::PositionStack &stack);
void OrderMoves(MoveList &moveList, const Position &position);

Evaluation QuietSearch(Chess::PositionStack &stack, Evaluation alpha,
                       Evaluation beta);

template <U8 depth, U8 depthFromRoot>
Evaluation Search(Chess::PositionStack &stack, Evaluation alpha,
                  Evaluation beta) {
  if constexpr (depthFromRoot == 0) {
    numPositionsSearched = 0;
  }
  if constexpr (depth == 0) {
    return QuietSearch(stack, alpha, beta);
    // return Evaluate(stack);
  } else {
    numPositionsSearched++;
    if constexpr (depthFromRoot > 0) {
      alpha = std::max(alpha, MateEval + depthFromRoot);
      beta = std::min(beta, -MateEval - depthFromRoot);
      if (alpha >= beta) {
        return beta;
      }
    }

    constexpr U8 depthMinusOne = depth - 1;
    constexpr U8 depthFromRootPlusOne = depthFromRoot + 1;
    MoveList moveList{};
    EngineMoveHandler::moveList = &moveList;
    MoveGenerator::EnumerateMoves<EngineMoveHandler, 1>(stack);

    if (moveList.empty()) {
      if (MoveGenerator::ComputedPositionData.check) {
        return MateEval + depthFromRoot;
      }
      return 0;
    }

    OrderMoves(moveList, stack.top());

    for (const Move &move : moveList) {
      MakeMove(stack, move);
      const Evaluation eval =
          -Search<depthMinusOne, depthFromRootPlusOne>(stack, -beta, -alpha);
      PopPosition(stack);

      if (eval >= beta) {
        return beta;
      }

      if (eval > alpha) {
        alpha = eval;
        if constexpr (depthFromRoot == 0) {
          bestMove = move;
        }
      }
    }

    return alpha;
  }
}

} // namespace Chess::Engine