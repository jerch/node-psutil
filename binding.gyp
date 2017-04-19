{
  'targets': [
    {
      'target_name': 'psutil',
      'include_dirs': ['<!(node -e "require(\'nan\')")'],
      'cflags': ['-std=c++11'],
      'conditions': [
            ['OS == "linux"',   {'sources': ['src/posix.cpp', 'src/module_linux.cpp']}],
            ['OS == "sunos"',   {'sources': ['src/posix.cpp', 'src/module_sunos.cpp']}],
            ['OS == "freebsd"', {'sources': ['src/posix.cpp', 'src/module_freebsd.cpp']}],
            ['OS == "netbsd"',  {'sources': ['src/posix.cpp', 'src/module_netbsd.cpp']}],
            ['OS == "mac"',     {'sources': ['src/posix.cpp', 'src/module_darwin.cpp']}],
      ['OS=="mac"', {
        "xcode_settings": {
          "OTHER_CPLUSPLUSFLAGS": [
            "-std=c++11",
            "-stdlib=libc++"
          ],
          "OTHER_LDFLAGS": [
            "-stdlib=libc++"
          ],
          "MACOSX_DEPLOYMENT_TARGET":"10.7"
        }
      }]
      ]
    }
  ]
}

