#pragma once

#include <functional>
#include <vector>

namespace Glue {

class Callbacks {
public:
	struct Move {
		int start_square, end_square;
	};
	struct ThinkArgs {
		int depth;
	};
	struct ThinkResult {
		Move best_move;
		double eval;
	};
	static inline std::function<void()> StartGame;
	static inline std::function<void(Move)> MakeMove;
	static inline std::function<ThinkResult(ThinkArgs)> Think;
	static inline std::function<std::vector<int>(int)> ValidMoves;
	static inline std::function<std::array<int, 64>()> QueryBoard;
};

}