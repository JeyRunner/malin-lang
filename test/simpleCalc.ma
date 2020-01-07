
let x: i32 = 3;
let y: f32 = 5.001;
let z: i32 = 11;
let myVar  = 1;


fun main(): i32 {
  putChar(65);
  putChar(66);
  putChar(67);
  putChar(10);
  putChar(101);
  putChar(101);
  putChar(10);
  rec(0);
  let val = 3;
  return plusMul(10, x, c= val) * plusMul(2, 3);
}

fun plusMul(a: i32, b: i32, c: i32 = 1): i32 {
  //return 1;
  //return 1*2 + 2 * 20 -3;
  return (a + b) * c;
}



fun plusF(a: f32, b: f32): f32 {
  return a + b;
}

fun rec(c: i32) {
  putChar(c);
  rec(c= c + 1);
}

fun extern putChar(c: i32)