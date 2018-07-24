<?hh //strict

class TestClass { // 2. Rename TestClass
  const CONSTANT = 5;
  const string STR_CONSTANT = "hello";
  const vec<int> VEC_INT_CONSTANT = vec [1, 2, 3, 4];
  public int $property;

  public function __construct(int $i) {
    $this->property = $i + TestClass::CONSTANT; // 1. Rename CONSTANT
  }

  public function test_method(): int {
    return $this->property;
  }

  /**
   * Some docblock
   *
   *
   */
  public function deprecated_method(int $x, string $y, int $v, ...): int {
    return $x;
  }
}

function test_rename(): void {
  $test_class = new TestClass(1);
  // 3. Rename 1st test_method
  $test_class->test_method(); $test_class->test_method();
  $const = TestClass::CONSTANT;
  $str_const = TestClass::STR_CONSTANT; // 4. Rename STR_CONSTANT
  $vec_int_const = TestClass::VEC_INT_CONSTANT; // 5. Rename VEC_INT_CONSTANT
  // 7. Renaming deprecated_method
  $num = $test_class->deprecated_method(5, "", 4, 4, 3, 5, "hello");
}

function test_rename_localvar(int $local): void {      //  Should match
  $local = 3;                                           //  Should match
  j($local) + $local + h("\$x = $local");     //  1st, 2nd, and 4th should match
  $lambda1 = $x ==> $x + 1;                         //  Should not match
  $lambda2 = $a ==> $local + $a; // 6. Renaming this $local    //  Should match
  $lambda3 = function($x) {                         //  Should not match
    return $x + 1; };                               //  Should not match
  $lambda4 = function($b) use($local) {                 //  Should match
    return $local + $b; };                              //  Should match
}
