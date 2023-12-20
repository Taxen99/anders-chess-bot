const DOMBoard = document.querySelector(".board");
const cells_COL = [...document.querySelectorAll(".cell")];
const cells = [];
for (let i = 7; i >= 0; i--) {
	for (let j = 0; j < 8; j++) {
		cells.push(cells_COL[i + j * 8]);
	}
}

const typeMap = { 1: "k", 2: "p", 3: "n", 4: "b", 5: "r", 6: "q" };
const colMap = { 16: "b", 8: "w" };

const SetPiece = (index, piece) => {
	const type = typeMap[(piece & 0b111)];
	const color = colMap[(piece & 0b11000)];
	cells[index].innerHTML = type && color ? `<img src="/images/${color}${type}.png"/>` : ``;
	return cells[index];
};

let __old_display_board = null;
const DisplayBoard = (board, reset = false) => {
	if (reset) __old_display_board = null;
	if (__old_display_board) {
		for (let i = 0; i < 64; i++) {
			if (__old_display_board[i] != board[i])
				SetPiece(i, board[i]);
		}
	}
	else {
		for (let i = 0; i < 64; i++) {
			SetPiece(i, board[i]);
		}
	}
	__old_display_board = board;
}

const events = {
	OnKeyDown: null,
	OnClickSquare: null,
}

document.addEventListener("keydown", function (e) { events.OnKeyDown(e.keyCode) });

cells.forEach(cell => cell.addEventListener("click", function () {
	events.OnClickSquare(cells.indexOf(this));
}))

const Mark = (index, special) => {
	const className = `square-${index}`;
	if (DOMBoard.getElementsByClassName(className).length > 0) return;
	const overlay = DOMBoard.appendChild(document.createElement("div"));
	overlay.classList.add(className, "marker");
	if (special) overlay.classList.add(`marker-extra`);
	overlay.dataset.rank = `${Math.floor(index / 8)}`;
	overlay.dataset.file = `${index % 8}`;
}

const UnMark = (index) => {
	const className = `square-${index}`;
	try {
		DOMBoard.removeChild(DOMBoard.getElementsByClassName(className)[0])
	} catch (_e) {
		throw new Error("Square Not Marked");
	}
}

const RemoveMarkers = () => {
	document.querySelectorAll(".marker").forEach(e => e.remove());
}