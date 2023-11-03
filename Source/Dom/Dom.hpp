#pragma once

#include "../Chess/Core.hpp"
#include "../Chess/Position.hpp"

namespace DOM
{

extern std::function<void(Chess::U8)> OnClickSquare;
extern std::function<void(Chess::U16)> OnKeyDown;

void MarkSquare(Chess::U8 square, bool other = false);
void SetPiece(Chess::U8 index, Chess::Piece piece);
void RemovePiece(Chess::U8 index);
void UnMarkAll();
void DisplayPosition(const Chess::Position& position);

}