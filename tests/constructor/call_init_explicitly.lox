// Foo.init(one)
// Foo.init(two)
// Foo instance
// init
// 0
class Foo {
  init(arg) {
    print "Foo.init(" + arg + ")";
    this.field = "init";
  }
}
var foo = Foo("one");
foo.field = "field";
var foo2 = foo.init("two");
print foo2;
print foo.field;
