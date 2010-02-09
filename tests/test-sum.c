/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2009-2010, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the LICENSE.txt file in this distribution for license
 * details.
 * ----------------------------------------------------------------------
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <check.h>

#include <push/basics.h>
#include <push/combinators.h>
#include <push/pairs.h>

#include <test-callbacks.h>


/*-----------------------------------------------------------------------
 * Folded sum callback
 */

static push_callback_t *
make_repeated_sum()
{
    push_callback_t  *sum;
    push_callback_t  *fold;

    sum = sum_callback_new();
    fold = push_fold_new(sum);

    return fold;
}


/*-----------------------------------------------------------------------
 * Sample data
 */

uint32_t  INT_0 = 0;

const uint32_t  DATA_01[] = { 1, 2, 3, 4, 5 };
const size_t  LENGTH_01 = 5 * sizeof(uint32_t);


/*-----------------------------------------------------------------------
 * Test cases
 */


START_TEST(test_sum_01)
{
    push_parser_t  *parser;
    push_callback_t  *callback;
    uint32_t  *result;

    PUSH_DEBUG_MSG("---\nStarting test_sum_01\n");

    callback = make_repeated_sum();
    fail_if(callback == NULL,
            "Could not allocate a new sum callback");

    parser = push_parser_new(callback);
    fail_if(parser == NULL,
            "Could not allocate a new push parser");

    fail_unless(push_parser_activate(parser, &INT_0)
                == PUSH_SUCCESS,
                "Could not activate parser");

    fail_unless(push_parser_submit_data
                (parser, &DATA_01, LENGTH_01) == PUSH_INCOMPLETE,
                "Could not parse data");

    fail_unless(push_parser_eof(parser) == PUSH_SUCCESS,
                "Shouldn't get parse error at EOF");

    result = (uint32_t *) callback->result;

    fail_unless(*result == 15,
                "Sum doesn't match (got %"PRIu32
                ", expected %"PRIu32")",
                *result, 15);

    push_parser_free(parser);
}
END_TEST


START_TEST(test_sum_02)
{
    push_parser_t  *parser;
    push_callback_t  *callback;
    uint32_t  *result;

    PUSH_DEBUG_MSG("---\nStarting test_sum_02\n");

    /*
     * If we submit the data twice, we should get twice the result.
     */

    callback = make_repeated_sum();
    fail_if(callback == NULL,
            "Could not allocate a new sum callback");

    parser = push_parser_new(callback);
    fail_if(parser == NULL,
            "Could not allocate a new push parser");

    fail_unless(push_parser_activate(parser, &INT_0)
                == PUSH_SUCCESS,
                "Could not activate parser");

    fail_unless(push_parser_submit_data
                (parser, &DATA_01, LENGTH_01) == PUSH_INCOMPLETE,
                "Could not parse data");

    fail_unless(push_parser_submit_data
                (parser, &DATA_01, LENGTH_01) == PUSH_INCOMPLETE,
                "Could not parse data");

    fail_unless(push_parser_eof(parser) == PUSH_SUCCESS,
                "Shouldn't get parse error at EOF");

    result = (uint32_t *) callback->result;

    fail_unless(*result == 30,
                "Sum doesn't match (got %"PRIu32
                ", expected %"PRIu32")",
                *result, 30);

    push_parser_free(parser);
}
END_TEST


START_TEST(test_misaligned_data)
{
    push_parser_t  *parser;
    push_callback_t  *callback;
    uint32_t  *result;
    size_t  FIRST_CHUNK_SIZE = 7; /* something not divisible by 4 */

    PUSH_DEBUG_MSG("---\nStarting test_misaligned_data\n");

    /*
     * Our callback processes ints on nice 32-bit boundaries.  If we
     * send the parser data that doesn't align with these boundaries,
     * we should still get the right answer.
     */

    callback = make_repeated_sum();
    fail_if(callback == NULL,
            "Could not allocate a new sum callback");

    parser = push_parser_new(callback);
    fail_if(parser == NULL,
            "Could not allocate a new push parser");

    fail_unless(push_parser_activate(parser, &INT_0)
                == PUSH_SUCCESS,
                "Could not activate parser");

    fail_unless(push_parser_submit_data
                (parser, &DATA_01, FIRST_CHUNK_SIZE) == PUSH_INCOMPLETE,
                "Could not parse data");

    fail_unless(push_parser_submit_data
                (parser,
                 ((void *) DATA_01) + FIRST_CHUNK_SIZE,
                 LENGTH_01 - FIRST_CHUNK_SIZE) == PUSH_INCOMPLETE,
                "Could not parse data");

    fail_unless(push_parser_eof(parser) == PUSH_SUCCESS,
                "Shouldn't get parse error at EOF");

    result = (uint32_t *) callback->result;

    fail_unless(*result == 15,
                "Sum doesn't match (got %"PRIu32
                ", expected %"PRIu32")",
                *result, 15);

    push_parser_free(parser);
}
END_TEST


