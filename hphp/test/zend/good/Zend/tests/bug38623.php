<?php <<__EntryPoint>> function main() {
try {
    switch(strtolower("apache")) {
        case "apache":
            throw new Exception("test");
            break;
    }
} catch (Exception $e) {
    echo "ok\n";
}
}
