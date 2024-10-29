
# GTest

GTest is a simple test framework for C++.

Example of usage: 

`example_test.cpp`:

```cpp
#include "testframework.hpp"
 
namespace test {
 
GTEST(ExampleTest) {
    constexpr auto i1{2};
    constexpr auto i2{3};
 
    auto addition = i1 + i2;
    auto expectedResult{5};
 
    GCHECK(addition, expectedResult);
}
 
} // namespace test
```

`run_tests.cpp`:

```cpp
#include "testframework.hpp"

int main(int argc, char *argv[]) {

    gtest::TestFramework::getInstance().executeTests();
    return 0;
}
```
