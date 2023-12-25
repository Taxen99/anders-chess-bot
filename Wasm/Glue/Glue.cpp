#include "Glue.hpp"
#include <emscripten/emscripten.h>
#include <iostream>
#include <string>

extern "C" EMSCRIPTEN_KEEPALIVE void _GLUE_StartGame() {
  return Glue::Callbacks::StartGame();
}

extern "C" EMSCRIPTEN_KEEPALIVE void _GLUE_MakeMove(int start_square,
                                                    int end_square) {
  return Glue::Callbacks::MakeMove(
      Glue::Callbacks::Move{start_square, end_square});
}

extern "C" EMSCRIPTEN_KEEPALIVE const char *_GLUE_Think(int depth) {
  static std::string str_repr;
  const auto res = Glue::Callbacks::Think(Glue::Callbacks::ThinkArgs{depth});
  str_repr = std::to_string(res.best_move.start_square) + ";" +
             std::to_string(res.best_move.end_square) + ";" +
             std::to_string(res.eval);
  return str_repr.c_str();
}

extern "C" EMSCRIPTEN_KEEPALIVE const char *_GLUE_ValidMoves(int start_square) {
  static std::string str_repr;
  const auto res = Glue::Callbacks::ValidMoves(start_square);
  str_repr.clear();
  for (const int move : res) { // move = end_square
    str_repr += std::to_string(move) + ";";
  }
  return str_repr.c_str();
}

extern "C" EMSCRIPTEN_KEEPALIVE const char *_GLUE_QueryBoard() {
  static std::string str_repr;
  const auto res = Glue::Callbacks::QueryBoard();
  str_repr.clear();
  for (const int piece : res) {
    str_repr += std::to_string(piece) + ";";
  }
  std::cout << "qb str_repr = " << str_repr << std::endl;
  return str_repr.c_str();
}

namespace Glue {}