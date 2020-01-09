module example.classes;
import {println} from std.io;
import {opt, List} from std.container;


class Student implements AgeCheck {
  pub name: String;
  pub age: i32;
  pub job: unique<Job>;

  pub constructor(
    this.name,
    this.age = 0,
    this.job,
  )
}



/**
 * Jobs of students
 */
interface Job {
  pub isPayed(): bool;
}


class PayedJob implements Job {
  pub moneyPerMonth: i32;

  constructor(
    this.moneyPerMonth,
  )

  pub isPayed(): bool overwrite {
    return true;
  }
}


class TempStudentJob: PayedJob {

  // this class inherits constructor of parent
  // when its not providing a constructor itself

  pub pay(money: i32) {
    if (money <= moneyPerMonth) {
      print("given money for paying {} is less then money per month {}",
        money,
        moneyPerMonth
      );
    }
  }
}




fun main(Vector<str> args) {
  // this construct inference only works if given types
  // have a required single argument constructor

  let heapStr: unique<String> = "ab";
  // same as
  // let heapStr: unique<String> = unique(String(""));


  let julia = Student(
    name= "julia",
    age= 22,
    job= TempStudentJob(
      moneyPerMonth= 350,
    ),
  );

  // not valid
  // let julia: Student = (...);
  // let x: String;

}