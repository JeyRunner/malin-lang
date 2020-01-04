let x: String = "abc";
let y: i32 = 15;
let z = (((12.66)));

let k = 10.6241;

/**
 * this is the main entry point
 * its return value is
 * the return code of the program
 */
fun main(): i32 {
  let someVar = "abc";
  let x: i32 = 12;

  let l = myCoolFunction(10, x + 3*z/(1*k + 2));
  return 10;
}

// this is a comment
// other comment
fun myCoolFunction(
    name: i32,
    age: i32,
    optionalParam: i32 = 10,
    optionalParam: f32 = 0.25): i32
{
  otherFunc(first="abc", named = x + 3*z/(1*k + 2));

  // other valid call
  let x = func(
    param: 12,
    name: "peter",
    inner: Person(
      name: "hans",
    ),
  );

  return 10 * otherFunc("abc" +15, named=12, x=0.1);
}

/* comment 101 */
let xx = 0;

/*
 * abcdef
 */
let globVar = "asdf";
let someNr = 11.2 * 1.0;