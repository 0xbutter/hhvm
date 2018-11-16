<?hh // strict

class E<reify T1, reify T2> {
  public function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
    var_dump(__hhvm_intrinsics\get_reified_type(T2));
  }
}

class D<reify T1, reify T2> extends E<reify (T1, T1), reify T2> {
  public function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T1));
    var_dump(__hhvm_intrinsics\get_reified_type(T2));
    parent::f();
  }
}

class C extends D<reify (int, (int, string)), reify int> {
  public function f() {
    parent::f();
  }
}

$c = new C();

$c->f();
