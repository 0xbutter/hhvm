<?hh

    trait TestTrait {
        public static function test() {
            return 'Test';
        }
    }

    class A {
        use TestTrait;
    }

    $class = "A";
    echo $class::test();

<<__EntryPoint>> function main(): void {}
