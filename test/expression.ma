fun main(): i32 {
  let a = 10;
  let b = 50;
  // let c = a >= b * 2 || false;

  //let bin: bool = c || a == 2 && b < a+1 && b != a*b || false;


  if true {
    //return 1;
    if false {

    }
    return 1;
  }
  else {
    return 0;
  }


  //return 0;
  //return i32(checkf(1.0, 1.1));
}


fun i32(boolean: bool): i32 {
  if boolean {
    return 1;
  }
  else {
    return 0;
  }
}


fun check(a: i32, b: i32): bool {
  let c = a*2;
  let t = a > b;
  return a == b;
}


fun checkf(a: f32, b: f32): bool {
  //let c = a*2.0;
  let t = a > b;

  if t {
    if a == b {
      return true;
    }
    else {
      //return a > b;
      if (a == 10.0) {
        if (checkf(a,b)) {
          return false;
        } else {
          test(1);
        }
      }
    }
  }
  else {
    //return false;
  }

  return a == b;
}


fun test(a: i32): bool {
  return a == 10;
}

//fun extern test(a: i32)
