#include "../Chess/Core.hpp"
#include "../Chess/Position.hpp"
#include <unordered_map>

namespace Chess::Book {

constexpr int NUM_MOVES_IN_BOOK = 12;

class OpeningBook {
public:
  OpeningBook() = default;
  void LoadGamaDatabase(const char *data, size_t size);
  Move LookUpPosition(const Position &position);

  inline bool Initialized() noexcept { return m_Initialized; }

private:
  Move ParseSanatizedSAN(const char *SAN, PositionStack &stack);
  std::unordered_map<U64, std::vector<Move>> book;
  bool m_Initialized = false;
};

class MoveHandler {
public:
  static MoveList *moveList;

  static inline void LeafNode(PositionStack &stack, const Move &move) {
    moveList->push_back(move);
  }
};

} // namespace Chess::Book