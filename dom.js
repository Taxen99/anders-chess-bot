const board = document.querySelector(".board");
const cells_COL = [...document.querySelectorAll(".cell")];
const cells = [];
for(let i = 7; i >= 0; i--)
{
	for(let j = 0; j < 8; j++)
	{
		cells.push(cells_COL[i + j * 8]);
	}
}

const typeMap = {1: "k", 2: "p", 3: "n", 4: "b", 5: "r", 6: "q"};
const colMap  = {16: "b", 8: "w"};

const SetPiece = (index, piece) => {
	const type = typeMap[(piece & 0b111)];
	const color = colMap[(piece & 0b11000)];
	cells[index].innerHTML = type && color ? `<img src="/images/${color}${type}.png"/>` : ``;
	return cells[index];
};

const DisplayBoard = (board) => {
	for(let i = 0; i < board.length; i++)
	{
		SetPiece(i, board[i]);
	}
}

const OnClickSquare = Module.cwrap('_OnClickSquare', null, ['number']);
const OnKeyDown = Module.cwrap('_OnKeyDown', null, ['number']);

document.addEventListener("keydown", function(e) { OnKeyDown(e.keyCode) });

cells.forEach(cell => cell.addEventListener("click", function() {
	OnClickSquare(cells.indexOf(this));
}))

const Mark = (index, special) => {
	const className = `square-${index}`;
	if(board.getElementsByClassName(className).length > 0) return;
	const overlay = board.appendChild(document.createElement("div"));
	overlay.classList.add(className, "marker");
	if(special) overlay.classList.add(`marker-extra`);
	overlay.dataset.rank = `${Math.floor(index / 8)}`;
	overlay.dataset.file = `${index % 8}`;
}

const UnMark = (index) => {
	const className = `square-${index}`;
	try {
	board.removeChild(board.getElementsByClassName(className)[0])
	} catch(_e) {
		throw new Error("Square Not Marked");
	}
}

const RemoveMarkers = () => {
	document.querySelectorAll(".marker").forEach(e => e.remove());
}