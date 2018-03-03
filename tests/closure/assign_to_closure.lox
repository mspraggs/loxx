// local
// after f
// after f
// after g
// 0
var f;
var g;
{
  var local = "local";
  fun f_() {
    print local;
    local = "after f";
    print local;
  }
  f = f_;
  fun g_() {
    print local;
    local = "after g";
    print local;
  }
  g = g_;
}
f();
g();
