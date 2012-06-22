#include <inttypes.h>

struct Foo {
    int64_t bar;
    int64_t baz;
};

int64_t staccess() {
    struct Foo x;
    x.baz = 20;
    return x.baz;
}
