
let x: i32 = 5546;
let y: f32 = 5.001;
let z: i32 = 10;
let myVar  = 1;


fun main(): i32 {
  complex(1, 2*fancy() , d=0.1 + 10.0);

  return 0;
}


fun fancy(a: i32 = 10): i32 {
  let other = 1;
  /*
  if (true) {
    x = 1;
  }
  */
  return a * 2;
  let tmp: i32 = z + x;
  return a + 2 *tmp;
}


fun other(a: i32, b: i32): i32 {
  let v: i32 = a*2;
  return fancy(v) + b;
}

fun complex(
    a: i32,
    b: i32,
    c: i32 = 66,
    d: f32 = 999.12,) {
}

//let z = "";