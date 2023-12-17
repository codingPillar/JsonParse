# Header only C++ Json Parsing Library

## Build Example/Tests
```
make
```
## Be sure to have gtest installed
## Compilation has been tested with c++11 and c++17

## Run Example/Tests
```
./main
```

# Usage

### You can include the jsonParse.hpp as a normal header file in any file and you must include with the macro JSON_PARSE_IMPLEMENTATION defined to get the implementation for compilation (See how it's done with the example main.cpp file)

```
#define JSON_PARSE_IMPLEMENTATION
#include "jsonParse.hpp"
```

# TODO

## Add support for for json bool type