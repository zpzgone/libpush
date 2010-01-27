/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2010, RedJack, LLC.
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

#include <push.h>
#include <push/eof.h>
#include <push/protobuf.h>


/*-----------------------------------------------------------------------
 * Our data type
 */

typedef struct _data
{
    uint32_t  int1;
    uint64_t  int2;
} data_t;

static push_protobuf_message_t *
create_data_message(data_t *dest)
{
    push_protobuf_message_t  *result =
        push_protobuf_message_new();

    if (result == NULL)
        return NULL;

#define CHECK(call) { if (!(call)) return NULL; }

    CHECK(push_protobuf_assign_uint32(result, 1, &dest->int1));
    CHECK(push_protobuf_assign_uint64(result, 2, &dest->int2));

#undef CHECK

    return result;
}

static bool
data_eq(const data_t *d1, const data_t *d2)
{
    if (d1 == d2) return true;
    if ((d1 == NULL) || (d2 == NULL)) return false;
    return (d1->int1 == d2->int1) && (d1->int2 == d2->int2);
}


/*-----------------------------------------------------------------------
 * Sample data
 */

const uint8_t  DATA_01[] =
    "\x08"                      /* field 1, wire type 0 */
    "\xac\x02"                  /*   value = 300 */
    "\x10"                      /* field 2, wire type 0 */
    "\x80\xe4\x97\xd0\x12";     /*   value = 5,000,000,000 */
const size_t  LENGTH_01 = 9;
const data_t  EXPECTED_01 = { 300, UINT64_C(5000000000) };


/*-----------------------------------------------------------------------
 * Helper functions
 */

