<?hh

<<__EntryPoint>>
function main() {
  $arrs = vec[
    array('x' => 1),
    shape('x' => 1),
    darray['x' => 1],
    new Map(dict['x' => 1]),
  ];
  foreach ($arrs as $arr) {
    var_dump($arr);
    var_dump(Shapes::idx($arr, 'x'));
  }
  throw new Exception('done');
}
