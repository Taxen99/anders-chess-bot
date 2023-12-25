#include "Engine.hpp"
#include <random>

namespace Chess::Engine {

MoveList *EngineMoveHandler::moveList = nullptr;

std::random_device dev;
std::mt19937 rng(dev());
std::uniform_int_distribution<std::mt19937::result_type> dist(-10, 10);

template <Piece::Type T> static constexpr Evaluation PieceValue() {
  if constexpr (T == Piece::King) {
    return 0;
  }
  if constexpr (T == Piece::Pawn) {
    return 100;
  }
  if constexpr (T == Piece::Knight) {
    return 300;
  }
  if constexpr (T == Piece::Bishop) {
    return 300;
  }
  if constexpr (T == Piece::Rook) {
    return 500;
  }
  if constexpr (T == Piece::Queen) {
    return 900;
  }
}

/*
   enum Type
   {
      King   = 0,
      Pawn   = 1,
      Knight = 2,
      Bishop = 3,
      Rook   = 4,
      Queen  = 5,
      None   = 6,
   };*/

//											   K
//P N	 B	  R	   Q
static constexpr Evaluation PieceValues[7] = {0, 100, 300, 300, 500, 900};

#define FLIP(x) ((x) ^ 56)

static constexpr int8_t PawnTable[64] = {
    0,  0,  0,  0,   0,   0,  0,  0,  50, 50, 50,  50, 50, 50,  50, 50,
    10, 10, 20, 30,  30,  20, 10, 10, 5,  5,  10,  25, 25, 10,  5,  5,
    0,  0,  0,  20,  20,  0,  0,  0,  5,  -5, -10, 0,  0,  -10, -5, 5,
    5,  10, 10, -20, -20, 10, 10, 5,  0,  0,  0,   0,  0,  0,   0,  0,
};

static constexpr int8_t KnightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50, -40, -20, 0,   0,   0,
    0,   -20, -40, -30, 0,   10,  15,  15,  10,  0,   -30, -30, 5,
    15,  20,  20,  15,  5,   -30, -30, 0,   15,  20,  20,  15,  0,
    -30, -30, 5,   10,  15,  15,  10,  5,   -30, -40, -20, 0,   5,
    5,   0,   -20, -40, -50, -40, -30, -30, -30, -30, -40, -50,
};

static constexpr int8_t BishopTable[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20, -10, 0,   0,   0,   0,
    0,   0,   -10, -10, 0,   5,   10,  10,  5,   0,   -10, -10, 5,
    5,   10,  10,  5,   5,   -10, -10, 0,   10,  10,  10,  10,  0,
    -10, -10, 10,  10,  10,  10,  10,  10,  -10, -10, 5,   0,   0,
    0,   0,   5,   -10, -20, -10, -10, -10, -10, -10, -10, -20,
};

static constexpr int8_t RookTable[64] = {
    0,  0, 0, 0, 0, 0, 0, 0,  5,  10, 10, 10, 10, 10, 10, 5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
    -5, 0, 0, 0, 0, 0, 0, -5, -5, 0,  0,  0,  0,  0,  0,  -5,
    -5, 0, 0, 0, 0, 0, 0, -5, 0,  0,  0,  5,  5,  0,  0,  0,
};

static constexpr int8_t QueenTable[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20, -10, 0,   0,   0,  0,  0,   0,   -10,
    -10, 0,   5,   5,  5,  5,   0,   -10, -5,  0,   5,   5,  5,  5,   0,   -5,
    0,   0,   5,   5,  5,  5,   0,   -5,  -10, 5,   5,   5,  5,  5,   0,   -10,
    -10, 0,   5,   0,  0,  0,   0,   -10, -20, -10, -10, -5, -5, -10, -10, -20,
};

static constexpr int8_t KingMidTable[64] = {
    -30, -40, -40, -50, -50, -40, -40, -30, -30, -40, -40, -50, -50,
    -40, -40, -30, -30, -40, -40, -50, -50, -40, -40, -30, -30, -40,
    -40, -50, -50, -40, -40, -30, -20, -30, -30, -40, -40, -30, -30,
    -20, -10, -20, -20, -20, -20, -20, -20, -10, 20,  20,  0,   0,
    0,   0,   20,  20,  20,  30,  10,  0,   0,   10,  30,  20,
};

// static constexpr int8_t KingEndTable[64] = {
// 	-50,-40,-30,-20,-20,-30,-40,-50,
// 	-30,-20,-10,  0,  0,-10,-20,-30,
// 	-30,-10, 20, 30, 30, 20,-10,-30,
// 	-30,-10, 30, 40, 40, 30,-10,-30,
// 	-30,-10, 30, 40, 40, 30,-10,-30,
// 	-30,-10, 20, 30, 30, 20,-10,-30,
// 	-30,-30,  0,  0,  0,  0,-30,-30,
// 	-50,-30,-30,-30,-30,-30,-30,-50
// };