#define READ_TEST(test_name)                                        \
    START_TEST(test_read_##test_name)                               \
    {                                                               \
        push_parser_t  *parser;                                     \
        push_protobuf_message_t  *message_callback;                 \
        data_t  actual;                                             \
                                                                    \
        PUSH_DEBUG_MSG("---\nStarting test case "                   \
                       "test_read_"                                 \
                       #test_name                                   \
                       "\n");                                       \
                                                                    \
        message_callback = create_data_message(&actual);            \
        fail_if(message_callback == NULL,                           \
                "Could not allocate a new message callback");       \
                                                                    \
        parser = push_parser_new(&message_callback->base);          \
        fail_if(parser == NULL,                                     \
                "Could not allocate a new push parser");            \
                                                                    \
        fail_unless(push_parser_submit_data                         \
                    (parser,                                        \
                     &DATA_##test_name,                             \
                     LENGTH_##test_name) == PUSH_SUCCESS,           \
                    "Could not parse data");                        \
                                                                    \
        fail_unless(push_parser_eof(parser) == PUSH_SUCCESS,        \
                    "Shouldn't get parse error at EOF");            \
                                                                    \
        fail_unless(data_eq(&actual, &EXPECTED_##test_name),        \
                    "Value doesn't match (got "                     \
                    "(%"PRIu32",%"PRIu64")"                         \
                    ", expected "                                   \
                    "(%"PRIu32",%"PRIu64")"                         \
                    ")\n",                                          \
                    (uint32_t) actual.int1,                         \
                    (uint64_t) actual.int2,                         \
                    (uint32_t) EXPECTED_##test_name.int1,           \
                    (uint64_t) EXPECTED_##test_name.int2);          \
                                                                    \
        push_parser_free(parser);                                   \
    }                                                               \
    END_TEST


/*
 * Just like READ_TEST, but sends in the data in two chunks.  Tests
 * the ability of the callback to maintain its state across calls to
 * process_bytes.
 */

#define TWO_PART_READ_TEST(test_name)                               \
    START_TEST(test_two_part_read_##test_name)                      \
    {                                                               \
        push_parser_t  *parser;                                     \
        push_protobuf_message_t  *message_callback;                 \
        data_t  actual;                                             \
        size_t  first_chunk_size;                                   \
                                                                    \
        PUSH_DEBUG_MSG("---\nStarting test case "                   \
                       "test_two_part_read_"                        \
                       #test_name                                   \
                       "\n");                                       \
                                                                    \
        message_callback = create_data_message(&actual);            \
        fail_if(message_callback == NULL,                           \
                "Could not allocate a new message callback");       \
                                                                    \
        parser = push_parser_new(&message_callback->base);          \
        fail_if(parser == NULL,                                     \
                "Could not allocate a new push parser");            \
                                                                    \
        first_chunk_size = LENGTH_##test_name / 2;                  \
                                                                    \
        fail_unless(push_parser_submit_data                         \
                    (parser,                                        \
                     &DATA_##test_name,                             \
                     first_chunk_size) == PUSH_SUCCESS,             \
                    "Could not parse data");                        \
                                                                    \
        fail_unless(push_parser_submit_data                         \
                    (parser,                                        \
                     &DATA_##test_name[first_chunk_size],           \
                     LENGTH_##test_name - first_chunk_size) ==      \
                    PUSH_SUCCESS,                                   \
                    "Could not parse data");                        \
                                                                    \
        fail_unless(push_parser_eof(parser) == PUSH_SUCCESS,        \
                    "Shouldn't get parse error at EOF");            \
                                                                    \
        fail_unless(data_eq(&actual, &EXPECTED_##test_name),        \
                    "Value doesn't match (got "                     \
                    "(%"PRIu32",%"PRIu64")"                         \
                    ", expected "                                   \
                    "(%"PRIu32",%"PRIu64")"                         \
                    "\n",                                           \
                    (uint32_t) actual.int1,                         \
                    (uint64_t) actual.int2,                         \
                    (uint32_t) EXPECTED_##test_name.int1,           \
                    (uint64_t) EXPECTED_##test_name.int2);          \
                                                                    \
        push_parser_free(parser);                                   \
    }                                                               \
    END_TEST


/*
 * Just like READ_TEST, but chops off one byte before sending in the
 * data.  This should yield a parse error at EOF.
 */

#define PARSE_ERROR_TEST(test_name)                                 \
    START_TEST(test_parse_error_##test_name)                        \
    {                                                               \
        push_parser_t  *parser;                                     \
        push_protobuf_message_t  *message_callback;                 \
        data_t  actual;                                             \
                                                                    \
        PUSH_DEBUG_MSG("---\nStarting test case "                   \
                       "test_parse_error_"                          \
                       #test_name                                   \
                       "\n");                                       \
                                                                    \
        message_callback = create_data_message(&actual);            \
        fail_if(message_callback == NULL,                           \
                "Could not allocate a new message callback");       \
                                                                    \
        parser = push_parser_new(&message_callback->base);          \
        fail_if(parser == NULL,                                     \
                "Could not allocate a new push parser");            \
                                                                    \
        fail_unless(push_parser_submit_data                         \
                    (parser,                                        \
                     &DATA_##test_name,                             \
                     LENGTH_##test_name - 1) == PUSH_SUCCESS,       \
                    "Could not parse data");                        \
                                                                    \
        fail_unless(push_parser_eof(parser) == PUSH_PARSE_ERROR,    \
                    "Should get parse error at EOF");               \
                                                                    \
        push_parser_free(parser);                                   \
    }                                                               \
    END_TEST


/*-----------------------------------------------------------------------
 * Test cases
 */

READ_TEST(01)

TWO_PART_READ_TEST(01)

PARSE_ERROR_TEST(01)


/*-----------------------------------------------------------------------
 * Testing harness
 */

Suite *
test_suite()
{
    Suite  *s = suite_create("protobuf-message");

    TCase  *tc = tcase_create("protobuf-message");
    tcase_add_test(tc, test_read_01);
    tcase_add_test(tc, test_two_part_read_01);
    tcase_add_test(tc, test_parse_error_01);
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