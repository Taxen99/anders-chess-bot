#pragma once

#include "Core.hpp"
#include <stack>

namespace Chess {

extern U64 KingMoves[64];
extern U64 KnightMoves[64];
extern U64 PawnAttacks[2][64];

struct Position {
  U64 pieceBB[6];
  U64 colorBB[2];
  U64 occupiedBB;
  U64 enPassantBB;
  U64 zobrist;
  Piece pieces[64];
  CastlingRights castlingRights[2];
  Piece::Color turn;

  template <Piece::Type T> inline U64 Get() const { return pieceBB[T]; }

  template <Piece::Color C> inline U64 Get() const { return colorBB[C]; }

  template <Piece::Color C, Piece::Type T> inline U64 Get() const {
    return pieceBB[T] & colorBB[C];
  }

  std::string ToString() const;
};

using PositionStack = std::stack<Position>;

void LoadFEN(PositionStack &stack, const std::string &FEN);
inline bool HasPieceAt(const Position &position, U8 index) {
  return position.pieces[index].type != Piece::None;
}
void CalculateCheckAndPinMask(const Position &position, U64 &checkMask,
                              U64 &pinHVMask, U64 &pinDIMask);
void MakeMove(PositionStack &stack, const Move &move);
void PopPosition(PositionStack &stack);

} // namespace Chess