#include <cassert>
#include "vectormath.h"

int main(int argc, char **argv)
{
    vec3f a(1, 0, 0), b(0, 1, 0);

    vec3f c = a + b;
    assert(c == vec3f(1, 1, 0));

    vec3f d = a - b;
    assert(d == vec3f(1, -1, 0));
}
