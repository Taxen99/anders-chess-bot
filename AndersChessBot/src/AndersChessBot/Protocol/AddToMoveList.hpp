#include "AndersChessBot/Chess/Core.hpp"
#include "AndersChessBot/Chess/MoveGenerator.hpp"
#include "AndersChessBot/Chess/Position.hpp"
namespace Chess::Protocol {
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
  static inline Chess::MoveList moveList{};
};

} // namespace Chess::Protocol