/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2009, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the LICENSE.txt file in this distribution for license
 * details.
 * ----------------------------------------------------------------------
 */

#ifndef PUSH_PRIMITIVES_H
#define PUSH_PRIMITIVES_H

/**
 * @file
 *
 * This file defines the built-in primitive parser callbacks.  These
 * callbacks operator on their own, and do not wrap any other
 * callbacks.
 */

#include <hwm-buffer.h>

#include <push/basics.h>


/**
 * Create a new callback that requires the end of the stream.  If any
 * data is present, it results in a parse error.
 */

push_callback_t *
push_eof_new(const char *name,
             void *parent,
             push_parser_t *parser);


/**
 * Create a new callback that reads in a fixed amount of data into a
 * buffer.  This can be used to read constant-sized data structures,
 * for instance.  The result pointer is allowed to point directly into
 * the data chunk, so the caller should copy this into a separate
 * buffer if the data needs to be available across multiple data
 * chunks.
 */

push_callback_t *
push_fixed_new(const char *name,
               void *parent,
               push_parser_t *parser,
               size_t size);


/**
 * Create a new callback that reads a string into a high-water mark
 * buffer.  This callback doesn't do anything to determine the length
 * of the string; instead, it takes in a pointer to a size_t as input,
 * and uses that as the length of the string.  The callback's result
 * will be a pointer to the data in the HWM buffer — not a pointer to
 * the hwm_buffer_t object itself.  We will ensure that there is a NUL
 * pointer at the end of the string.
 */

push_callback_t *
push_hwm_string_new(const char *name,
                    void *parent,
                    push_parser_t *parser,
                    hwm_buffer_t *buf);


/**
 * Create a new callback that does nothing.  It parses no data, and
 * copies its input to its output.
 */

push_callback_t *
push_noop_new(const char *name,
              void *parent,
              push_parser_t *parser);


/**
 * Create a new callback that skips the specified number of bytes.
 * The callback's input should be a pointer to a size_t, indicating
 * the number of bytes to skip.
 */

push_callback_t *
push_skip_new(const char *name,
              void *parent,
              push_parser_t *parser);


#endif  /* PUSH_PRIMITIVES_H */
