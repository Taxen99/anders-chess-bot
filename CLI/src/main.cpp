#include "AndersChessBot/Chess/Core.hpp"
#include <AndersChessBot/Protocol/Protocol.hpp>
#include <cstdlib>
#include <filesystem>

void InitializeEngine() {
  if (!Chess::Protocol::Init()) {
    std::cerr << "Failed to Init, exiting..." << std::endl;
    exit(EXIT_FAILURE);
  }
}

std::string get_file_contents(const char *filename) {
  std::FILE *fp = std::fopen(filename, "rb");
  if (fp) {
    std::string contents;
    std::fseek(fp, 0, SEEK_END);
    contents.resize(std::ftell(fp));
    std::rewind(fp);
    std::fread(&contents[0], 1, contents.size(), fp);
    std::fclose(fp);
    return (contents);
  }
  throw(errno);
}

void InitializeBook() {
  std::string bookContents =
      get_file_contents("resources/game_database/lichess_elite_2019-05.pgn");
  if (!Chess::Protocol::PopulateOpeningBook(bookContents.c_str(),
                                            bookContents.length())) {
    std::cerr << "Failed to InitializeBook, exiting..." << std::endl;
    exit(EXIT_FAILURE);
  }
}

void SetupBoard() {
  Chess::Protocol::SetupBoard(Chess::StartPositionFEN.c_str());
}

int main(int argc, char **argv) {
  std::filesystem::current_path(WORKING_DIRECTORY);
  InitializeEngine();
  InitializeBook();
  SetupBoard();
  std::cout << "TODO: Fix bad bug in commit and accept san moves" << std::endl;
  while (true) {
    Chess::Move move = Chess::Move::NoMove;
    try {
      Chess::MoveList avail = Chess::Protocol::QueryMoves();
      if (avail.empty()) {
        break;
      }
      std::string input{};
      std::cin >> input;
      std::string start = input.substr(0, 2);
      std::string end = input.substr(2, 4);
      uint64_t startBB = Chess::CharsToBB(start.at(0), start.at(1));
      uint64_t endBB = Chess::CharsToBB(end.at(0), end.at(1));
      uint8_t startSquare = Chess::bitScanForward(startBB);
      uint8_t endSquare = Chess::bitScanForward(endBB);
      Chess::Move move_ = Chess::FindMove(avail, startSquare, endSquare);
      if (Chess::Move::Valid(move_)) {
        move = move_;
      } else {
        throw 0;
      }
    } catch (...) {
      std::cout << "invalid move, try `[start_square][end_square]` (ex. e4e5)"
                << std::endl;
      continue;
    }
    Chess::Protocol::MakeMove(move);
    Chess::Protocol::PrintBoard();
    const Chess::Move aiMove = Chess::Protocol::BestMove(6, true);
    if (!Chess::Move::Valid(aiMove)) {
      break;
    }
    std::cout << "ai: " << aiMove.ToString() << std::endl;
    Chess::Protocol::MakeMove(aiMove);
    Chess::Protocol::PrintBoard();
  }
  return 0;
}