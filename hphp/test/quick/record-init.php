<?hh

final record Foo {
  x: int,
  y: string,
}

$foo = Foo['x' => 1];