Evaluation Evaluate(Chess::PositionStack &stack) {
  const Position &position = stack.top();
  // const Piece::Color friendlyColor = position.turn;
  const Piece::Color enemyColor = REVERSE_TURN(position.turn);
  // U64 fPawns = position.Get<Piece::Pawn>() & position.colorBB[friendlyColor];
  // U64 fKnights = position.Get<Piece::Knight>() &
  // position.colorBB[friendlyColor]; U64 fBishops =
  // position.Get<Piece::Bishop>() & position.colorBB[friendlyColor]; U64 fRooks
  // = position.Get<Piece::Rook>() & position.colorBB[friendlyColor]; U64
  // fQueens = position.Get<Piece::Queen>() & position.colorBB[friendlyColor];

  // U64 ePawns = position.Get<Piece::Pawn>() & position.colorBB[enemyColor];
  // U64 eKnights = position.Get<Piece::Knight>() &
  // position.colorBB[enemyColor]; U64 eBishops = position.Get<Piece::Bishop>()
  // & position.colorBB[enemyColor]; U64 eRooks = position.Get<Piece::Rook>() &
  // position.colorBB[enemyColor]; U64 eQueens = position.Get<Piece::Queen>() &
  // position.colorBB[enemyColor];

  // Evaluation eval = 0;
  // if(fPawns) do { eval += PieceValue<Piece::Pawn>(); } while(fPawns &= fPawns
  // - 1); if(fKnights) do { eval += PieceValue<Piece::Knight>(); }
  // while(fKnights &= fKnights - 1); if(fBishops) do { eval +=
  // PieceValue<Piece::Bishop>(); } while(fBishops &= fBishops - 1); if(fRooks)
  // do { eval += PieceValue<Piece::Rook>(); } while(fRooks &= fRooks - 1);
  // if(fQueens) do { eval += PieceValue<Piece::Queen>(); } while(fQueens &=
  // fQueens - 1);

  // if(ePawns) do { eval -= PieceValue<Piece::Pawn>(); } while(ePawns &= ePawns
  // - 1); if(eKnights) do { eval -= PieceValue<Piece::Knight>(); }
  // while(eKnights &= eKnights - 1); if(eBishops) do { eval -=
  // PieceValue<Piece::Bishop>(); } while(eBishops &= eBishops - 1); if(eRooks)
  // do { eval -= PieceValue<Piece::Rook>(); } while(eRooks &= eRooks - 1);
  // if(eQueens) do { eval -= PieceValue<Piece::Queen>(); } while(eQueens &=
  // eQueens - 1);

  Evaluation eval = 0;

  for (size_t i = 0; i < 64; i++) {
    const Piece piece = position.pieces[i];
    Evaluation pieceValue = PieceValues[piece.type];

    const U8 sq = (piece.color == Piece::White) ? i : FLIP(i);

    if (piece.type == Piece::King)
      pieceValue += KingMidTable[sq];
    else if (piece.type == Piece::Pawn)
      pieceValue += PawnTable[sq];
    else if (piece.type == Piece::Knight)
      pieceValue += KnightTable[sq];
    else if (piece.type == Piece::Bishop)
      pieceValue += BishopTable[sq];
    else if (piece.type == Piece::Rook)
      pieceValue += RookTable[sq];
    else if (piece.type == Piece::Queen)
      pieceValue += QueenTable[sq];

    if (piece.color == enemyColor)
      pieceValue *= -1;
    eval += pieceValue;
  }

  return eval;
}

constexpr U8 maxMoves = 218;
Evaluation moveScores[maxMoves];

static void sortMoves(MoveList &moveList) {
  // Sort the moves list based on scores
  for (size_t i = 0; i < moveList.size() - 1; i++) {
    for (int j = i + 1; j > 0; j--) {
      int swapIndex = j - 1;
      if (moveScores[swapIndex] < moveScores[j]) {
        std::swap(moveList[swapIndex],
                  moveList[j]); // TODO : DON'T USE STD::SWAP !!!
        std::swap(moveScores[swapIndex], moveScores[j]);
      }
    }
  }
}

void OrderMoves(MoveList &moveList, const Position &position) {
  const U8 enemyTurn = REVERSE_TURN(position.turn);
  const U64 enemyPawns =
      position.Get<Piece::Pawn>() & position.colorBB[enemyTurn];
  U64 enemyPawnAttacks = 0;
  if (enemyTurn == Piece::White)
    enemyPawnAttacks = noWeOne(enemyPawns) | noEaOne(enemyPawns);
  else
    enemyPawnAttacks = soWeOne(enemyPawns) | soEaOne(enemyPawns);

  for (size_t i = 0; i < moveList.size(); i++) {
    Evaluation score = 0;

    const Move &move = moveList[i];
    const Piece piece = position.pieces[move.startSquare];
    const Piece capture = position.pieces[move.endSquare];

    if (capture.type != Piece::None)
      score = 10 * PieceValues[capture.type] - PieceValues[piece.type];

    if (piece.type == Piece::Pawn) {
      if (move.promotion != Piece::None)
        score += PieceValues[move.promotion];
    } else if ((1ULL << move.endSquare) & enemyPawnAttacks)
      score -= 350;

    moveScores[i] = score;
  }
  sortMoves(moveList);
}

Evaluation QuietSearch(Chess::PositionStack &stack, Evaluation alpha,
                       Evaluation beta) {
  numPositionsSearched++;

  Evaluation eval = Evaluate(stack);
  if (eval >= beta) {
    return beta;
  }
  if (eval > alpha) {
    alpha = eval;
  }

  MoveList moveList{};
  EngineMoveHandler::moveList = &moveList;
  MoveGenerator::EnumerateMoves<EngineMoveHandler, 1, true>(stack);

  if (moveList.empty())
    return alpha;

  OrderMoves(moveList, stack.top());

  for (const Move &move : moveList) {
    MakeMove(stack, move);
    const Evaluation eval = -QuietSearch(stack, -beta, -alpha);
    PopPosition(stack);

    if (eval >= beta)
      return beta;

    if (eval > alpha)
      alpha = eval;
  }

  return alpha;
}

Evaluation bestEval = 0;
Move bestMove{};
U64 numPositionsSearched = 0;

} // namespace Chess::Engine