START_TEST(test_parse_error_01)
{
    push_parser_t  *parser;
    push_callback_t  *callback;
    size_t  FIRST_CHUNK_SIZE = 7; /* something not divisible by 4 */

    PUSH_DEBUG_MSG("---\nStarting test_parse_error_01\n");

    /*
     * Our callback processes ints on nice 32-bit boundaries.  If we
     * send the parser data that doesn't align with these boundaries,
     * and then reach EOF, we should get a parse error.
     */

    callback = make_repeated_sum();
    fail_if(callback == NULL,
            "Could not allocate a new sum callback");

    parser = push_parser_new(callback);
    fail_if(parser == NULL,
            "Could not allocate a new push parser");

    fail_unless(push_parser_activate(parser, &INT_0)
                == PUSH_SUCCESS,
                "Could not activate parser");

    fail_unless(push_parser_submit_data
                (parser, &DATA_01, FIRST_CHUNK_SIZE) == PUSH_INCOMPLETE,
                "Could not parse data");

    fail_unless(push_parser_eof(parser) == PUSH_PARSE_ERROR,
                "Should get parse error at EOF");

    push_parser_free(parser);
}
END_TEST


START_TEST(test_max_01)
{
    push_parser_t  *parser;
    push_callback_t  *sum;
    push_callback_t  *callback;
    uint32_t  *result;

    PUSH_DEBUG_MSG("---\nStarting test_max_01\n");

    /*
     * If we use max-bytes to limit ourselves to three numbers, we
     * should get a smaller sum.
     */

    sum = make_repeated_sum();
    fail_if(sum == NULL,
            "Could not allocate a new sum callback");

    callback = push_max_bytes_new(sum, sizeof(uint32_t) * 3);
    fail_if(callback == NULL,
            "Could not allocate a new max-bytes callback");

    parser = push_parser_new(callback);
    fail_if(parser == NULL,
            "Could not allocate a new push parser");

    fail_unless(push_parser_activate(parser, &INT_0)
                == PUSH_SUCCESS,
                "Could not activate parser");

    fail_unless(push_parser_submit_data
                (parser, &DATA_01, LENGTH_01) == PUSH_SUCCESS,
                "Could not parse data");

    fail_unless(push_parser_eof(parser) == PUSH_SUCCESS,
                "Shouldn't get parse error at EOF");

    result = (uint32_t *) callback->result;

    fail_unless(*result == 6,
                "Sum doesn't match (got %"PRIu32
                ", expected %"PRIu32")",
                *result, 6);

    push_parser_free(parser);
}
END_TEST


START_TEST(test_max_02)
{
    push_parser_t  *parser;
    push_callback_t  *sum1;
    push_callback_t  *sum2;
    push_callback_t  *max1;
    push_callback_t  *max2;
    push_callback_t  *callback;
    push_pair_t  *pair;
    uint32_t  *result1;
    uint32_t  *result2;

    PUSH_DEBUG_MSG("---\nStarting test_max_02\n");

    /*
     * If we use max-bytes to limit ourselves to two numbers, we
     * should get a smaller sum.  Then we repeat to get another
     * smaller sum.
     */

    sum1 = make_repeated_sum();
    fail_if(sum1 == NULL,
            "Could not allocate a new sum callback");

    max1 = push_max_bytes_new(sum1, sizeof(uint32_t) * 2);
    fail_if(max1 == NULL,
            "Could not allocate a new max-bytes callback");

    sum2 = make_repeated_sum();
    fail_if(sum2 == NULL,
            "Could not allocate a new sum callback");

    max2 = push_max_bytes_new(sum2, sizeof(uint32_t) * 2);
    fail_if(max2 == NULL,
            "Could not allocate a new max-bytes callback");

    callback = push_both_new(max1, max2);
    fail_if(callback == NULL,
            "Could not allocate a new both callback");

    parser = push_parser_new(callback);
    fail_if(parser == NULL,
            "Could not allocate a new push parser");

    fail_unless(push_parser_activate(parser, &INT_0)
                == PUSH_SUCCESS,
                "Could not activate parser");

    fail_unless(push_parser_submit_data
                (parser, &DATA_01, LENGTH_01) == PUSH_SUCCESS,
                "Could not parse data");

    fail_unless(push_parser_eof(parser) == PUSH_SUCCESS,
                "Shouldn't get parse error at EOF");

    pair = (push_pair_t *) callback->result;
    result1 = (uint32_t *) pair->first;
    result2 = (uint32_t *) pair->second;

    fail_unless(*result1 == 3,
                "Sum doesn't match (got %"PRIu32
                ", expected %"PRIu32")",
                *result1, 3);

    fail_unless(*result2 == 7,
                "Sum doesn't match (got %"PRIu32
                ", expected %"PRIu32")",
                *result2, 7);

    push_parser_free(parser);
}
END_TEST


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("sum");

    TCase  *tc = tcase_create("sum");
    tcase_add_test(tc, test_sum_01);
    tcase_add_test(tc, test_sum_02);
    tcase_add_test(tc, test_misaligned_data);
    tcase_add_test(tc, test_parse_error_01);
    tcase_add_test(tc, test_max_01);
    tcase_add_test(tc, test_max_02);
    suite_add_tcase(s, tc);

    return s;
}


int
main(int argc, const char **argv)
{
    int  number_failed;
    Suite  *suite = test_suite();
    SRunner  *runner = srunner_create(suite);

    srunner_run_all(runner, CK_NORMAL);
    number_failed = srunner_ntests_failed(runner);
    srunner_free(runner);

    return (number_failed == 0)? EXIT_SUCCESS: EXIT_FAILURE;
}
