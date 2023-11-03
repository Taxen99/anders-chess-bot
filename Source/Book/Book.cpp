#include "Book.hpp"
#include <array>
// #include "../Dom/Dom.hpp"
#include "../Chess/MoveGenerator.hpp"
#include <random>

namespace Chess::Book
{

static constexpr int FIELD_MAX_BUF_SIZE = 100;

static PositionStack reverseStackTEMP(PositionStack& orig)
{
    PositionStack result;
    while(!orig.empty()) {
        result.push(orig.top());
        orig.pop(); 
    }
    return result;
}

void OpeningBook::ParsePGNFile(emscripten_fetch_t *fetch)
{
	const char* ptr = fetch->data;
	const char* end = fetch->data + fetch->numBytes;
	while(ptr < end)
	{
		if(*ptr == '[')
		{
			ptr++;
			char field[FIELD_MAX_BUF_SIZE];
			size_t cpy = 0;
			while (*ptr != ']' && cpy < FIELD_MAX_BUF_SIZE - 1)
			{
				field[cpy] = *ptr;
				cpy++;
				ptr++;
			}
			field[cpy] = '\0';
			ptr += 2;
		}
		else if(*ptr == '\n')
		{
			ptr++;

			PositionStack stack{};
			LoadFEN(stack, StartPositionFEN);

			int numBookMoves = 0;

			while (!(*ptr == '\n' && *(ptr + 1) == '\n'))
			{
				if(*ptr != '\n')
				{
					char pngMove[8];
					size_t cpy = 0;
					while(*ptr != ' ' && *ptr != '\n')
					{
						if(*ptr == '+' || *ptr == '#' || *ptr == 'x')
						{
							ptr++;
							continue;
						}
						pngMove[cpy] = *ptr;
						cpy++;
						if(*(ptr + 1) != '\n')
							ptr++;
						else
							break;
					}
					pngMove[cpy] = '\0';
					if(pngMove[cpy - 1] != '.' && pngMove[0] != '1' && pngMove[0] != '0')
					{
						if(numBookMoves < NUM_MOVES_IN_BOOK)
						{
							const Move move = ParseSanatizedSAN(pngMove, stack);
							if(!Move::Valid(move))
								break;
							book[stack.top().zobrist].push_back(move);
							MakeMove(stack, move);
							numBookMoves++;
						}
					}
				}
				ptr++;
			}
			ptr += 2;
		}
	}
}

Move OpeningBook::LookUpPosition(const Position &position)
{
	if(!book.contains(position.zobrist))
	{
		std::cout << "DOES NOT CONTAIN MOVE\n";
		return Move::NoMove;
	}
	const std::vector<Move>& possibleMoves = book.at(position.zobrist);
    std::vector<Move> out;
    size_t nelems = 1;
    std::sample(
        possibleMoves.begin(),
        possibleMoves.end(),
        std::back_inserter(out),
        nelems,
        std::mt19937{std::random_device{}()}
    );
	return out.at(0);
}

MoveList* MoveHandler::moveList = nullptr;

Move OpeningBook::ParseSanatizedSAN(const char* SAN, PositionStack& stack)
{
	const Position& position = stack.top();
	const size_t len = strlen(SAN);
	const Piece::Color friendlyColor = position.turn;
	if(SAN[0] == 'O')
	{
		const U8 kingIndex = bitScanForward(position.Get<Piece::King>() & position.colorBB[friendlyColor]);
		if(len == 3)
			return { kingIndex, castlingEndSquares[friendlyColor].kingSide, Piece::None, CastlingSide::KingSide };
		if(len == 5)
			return { kingIndex, castlingEndSquares[friendlyColor].queenSide, Piece::None, CastlingSide::QueenSide };
	}

	MoveList moveList{};
	MoveHandler::moveList = &moveList;
	MoveGenerator::EnumerateMoves<MoveHandler, 1>(stack);

	if(SAN[len - 2] == '=')
	{
		const Piece::Type promotionType = CharToPieceType(SAN[len - 1]);
		if(len == 4)
		{
			const U64 endBB = CharsToBB(SAN[0], SAN[1]);
			for(size_t i = 0; i < moveList.size(); i++)
			{
				const Move& move = moveList[i];
				if((position.Get<Piece::Pawn>() & (1ULL << move.startSquare)) && endBB == (1ULL << move.endSquare) && move.promotion == promotionType)
					return move;
			}
			assert(false);
			return Move::NoMove;
		}
		if(len == 5)
		{
			const U64 fileMask = CharToFile(SAN[0]);
			const U64 endBB = CharsToBB(SAN[1], SAN[2]);
			for(size_t i = 0; i < moveList.size(); i++)
			{
				const Move& move = moveList[i];
				if((position.Get<Piece::Pawn>() & (1ULL << move.startSquare) & fileMask) && endBB == (1ULL << move.endSquare) && move.promotion == promotionType)
					return move;
			}
			assert(false);
			return Move::NoMove;
		}
		else
		{
			assert(false);
		}
		return Move::NoMove;
	}
	if(isupper(SAN[0]))
	{
		if(len == 3)
		{
			const Piece::Type pieceType = CharToPieceType(SAN[0]);
			const U64 endBB = CharsToBB(SAN[1], SAN[2]);
			for(size_t i = 0; i < moveList.size(); i++)
			{
				const Move& move = moveList[i];
				if((position.pieceBB[pieceType] & (1ULL << move.startSquare)) && endBB == (1ULL << move.endSquare))
					return move;
			}
			assert(false);
		}
		else if(len == 4)
		{
			const Piece::Type pieceType = CharToPieceType(SAN[0]);
			const U64 mask = CharToMask(SAN[1]);
			const U64 endBB = CharsToBB(SAN[2], SAN[3]);
			for(size_t i = 0; i < moveList.size(); i++)
			{
				const Move& move = moveList[i];
				if((position.pieceBB[pieceType] & (1ULL << move.startSquare) & mask) && endBB == (1ULL << move.endSquare))
					return move;
			}
			assert(false);
		}
		else
		{
			std::cout << "ERROR, ERROR : " << SAN << " : " << len << std::endl;
			throw std::invalid_argument("ERRROR 5");
			assert(false);
		}
		assert(false);
	}
	if(islower(SAN[0]))
	{
		if(len == 2)
		{
			const U64 endBB = CharsToBB(SAN[0], SAN[1]);
			for(size_t i = 0; i < moveList.size(); i++)
			{
				const Move& move = moveList[i];
				if((position.Get<Piece::Pawn>() & (1ULL << move.startSquare)) && endBB == (1ULL << move.endSquare))
					return move;
			}
			assert(false);
		}
		else if(len == 3)
		{
			const U64 fileMask = CharToFile(SAN[0]);
			const U64 endBB = CharsToBB(SAN[1], SAN[2]);
			for(size_t i = 0; i < moveList.size(); i++)
			{
				const Move& move = moveList[i];
				if((position.Get<Piece::Pawn>() & (1ULL << move.startSquare) & fileMask) && endBB == (1ULL << move.endSquare))
					return move;
			}
			assert(false);
		}
		else
		{
			assert(false);
		}
		assert(false);
	}
	assert(false);
}

void downloadSucceeded(emscripten_fetch_t *fetch) {
	printf("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
	auto book = static_cast<OpeningBook*>(fetch->userData);
	book->ParsePGNFile(fetch);
	emscripten_fetch_close(fetch);
}

void downloadFailed(emscripten_fetch_t *fetch) {
	printf("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
	emscripten_fetch_close(fetch); // Also free data on failure.
}

void OpeningBook::LoadGameDatabase(const char *pgnFilePath)
{
	emscripten_fetch_attr_t attr;
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "GET");
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
	attr.onsuccess = downloadSucceeded;
	attr.onerror = downloadFailed;
	attr.userData = this;
	emscripten_fetch(&attr, pgnFilePath);
}

}