<?php

class Base
{
    function __construct($msg)
    {
        echo __METHOD__ . "($msg)\n";
    }
}

class Derived_1 extends Base
{
    public function __construct(...$args)
    {
        $x = array($this, 'Base::__construct'); $x(...$args);
    }
}

class Derived_2 extends Base
{
    public function __construct(...$args)
    {
        $x = array($this, 'parent::__construct'); $x(...$args);
    }
}

class Derived_3 extends Base
{
    public function __construct(...$args)
    {
        $x = 'Base::__construct'; $x(...$args);
    }
}

class Derived_4 extends Base
{
    public function __construct(...$args)
    {
        $x = 'parent::__construct'; $x(...$args);
    }
}

class Derived_5 extends Base
{
    public function __construct(...$args)
    {
        $x = array('Base', '__construct'); $x(...$args);
    }
}

class Derived_6 extends Base
{
    public function __construct(...$args)
    {
        $x = array('parent', '__construct'); $x(...$args);
    }
}
<<__EntryPoint>> function main() {
new Derived_1('1');
new Derived_2('2');
new Derived_3('3');
new Derived_4('4');
new Derived_5('5');
new Derived_6('6');

echo "===DONE===\n";
}
