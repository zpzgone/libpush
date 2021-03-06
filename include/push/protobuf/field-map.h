/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2010, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the LICENSE.txt file in this distribution for license
 * details.
 * ----------------------------------------------------------------------
 */

#ifndef PUSH_PROTOBUF_FIELD_MAP_H
#define PUSH_PROTOBUF_FIELD_MAP_H

#include <stdbool.h>

#include <hwm-buffer.h>

#include <push/basics.h>
#include <push/protobuf/basics.h>


/**
 * A map of field numbers to callbacks for reading that field.  Used
 * within a message callback; once it reads in a field number, it
 * dispatches to a callback that can read that field.  This map stores
 * the list of possible callbacks.
 */

typedef struct _push_protobuf_field_map  push_protobuf_field_map_t;


/**
 * Create a new field map.  The field map should be created and
 * populated before creating the message callback that will use it.
 */

push_protobuf_field_map_t *
push_protobuf_field_map_new(void *parent);


/**
 * Sets the success continuation pointer for all of the callbacks in
 * the field map.
 */

void
push_protobuf_field_map_set_success
(push_protobuf_field_map_t *field_map,
 push_success_continuation_t *success);


/**
 * Sets the incomplete continuation pointer for all of the callbacks
 * in the field map.
 */

void
push_protobuf_field_map_set_incomplete
(push_protobuf_field_map_t *field_map,
 push_incomplete_continuation_t *incomplete);


/**
 * Sets the error continuation pointer for all of the callbacks in the
 * field map.
 */

void
push_protobuf_field_map_set_error
(push_protobuf_field_map_t *field_map,
 push_error_continuation_t *error);


/**
 * Add a new field to a field map.  The value callback will be called
 * whenever we read a field with the matching field number.  This
 * value callback should also store the parsed value away somewhere,
 * if needed.  We also verify that the tag type matches what we
 * expect, throwing a parse error if it doesn't.
 *
 * @return <code>false</code> if we cannot add the new field.
 */

bool
push_protobuf_field_map_add_field
(const char *field_name,
 push_parser_t *parser,
 push_protobuf_field_map_t *field_map,
 push_protobuf_tag_number_t field_number,
 push_protobuf_tag_type_t expected_tag_type,
 push_callback_t *value_callback);


/**
 * Get the field callback for the specified field.  If that field
 * isn't in the field map, return NULL.
 */

push_callback_t *
push_protobuf_field_map_get_field
    (push_protobuf_field_map_t *field_map,
     push_protobuf_tag_number_t field_number);


/**
 * Add a new submessage to a field map.
 *
 * @return <code>false</code> if we cannot add the new field.
 */

bool
push_protobuf_add_submessage(const char *message_name,
                             const char *field_name,
                             void *parent,
                             push_parser_t *parser,
                             push_protobuf_field_map_t *field_map,
                             push_protobuf_tag_number_t field_number,
                             push_callback_t *message);


/**
 * Create a new callback that reads a length-prefixed Protocol Buffer
 * string into a high-water mark buffer.
 */

bool
push_protobuf_add_hwm_string(const char *message_name,
                             const char *field_name,
                             void *parent,
                             push_parser_t *parser,
                             push_protobuf_field_map_t *field_map,
                             push_protobuf_tag_number_t field_number,
                             hwm_buffer_t *dest);


/**
 * Add a new <code>uint32</code> field to a field map.  When parsing,
 * the field's value will be assigned to the dest pointer.
 *
 * @return <code>false</code> if we cannot add the new field.
 */

bool
push_protobuf_assign_uint32(const char *message_name,
                            const char *field_name,
                            void *parent,
                            push_parser_t *parser,
                            push_protobuf_field_map_t *field_map,
                            push_protobuf_tag_number_t field_number,
                            uint32_t *dest);


/**
 * Add a new <code>uint64</code> field to a field map.  When parsing,
 * the field's value will be assigned to the dest pointer.
 *
 * @return <code>false</code> if we cannot add the new field.
 */

bool
push_protobuf_assign_uint64(const char *message_name,
                            const char *field_name,
                            void *parent,
                            push_parser_t *parser,
                            push_protobuf_field_map_t *field_map,
                            push_protobuf_tag_number_t field_number,
                            uint64_t *dest);


/**
 * Add a new <code>int32</code> field to a field map.  When parsing,
 * the field's value will be assigned to the dest pointer.
 *
 * @return <code>false</code> if we cannot add the new field.
 */

bool
push_protobuf_assign_int32(const char *message_name,
                           const char *field_name,
                           void *parent,
                           push_parser_t *parser,
                           push_protobuf_field_map_t *field_map,
                           push_protobuf_tag_number_t field_number,
                           int32_t *dest);


/**
 * Add a new <code>int64</code> field to a field map.  When parsing,
 * the field's value will be assigned to the dest pointer.
 *
 * @return <code>false</code> if we cannot add the new field.
 */

bool
push_protobuf_assign_int64(const char *message_name,
                           const char *field_name,
                           void *parent,
                           push_parser_t *parser,
                           push_protobuf_field_map_t *field_map,
                           push_protobuf_tag_number_t field_number,
                           int64_t *dest);


/**
 * Add a new <code>sint32</code> field to a field map.  When parsing,
 * the field's value will be assigned to the dest pointer.
 *
 * @return <code>false</code> if we cannot add the new field.
 */

bool
push_protobuf_assign_sint32(const char *message_name,
                            const char *field_name,
                            void *parent,
                            push_parser_t *parser,
                            push_protobuf_field_map_t *field_map,
                            push_protobuf_tag_number_t field_number,
                            int32_t *dest);


/**
 * Add a new <code>sint64</code> field to a field map.  When parsing,
 * the field's value will be assigned to the dest pointer.
 *
 * @return <code>false</code> if we cannot add the new field.
 */

bool
push_protobuf_assign_sint64(const char *message_name,
                            const char *field_name,
                            void *parent,
                            push_parser_t *parser,
                            push_protobuf_field_map_t *field_map,
                            push_protobuf_tag_number_t field_number,
                            int64_t *dest);


#endif  /* PUSH_PROTOBUF_FIELD_MAP_H */
