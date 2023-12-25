#pragma once

#include <array>
#include <assert.h>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace Chess {

using U64 = uint64_t;
using U32 = uint32_t;
using U16 = uint16_t;
using U8 = uint8_t;

constexpr U64 notAFile = 0xfefefefefefefefeULL; // ~0x0101010101010101
constexpr U64 notHFile = 0x7f7f7f7f7f7f7f7fULL; // ~0x8080808080808080
constexpr U64 notEFile = ~0x1010101010101010ULL;
constexpr U64 notEAFile = notEFile & notAFile;
constexpr U64 notEHFile = notEFile & notHFile;
constexpr U64 fifthRank = 0x000000FF00000000ULL;
constexpr U64 eigthRank = 0xFF00000000000000ULL;
constexpr U64 firstRank = 0x00000000000000FFULL;
constexpr U64 aFile = ~notAFile;
constexpr U64 hFile = ~notHFile;

const std::string StartPositionFEN =
    "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

struct Piece {
  enum Type {
    King = 0,
    Pawn = 1,
    Knight = 2,
    Bishop = 3,
    Rook = 4,
    Queen = 5,
    None = 6,
  };
  enum Color {
    Black = 0,
    White = 1,
  };
  Piece::Type type;
  Piece::Color color;

  static const Piece NoPiece;

  char ToChar() const;
};

constexpr U8 lsb_64_table[64] = {
    63, 30, 3,  32, 59, 14, 11, 33, 60, 24, 50, 9,  55, 19, 21, 34,
    61, 29, 2,  53, 51, 23, 41, 18, 56, 28, 1,  43, 46, 27, 0,  35,
    62, 31, 58, 4,  5,  49, 54, 6,  15, 52, 12, 40, 7,  42, 45, 16,
    25, 57, 48, 13, 10, 39, 8,  44, 20, 47, 38, 22, 17, 37, 36, 26};

/**
 * bitScanForward
 * @author Matt Taylor (2003)
 * @param bb bitboard to scan
 * @precondition bb != 0
 * @return index (0..63) of least significant one bit
 */
inline U8 bitScanForward(U64 bb) {
  unsigned int folded;
  // assert (bb != 0);
  bb ^= bb - 1;
  folded = (int)bb ^ (bb >> 32);
  return lsb_64_table[folded * 0x78291ACF >> 26];
}

#if 0

const inline std::vector<U8> SerializeBitBoard(U64 bb) // TODO CHANGE TO LIST
{
   std::vector<U8> indices;
	if ( bb ) do {
		int idx = bitScanForward(bb);
      indices.push_back(idx);
	} while (bb &= bb - 1); // reset LS1B
   return indices;
}

#endif

#ifdef DEBUG

#define DEBUG_LOG(x) std::cout << x << std::endl;
#define DEBUG_LOG_LABEL(label, x) std::cout << label << ": " << x << std::endl;

#else

#define DEBUG_LOG(x)
#define DEBUG_LOG_LABEL(label, x)

#endif

#define REVERSE_TURN(x) (x == Piece::White ? Piece::Black : Piece::White)

enum Direction {
  South = 0,
  North = 1,
  East = 2,
  NorthEast = 3,
  SouthEast = 4,
  West = 5,
  SouthWest = 6,
  NorthWest = 7,
};

struct CastlingRights {
  bool kingSide = true;
  bool queenSide = true;
};

struct CastlingMask {
  U64 kingSide, queenSide;
};

constexpr CastlingMask castlingCheckMask[2] = {
    {0b01110000ULL << 56, 0b00011100ULL << 56},
    {0b01110000ULL, 0b00011100ULL},
};
constexpr CastlingMask castlingObstructMask[2] = {
    {0b01100000ULL << 56, 0b00001110ULL << 56},
    {0b01100000ULL, 0b00001110ULL},
};

enum CastlingSide {
  None = 0,
  KingSide = 1,
  QueenSide = 2,
};

struct CastlingEndSquares {
  U8 kingSide, queenSide;
};

constexpr CastlingEndSquares castlingEndSquares[2] = {
    {62, 58},
    {6, 2},
};

constexpr CastlingMask castlingRookMask[2] = {
    {hFile & eigthRank, aFile &eigthRank},
    {hFile & firstRank, aFile &firstRank},
};

constexpr CastlingMask castlingRookEnds[2] = {
    {0b00100000ULL << 56, 0b00001000ULL << 56},
    {0b00100000ULL, 0b00001000ULL},
};

inline Piece::Type CharToPieceType(char c) {
  if (c == 'K')
    return Piece::King;
  else if (c == 'N')
    return Piece::Knight;
  else if (c == 'B')
    return Piece::Bishop;
  else if (c == 'R')
    return Piece::Rook;
  else if (c == 'Q')
    return Piece::Queen;
  else
    return Piece::None;
}

inline U64 CharToFile(char c) {
  const char file = c - 97;
  return aFile << file;
}

inline U64 CharToRank(char c) {
  const char rank = c - 49;
  return firstRank << (8 * rank);
}

inline U64 CharToMask(char c) {
  if (isalpha(c))
    return CharToFile(c);
  else if (isdigit(c))
    return CharToRank(c);
  return 0;
}

inline U64 CharsToBB(char f, char r) { return CharToFile(f) & CharToRank(r); }

inline std::string SquareToString(U8 square) {
  int file = square % 8;
  int rank = square / 8;
  char fileC = 97 + file, rankC = 49 + rank;
  std::string str;
  str += fileC;
  str += rankC;
  return str;
}

struct Move {
  U8 startSquare, endSquare;
  Piece::Type promotion = Piece::None;
  CastlingSide castlingSide = CastlingSide::None;
  bool doublePush = false, takeEnPassant = false;

  static const Move NoMove;
  inline static bool Valid(const Move &move) {
    return move.startSquare != NoMove.startSquare;
  }
  std::string ToString() const;
};

using MoveList = std::vector<Move>;

inline bool HasMove(MoveList moveList, U8 startSquare, U8 endSquare) {
  for (auto move : moveList) {
    if (move.startSquare == startSquare && move.endSquare == endSquare)
      return true;
  }
  return false;
}

inline Move FindMove(MoveList moveList, U8 startSquare, U8 endSquare) {
  for (auto move : moveList) {
    if (move.startSquare == startSquare && move.endSquare == endSquare)
      return move;
  }
  return Move::NoMove;
}

class Timer {
public:
  Timer(const char *label) : m_label(label) {}

  inline void Start() { m_begin = std::chrono::steady_clock::now(); }
  inline void End() {
    m_end = std::chrono::steady_clock::now();
    std::cout << m_label << " : "
              << std::chrono::duration_cast<std::chrono::milliseconds>(m_end -
                                                                       m_begin)
                     .count()
              << "[ms]" << std::endl;
  }

private:
  std::string m_label;
  std::chrono::steady_clock::time_point m_begin;
  std::chrono::steady_clock::time_point m_end;
};

} // namespace Chess