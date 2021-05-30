module example.classes;
import {println} from std.io;
import {opt, List} from std.container;

/**
 * wrapper classes hold only one type
 * access to properties of that type is done via .typesMethodName()
 * like:
 * ```
 * let heapStr: shared<String> = "ab";
 * heapStr.size(); // size() is member of String
 * heapStr*.size(); // size() is member of String, same as above
 * ```
 *
 * but move and copy of a wrapper will copy the wrapper not the contained value.
 * to copy the contained value the deref operator '*' has to be used
 * ```
 * let heapStr: shared<String> = "ab";
 * let heapMoved: shared<String> = heapStr;
 * let heapCopy: shared<String> = heapMoved.copy();
 * let strCopy: String = heapCopy*.copy();
 * ```
 *
 */
 @DefaultAssignBehavior(COPY)
class shared<T> implements Wrapper, Copy {
  value: ? &T = Empty;
  refCount: i32 = 0;

  constructor(T newValue) {
    unsafe {
      let heapRef: &T = mem::malloc(T);
      heapRef = newValue;
      value = heapRef;
    }
    refCount = 1;
  }

  destructor() {
    refCount-= 1;
    unsafe {
      if (reCount == 0) {
        mem::free(value);
      }
    }
  }

  access(): &T {
    return *value;
  }

  deref(): &T {
    return value;
  }

  copy(): shared<T> {
    refCount+= 1;
    return this*;
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

  // copy contend of pointer
  // deref pointer contend with *
  // 'real' deep copy of the string
  let strCopy: String = pointerStr*.copy();

  // members of the contained type can directly accessed via . or *.
  pointerStr.size(); // is 2, same as pointerStr*.size()
  // .copy() will be called from the pointer


  // this construct inference only works if given types
  // have a required single argument constructor


  // primitive types are always copied
  let num: i32 = 10;
  let c: i32 = num; // c is copy of num
}


fun alternativeDeref() {
    // shared heap
    let pointerStr: shared<String> = "ab";

    // copy
    // with ~ the smart pointer itself is addressed not its contend
    let pointerCopy: shared<String> = pointerStr~.copy();

    // copy contend of pointer
    // 'real' deep copy of the string
    let strCopy: String = pointerStr.copy();
}