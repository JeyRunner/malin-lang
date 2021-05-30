module example.classes;
import {println} from std.io;
import {opt, List} from std.container;


class array<T, S> {
  // intern array
  size: i32;
}

class String {
  buffer: ? &array<i8>; // opt ref to i8 array

  constructor(simpleStr: &str) {
    unsafe {
      let heapRef: &array<i8> = mem::malloc(array<i8, simpleStr.size()>);
      let strContend: &array<i8> = simpleStr.getChars();
      // copies array
      heapRef = strContend;
      // save pointer
      buffer = heapRef;
    }
  }

  destructor() {
    unsafe {
      mem::free(buffer);
    }
  }

  access(): &T {
    return *value;
  }

  deref(): &T {
    return value;
  }

  // mem copy before, this is copy
  copied(old: &String): String {
    unsafe {
      let heapRef: &array<i8> = mem::malloc(array<i8, simpleStr.size()>);
      // copies array
      heapRef = old.buffer;
      // save pointer
      buffer = heapRef;
    }
  }
}



fun main(Vector<str> args) {
  // same as
  // let heapStr: unique<String> = unique(String(""));
  let heapStr: unique<String> = "ab";
  // access contained type with . or *.
  heapStr.size(); // is 2

  let v: opt<unique<String>> = None;
  // same as
  // let v: opt<unique<String>> = Some(unique(String("")));
  let v: opt<unique<String>> = "";

  // move
  let moved: unique<String> = v.getOrPanic()*;
  // now opt = None, because value moved out

  // copy not allowed for unique
  // let copied: unique<String> = moved.copy(); // or moved.c()


  // shared heap
  let pointerStr: shared<String> = "ab";

  // copy
  let pointerCopy: shared<String> = pointerStr.copy();
}