#include "Position.hpp"
#include "PreCalculated.hpp"
#include "Zobrist.hpp"
#include <sstream>
#include <string>

namespace Chess {

extern U64 HVRays[64];
extern U64 DIRays[64];

inline void SetPieceAt(Position &position, U8 index, Piece piece) {
  const U64 bb = 1ULL << index;
  position.pieceBB[piece.type] |= bb;
  position.colorBB[piece.color] |= bb;
  position.occupiedBB |= bb;

  // DOM::SetPiece(index, piece);
  position.pieces[index] = piece;
}

void LoadFEN(PositionStack &stack, const std::string &FEN) {
  stack = PositionStack();
  stack.push({});
  Position &position = stack.top();

  for (int i = 0; i < 64; i++) {
    position.pieces[i] = Piece::NoPiece;
  }

  std::vector<std::string> sections;
  std::istringstream f(FEN);
  std::string section;
  while (std::getline(f, section, ' ')) {
    DEBUG_LOG_LABEL("FEN SECTION", section);
    sections.push_back(section);
  }
  U8 index = 56;
  for (const char &c : sections.at(0)) {
    switch (c) {
    case '/':
      index -= 16;
      break;
    case '1':
      index += 1;
      break;
    case '2':
      index += 2;
      break;
    case '3':
      index += 3;
      break;
    case '4':
      index += 4;
      break;
    case '5':
      index += 5;
      break;
    case '6':
      index += 6;
      break;
    case '7':
      index += 7;
      break;
    case '8':
      index += 8;
      break;
    case 'k':
      SetPieceAt(position, index, {Piece::King, Piece::Black});
      index++;
      break;
    case 'p':
      SetPieceAt(position, index, {Piece::Pawn, Piece::Black});
      index++;
      break;
    case 'n':
      SetPieceAt(position, index, {Piece::Knight, Piece::Black});
      index++;
      break;
    case 'b':
      SetPieceAt(position, index, {Piece::Bishop, Piece::Black});
      index++;
      break;
    case 'r':
      SetPieceAt(position, index, {Piece::Rook, Piece::Black});
      index++;
      break;
    case 'q':
      SetPieceAt(position, index, {Piece::Queen, Piece::Black});
      index++;
      break;
    case 'K':
      SetPieceAt(position, index, {Piece::King, Piece::White});
      index++;
      break;
    case 'P':
      SetPieceAt(position, index, {Piece::Pawn, Piece::White});
      index++;
      break;
    case 'N':
      SetPieceAt(position, index, {Piece::Knight, Piece::White});
      index++;
      break;
    case 'B':
      SetPieceAt(position, index, {Piece::Bishop, Piece::White});
      index++;
      break;
    case 'R':
      SetPieceAt(position, index, {Piece::Rook, Piece::White});
      index++;
      break;
    case 'Q':
      SetPieceAt(position, index, {Piece::Queen, Piece::White});
      index++;
      break;

    default:
      DEBUG_LOG_LABEL("ERROR IN FEN PARSING", c);
      break;
    }
  }
  if (sections.at(1) == "w") {
    position.turn = Piece::White;
    DEBUG_LOG("FEN TURN WHITE");
  } else if (sections.at(1) == "b") {
    position.turn = Piece::Black;
    DEBUG_LOG("FEN TURN BLACK");
  } else {
    DEBUG_LOG_LABEL("INVALID FEN TURN", sections.at(1));
  }

  const std::string &castles = sections.at(2);
  position.castlingRights[Piece::Black] = {.kingSide = false,
                                           .queenSide = false};
  position.castlingRights[Piece::White] = {.kingSide = false,
                                           .queenSide = false};
  if (castles == "-") {
    DEBUG_LOG("NO FEN CASTLING RIGHTS");
  } else {
    if (castles.find('k') != std::string::npos) {
      DEBUG_LOG("FEN BLACK CAN CASTLE KINGSIDE");
      position.castlingRights[Piece::Black].kingSide = true;
    }
    if (castles.find('K') != std::string::npos) {
      DEBUG_LOG("FEN WHITE CAN CASTLE KINGSIDE");
      position.castlingRights[Piece::White].kingSide = true;
    }
    if (castles.find('q') != std::string::npos) {
      DEBUG_LOG("FEN BLACK CAN CASTLE QUEENSIDE");
      position.castlingRights[Piece::Black].queenSide = true;
    }
    if (castles.find('Q') != std::string::npos) {
      DEBUG_LOG("FEN WHITE CAN CASTLE QUEENSIDE");
      position.castlingRights[Piece::White].queenSide = true;
    }
  }

  const std::string &epCaptureSquare = sections.at(3);
  position.enPassantBB = 0;
  if (epCaptureSquare == "-") {
    DEBUG_LOG("NO FEN EN PASSANT");
  } else {
    const char epFile = epCaptureSquare.at(0) - 97;
    const char epRank =
        epCaptureSquare.at(1) + (position.turn == Piece::White ? -1 : 1) - 49;
    DEBUG_LOG_LABEL("FEN EP FILE", (U64)epFile);
    DEBUG_LOG_LABEL("FEN EP RANK", (U64)epRank);
    position.enPassantBB = (aFile << epFile) & (firstRank << (8 * epRank));
  }

  // DOM::DisplayPosition(position);
  U64 zobrist = Zobrist::Hash(position);
  position.zobrist = zobrist;
}

static inline U64 GetNonSlidingAttackers(const Position &position, U8 index) {
  const Piece::Color enemyColor = REVERSE_TURN(position.turn);
  const U64 enemyBB = position.colorBB[enemyColor];
  const U64 enemyKnights = enemyBB & position.pieceBB[Piece::Knight];
  const U64 enemyPawns = enemyBB & position.pieceBB[Piece::Pawn];
  return (KnightMoves[index] & enemyKnights) |
         (PawnAttacks[position.turn][index] & enemyPawns);
}

void CalculateCheckAndPinMask(const Position &position, U64 &checkMask,
                              U64 &pinHVMask, U64 &pinDIMask) {
  checkMask = 0, pinHVMask = 0, pinDIMask = 0;
  const U64 kingBB =
      position.pieceBB[Piece::King] & position.colorBB[position.turn];
  const U8 kingIndex = bitScanForward(kingBB);
  const Piece::Color enemyColor = REVERSE_TURN(position.turn);
  const U64 emptyBB = ~position.occupiedBB;
  const U64 enemyBB = position.colorBB[enemyColor];
  const U64 friendlyBB = position.colorBB[position.turn];
  // const U64 enemyOrEmptyBB = enemyBB | emptyBB;

  // North of King Rays[kingIndex][Direction::North]
  U64 kingNorthRay = nortAttacks(kingBB, emptyBB);
  U64 kingSouthRay = soutAttacks(kingBB, emptyBB);
  U64 kingEastRay = eastAttacks(kingBB, emptyBB);
  U64 kingNorthEastRay = noEaAttacks(kingBB, emptyBB);
  U64 kingSouthEastRay = soEaAttacks(kingBB, emptyBB);
  U64 kingWestRay = westAttacks(kingBB, emptyBB);
  U64 kingSouthWestRay = soWeAttacks(kingBB, emptyBB);
  U64 kingNorthWestRay = noWeAttacks(kingBB, emptyBB);

  U64 pieceSouthRay = soutOccl(
      (position.pieceBB[Piece::Rook] | position.pieceBB[Piece::Queen]) &
          enemyBB,
      emptyBB);
  U64 pieceNorthRay = nortOccl(
      (position.pieceBB[Piece::Rook] | position.pieceBB[Piece::Queen]) &
          enemyBB,
      emptyBB);
  U64 pieceWestRay = westOccl(
      (position.pieceBB[Piece::Rook] | position.pieceBB[Piece::Queen]) &
          enemyBB,
      emptyBB);
  U64 pieceSouthWestRay = soWeOccl(
      (position.pieceBB[Piece::Bishop] | position.pieceBB[Piece::Queen]) &
          enemyBB,
      emptyBB);
  U64 pieceNorthWestRay = noWeOccl(
      (position.pieceBB[Piece::Bishop] | position.pieceBB[Piece::Queen]) &
          enemyBB,
      emptyBB);
  U64 pieceEastRay = eastOccl(
      (position.pieceBB[Piece::Rook] | position.pieceBB[Piece::Queen]) &
          enemyBB,
      emptyBB);
  U64 pieceNorthEastRay = noEaOccl(
      (position.pieceBB[Piece::Bishop] | position.pieceBB[Piece::Queen]) &
          enemyBB,
      emptyBB);
  U64 pieceSouthEastRay = soEaOccl(
      (position.pieceBB[Piece::Bishop] | position.pieceBB[Piece::Queen]) &
          enemyBB,
      emptyBB);

  checkMask |= kingNorthRay & pieceSouthRay;
  checkMask |= kingSouthRay & pieceNorthRay;
  checkMask |= kingEastRay & pieceWestRay;
  checkMask |= kingNorthEastRay & pieceSouthWestRay;
  checkMask |= kingSouthEastRay & pieceNorthWestRay;
  checkMask |= kingWestRay & pieceEastRay;
  checkMask |= kingSouthWestRay & pieceNorthEastRay;
  checkMask |= kingNorthWestRay & pieceSouthEastRay;

  pinHVMask |= (kingSouthRay & nortOne(pieceNorthRay));
  pinHVMask |= (kingNorthRay & soutOne(pieceSouthRay));
  pinHVMask |= (kingEastRay & westOne(pieceWestRay));
  pinHVMask |= (kingWestRay & eastOne(pieceEastRay));
  pinDIMask |= (kingNorthEastRay & soWeOne(pieceSouthWestRay));
  pinDIMask |= (kingSouthEastRay & noWeOne(pieceNorthWestRay));
  pinDIMask |= (kingSouthWestRay & noEaOne(pieceNorthEastRay));
  pinDIMask |= (kingNorthWestRay & soEaOne(pieceSouthEastRay));

  pinHVMask &= friendlyBB;
  pinHVMask |= HVRays[kingIndex] & (emptyBB | enemyBB);

  pinDIMask &= friendlyBB;
  pinDIMask |= DIRays[kingIndex] & (emptyBB | enemyBB);

  checkMask |= GetNonSlidingAttackers(position, kingIndex);
}

#define TRACK(x)                                                               \
  x;                                                                           \
  std::cout << #x << std::endl

void MakeMove(PositionStack &stack, const Move &move) {
  stack.push(stack.top());
  Position &position = stack.top();

  Piece piece = position.pieces[move.startSquare];

  const U64 startBB = (1ULL << move.startSquare);
  const U64 endBB = (1ULL << move.endSquare);

  if (position.occupiedBB & endBB) // This is a capture
  {
    const Piece capturePiece = position.pieces[move.endSquare];
    position.pieceBB[capturePiece.type] &= ~endBB;
    position.colorBB[capturePiece.color] &= ~endBB;
    position.occupiedBB &= ~endBB;
    position.pieces[move.endSquare] = Piece::NoPiece;

    position.zobrist ^= Zobrist::Pieces[capturePiece.type][capturePiece.color]
                                       [move.endSquare]; // ZOBRIST
  }

  const U64 bb = startBB ^ endBB;
  if (move.promotion == Piece::None) {
    position.pieceBB[piece.type] ^= bb;

    position.zobrist ^=
        Zobrist::Pieces[piece.type][piece.color][move.startSquare]; // ZOBRIST
    position.zobrist ^=
        Zobrist::Pieces[piece.type][piece.color][move.endSquare]; // ZOBRIST
  } else {
    position.pieceBB[piece.type] ^= startBB;
    position.pieceBB[move.promotion] ^= endBB;

    position.zobrist ^=
        Zobrist::Pieces[piece.type][piece.color][move.startSquare]; // ZOBRIST
    position.zobrist ^=
        Zobrist::Pieces[move.promotion][piece.color][move.endSquare]; // ZOBRIST

    piece.type = move.promotion;
  }
  position.colorBB[piece.color] ^= bb;
  position.occupiedBB ^= bb;

  position.pieces[move.startSquare] = Piece::NoPiece;
  position.pieces[move.endSquare] = piece;

  // DOM::RemovePiece(move.startSquare);
  // DOM::RemovePiece(move.endSquare);
  // DOM::SetPiece(move.endSquare, piece);

  if (move.castlingSide == CastlingSide::KingSide) {
    const U8 rsi = bitScanForward(castlingRookMask[piece.color].kingSide);
    const U8 rei = bitScanForward(castlingRookEnds[piece.color].kingSide);
    position.pieces[rsi] = Piece::NoPiece;
    position.pieces[rei] = {.type = Piece::Rook, .color = piece.color};
    const U64 rookSwapBB = castlingRookMask[piece.color].kingSide ^
                           castlingRookEnds[piece.color].kingSide;
    position.pieceBB[Piece::Rook] ^= rookSwapBB;
    position.colorBB[piece.color] ^= rookSwapBB;
    position.occupiedBB ^= rookSwapBB;

    position.zobrist ^=
        Zobrist::Pieces[Piece::Rook][piece.color][rsi]; // ZOBRIST
    position.zobrist ^=
        Zobrist::Pieces[Piece::Rook][piece.color][rei]; // ZOBRIST
  } else if (move.castlingSide == CastlingSide::QueenSide) {
    const U8 rsi = bitScanForward(castlingRookMask[piece.color].queenSide);
    const U8 rei = bitScanForward(castlingRookEnds[piece.color].queenSide);
    position.pieces[rsi] = Piece::NoPiece;
    position.pieces[rei] = {.type = Piece::Rook, .color = piece.color};
    const U64 rookSwapBB = castlingRookMask[piece.color].queenSide ^
                           castlingRookEnds[piece.color].queenSide;
    position.pieceBB[Piece::Rook] ^= rookSwapBB;
    position.colorBB[piece.color] ^= rookSwapBB;
    position.occupiedBB ^= rookSwapBB;

    position.zobrist ^=
        Zobrist::Pieces[Piece::Rook][piece.color][rsi]; // ZOBRIST
    position.zobrist ^=
        Zobrist::Pieces[Piece::Rook][piece.color][rei]; // ZOBRIST
  }

  if (move.takeEnPassant) {
    const U8 epIndex = bitScanForward(position.enPassantBB);
    position.pieces[epIndex] = Piece::NoPiece;
    position.pieceBB[Piece::Pawn] ^= position.enPassantBB;
    position.colorBB[REVERSE_TURN(position.turn)] ^= position.enPassantBB;
    position.occupiedBB ^= position.enPassantBB;

    position.zobrist ^=
        Zobrist::Pieces[Piece::Pawn][REVERSE_TURN(position.turn)]
                       [epIndex]; // ZOBRIST
  }

  if (piece.type == Piece::King) {
    if (position.castlingRights[piece.color].kingSide)
      position.zobrist ^=
          Zobrist::Castling[0 + piece.color *
                                    2]; // 0 for black & 2 for white ZOBRIST
    if (position.castlingRights[piece.color].queenSide)
      position.zobrist ^=
          Zobrist::Castling[1 + piece.color *
                                    2]; // 1 for black & 3 for white ZOBRIST

    position.castlingRights[piece.color] = {false, false};
  }

  if (bb & castlingRookMask[Piece::White].kingSide) {
    if (position.castlingRights[Piece::White].kingSide)
      position.zobrist ^= Zobrist::Castling[2]; // ZOBRIST

    position.castlingRights[Piece::White].kingSide = false;
  }
  if (bb & castlingRookMask[Piece::White].queenSide) {
    if (position.castlingRights[Piece::White].queenSide)
      position.zobrist ^= Zobrist::Castling[3]; // ZOBRIST

    position.castlingRights[Piece::White].queenSide = false;
  }

  if (bb & castlingRookMask[Piece::Black].kingSide) {
    if (position.castlingRights[Piece::Black].kingSide)
      position.zobrist ^= Zobrist::Castling[0]; // ZOBRIST

    position.castlingRights[Piece::Black].kingSide = false;
  }
  if (bb & castlingRookMask[Piece::Black].queenSide) {
    if (position.castlingRights[Piece::Black].queenSide)
      position.zobrist ^= Zobrist::Castling[1]; // ZOBRIST

    position.castlingRights[Piece::Black].queenSide = false;
  }

  if (move.doublePush) {
    DEBUG_LOG_LABEL("DOUBLE PUSH", "TRUE!!!");
    if (position.enPassantBB) {
      position.zobrist ^=
          Zobrist::EnPassant[bitScanForward(position.enPassantBB) &
                             7]; // ZOBRIST
    }
    position.enPassantBB = endBB;

    position.zobrist ^=
        Zobrist::EnPassant[bitScanForward(position.enPassantBB) & 7]; // ZOBRIST
  } else {
    if (position.enPassantBB) {
      position.zobrist ^=
          Zobrist::EnPassant[bitScanForward(position.enPassantBB) &
                             7]; // ZOBRIST
    }
    position.enPassantBB = 0;
  }

  position.turn = REVERSE_TURN(position.turn);

  position.zobrist ^= Zobrist::SideToMove; // ZOBRIST
}

void PopPosition(PositionStack &stack) {
  if (stack.size() == 1) {
    DEBUG_LOG("CANNOT POP LAST POSITION FROM STACK");
    return;
  }
  stack.pop();
  // DOM::DisplayPosition(stack.top());
}

std::string Position::ToString() const {
  std::string str{};
  for (int y = 7; y >= 0; y--) {
    for (int x = 0; x < 8; x++) {
      str += pieces[y * 8 + x].ToChar();
      str += ' ';
    }
    if (y != 0) {
      str += '\n';
    }
    str += '\n';
  }
  return str;
}

} // namespace Chess