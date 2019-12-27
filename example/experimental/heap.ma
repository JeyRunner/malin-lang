module example.classes;
import {println} from std.io;
import {opt, List} from std.container;



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