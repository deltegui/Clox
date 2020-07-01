#include "common.h"
#include "../scanner.h"

#define ASSERT_TOKEN_TYPE(TOKEN) assert_int_equal(scan_token().type, TOKEN);

static void should_scan_single_character(void **state) {
  init_scanner("{}(),.-+;/*%");
  ASSERT_TOKEN_TYPE(TOKEN_LEFT_BRACE);
  ASSERT_TOKEN_TYPE(TOKEN_RIGHT_BRACE);
  ASSERT_TOKEN_TYPE(TOKEN_LEFT_PAREN);
  ASSERT_TOKEN_TYPE(TOKEN_RIGHT_PAREN);
  ASSERT_TOKEN_TYPE(TOKEN_COMMA);
  ASSERT_TOKEN_TYPE(TOKEN_DOT);
  ASSERT_TOKEN_TYPE(TOKEN_MINUS);
  ASSERT_TOKEN_TYPE(TOKEN_PLUS);
  ASSERT_TOKEN_TYPE(TOKEN_SEMICOLON);
  ASSERT_TOKEN_TYPE(TOKEN_SLASH);
  ASSERT_TOKEN_TYPE(TOKEN_STAR);
  ASSERT_TOKEN_TYPE(TOKEN_PERCENT);
}

static void should_scan_two_character(void **state) {
  init_scanner("! != = == > >= < <=");
  ASSERT_TOKEN_TYPE(TOKEN_BANG);
  ASSERT_TOKEN_TYPE(TOKEN_BANG_EQUAL);
  ASSERT_TOKEN_TYPE(TOKEN_EQUAL);
  ASSERT_TOKEN_TYPE(TOKEN_EQUAL_EQUAL);
  ASSERT_TOKEN_TYPE(TOKEN_GREATER);
  ASSERT_TOKEN_TYPE(TOKEN_GREATER_EQUAL);
  ASSERT_TOKEN_TYPE(TOKEN_LESS);
  ASSERT_TOKEN_TYPE(TOKEN_LESS_EQUAL);
}

static void should_scan_literals(void **state) {
  init_scanner("hola \"hola\" 22 22.11");
  ASSERT_TOKEN_TYPE(TOKEN_IDENTIFIER);
  ASSERT_TOKEN_TYPE(TOKEN_STRING);
  ASSERT_TOKEN_TYPE(TOKEN_NUMBER);
  ASSERT_TOKEN_TYPE(TOKEN_NUMBER);
}

static void should_scan_keywords(void **state) {
  init_scanner("and class else false for fun if nil or print return super this true var while break continue");
  ASSERT_TOKEN_TYPE(TOKEN_AND);
  ASSERT_TOKEN_TYPE(TOKEN_CLASS);
  ASSERT_TOKEN_TYPE(TOKEN_ELSE);
  ASSERT_TOKEN_TYPE(TOKEN_FALSE);
  ASSERT_TOKEN_TYPE(TOKEN_FOR);
  ASSERT_TOKEN_TYPE(TOKEN_FUN);
  ASSERT_TOKEN_TYPE(TOKEN_IF);
  ASSERT_TOKEN_TYPE(TOKEN_NIL);
  ASSERT_TOKEN_TYPE(TOKEN_OR);
  ASSERT_TOKEN_TYPE(TOKEN_PRINT);
  ASSERT_TOKEN_TYPE(TOKEN_RETURN);
  ASSERT_TOKEN_TYPE(TOKEN_SUPER);
  ASSERT_TOKEN_TYPE(TOKEN_THIS);
  ASSERT_TOKEN_TYPE(TOKEN_TRUE);
  ASSERT_TOKEN_TYPE(TOKEN_VAR);
  ASSERT_TOKEN_TYPE(TOKEN_WHILE);
  ASSERT_TOKEN_TYPE(TOKEN_BREAK);
  ASSERT_TOKEN_TYPE(TOKEN_CONTINUE);
}

static void should_return_token_error(void **state) {
  init_scanner("    \"this sting does not terminate   ");
  ASSERT_TOKEN_TYPE(TOKEN_ERROR);
}

static void should_return_token_eof(void **state) {
  init_scanner("");
  ASSERT_TOKEN_TYPE(TOKEN_EOF);
  init_scanner("       ");
  ASSERT_TOKEN_TYPE(TOKEN_EOF);
  init_scanner("   and    ");
  ASSERT_TOKEN_TYPE(TOKEN_AND);
  ASSERT_TOKEN_TYPE(TOKEN_EOF);
}

int main(void) {
  const struct CMUnitTest tests[] = {
    cmocka_unit_test(should_scan_single_character),
    cmocka_unit_test(should_scan_two_character),
    cmocka_unit_test(should_scan_literals),
    cmocka_unit_test(should_scan_keywords),
    cmocka_unit_test(should_return_token_error),
    cmocka_unit_test(should_return_token_eof),
  };
  return cmocka_run_group_tests(tests, NULL, NULL);
}
