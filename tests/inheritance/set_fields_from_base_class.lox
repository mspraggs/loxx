// foo 1
// foo 2
// bar 1
// bar 2
// bar 1
// bar 2
// 0
class Foo {
  foo(a, b) {
    this.field1 = a;
    this.field2 = b;
  }
  fooPrint() {
    print this.field1;
    print this.field2;
  }
}
class Bar < Foo {
  bar(a, b) {
    this.field1 = a;
    this.field2 = b;
  }
  barPrint() {
    print this.field1;
    print this.field2;
  }
}
var bar = Bar();
bar.foo("foo 1", "foo 2");
bar.fooPrint();
bar.bar("bar 1", "bar 2");
bar.barPrint();
bar.fooPrint();
