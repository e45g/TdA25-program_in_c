// src/frontend/modules/api/game.ts
async function getGame(id) {
  const response = await fetch(`/api/v1/games/${id}`);
  return await response.json();
}

// src/frontend/modules/game/field.ts
function onFieldClick(board, fieldKey) {
  const field = board.map.get(fieldKey);
  if (!field) {
    return;
  }
  if (field.color) {
    return;
  }
  field.color = board.turn;
  board.turn = board.turn === "x" ? "o" : "x";
  board.map.set(fieldKey, field);
  renderField(field);
}
function renderField(field) {
  if (field.element) {
    if (field.color) {
      field.element.dataset.color = field.color;
      field.element.innerHTML = `<img src="/icon/${field.color === "x" ? "x" : "circle"}.svg" />`;
    } else {
      delete field.element.dataset.color;
      field.element.innerHTML = "";
    }
  }
}

// src/frontend/modules/game/initialize-board.ts
function initializeBoard(board) {
  console.log(board.element.children);
  for (let i = 0;i < 15 ** 2; i++) {
    const el = document.createElement("div");
    const row = Math.floor(i / 15);
    const col = i % 15;
    el.addEventListener("click", () => onFieldClick(board, `${row}:${col}`));
    board.map.set(`${row}:${col}`, {
      element: el,
      row,
      col,
      color: null
    });
    board.element.appendChild(el);
  }
}

// src/frontend/modules/game/board.ts
function boardIterator(board) {
  return {
    [Symbol.iterator]: function* () {
      for (const [rowIndex, row] of board.entries()) {
        for (const [colIndex, field2] of row.entries()) {
          yield { row: rowIndex, col: colIndex, field: field2 };
        }
      }
    }
  };
}

// src/frontend/modules/game/render-game.ts
function renderGame(board2, game) {
  let i = 0;
  for (const { field: field3 } of boardIterator(game.board)) {
    if (field3.toLocaleLowerCase() === "x") {
      i++;
    } else if (field3.toLocaleLowerCase() === "o") {
      i--;
    }
  }
  if (i < 0) {
    board2.turn = "x";
  } else {
    board2.turn = "o";
  }
  for (const [key, field3] of board2.map.entries()) {
    const [row, col] = key.split(":").map(Number);
    const boardField = game.board[row]?.[col];
    const newField = {
      ...field3,
      color: boardField.toLowerCase() || null
    };
    board2.map.set(key, newField);
    renderField(newField);
  }
}

// src/frontend/modules/game/search.ts
var init = function() {
  if (el.query === null) {
    el.query = document.getElementById("game-search-input");
  }
  if (el.results === null) {
    el.results = document.getElementById("game-search-results");
  }
  if (el.difficulty === null) {
    el.difficulty = document.getElementById("game-search-difficulty");
  }
  if (el.date === null) {
    el.date = document.getElementById("game-search-date");
  }
};
function onGameSearchFormChange() {
  init();
  const request = {
    name: el.query?.value ? el.query?.value : undefined,
    difficulty: el.difficulty?.value !== "none" ? el.difficulty?.value : undefined,
    date: el.date?.value ? new Date(el.date?.value).toISOString() : undefined
  };
  console.log(request);
  const res = fetch("/game/search", {
    method: "POST",
    body: JSON.stringify(request)
  });
  res.then((res2) => {
    return res2.text();
  }).then((data) => {
    console.log(data);
    el.results?.innerHTML && (el.results.innerHTML = data);
  });
}
var el = {
  query: null,
  difficulty: null,
  date: null,
  results: null
};

// src/frontend/modules/game/main.ts
var findCard = function(el2) {
  if (el2.dataset.searchId !== undefined) {
    return el2;
  }
  if (el2.parentElement === null) {
    return null;
  }
  return findCard(el2.parentElement);
};
var onGameSearchFormChange2 = onGameSearchFormChange;
console.log(onGameSearchFormChange2);
var board2 = new Proxy({
  map: new Map,
  element: document.getElementById("board"),
  turn: "x"
}, {});
initializeBoard(board2);
var searchResultsEl = document.getElementById("game-search-results");
var gameCache = new Map;
searchResultsEl.addEventListener("mousedown", async (e) => {
  const el2 = e.target;
  const cardEl = findCard(el2);
  if (!cardEl) {
    return;
  }
  const gameId = cardEl.dataset.searchId;
  if (!gameId) {
    return;
  }
  let game2 = gameCache.get(gameId);
  if (!game2) {
    game2 = await getGame(gameId);
  }
  if (game2?.loading instanceof Promise) {
    await game2.loading;
    game2 = gameCache.get(gameId);
  }
  if (game2?.loading === false) {
    renderGame(board2, game2);
  }
});
searchResultsEl.addEventListener("mouseover", (e) => {
  const cardEl = findCard(e.target);
  if (!cardEl) {
    return;
  }
  const gameId = cardEl.dataset.searchId;
  if (!gameId) {
    return;
  }
  const game2 = gameCache.get(gameId);
  if (game2) {
    return;
  }
  const res = getGame(gameId);
  gameCache.set(gameId, { loading: res.then(() => false) });
  res.then((game3) => {
    gameCache.set(gameId, { ...game3, loading: false });
  });
});
var filtersEl = document.getElementById("game-search-filters");
var filtersTriggerEl = document.getElementById("game-search-trigger");
filtersTriggerEl.addEventListener("click", () => {
  filtersEl.dataset.state = filtersEl.dataset.state === "closed" ? "open" : "closed";
  filtersTriggerEl.dataset.state = filtersEl.dataset.state;
});
var searchMenuTrigger = document.getElementById("game-search-menu-trigger");
var searchMenu = document.getElementById("game-search-menu-modal");
searchMenuTrigger.addEventListener("click", () => {
  searchMenu.dataset.state = searchMenu.dataset.state === "closed" ? "open" : "closed";
  searchMenuTrigger.dataset.state = searchMenu.dataset.state;
});

//# debugId=6A59F9CA9BB6C48564756E2164756E21
//# sourceMappingURL=main.js.map
