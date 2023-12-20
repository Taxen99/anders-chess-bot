const GLUE = {};
GLUE.StartGame = Module.cwrap('_GLUE_StartGame', null);
GLUE.MakeMove = Module.cwrap('_GLUE_MakeMove', null, ['number', 'number']);
GLUE.Think = Module.cwrap('_GLUE_Think', 'string', ['number']);
GLUE.ValidMoves = Module.cwrap('_GLUE_ValidMoves', 'string', ['number']);
GLUE.QueryBoard = Module.cwrap('_GLUE_QueryBoard', 'string');