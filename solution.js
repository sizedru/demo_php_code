// Реализация задачки
// https://gist.github.com/xanf/9e11a5faf618a018737a9efd252746da


const sides = { 'left': 1, 'top': 2, 'right': 3, 'bottom': 4 };

function turnClock(e) {   // Поворот по часовой стрелке
  [e.right, e.top, e.left, e.bottom] = [e.top, e.left, e.bottom, e.right]
}

function turnFromTo(e, from, to) {  // Поворот из одного положения в другое
  cnt = sides[to] - sides[from];
  if (cnt < 0) cnt += 4;
  for (i = 1; i <= cnt; i++)turnClock(e);
}

function turnFirst(piece) {   // Поворот первого элемента
  piece.ok = true;
  if (piece.edges.top == null && piece.edges.right == null) {
    turnFromTo(piece.edges, "right", "top");
  } else if (piece.edges.right == null && piece.edges.bottom == null) {
    turnFromTo(piece.edges, "right", "left");
  } else if (piece.edges.bottom == null && piece.edges.left == null) {
    turnFromTo(piece.edges, "left", "top");
  }
}

function checkPart(piece, side, value) {  // Проверка и поворот (если требуется) элемента
  if (piece.ok) return false;
  for (const [key, val] of Object.entries(piece.edges)) {
    if (val != null) {
      if (val.edgeTypeId == value) {
        piece.ok = true;
        turnFromTo(piece.edges, key, side)
        break;
      }
    }
  }
  return piece.ok;
}

function solvePuzzle(pieces) {
  turnFirst(pieces[0]); // Правильно поворачиваем первый элемент

  for (i = 1; i < pieces.length; i++) // Добавляем флаг того что элемент найден
    pieces[i].ok = false;
  
  var rez = [pieces[0].id];           // Создаем массив с результатами
  var first = pieces[0];              // Первый элемент для выборки по строкам
  var next = first;                   // Первый (следующий) элемент для выборки по столбцам.

  for (i = 0; i < pieces.length; i++) {
    if (next.edges.right == null) {             // Правая грань пустая? тогда колонки кончились добавляем строку
      if (first.edges.bottom == null) break;    // Нижняя грянь пустая? тогда все
      for (j = 0; j < pieces.length; j++) {     // Перебираем все элементы ищем подходящий снизу к первому в строке
        if (checkPart(pieces[j], "top", first.edges.bottom.edgeTypeId)) {
          rez.push(pieces[j].id);               // Нашли? делаем его первым в строке и переходим к циклу по столбцам
          first = pieces[j];
          next = first;
          break;
        }
      }
    }

    for (j = 0; j < pieces.length; j++) {       // Цикл по колонкам. Здесь ищем подходящий справа к текущему
      if (checkPart(pieces[j], "left", next.edges.right.edgeTypeId)) {
        rez.push(pieces[j].id);                 // Нашли? Меняем текущий элемент 
        next = pieces[j];
        break;
      }
    }
  }
  return rez;
}


// Не удаляйте эту строку
window.solvePuzzle = solvePuzzle;
