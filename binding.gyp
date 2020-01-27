{
    "targets": [{
        "target_name": "perlapi",
        "cflags!": [ "-fno-exceptions" ],
        "cflags_cc!": [ "-fno-exceptions" ],
        "sources": [
            "src/main.cpp",
            "src/perlapi.cpp",
            "src/perlxsi.cpp"
        ],
        'libraries': [
            '<!@(perl -MExtUtils::Embed -e ldopts)'
        ],
        'include_dirs': [
            "<!@(node -p \"require('node-addon-api').include\")"
        ],
        'libraries': [],
        'conditions': [
            ['OS=="mac"', {
                'xcode_settings': {
                    'OTHER_LDFLAGS': [
                        '<!@(perl -MExtUtils::Embed -e ldopts)'
                    ],
                    'OTHER_CFLAGS': [
                        '<!@(perl -MExtUtils::Embed -e ccopts)',
                    '<!@(perl utils/libperl.pl)'
                    ]
                },
            }],
        ],
        'dependencies': [
            "<!(node -p \"require('node-addon-api').gyp\")"
        ],
        'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ]
    }]
}
