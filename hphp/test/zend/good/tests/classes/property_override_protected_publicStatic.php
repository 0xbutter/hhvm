<?hh
class A
{
    protected $p = "A::p";
    function showA()
    {
        echo $this->p . "\n";
    }
}

class B extends A
{
    public static $p = "B::p (static)";
    static function showB()
    {
        echo self::$p . "\n";
    }
}

<<__EntryPoint>> function main() {
$a = new A;
$a->showA();

$b = new B;
$b->showA();
B::showB();
}
