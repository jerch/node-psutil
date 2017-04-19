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
            ['OS == "netbsd"',  {'sources': ['src/posix.cpp', 'src/module_netbsd.cpp']}]
      ]
    }
  ]
}

