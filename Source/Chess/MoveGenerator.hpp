#pragma once

#include "Position.hpp"
#include <vector>
#include "PreCalculated.hpp"

extern "C"
{
#include "magicmoves/magicmoves.h"
}

namespace Chess
{

class MoveGenerator
{
public:
	struct PositionData
	{
		bool check = false;
	};

	static PositionData ComputedPositionData;

	template<class CALLBACK, U8 depth, bool OC = false>
	static inline void EnumerateMoves(PositionStack& stack)
	{
		const Position& position = stack.top();
		if(position.turn == Piece::White)
		{
			if(position.enPassantBB)
				GenerateMovesTemplate<CALLBACK, Piece::White, true, depth, OC>(stack);
			else
				GenerateMovesTemplate<CALLBACK, Piece::White, false, depth, OC>(stack);
		}
		else
		{
			if(position.enPassantBB)
				GenerateMovesTemplate<CALLBACK, Piece::Black, true, depth, OC>(stack);
			else
				GenerateMovesTemplate<CALLBACK, Piece::Black, false, depth, OC>(stack);
		}
	}
	template<class CALLBACK, Piece::Color C, bool EP, U8 depth, bool OC>
	static inline void EnumerateMoves(PositionStack& stack, const Move& move)
	{
		constexpr Piece::Color reverseTurn = REVERSE_TURN(C);
		MakeMove(stack, move);
		GenerateMovesTemplate<CALLBACK, reverseTurn, EP, depth, OC>(stack);
		PopPosition(stack);
	}
private:
	template<class CALLBACK, Piece::Color C, bool EP, U8 depth, bool OC>
	static inline void HandleMove(PositionStack& stack, const Move&& move)
	{
		if constexpr(depth == 1)
			return CALLBACK::LeafNode(stack, move);
		else
			return CALLBACK::template Node<C, EP, depth, OC>(stack, move);
	}
	template<class CALLBACK, Piece::Color C, bool EP, U8 depth, bool OC>
	static void GenerateMovesTemplate(PositionStack& stack)
	{
		constexpr Piece::Color friendlyColor = C;
		constexpr Piece::Color enemyColor = REVERSE_TURN(C);
		constexpr bool hasEP = EP;
		constexpr bool onlyCaptures = OC;

		const Position& position = stack.top();

		const U64 friendlyBB = position.Get<friendlyColor>();
		const U64 enemyBB = position.Get<enemyColor>();
		const U64 empty = ~position.occupiedBB;

		U64 valid = ~friendlyBB;
		if constexpr(onlyCaptures)
		{
			valid = enemyBB;
		}

		const U64 attackBB = GetAttackBB<enemyColor>(position);

		const U64 kingBB = position.Get<friendlyColor, Piece::King>();
		const U8 kingIndex = bitScanForward(kingBB);

		U64 checkMask = 0, pinHVMask = 0, pinDIMask = 0;
		CalculateCheckAndPinMask(position, checkMask, pinHVMask, pinDIMask);

		const bool check = (kingBB & attackBB);

		const U64 trueCheckMask = check ? checkMask : ~0ULL;

		{
			U64 moves = KingMoves[kingIndex] & valid & ~attackBB;

			if ( moves ) do {
				U8 idx = bitScanForward(moves);
				HandleMove<CALLBACK, C, false, depth, OC>(stack, { kingIndex, idx });
			} while (moves &= moves - 1);

			if constexpr(onlyCaptures == false)
			{
				if(position.castlingRights[friendlyColor].kingSide)
				{
					if(!(castlingCheckMask[friendlyColor].kingSide & attackBB) && !(castlingObstructMask[friendlyColor].kingSide & position.occupiedBB))
						HandleMove<CALLBACK, C, false, depth, OC>(stack, { kingIndex, castlingEndSquares[friendlyColor].kingSide, Piece::None, CastlingSide::KingSide });
				}
				if(position.castlingRights[friendlyColor].queenSide)
				{
					if(!(castlingCheckMask[friendlyColor].queenSide & attackBB) && !(castlingObstructMask[friendlyColor].queenSide & position.occupiedBB))
						HandleMove<CALLBACK, C, false, depth, OC>(stack, { kingIndex, castlingEndSquares[friendlyColor].queenSide, Piece::None, CastlingSide::QueenSide });
				}
			}
		}

		const U64 checkingPieces = checkMask & enemyBB;

		// NOT (checking pieces is a power of two or 0)
		const bool doubleCheck = checkingPieces & (checkingPieces - 1);

		if(doubleCheck)
		{
			DEBUG_LOG("DOUBLE CHECK");
			return;
		}

		U64 pawns_   = position.Get<friendlyColor, Piece::Pawn>();
		U64 knights_ = position.Get<friendlyColor, Piece::Knight>() & (~pinHVMask) & (~pinDIMask);
		U64 bishops_ = (position.Get<Piece::Bishop>() | position.Get<Piece::Queen>()) & friendlyBB & ~pinHVMask;
		U64 rooks_   = (position.Get<Piece::Rook>()   | position.Get<Piece::Queen>()) & friendlyBB & ~pinDIMask;

		constexpr U64 promotionRank = friendlyColor == Piece::White ? eigthRank : firstRank;
		constexpr U64 doublePushRank = (fifthRank >> (friendlyColor << 3));

		if ( pawns_ ) do {
			U8 pawnIndex = bitScanForward(pawns_);
			const U64 pawn = 1ULL << pawnIndex;

			U64 pushMoves = PushPawns1(pawn, empty, friendlyColor);
			U64 doublePawnPushes = PushPawns1(pushMoves, empty, friendlyColor) & doublePushRank;

			U64 attackMoves = PawnAttacks[friendlyColor][pawnIndex] & enemyBB;

			if(pawn & pinHVMask)
			{
				attackMoves = 0;

				pushMoves &= pinHVMask;
				doublePawnPushes &= pinHVMask;
			}
			if(pawn & pinDIMask)
			{
				pushMoves = 0;
				doublePawnPushes = 0;

				attackMoves &= pinDIMask;
			}

			if constexpr(onlyCaptures)
			{
				pushMoves = 0;
				doublePawnPushes = 0; // REDUNDANT
			}

			U64 moves = (pushMoves | attackMoves) & trueCheckMask;
			doublePawnPushes &= trueCheckMask;

			// if any of the moves are a promotion, then all of them must be promotions
			// also we can ignore en passant & double pushes here since you can't en passant or double push & promote at the same time

			if(moves & promotionRank)
			{
				if ( moves ) do {
					U8 idx = bitScanForward(moves);
					HandleMove<CALLBACK, C, false, depth, OC>(stack, { pawnIndex, idx, Piece::Knight });
					HandleMove<CALLBACK, C, false, depth, OC>(stack, { pawnIndex, idx, Piece::Bishop });
					HandleMove<CALLBACK, C, false, depth, OC>(stack, { pawnIndex, idx, Piece::Rook   });
					HandleMove<CALLBACK, C, false, depth, OC>(stack, { pawnIndex, idx, Piece::Queen  });
				} while (moves &= moves - 1);
			}
			else
			{
				if ( moves ) do {
					U8 idx = bitScanForward(moves);
					HandleMove<CALLBACK, C, false, depth, OC>(stack, { pawnIndex, idx });
				} while (moves &= moves - 1);

				if constexpr(onlyCaptures == false)
				{
					if(doublePawnPushes)
					{
						HandleMove<CALLBACK, C, true, depth, OC>(stack, { pawnIndex, bitScanForward(doublePawnPushes), Piece::None, CastlingSide::None, true });
					}
				}
			}

			if constexpr(hasEP)
			{
				U64 enPassantTake = PushPawns1(position.enPassantBB, empty, friendlyColor);
				U64 enPassantMove = PawnAttacks[friendlyColor][pawnIndex] & enPassantTake;

				if(pawn & pinHVMask)
					enPassantMove = 0;
				if(pawn & pinDIMask)
					enPassantMove &= pinDIMask;
				
				if(enPassantMove && (position.enPassantBB & trueCheckMask))
				{
					DEBUG_LOG("CAN EN PASSANT MAYBE");
					constexpr U64 epClearRank = fifthRank >> (enemyColor << 3);
					const U64 enemyRooksAndQueens = (position.Get<Piece::Rook>() | position.Get<Piece::Queen>()) & position.colorBB[enemyColor];
					const U64 enemyRQOnEPRank = enemyRooksAndQueens & epClearRank;
					if((epClearRank & kingBB) && enemyRQOnEPRank)
					{
						// Check for deep pin
						DEBUG_LOG("CHECKING FOR DEEP PIN");
						const U64 emptyNoEnPassantPawns = empty | (pawn | position.enPassantBB);
						U64 kingEastRay = eastOccl(kingBB, emptyNoEnPassantPawns);
						U64 kingWestRay = westOccl(kingBB, emptyNoEnPassantPawns);

						U64 rqEastRay = eastOccl(enemyRQOnEPRank, emptyNoEnPassantPawns);
						U64 rqWestRay = westOccl(enemyRQOnEPRank, emptyNoEnPassantPawns);

						if((kingEastRay & rqWestRay) | (kingWestRay & rqEastRay))
						{
							DEBUG_LOG("NO EN PASSANT!!!!!!");
							continue;
						}
					}
					DEBUG_LOG("ADDING EN PASSANT TO MOVE LIST");
					HandleMove<CALLBACK, C, false, depth, OC>(stack, { pawnIndex, bitScanForward(enPassantMove), Piece::None, CastlingSide::None, false, true });
				}
			}
		} while (pawns_ &= pawns_ - 1);

		if (knights_) do {
			U8 knightIndex = bitScanForward(knights_);
			U64 moves = KnightMoves[knightIndex] & valid & trueCheckMask;

			if ( moves ) do {
					U8 idx = bitScanForward(moves);
					HandleMove<CALLBACK, C, false, depth, OC>(stack, { knightIndex, idx });
			} while (moves &= moves - 1);

		} while (knights_ &= knights_ - 1);

		if (bishops_) do {
			U8 bishopIndex = bitScanForward(bishops_);
			U64 moves = Bmagic(bishopIndex, position.occupiedBB) & valid & trueCheckMask;
			if((1ULL << bishopIndex) & (pinDIMask))
				moves &= pinDIMask;

			if ( moves ) do {
					U8 idx = bitScanForward(moves);
					HandleMove<CALLBACK, C, false, depth, OC>(stack, { bishopIndex, idx });
			} while (moves &= moves - 1);

		} while (bishops_ &= bishops_ - 1);

		if (rooks_) do {
			U8 rookIndex = bitScanForward(rooks_);
			U64 moves = Rmagic(rookIndex, position.occupiedBB) & valid & trueCheckMask;
			if((1ULL << rookIndex) & (pinHVMask))
				moves &= pinHVMask;
			
			if ( moves ) do {
					U8 idx = bitScanForward(moves);
					HandleMove<CALLBACK, C, false, depth, OC>(stack, { rookIndex, idx });
			} while (moves &= moves - 1);
			
		} while (rooks_ &= rooks_ - 1);

		ComputedPositionData.check = check; // FIXME: Is this a bug in case of early return?
	}
private:
	template<Piece::Color C>
	static U64 GetAttackBB(const Position& position)
	{
		const U64 enemyBB = position.Get<C>();
		const U64 enemyPawnBB   = position.Get<Piece::Pawn>()   & enemyBB;
		const U64 enemyKnightBB = position.Get<Piece::Knight>() & enemyBB;
		U64 enemyBishopBB = (position.Get<Piece::Bishop>() | position.Get<Piece::Queen>()) & enemyBB;
		U64 enemyRookBB   = (position.Get<Piece::Rook>()	 | position.Get<Piece::Queen>()) & enemyBB;
		const U8 enemyKingIndex = bitScanForward(position.Get<Piece::King>() & enemyBB);

		U64 attack = knightAttacks(enemyKnightBB) | KingMoves[enemyKingIndex];
		if constexpr(C == Piece::Black)
		{
			attack |= soEaOne(enemyPawnBB) | soWeOne(enemyPawnBB);
		}
		if constexpr(C == Piece::White)
		{
			attack |= noEaOne(enemyPawnBB) | noWeOne(enemyPawnBB);
		}

		const U64 friendlyKing = position.Get<REVERSE_TURN(C), Piece::King>();

		if ( enemyRookBB ) do {
				U8 idx = bitScanForward(enemyRookBB);
				attack |= Rmagic(idx, position.occupiedBB ^ friendlyKing);
		} while (enemyRookBB &= enemyRookBB - 1);

		if ( enemyBishopBB ) do {
				U8 idx = bitScanForward(enemyBishopBB);
				attack |= Bmagic(idx, position.occupiedBB ^ friendlyKing);
		} while (enemyBishopBB &= enemyBishopBB - 1);

		return attack;
	}
};

}
