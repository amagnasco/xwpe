/**
 *  XWPE - XWindows Programming Editor
 *  Copyright (C) 2017 Guus Bonnema.
    This file is part of Xwpe - Xwindows Programming Editor.

    Xwpe - Xwindows Programming Editor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Xwpe - Xwindows Programming Editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Xwpe - Xwindows Programming Editor.  If not, see <http://www.gnu.org/licenses/>.

   */
#include <check.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/we_find.h"
#include "check_search.h"

START_TEST(test_search_empty_string_empty)
{
    unsigned char *str = (unsigned char *)"";
    int start_offset = 0;
    size_t end_offset = strlen((const char *)str);
    unsigned char *search_expr = (unsigned char *)"";
    _Bool case_sensitive = 1;
    size_t end_match;
    int start_match = e_strstr(start_offset, end_offset,
                                str, search_expr, &end_match,
				case_sensitive);
    int expected_start_match = 0;
    int expected_end_match = 0;
    ck_assert_int_eq(expected_start_match,start_match);
    ck_assert_int_eq(expected_end_match,end_match);
}
END_TEST

START_TEST(test_search_string)
{
    unsigned char *str = (unsigned char *)"This is a long string.\n";
    int start_offset = 0;
    int end_offset = strlen((const char *)str);
    unsigned char *search_expr = (unsigned char *)"is";
    _Bool case_sensitive = 1;
    size_t end_match = 0;
    int start_match = e_strstr(start_offset, end_offset,
                                str, search_expr, &end_match,
				case_sensitive);
    int expected_start_match = 2;
    size_t expected_end_match = 4;
    ck_assert_int_eq(expected_start_match,start_match);
    ck_assert_int_eq(expected_end_match, end_match);
}
END_TEST

START_TEST(test_search_regex_empty_string_empty)
{
    unsigned char *str = (unsigned char *)"";
    int start_offset = 0;
    int end_offset = strlen((const char *)str);
    unsigned char *reg_expr = (unsigned char *)"";
    _Bool case_sensitive = 1;
    size_t end_match = 0;
    int start_match = e_rstrstr(start_offset, end_offset,
                                str, reg_expr, &end_match, case_sensitive);
    int expected_start_match = 0;
    size_t expected_end_match = 0;
    ck_assert_int_eq(expected_start_match,start_match);
    ck_assert_int_eq(expected_end_match, end_match);
}
END_TEST

START_TEST(test_search_regex_empty)
{
    unsigned char *str = (unsigned char *)"This is a long string.\n";
    int start_offset = 0;
    int end_offset = strlen((const char *)str);
    unsigned char *reg_expr = (unsigned char *)"";
    _Bool case_sensitive = 1;
    size_t end_match = 0;
    int start_match = e_rstrstr(start_offset, end_offset,
                                str, reg_expr, &end_match, case_sensitive);
    int expected_start_match = 0;
    size_t expected_end_match = 0;
    ck_assert_int_eq(expected_start_match,start_match);
    ck_assert_int_eq(expected_end_match, end_match);
}
END_TEST

START_TEST(test_search_regex_string)
{
    unsigned char *str = (unsigned char *)"This is a long string.\n";
    int start_offset = 0;
    int end_offset = strlen((const char *)str);
    unsigned char *reg_expr = (unsigned char *)"is";
    _Bool case_sensitive = 1;
    size_t end_match = 0;
    int start_match = e_rstrstr(start_offset, end_offset,
                                str, reg_expr, &end_match, case_sensitive);
    int expected_start_match = 2;
    size_t expected_end_match = 4;
    ck_assert_int_eq(expected_start_match,start_match);
    ck_assert_int_eq(expected_end_match, end_match);
}
END_TEST

START_TEST(test_search_regex_string_word_boundary)
{
    unsigned char *str = (unsigned char *)"This is a long string.\n";
    int start_offset = 0;
    int end_offset = strlen((const char *)str);
    unsigned char *reg_expr = (unsigned char *)"\\bis\\b";
    _Bool case_sensitive = 1;
    size_t end_match = 0;
    int start_match = e_rstrstr(start_offset, end_offset,
                                str, reg_expr, &end_match, case_sensitive);
    int expected_start_match = 5;
    size_t expected_end_match = 7;
    ck_assert_int_eq(expected_start_match,start_match);
    ck_assert_int_eq(expected_end_match, end_match);
}
END_TEST

START_TEST(test_search_regex_backward_string)
{
    unsigned char *str = (unsigned char *)"This is a long string.\n";
    int start_offset = 0;
    int end_offset = strlen((const char *)str);
    unsigned char *reg_expr = (unsigned char *)"is";
    _Bool case_sensitive = 1;
    size_t end_match = 0;
    int start_match = e_rstrstr(end_offset, start_offset,
                                str, reg_expr, &end_match, case_sensitive);
    int expected_start_match = 5;
    size_t expected_end_match = 7;
    ck_assert_int_eq(expected_start_match,start_match);
    ck_assert_int_eq(expected_end_match, end_match);
}
END_TEST

Suite * search_suite(void)
{
    Suite *s;
    TCase *tc_search;

    s = suite_create( "Search" );

    tc_search = tcase_create( "Search Test" );

    tcase_add_test(tc_search, test_search_empty_string_empty);
    tcase_add_test(tc_search, test_search_string);
    tcase_add_test(tc_search, test_search_regex_string);
    tcase_add_test(tc_search, test_search_regex_string_word_boundary);
    tcase_add_test(tc_search, test_search_regex_empty);
    tcase_add_test(tc_search, test_search_regex_empty_string_empty);
    tcase_add_test(tc_search, test_search_regex_backward_string);
    suite_add_tcase(s, tc_search);

    return s;
}
