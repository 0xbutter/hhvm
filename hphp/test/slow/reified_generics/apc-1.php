<?hh

class C<reify T> {
  function f() {
    var_dump(HH\ReifiedGenerics\getTypeStructure<T>());
  }
}

<<__EntryPoint>>
function main() {
 $c = new C<int>();
 $c->f();

 apc_store('c', $c);
 $d = apc_fetch('c');

 $d->f();
}
