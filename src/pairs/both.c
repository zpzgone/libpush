/* -*- coding: utf-8 -*-
 * ----------------------------------------------------------------------
 * Copyright © 2009-2010, RedJack, LLC.
 * All rights reserved.
 *
 * Please see the LICENSE.txt file in this distribution for license
 * details.
 * ----------------------------------------------------------------------
 */

#include <push/basics.h>
#include <push/combinators.h>
#include <push/pairs.h>
#include <push/pure.h>
#include <push/primitives.h>
#include <push/talloc.h>


static bool
duplicate(push_pair_t *result, void *input, void **output)
{
    result->first = input;
    result->second = input;

    *output = result;
    return true;
}


push_define_pure_data_callback(dup_new, duplicate, "dup",
                               void, void, push_pair_t);


push_callback_t *
push_dup_new(const char *name,
             void *parent,
             push_parser_t *parser)
{
    return dup_new(name, parent, parser, NULL);
}


/*
 * For now, we just implement the “both” combinator using the
 * definition from Hughes's paper:
 *
 *   a &&& b = arr (\a -> (a,a)) >>> (a *** b)
 */

push_callback_t *
push_both_new(const char *name,
              void *parent,
              push_parser_t *parser,
              push_callback_t *a,
              push_callback_t *b)
{
    void  *context;
    push_callback_t  *dup;
    push_callback_t  *par;
    push_callback_t  *callback;

    /*
     * If either wrapped callback is NULL, return NULL ourselves.
     */

    if ((a == NULL) || (b == NULL))
        return NULL;

    /*
     * Create a memory context for the objects we're about to create.
     */

    context = push_talloc_new(parent);
    if (context == NULL) return NULL;

    /*
     * Create the callbacks.
     */

    if (name == NULL) name = "both";

    dup = push_dup_new
        (push_talloc_asprintf(context, "%s.dup", name),
         context, parser);
    par = push_par_new
        (push_talloc_asprintf(context, "%s.par", name),
         context, parser, a, b);
    callback = push_compose_new
        (push_talloc_asprintf(context, "%s.compose", name),
         context, parser, dup, par);

    /*
     * Because of NULL propagation, we only have to check the last
     * result to see if everything was created okay.
     */

    if (callback == NULL) goto error;
    return callback;

  error:
    /*
     * Before returning, free any objects we created before the error.
     */

    push_talloc_free(context);
    return NULL;
}
