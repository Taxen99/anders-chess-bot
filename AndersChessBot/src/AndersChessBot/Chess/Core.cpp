#include "Core.hpp"
#include <_ctype.h>

namespace Chess {

const Piece Piece::NoPiece = {Piece::None, Piece::Black};
const Move Move::NoMove = {200, 200};

std::string Move::ToString() const {
  return SquareToString(startSquare) + SquareToString(endSquare);
}

char Piece::ToChar() const {
  char c = '\\';
  switch (type) {
  case King:
    c = 'k';
    break;
  case Pawn:
    c = 'p';
    break;
  case Knight:
    c = 'n';
    break;
  case Bishop:
    c = 'b';
    break;
  case Rook:
    c = 'r';
    break;
  case Queen:
    c = 'q';
    break;
  case None:
    c = '.';
    break;
  };
  if (color == Color::White)
    c = toupper(c);
  return c;
}

} // namespace Chess