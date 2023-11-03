#include "PreCalculated.hpp"

namespace Chess
{

U64 KingMoves[64];
U64 KnightMoves[64];
U64 PawnAttacks[2][64];
U64 Rays[64][8];
U64 HVRays[64];
U64 DIRays[64];

void PreCalculateMoveData()
{
	// King
	for(int i = 0; i < 64; i++)
	{
		KingMoves[i] = allOne(1ULL << i);
		KnightMoves[i] = knightAttacks(1ULL << i);
		PawnAttacks[0][i] = soEaOne(1ULL << i) | soWeOne(1ULL << i);
		PawnAttacks[1][i] = noEaOne(1ULL << i) | noWeOne(1ULL << i);

		HVRays[i] |= (Rays[i][Direction::South] 	= soutAttacks(1ULL << i, ~0ULL));
		HVRays[i] |= (Rays[i][Direction::North] 	= nortAttacks(1ULL << i, ~0ULL));
		HVRays[i] |= (Rays[i][Direction::East] 	 	= eastAttacks(1ULL << i, ~0ULL));
		HVRays[i] |= (Rays[i][Direction::West] 	 	= westAttacks(1ULL << i, ~0ULL));
		DIRays[i] |= (Rays[i][Direction::NorthEast] = noEaAttacks(1ULL << i, ~0ULL));
		DIRays[i] |= (Rays[i][Direction::SouthEast] = soEaAttacks(1ULL << i, ~0ULL));
		DIRays[i] |= (Rays[i][Direction::SouthWest] = soWeAttacks(1ULL << i, ~0ULL));
		DIRays[i] |= (Rays[i][Direction::NorthWest] = noWeAttacks(1ULL << i, ~0ULL));
	}
}

}