let globalVar: i32 = 10;

/**
 * program entry point.
 */
fun main(): i32 {
  // start, print A
  putChar(65);

  let k = globalVar;

  // reassign value
  let x: i32 = 0;
  x = globalVar;

  if isZero(x) {
    // print B
    putChar(66);
  }
  else {
    // print C
    putChar(67);
  }

  return x;
}


fun isZero(value: i32): bool {
  return value == 0;
}


/**
 * Print a char in the console.
 * Uses extern c putChar.
 */
fun extern putChar(c: i32)