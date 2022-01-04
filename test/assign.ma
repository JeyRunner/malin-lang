let globalVar: i32 = 10;
let otherGlobalVar: i32 = 5;

/**
 * program entry point.
 */
fun main(): i32 {
  // start, print A
  putChar(65);

  let k = globalVar;
  let compareVal = k <= 2;

  let boolVal: bool = false;
  boolVal = true;

  // reassign value
  let x: i32 = 0;
  x = globalVar + 2*3;

  if isZero(x) {
    // print B
    putChar(66);
  }
  else {
    // print C
    putChar(67);
    if (x < 10) {} else {
        putChar(-10);
    }
  }


  // other if
  if (x > 0) {
    putChar(68);
  }

  isSame();

  return x;
}


fun isSame(value1: i32 = 0, value2: i32 = 2): bool {
  let x = value1;
  //isSame(value2 = 5);
  return value1 == value2;
}

fun isZero(valueOfIsZero: i32): bool {
  return valueOfIsZero == 0;
}


/**
 * Print a char in the console.
 * Uses extern c putChar.
 */
fun extern putChar(c: i32)