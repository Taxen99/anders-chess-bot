#include "Zobrist.hpp"

namespace Chess {

U64 Zobrist::SideToMove = 0;
U64 Zobrist::Pieces[6][2][64] = {};
U64 Zobrist::Castling[4] = {};
U64 Zobrist::EnPassant[8] = {};

#define RND dis(gen)

void Zobrist::InitRandomNumbers() {
  std::default_random_engine rd{1678004918};
  std::mt19937_64 gen(rd());
  std::uniform_int_distribution<uint64_t> dis;
  SideToMove = RND;

  Castling[0] = RND;
  Castling[1] = RND;
  Castling[2] = RND;
  Castling[3] = RND;

  EnPassant[0] = RND;
  EnPassant[1] = RND;
  EnPassant[2] = RND;
  EnPassant[3] = RND;
  EnPassant[4] = RND;
  EnPassant[5] = RND;
  EnPassant[6] = RND;
  EnPassant[7] = RND;

  for (size_t p = 0; p < 6; p++) {
    for (size_t c = 0; c < 2; c++) {
      for (size_t s = 0; s < 64; s++) {
        Pieces[p][c][s] = RND;
      }
    }
  }
}

U64 Zobrist::Hash(const Position &position) {
  U64 zobrist = 0;

  for (size_t i = 0; i < 64; i++) {
    const auto &piece = position.pieces[i];
    if (piece.type != Piece::None) {
      zobrist ^= Pieces[piece.type][piece.color][i];
    }
  }
  if (position.turn == Piece::Black) {
    zobrist ^= SideToMove;
  }
  if (position.enPassantBB) {
    zobrist ^= EnPassant[bitScanForward(position.enPassantBB) &
                         7]; // (EP index) mod 8 (aka ep file index)
  }
  if (position.castlingRights[Piece::White]
          .kingSide) // BBWW (0 + 2c)k (1 + 2c)q
    zobrist ^= Castling[2];
  if (position.castlingRights[Piece::White].queenSide)
    zobrist ^= Castling[3];
  if (position.castlingRights[Piece::Black].kingSide)
    zobrist ^= Castling[0];
  if (position.castlingRights[Piece::Black].queenSide)
    zobrist ^= Castling[1];

  return zobrist;
}

} // namespace Chess