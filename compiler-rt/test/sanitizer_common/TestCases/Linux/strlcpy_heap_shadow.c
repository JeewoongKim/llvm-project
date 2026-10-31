// REQUIRES: msan, glibc-2.38
// RUN: %clang -O0 -g -fno-builtin -fPIE -pie %s -o %t
// RUN: %run %t

#include <assert.h>
#include <sanitizer/msan_interface.h>
#include <stdlib.h>
#include <string.h>

static size_t (*volatile call_strlcpy)(char *, const char *, size_t) = strlcpy;
static size_t (*volatile call_strlcat)(char *, const char *, size_t) = strlcat;

int main(void) {
  {
    char *dst = malloc(8);
    assert(dst);
    __msan_poison(dst, 8);

    assert(call_strlcpy(dst, "abc", 4) == 3);
    assert(__msan_test_shadow(dst, 4) == -1);
    assert(__msan_test_shadow(dst + 4, 4) == 0);

    free(dst);
  }

  {
    char *dst = malloc(8);
    assert(dst);
    __msan_poison(dst, 8);
    dst[0] = 'a';
    dst[1] = '\0';

    assert(call_strlcat(dst, "bc", 5) == 3);
    assert(__msan_test_shadow(dst, 4) == -1);
    assert(__msan_test_shadow(dst + 4, 4) == 0);

    free(dst);
  }

  {
    char *dst = malloc(8);
    assert(dst);
    memset(dst, 0, 8);
    __msan_poison(dst + 1, 7);

    assert(call_strlcpy(dst, "abc", 0) == 3);
    assert(__msan_test_shadow(dst + 1, 7) == 0);

    free(dst);
  }

  return 0;
}
