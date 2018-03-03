// Outer instance
// Outer instance
// Inner instance
// 0
class Outer {
  method() {
    print this;
    fun f() {
      print this;
      class Inner {
        method() {
          print this;
        }
      }
      Inner().method();
    }
    f();
  }
}
Outer().method();
