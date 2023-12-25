#include "../Chess/Core.hpp"
#include "../Chess/Position.hpp"
#include <emscripten/fetch.h>
#include <unordered_map>

namespace Chess::Book {

constexpr int NUM_MOVES_IN_BOOK = 12;

class OpeningBook {
public:
  OpeningBook() = default;
  void LoadGameDatabase(const char *pgnFilePath);
  void ParsePGNFile(emscripten_fetch_t *fetch);
  Move LookUpPosition(const Position &position);

private:
  Move ParseSanatizedSAN(const char *SAN, PositionStack &stack);
  std::unordered_map<U64, std::vector<Move>> book;
};

class MoveHandler {
public:
  static MoveList *moveList;

  static inline void LeafNode(PositionStack &stack, const Move &move) {
    moveList->push_back(move);
  }
};

} // namespace Chess::Book