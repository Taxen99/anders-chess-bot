#include "Dom.hpp"
#include <emscripten.h>
#include <iostream>

namespace DOM {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdollar-in-identifier-extension"

std::function<void(Chess::U8)> OnClickSquare = [](int _) {};
std::function<void(Chess::U16)> OnKeyDown = [](int _) {};

extern "C" EMSCRIPTEN_KEEPALIVE void _OnClickSquare(int squareIndex) {
  // std::cout << "OnClickSquare Called, index: " << squareIndex << '\n';
  OnClickSquare(squareIndex);
}

extern "C" EMSCRIPTEN_KEEPALIVE void _OnKeyDown(int keyCode) {
  // std::cout << "OnClickSquare Called, index: " << squareIndex << '\n';
  OnKeyDown(keyCode);
}

void MarkSquare(Chess::U8 square, bool other) {
  EM_ASM({ Mark($0, $1); }, square, other);
}

void SetPiece(Chess::U8 index, Chess::Piece piece) {
  EM_ASM({ SetPiece($0, $1); }, index,
         (piece.type + 1) + (piece.color == Chess::Piece::Black ? 16 : 8));
}

void RemovePiece(Chess::U8 index) {
  EM_ASM({ SetPiece($0, 0); }, index);
}

void UnMarkAll() {
  EM_ASM({ RemoveMarkers(); });
}

static Chess::Piece internalPieces[64];

void DisplayPosition(const Chess::Position &position) {
  static bool init = false;
  if (!init) {
    init = true;
    for (int i = 0; i < 64; i++) {
      internalPieces[i] = Chess::Piece::NoPiece;
    }
    DEBUG_LOG("INIT INTERNAL PIECES");
  }

  for (int i = 0; i < 64; i++) {
    const Chess::Piece &piece = position.pieces[i];
    if (piece.type != internalPieces[i].type ||
        piece.color != internalPieces[i].color) {
      internalPieces[i] = piece;

      SetPiece(i, piece);

      DEBUG_LOG("UPDATED PIECE IN DOM");
    }
  }
}

#pragma GCC diagnostic pop

} // namespace DOM