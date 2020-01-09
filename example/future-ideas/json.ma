module example.json;
import {println} from std.io;
import {opt, List} from std.container;

class Person {
  pub constructor(
    pub name: opt String = None(),

    @JsonProp(name="age_in_years")
    pub age: opt<i32> = None(),

    @JsonPropIgnore
    pub secret: opt String = None(),

    pub parents: List<Person> = List[],
  )
}

/*
 * let x: String = String("ab");
 * is same as
 * let x: String = "ab";
 */


fun main(Vector<str> args) {
  let person = Person(
    name= "peter",
    parents= [
      Person("julia", 40),
      Person(
        name= "ted",
        age= 41,
      )
    ]
  );

  let json: String = Json.toJsonString(person);
  println("{}", json);
}


class Json {
  pub toJsonString<T>(T &obj): String {
    let type = reflect(obj);

    if (type.type != TypeClass) {
      throw JsonError();
    }
    let classType = type.as(class);

    let json: String = "{";
    for (let field : classType.fields) {
      json += field.name + ": ";
      json += field.reference;
    }

    return json + "}";
  }
}