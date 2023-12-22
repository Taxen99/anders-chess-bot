console.warn("FIX THE CHECK DETECTION BUG!!!");
console.warn("Remember to rebuild ;)");

const Piece = (type, color) => type + (color == 0 ? 16 : 8);
const Pieces = {
	NONE: 0,
	KING: 1,
	PAWN: 2,
	KNIGHT: 3,
	BISHOP: 4,
	ROOK: 5,
	QUEEN: 6,

	BLACK: 0,
	WHITE: 1,
};

let turn = Pieces.WHITE;
let board = null;

const SetupBoard = fen => {
	board = Array(64).fill(0);
	let index = 56;
	let sections = fen.split(' ');
	for (const c of sections[0]) {
		switch (c) {
			case '/':
				index -= 16;
				break;
			case '1':
				index += 1;
				break;
			case '2':
				index += 2;
				break;
			case '3':
				index += 3;
				break;
			case '4':
				index += 4;
				break;
			case '5':
				index += 5;
				break;
			case '6':
				index += 6;
				break;
			case '7':
				index += 7;
				break;
			case '8':
				index += 8;
				break;
			case 'k':
				board[index] = Piece(Pieces.KING, Pieces.BLACK);
				index++;
				break;
			case 'p':
				board[index] = Piece(Pieces.PAWN, Pieces.BLACK);
				index++;
				break;
			case 'n':
				board[index] = Piece(Pieces.KNIGHT, Pieces.BLACK);
				index++;
				break;
			case 'b':
				board[index] = Piece(Pieces.BISHOP, Pieces.BLACK);
				index++;
				break;
			case 'r':
				board[index] = Piece(Pieces.ROOK, Pieces.BLACK);
				index++;
				break;
			case 'q':
				board[index] = Piece(Pieces.QUEEN, Pieces.BLACK);
				index++;
				break;
			case 'K':
				board[index] = Piece(Pieces.KING, Pieces.WHITE);
				index++;
				break;
			case 'P':
				board[index] = Piece(Pieces.PAWN, Pieces.WHITE);
				index++;
				break;
			case 'N':
				board[index] = Piece(Pieces.KNIGHT, Pieces.WHITE);
				index++;
				break;
			case 'B':
				board[index] = Piece(Pieces.BISHOP, Pieces.WHITE);
				index++;
				break;
			case 'R':
				board[index] = Piece(Pieces.ROOK, Pieces.WHITE);
				index++;
				break;
			case 'Q':
				board[index] = Piece(Pieces.QUEEN, Pieces.WHITE);
				index++;
				break;

			default:
				DEBUG_LOG_LABEL("ERROR IN FEN PARSING", c);
				break;
		}
	}
	if (sections[1] == "w") {
		trun = Pieces.WHITE
		console.log("js: FEN TURN WHITE");
	}
	else if (sections[1] == "b") {
		position.turn = Pieces.BLACK;
		console.log("js: FEN TURN BLACK");
	}
	else {
		console.log("js: INVALID FEN TURN", sections.at(1));
	}
}

let game_started = false;
let ai_turn = 1;

const AI_DELAY = 100;

const NewGame = _ => {
	console.warn("WE NEED TO PASS FEN TO GLUE.StartGame!!!");
	GLUE.StartGame();
	SetupBoard('rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1');
	DisplayBoard(board, true);
	game_started = true;
	if (turn == ai_turn) {
		setTimeout(_ => AIMove(), AI_DELAY)
	}
}

const AIMove = async _ => {
	const [start_square, end_square, eval] = GLUE.Think(6).split(';').map(n => +n);
	console.log(eval)
	GLUE.MakeMove(start_square, end_square);

	let str_repr = GLUE.QueryBoard();
	console.log("old", board);
	board = str_repr.split(';').slice(0, -1).map(n => +n);
	console.log("old", board);
	DisplayBoard(board);
	turn = !turn;
}

events.OnKeyDown = code => {
	console.log('js: kc = ' + code);
}

let marked = null;
events.OnClickSquare = index => {
	if (game_started && turn != ai_turn) {
		if (marked === null) {
			let moves = GLUE.ValidMoves(index).split(';').slice(0, -1).map(n => +n);
			if (moves.length === 0) {
				return;
			}
			marked = index;
			for (const move_end_square of moves) {
				console.log(moves);
				Mark(move_end_square);
			}
			return;
		}
		if (marked == index) {
			RemoveMarkers();
			marked = null;
			return;
		}

		if (!GLUE.ValidMoves(marked).split(';').slice(0, -1).map(n => +n).includes(index)) { return; }
		GLUE.MakeMove(marked, index);
		marked = null;

		let str_repr = GLUE.QueryBoard();
		console.log("old", board);
		board = str_repr.split(';').slice(0, -1).map(n => +n);
		console.log("old", board);
		RemoveMarkers();
		DisplayBoard(board);
		turn = !turn;

		setTimeout(_ => AIMove(), AI_DELAY);
	}
}