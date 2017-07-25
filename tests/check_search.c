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
#include "../src/we_find.h"
#include "check_search.h"

START_TEST(test_search_null_request)
{
	Search_request *request = NULL;
	_Bool expected_result = 0;
	Search_result result = e_search_line(request);
	_Bool actual_result = result.match_result;
	ck_assert_int_eq (expected_result, actual_result);
}
END_TEST

START_TEST(test_search_null)
{
	const char *str = NULL;
	const char *needle = NULL;
	Search_request request;
	request.start_offset = 0;
	request.end_offset = 0;
	request.haystack = (unsigned char *)str;
	request.needle = (unsigned char *)needle;
	_Bool expected_result = 0;
	Search_result result = e_search_line(&request);
	_Bool actual_result = result.match_result;
	ck_assert_int_eq (expected_result, actual_result);
}
END_TEST

START_TEST(test_search_normal)
{
	const char *str = "This is a big, Big, BiG String to Search In. Maybe not BIG enough.\n";
	const char *needle = "big";
	Search_request request;
	request.start_offset = 0;
	request.end_offset = 1;
	request.haystack = (unsigned char *)str;
	request.needle = (unsigned char *)needle;
	_Bool expected_result = 0;
	Search_result result = e_search_line(&request);
	_Bool actual_result = result.match_result;
	ck_assert_int_eq (expected_result, actual_result);
}
END_TEST

Suite * search_suite(void)
{
	Suite *s;
	TCase *tc_search;

	s = suite_create( "Search" );

	tc_search = tcase_create( "Search Test" );

	tcase_add_test(tc_search, test_search_null_request);
	tcase_add_test(tc_search, test_search_null);
	tcase_add_test(tc_search, test_search_normal);
	suite_add_tcase(s, tc_search);

	return s;
}
