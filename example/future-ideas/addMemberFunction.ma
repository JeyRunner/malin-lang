module example.json;
import {println} from std.io;
import {opt, List} from std.container;


@Getter!(forMember="name")
class Person {
  priv name: string;

  pub constructor(this.name) {
  }
}



fun main(Vector<str> args) {
  let person = Person("marius");
  println("name {}", person.get)
}



// '!' means it can change(only adding things is allowed) the class its applied to
@Annotation!
comptime class Getter implements AnnotationWritable {
  priv forMember: string;

  constructor(this.forMember) {
  }

  /**
   * change the type, target is a already reflected type with write enabled.
   * '~' is shared pointer
   * '?' is optional
   */
  fun applyChanges(target: ~Type<changeable=true>) {
    // target is comptime value
    // dynamic cast
    let classTargetOpt: ?_ = target as ClassType<changeable=true>; // _ means infer type, the type is here optional
    // check for error
    let classTarget = classTargetOpt.getOrThrow( CompTimeException("Getter can only be applied to class types") );
    // get member
    let member = classTarget.members.findOne(this.forMember).getOrThrow(CompTimeException("Member not found"));
    let memberType = member.getType();
    // add getter
    classTarget.addMemberFunction(functionName="get"+this.forMember,  function= (classInstance: ClassInstance /* here could follow parameters */) -> {
      // classInstance is runtime value
      // concreteMember is runtime ref to member field of the specific classInstance
      // direct access to classInstance members like 'classInstance.memberName' is not allowed
      let concreteMember: memberType = member.getConcreteRef(instance=classInstance);
      return &concreteMember; // runtime
    });
  }
}