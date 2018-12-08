<?hh

class C<reify T> {
  function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T));
  }
  function a() {
    $c = new self();
    $c->f();
  }
}

class D<reify T> extends C<reify bool> {
  function f() {
    var_dump(__hhvm_intrinsics\get_reified_type(T));
  }
  function h() {
    $this->a();
  }
}


$d = new D<reify int>();
$d->h();
