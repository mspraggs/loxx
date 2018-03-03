// Base.method()
// Base.method()
// 0
class Base {
  method() {
    print "Base.method()";
  }
}
class Derived < Base {
  method() {
    super.method();
  }
}
class OtherBase {
  method() {
    print "OtherBase.method()";
  }
}
var derived = Derived();
derived.method();
Base = OtherBase;
derived.method();
