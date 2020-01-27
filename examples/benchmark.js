const np = require('../build/Release/perlapi.node');
const perl = new np.NodePerl();

console.time('iterations');
let t0 = new Date();

var iter = 50000;
for(let i=0; i<iter; i++) {
    var x = perl.evaluate(`my $x = 123; $x`);
}
let t1 = new Date();
console.timeEnd('iterations');

console.log("%s iter/sec", (1000 * iter/(t1 - t0)).toFixed(2));
