{
  "targets": [
    {
      "target_name": "psutil",
      "sources": [ "src/module.cpp" ],
      "include_dirs" : ['<!(node -e "require(\'nan\')")'],
        "cflags" : [ "-std=c++11"],

    }
  ]
}

