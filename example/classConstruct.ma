module example.classes;
import {println} from std.io;
import {opt, List} from std.container;



class Student implements AgeCheck {
  pub name: String;
  pub age: i32;
  pub thesis: opt<Thesis>;

  pub constructor(
    this.name,
    this.age = 0,
    thesis = None(),
  )
}

fun main(Vector<str> args) {
  let peter = Student("peter");
  let julia = Student(
    name= "julia",
    age= 22,
  );

  // not valid:
  // let p = Student()

  println("julia {}", julia.toString());
}