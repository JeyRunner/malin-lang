module example.classes;
import {println} from std.io;
import {opt, List} from std.container;

export interface AgeCheck {
  isAdult(): bool;
}

/**
 * @ToString!() will generate method toString(): String;
 */
@ToString!()
class Student implements AgeCheck {
  pub name: String = "";
  pub age: i32 = 0;
  pub thesis: opt<Thesis> = None();

  // @todo name need initial value :(
  pub constructor(name: String) {
    this.name = name;
  }

  pub isAdult(): bool {
    return age >= 18;
  }
}


class Thesis {
  pub constructor(
    pub department: Department,
  )
}


enum Department {
  Robotic,
  VisualComputing,
}


fun main(Vector<str> args) {
  let peter = Student("peter");
  let julia = Student(name="julia");

  julia.thesis = Some(
    Thesis(department= Department.Robotic)
  );

  println("julia {}", julia.toString());
}