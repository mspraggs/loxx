// a
// 0
var closure;
{
  var a = "a";
  {
    var b = "b";
    fun returnA() {
      return a;
    }
    closure = returnA;
    if (false) {
      fun returnB() {
        return b;
      }
    }
  }
  print closure();
}
