
// disable copy
@NoCopy()
class ClassMovable {

}

// copy and move by default
class ClassCopyable {

}


fun main(): i32 {
  let x = ClassMovable();
  let x2 = x; // do move
  let x3 = x; // error moved before

  {
    let y = ClassCopyable();
    let y1 = y; // moved because no usage afterwards
  }

  {
    let y = ClassCopyable();
    let y1 = y; // copy
    let y2 = y; // moved because no usage afterwards
  }
}

// ok
fun complex(cond: bool) {
  let x = ClassMovable();
  // ok only one branch taken
  if cond {
    let x2 = x; // do move
  }
  else {
    let x3 = x; // do move
  }
}

// error
fun complex2(cond: bool) {
  let x = ClassMovable();
  // error
  while cond {
    let x2 = x; // error: moved in previous iteration, because ClassMovable can't be copied
  }
}