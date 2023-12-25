#pragma once

#include "MoveGenerator.hpp"
#include "Position.hpp"

namespace Chess {

class IncrementMoveTotal {
public:
  static U64 count;
  static inline void Reset() { count = 0; }
  template <Piece::Color C, bool EP, U8 DEPTH, bool OC>
  static inline void Node(PositionStack &stack, const Move &move) {
    constexpr U8 depth = DEPTH - 1;
    MoveGenerator::EnumerateMoves<IncrementMoveTotal, C, EP, depth, OC>(stack,
                                                                        move);
  }
  static inline void LeafNode(PositionStack &stack, const Move &move) {
    count++;
  }
};

template <U8 depth> static U64 _Perft(PositionStack &stack) {
  IncrementMoveTotal::Reset();
  MoveGenerator::EnumerateMoves<IncrementMoveTotal, depth>(stack);
  return IncrementMoveTotal::count;
}

template <U8 depth> U64 Perft(const std::string &FEN) {
  PositionStack stack{};
  LoadFEN(stack, FEN);
  return _Perft<depth>(stack);
}

template <U8 depth> U64 Perft(PositionStack &stack) {
  return _Perft<depth>(stack);
}

template <U8 depth>
void RunPerftTest(const std::string &FEN, const std::string &label) {
  // for(int i = 0; i < maxDepth; i++)
  // {
  const auto label_ = label + " DEPTH " + std::to_string((depth));
  Timer timer(label_.c_str());
  timer.Start();
  U64 result = Perft<depth>(FEN);
  timer.End();
  std::cout << "RESULT = " << result << std::endl;
  // }
}

} // namespace Chess