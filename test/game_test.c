/*
 * test_combined.c – модульные тесты для console.c (clear_str)
 * и map_editor.c (CreateDefaultConfig, ExpandMapToEditorSize)
 * Использует заголовочные файлы из ../src
 *
 * Сборка: gcc -o test_combined test_combined.c -I../src -lcheck -lm -lpthread -lrt
 * Запуск: ./test_combined
 */

#include <check.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
/* Подключаем заголовочные файлы проекта */
#include "../src/engine/console.h"      /* clear_str */


/* --------------------------------------------------------------------------
 * Тесты для clear_str (функция из console.h)
 * -------------------------------------------------------------------------- */
START_TEST(test_clear_str_removes_escape_sequences) {
    char input[] = "\033[31mHello\033[0m World";
    char *result = clear_str(input);
    ck_assert_str_eq(result, "Hello World");
}
END_TEST

START_TEST(test_clear_str_removes_control_chars_except_newline) {
    char input[] = "Line1\n\033[1mLine2\033[0m\r\nLine3\007";
    char *result = clear_str(input);
    ck_assert_str_eq(result, "Line1\nLine2\nLine3");
}
END_TEST

START_TEST(test_clear_str_trims_tilde_and_hash) {
    char input1[] = "~ #";
    char *result1 = clear_str(input1);
    ck_assert_str_eq(result1, "");
}
END_TEST

START_TEST(test_clear_str_keeps_newlines) {
    char input[] = "first\nsecond\nthird";
    char *result = clear_str(input);
    ck_assert_str_eq(result, "first\nsecond\nthird");
}
END_TEST



/* --------------------------------------------------------------------------
 * Сборка тестовых наборов
 * -------------------------------------------------------------------------- */
Suite *console_suite(void) {
    Suite *s = suite_create("Console");
    TCase *tc = tcase_create("Core");
    tcase_add_test(tc, test_clear_str_removes_escape_sequences);
    tcase_add_test(tc, test_clear_str_removes_control_chars_except_newline);
    tcase_add_test(tc, test_clear_str_trims_tilde_and_hash);
    tcase_add_test(tc, test_clear_str_keeps_newlines);
    suite_add_tcase(s, tc);
    return s;
}

/* --------------------------------------------------------------------------
 * Точка входа
 * -------------------------------------------------------------------------- */
int main(void) {
    int number_failed;
    SRunner *sr = srunner_create(NULL);
    srunner_add_suite(sr, console_suite());

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? 0 : 1;
}