<?hh

class test {

    function foo () {
        set_exception_handler(array($this, "bar"));
    }

    function bar($e) {
        var_dump(get_class($e)." thrown!");
    }
}
<<__EntryPoint>> function main() {
$a = new test;
$a->foo();
throw new Exception();

echo "Done\n";
}
