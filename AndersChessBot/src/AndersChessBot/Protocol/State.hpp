#include "AndersChessBot/Book/Book.hpp"
#include "AndersChessBot/Chess/Position.hpp"
namespace Chess {

class State {
public:
  ~State() = default;

  State(const State &) = delete;
  State &operator=(const State &) = delete;

  State(State &&) = delete;
  State &operator=(State &&) = delete;

  static inline State &GetInstance() {
    static State instance{};
    return instance;
  }

  PositionStack PositionStack{};
  Book::OpeningBook OpeningBook{};

private:
  State() = default;
};
} // namespace Chess