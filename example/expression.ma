fun main(): i32 {
  let a = 10;
  let b = 50;
  let c = a >= b * 2 || false;

  let bin: bool = c || a == 2 && b < a+1 && b != a*b || false;

/*
  if (c) {
    return 1;
  }
  else {
    return 0;
  }
  */

  return 0;
}


fun check(a: i32, b: i32): bool {
  let c = a*2;
  let t = a > b;
  return a == b;
}

fun checkf(a: f32, b: f32): bool {
  let c = a*2.0;
  let t = a > b;
  return a == b;
}


fun test(a: i32): bool {
  return a == 10;
}

//fun extern test(a: i32)