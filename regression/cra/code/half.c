#include "assert.h"
int main() {
    int i = 0;
    int n = 0;
    int k = __VERIFIER_nondet_int();
    __VERIFIER_assume(k <= LARGE_INT && k >= -LARGE_INT);
    for(i = 0; i < 2*k; i++) {
        if (i % 2 == 0) {
            n ++;
        }
    }
    __VERIFIER_assert(k < 0 || n == k); // safe, but currently unprovable
					// (need to be able to split loops
					// into cycles)
    return 0;
}
