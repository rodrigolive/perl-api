# perl-api

Call Perl from NodeJS.

    const perl = require('perl-api');
    const api = new perl.PerlApi();
    const x = api.evaluate("my $x = 1 + 1");
    console.log(x); // 2

This is still an __alpha release__, most APIs are not in place.

This module is based on the `node-perl` module, which is now unmantained and uses an older Nan-based api.
For historic reasons, and the fact that this module diverges in concepts from `node-perl`, we've decided
to start `perl-api` as a separate module.
