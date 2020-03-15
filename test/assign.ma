/**
 * program entry point.
 */
fun main(): i32 {
  // start, print A
  putChar(65);

  // reassign value
  let x: i32 = 0;
  x = 1;

  if x == 0 {
    // print B
    putChar(66);
  }
  else {
    // print C
    putChar(67);
  }

  return 0;
}


/**
 * Print a char in the console.
 * Uses extern c putChar.
 */
fun extern putChar(c: i32)