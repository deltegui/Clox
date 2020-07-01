#include "common.h"
#include "../scanner.h"

static void integer_should_equal_putooo(void **state) {
  init_scanner("{}");
  assert_int_equal(scan_token().type, TOKEN_LEFT_PAREN);
  assert_int_equal(scan_token().type, TOKEN_RIGHT_PAREN);
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(integer_should_equal_putooo),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
