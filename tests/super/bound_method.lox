// A.method(arg)
// 0
class A {
  method(arg) {
    print "A.method(" + arg + ")";
  }
}
class B < A {
  getClosure() {
    return super.method;
  }
  method(arg) {
    print "B.method(" + arg + ")";
  }
}
var closure = B().getClosure();
closure("arg");
