#pragma once

#include "Core.hpp"

namespace Chess
{

inline U64 soutOne (U64 b) { return  b >> 8; }
inline U64 nortOne (U64 b) { return  b << 8; }
inline U64 eastOne (U64 b) { return (b & notHFile) << 1; }
inline U64 noEaOne (U64 b) { return (b & notHFile) << 9; }
inline U64 soEaOne (U64 b) { return (b & notHFile) >> 7; }
inline U64 westOne (U64 b) { return (b & notAFile) >> 1; }
inline U64 soWeOne (U64 b) { return (b & notAFile) >> 9; }
inline U64 noWeOne (U64 b) { return (b & notAFile) << 7; }
inline U64 soutOneAdd (U64 b) { return b | ( b >> 8); }
inline U64 nortOneAdd (U64 b) { return b | ( b << 8); }
inline U64 eastOneAdd (U64 b) { return b | ((b & notHFile) << 1); }
inline U64 noEaOneAdd (U64 b) { return b | ((b & notHFile) << 9); }
inline U64 soEaOneAdd (U64 b) { return b | ((b & notHFile) >> 7); }
inline U64 westOneAdd (U64 b) { return b | ((b & notAFile) >> 1); }
inline U64 soWeOneAdd (U64 b) { return b | ((b & notAFile) >> 9); }
inline U64 noWeOneAdd (U64 b) { return b | ((b & notAFile) << 7); }
inline U64 allOne(U64 b)
{
	return soutOne(b) | nortOne(b) | eastOne(b) | westOne(b) | soEaOne(b) | soWeOne(b) | noEaOne(b) | noWeOne(b);
}
inline U64 knightAttacks(U64 knights) {
   U64 l1 = (knights >> 1) & (0x7f7f7f7f7f7f7f7fULL);
   U64 l2 = (knights >> 2) & (0x3f3f3f3f3f3f3f3fULL);
   U64 r1 = (knights << 1) & (0xfefefefefefefefeULL);
   U64 r2 = (knights << 2) & (0xfcfcfcfcfcfcfcfcULL);
   U64 h1 = l1 | r1;
   U64 h2 = l2 | r2;
   return (h1<<16) | (h1>>16) | (h2<<8) | (h2>>8);
}

inline U64 PushPawns1(U64 pawns, U64 empty, int color) {
    return ( (pawns >> 8) << (color << 4) ) & empty;
}

inline U64 soutOccl(U64 gen, U64 pro) {
   gen |= pro & (gen >>  8);
   pro &=       (pro >>  8);
   gen |= pro & (gen >> 16);
   pro &=       (pro >> 16);
   gen |= pro & (gen >> 32);
   return gen;
}

inline U64 nortOccl(U64 gen, U64 pro) {
   gen |= pro & (gen <<  8);
   pro &=       (pro <<  8);
   gen |= pro & (gen << 16);
   pro &=       (pro << 16);
   gen |= pro & (gen << 32);
   return gen;
}

inline U64 eastOccl(U64 gen, U64 pro) {
   pro &= notAFile;
   gen |= pro & (gen << 1);
   pro &=       (pro << 1);
   gen |= pro & (gen << 2);
   pro &=       (pro << 2);
   gen |= pro & (gen << 4);
   return gen;
}

inline U64 noEaOccl(U64 gen, U64 pro) {
   pro &= notAFile;
   gen |= pro & (gen <<  9);
   pro &=       (pro <<  9);
   gen |= pro & (gen << 18);
   pro &=       (pro << 18);
   gen |= pro & (gen << 36);
   return gen;
}

inline U64 soEaOccl(U64 gen, U64 pro) {
   pro &= notAFile;
   gen |= pro & (gen >>  7);
   pro &=       (pro >>  7);
   gen |= pro & (gen >> 14);
   pro &=       (pro >> 14);
   gen |= pro & (gen >> 28);
   return gen;
}

inline U64 westOccl(U64 gen, U64 pro) {
   pro &= notHFile;
   gen |= pro & (gen >> 1);
   pro &=       (pro >> 1);
   gen |= pro & (gen >> 2);
   pro &=       (pro >> 2);
   gen |= pro & (gen >> 4);
   return gen;
}

inline U64 soWeOccl(U64 gen, U64 pro) {
   pro &= notHFile;
   gen |= pro & (gen >>  9);
   pro &=       (pro >>  9);
   gen |= pro & (gen >> 18);
   pro &=       (pro >> 18);
   gen |= pro & (gen >> 36);
   return gen;
}

inline U64 noWeOccl(U64 gen, U64 pro) {
   pro &= notHFile;
   gen |= pro & (gen <<  7);
   pro &=       (pro <<  7);
   gen |= pro & (gen << 14);
   pro &=       (pro << 14);
   gen |= pro & (gen << 28);
   return gen;
}

inline U64 soutAttacks (U64 rooks,   U64 empty) { return soutOne(soutOccl(rooks,   empty)); }
inline U64 nortAttacks (U64 rooks,   U64 empty) { return nortOne(nortOccl(rooks,   empty)); }
inline U64 eastAttacks (U64 rooks,   U64 empty) { return eastOne(eastOccl(rooks,   empty)); }
inline U64 noEaAttacks (U64 bishops, U64 empty) { return noEaOne(noEaOccl(bishops, empty)); }
inline U64 soEaAttacks (U64 bishops, U64 empty) { return soEaOne(soEaOccl(bishops, empty)); }
inline U64 westAttacks (U64 rooks,   U64 empty) { return westOne(westOccl(rooks,   empty)); }
inline U64 soWeAttacks (U64 bishops, U64 empty) { return soWeOne(soWeOccl(bishops, empty)); }
inline U64 noWeAttacks (U64 bishops, U64 empty) { return noWeOne(noWeOccl(bishops, empty)); }

void PreCalculateMoveData();

}