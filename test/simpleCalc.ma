
let x: i32 = 5546;
let y: f32 = 5.001;
let z: i32 = (1 + 6) - 321 * 10;
let myVar  = 1;


fun main(): i32 {
  complex(1, 2, d=fancy() * 0.1 + 10.0);

  return 0;
}


fun fancy(a: i32 = 10): f32 {
  return 12.99;
}


fun other(a: i32, b: i32): i32 {
  let v: i32 = a*2;
  return v;
}

fun complex(
    a: i32,
    b: i32,
    c: i32 = 66,
    d: f32 = 999.12,) {
}

//let z = "";