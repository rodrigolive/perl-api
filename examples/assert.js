const np = require('../build/Release/perlapi.node');
const assert = require('assert');

/*
console.log('addon',testAddon);
console.log('hello ', testAddon.hello());
console.log('add ', testAddon.add(5, 10));

const prevInstance = new testAddon.ClassExample(4.3);
console.log('Initial value : ', prevInstance.getValue());
console.log('After adding 3.3 : ', prevInstance.add(3.3));

const newFromExisting = new testAddon.ClassExample(prevInstance);

console.log('Testing class initial value for derived instance');
console.log(newFromExisting.getValue());
*/

const perl = new np.PerlApi();

perl.evaluate('print "Lets go perl!\n"');

assert.equal(perl.evaluate('undef'), undefined);
assert.equal(perl.evaluate('undef'), null);

assert.equal(perl.evaluate('123'), 123);

assert.deepEqual(perl.evaluate('[11,22,33]'), [11, 22, 33]);
assert.deepEqual(perl.evaluate('{ aa => 10, bb => 20 }'), { bb: 20, aa: 10 });

var obj = perl.evaluate(`
    { package Foo;
        sub new {
            bless { aa => 11 } => __PACKAGE__;
        }
    }
    Foo->new;
`);

console.log(`Obj=`, obj);

var foo = perl.evaluate(`
    $foo = sub {
        print "IN sub\nARGS=[@_]\n";
        123
    };
    $foo;`);

console.log(`Sub=`, foo);
console.log(`Sub run=`, foo(11,22));

perl.evaluate(`
    if($pid=fork) {
        $foo->();
        print "In parent (chi=$pid)...\n";
    } else {
        print "In child.....\n";
        sleep 10;
        print "Child done\n";
    }
`);

setTimeout(()=>{console.log('done')}, 20000);

module.exports = np;
