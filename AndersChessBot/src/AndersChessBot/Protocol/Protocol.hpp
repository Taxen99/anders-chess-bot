#include "AndersChessBot/Chess/Core.hpp"

namespace Chess::Protocol {

auto Init() noexcept -> bool;
auto SetupBoard(const char *FEN) noexcept -> bool;
auto PrintBoard() noexcept -> void;
auto MakeMove(Chess::Move move) noexcept -> bool;
auto QueryMoves() noexcept -> MoveList;
auto PopulateOpeningBook(const char *data, size_t len) noexcept -> bool;
// auto Evaluate() noexcept -> double;
auto BestMove(int depth, bool playFromBook) noexcept -> Chess::Move;

} // namespace Chess::Protocol