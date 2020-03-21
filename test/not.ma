let globalVar: i32 = 10;

/**
 * program entry point.
 */
fun main(): i32 {
  let x = 5;
  if !(x == 5) {
    printSpace();
    printColon(); // :
    printSpace();
  }
  else {
    printSpace();
    printMinus(); // -
    printSpace();
  }
  printNextLine();
  printNextLine();
  return 0;
}


fun printNextLine() {
  putChar(10);
}

fun printSpace() {
  putChar(32);
}

fun printColon() {
  putChar(58);
}

fun printMinus() {
  putChar(45);
}

/**
 * Print a char in the console.
 * Uses extern c putChar.
 */
fun extern putChar(c: i32)