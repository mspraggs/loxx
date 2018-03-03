// foo1
// 0
class Foo {
  sayName() {
    print this.name;
  }
}
var foo1 = Foo();
foo1.name = "foo1";
var foo2 = Foo();
foo2.name = "foo2";
foo2.fn = foo1.sayName;
foo2.fn();
