/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "testutil.h"
#include "apr.h"
#include "apr_strings.h"
#include "apr_general.h"
#include "apr_pools.h"
#include "apr_skiplist.h"
#if APR_HAVE_STDIO_H
#include <stdio.h>
#endif
#if APR_HAVE_STDLIB_H
#include <stdlib.h>
#endif
#if APR_HAVE_STRING_H
#include <string.h>
#endif

static apr_pool_t *ptmp = NULL;
static apr_skiplist *skiplist = NULL;

static int skiplist_get_size(abts_case *tc, apr_skiplist *sl)
{
    size_t size = 0;
    apr_skiplistnode *n;
    for (n = apr_skiplist_getlist(sl); n; apr_skiplist_next(sl, &n)) {
        ++size;
    }
    ABTS_TRUE(tc, size == apr_skiplist_size(sl));
    return size;
}

static void skiplist_init(abts_case *tc, void *data)
{
    apr_time_t now = apr_time_now();
    srand((unsigned int)(((now >> 32) ^ now) & 0xffffffff));

    ABTS_INT_EQUAL(tc, APR_SUCCESS, apr_skiplist_init(&skiplist, p));
    ABTS_PTR_NOTNULL(tc, skiplist);
    apr_skiplist_set_compare(skiplist, (apr_skiplist_compare)strcmp,
                                       (apr_skiplist_compare)strcmp);
}

static void skiplist_find(abts_case *tc, void *data)
{
    const char *val;

    ABTS_PTR_NOTNULL(tc, apr_skiplist_insert(skiplist, "baton"));
    val = apr_skiplist_find(skiplist, "baton", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "baton", val);
}

static void skiplist_dontfind(abts_case *tc, void *data)
{
    const char *val;

    val = apr_skiplist_find(skiplist, "keynotthere", NULL);
    ABTS_PTR_EQUAL(tc, NULL, (void *)val);
}

static void skiplist_insert(abts_case *tc, void *data)
{
    const char *val;
    int i, height = 0;

    for (i = 0; i < 10; ++i) {
        ABTS_PTR_EQUAL(tc, NULL, apr_skiplist_insert(skiplist, "baton"));
        ABTS_TRUE(tc, 1 == skiplist_get_size(tc, skiplist));
        val = apr_skiplist_find(skiplist, "baton", NULL);
        ABTS_PTR_NOTNULL(tc, val);
        ABTS_STR_EQUAL(tc, "baton", val);

        if (height == 0) {
            height = apr_skiplist_height(skiplist);
        }
        else {
            ABTS_INT_EQUAL(tc, height, apr_skiplist_height(skiplist));
        }
    }

    ABTS_PTR_NOTNULL(tc, apr_skiplist_insert(skiplist, "foo"));
    ABTS_TRUE(tc, 2 == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "foo", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "foo", val);

    ABTS_PTR_NOTNULL(tc, apr_skiplist_insert(skiplist, "atfirst"));
    ABTS_TRUE(tc, 3 == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "atfirst", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "atfirst", val);
}

#define NUM_ADDS 100
static void skiplist_add(abts_case *tc, void *data)
{
    const char *val;
    size_t i, n = 0;

    apr_skiplist_remove_all(skiplist, NULL);
    ABTS_TRUE(tc, 0 == skiplist_get_size(tc, skiplist));

    for (i = 0; i < NUM_ADDS; ++i) {
        n++;
        ABTS_PTR_NOTNULL(tc, apr_skiplist_add(skiplist, "daton"));
        ABTS_TRUE(tc, n == skiplist_get_size(tc, skiplist));
        val = apr_skiplist_find(skiplist, "daton", NULL);
        ABTS_PTR_NOTNULL(tc, val);
        ABTS_STR_EQUAL(tc, "daton", val);

        n++;
        ABTS_PTR_NOTNULL(tc, apr_skiplist_add(skiplist, "baton"));
        ABTS_TRUE(tc, n == skiplist_get_size(tc, skiplist));
        val = apr_skiplist_find(skiplist, "baton", NULL);
        ABTS_PTR_NOTNULL(tc, val);
        ABTS_STR_EQUAL(tc, "baton", val);

        n++;
        ABTS_PTR_NOTNULL(tc, apr_skiplist_add(skiplist, "caton"));
        ABTS_TRUE(tc, n == skiplist_get_size(tc, skiplist));
        val = apr_skiplist_find(skiplist, "caton", NULL);
        ABTS_PTR_NOTNULL(tc, val);
        ABTS_STR_EQUAL(tc, "caton", val);

        n++;
        ABTS_PTR_NOTNULL(tc, apr_skiplist_add(skiplist, "aaton"));
        ABTS_TRUE(tc, n == skiplist_get_size(tc, skiplist));
        val = apr_skiplist_find(skiplist, "aaton", NULL);
        ABTS_PTR_NOTNULL(tc, val);
        ABTS_STR_EQUAL(tc, "aaton", val);
    }
}

static void skiplist_replace(abts_case *tc, void *data)
{
    const char *val;
    size_t n = skiplist_get_size(tc, skiplist);

    n -= NUM_ADDS - 1;
    apr_skiplist_replace(skiplist, "daton", NULL);
    ABTS_TRUE(tc, n == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "daton", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "daton", val);

    n -= NUM_ADDS - 1;
    apr_skiplist_replace(skiplist, "baton", NULL);
    ABTS_TRUE(tc, n == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "baton", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "baton", val);

    n -= NUM_ADDS - 1;
    apr_skiplist_replace(skiplist, "caton", NULL);
    ABTS_TRUE(tc, n == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "caton", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "caton", val);

    n -= NUM_ADDS - 1;
    apr_skiplist_replace(skiplist, "aaton", NULL);
    ABTS_TRUE(tc, n == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "aaton", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "aaton", val);

    ABTS_TRUE(tc, n == 4);
}

static void skiplist_destroy(abts_case *tc, void *data)
{
    apr_skiplist_destroy(skiplist, NULL);
    ABTS_TRUE(tc, 0 == apr_skiplist_size(skiplist));
    ABTS_TRUE(tc, 1 == apr_skiplist_height(skiplist));
    ABTS_TRUE(tc, NULL == apr_skiplist_getlist(skiplist));
}

static void skiplist_size(abts_case *tc, void *data)
{
    const char *val;

    ABTS_TRUE(tc, 0 == skiplist_get_size(tc, skiplist));

    ABTS_PTR_NOTNULL(tc, apr_skiplist_insert(skiplist, "abc"));
    ABTS_PTR_NOTNULL(tc, apr_skiplist_insert(skiplist, "ghi"));
    ABTS_PTR_NOTNULL(tc, apr_skiplist_insert(skiplist, "def"));
    val = apr_skiplist_find(skiplist, "abc", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "abc", val);
    val = apr_skiplist_find(skiplist, "ghi", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "ghi", val);
    val = apr_skiplist_find(skiplist, "def", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "def", val);

    ABTS_TRUE(tc, 3 == skiplist_get_size(tc, skiplist));
    apr_skiplist_destroy(skiplist, NULL);
}

static void skiplist_remove(abts_case *tc, void *data)
{
    const char *val;

    ABTS_TRUE(tc, 0 == skiplist_get_size(tc, skiplist));

    ABTS_PTR_NOTNULL(tc, apr_skiplist_add(skiplist, "baton"));
    ABTS_TRUE(tc, 1 == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "baton", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "baton", val);

    ABTS_PTR_NOTNULL(tc, apr_skiplist_add(skiplist, "baton"));
    ABTS_TRUE(tc, 2 == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "baton", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "baton", val);

    ABTS_TRUE(tc, apr_skiplist_remove(skiplist, "baton", NULL) != 0);
    ABTS_TRUE(tc, 1 == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "baton", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "baton", val);

    ABTS_PTR_NOTNULL(tc, apr_skiplist_add(skiplist, "baton"));
    ABTS_TRUE(tc, 2 == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "baton", NULL);
    ABTS_PTR_NOTNULL(tc, val);
    ABTS_STR_EQUAL(tc, "baton", val);

    /* remove all "baton"s */
    while (apr_skiplist_remove(skiplist, "baton", NULL))
        ;
    ABTS_TRUE(tc, 0 == skiplist_get_size(tc, skiplist));
    val = apr_skiplist_find(skiplist, "baton", NULL);
    ABTS_PTR_EQUAL(tc, NULL, val);
}

#define NUM_RAND (100)
#define NUM_FIND (3 * NUM_RAND)
static void skiplist_random_loop(abts_case *tc, void *data)
{
    char **batons;
    apr_skiplist *sl;
    const char *val;
    int i;

    ABTS_INT_EQUAL(tc, APR_SUCCESS, apr_skiplist_init(&sl, ptmp));
    apr_skiplist_set_compare(sl, (apr_skiplist_compare)strcmp,
                                 (apr_skiplist_compare)strcmp);
    apr_skiplist_set_preheight(sl, 7);
    ABTS_INT_EQUAL(tc, 7, apr_skiplist_preheight(sl));

    batons = apr_palloc(ptmp, NUM_FIND * sizeof(char*));

    for (i = 0; i < NUM_FIND; ++i) {
        if (i < NUM_RAND) {
            batons[i] = apr_psprintf(ptmp, "%.6u", rand() % 1000000);
        }
        else {
            batons[i] = apr_pstrdup(ptmp, batons[i % NUM_RAND]);
        }
        ABTS_PTR_NOTNULL(tc, apr_skiplist_add(sl, batons[i]));
        val = apr_skiplist_find(sl, batons[i], NULL);
        ABTS_PTR_NOTNULL(tc, val);
        ABTS_STR_EQUAL(tc, batons[i], val);
    }

    apr_pool_clear(ptmp);
}

typedef struct elem {
    int b;
    int a;
} elem;


static void add_int_to_skiplist(abts_case *tc, apr_skiplist *list, int n){
    int* a = apr_skiplist_alloc(list, sizeof(int));
    *a = n;
    ABTS_PTR_NOTNULL(tc, apr_skiplist_add(list, a));
}

static void add_elem_to_skiplist(abts_case *tc, apr_skiplist *list, elem n){
    elem* a = apr_skiplist_alloc(list, sizeof(elem));
    *a = n;
    ABTS_PTR_NOTNULL(tc, apr_skiplist_add(list, a));
}

static int comp(void *a, void *b){
    return *((int*) a) - *((int*) b);
}

static int scomp(void *a, void *b){
    return ((elem*) a)->a - ((elem*) b)->a;
}

static int ecomp(void *a, void *b)
{
    elem const * const e1 = a;
    elem const * const e2 = b;
    if (e1->a < e2->a) {
        return -1;
    }
    else if (e1->a > e2->a) {
        return +1;
    }
    else if (e1->b < e2->b) {
        return -1;
    }
    else if (e1->b > e2->b) {
        return +1;
    }
    else {
        return 0;
    }
}

/* Some tests below add multiple duplicates and then try to remove each one
 * individually, in arbitrary order.
 *
 * Using apr_skiplist_remove_compare(..., scomp, NULL) would not work because
 * it will likely remove any duplicate (the first one) encountered on the path,
 * hence possibly not the expected one.
 * 
 * Using apr_skiplist_remove_compare(..., ecomp, NULL) works provided all the
 * duplicates (same a) don't also have the same b (which is the case in the
 * test below), hence uniqueness is cooked in the elem itself.
 *
 * Another possibility is to rely on unique pointers, and then cook a remove
 * function, like the following skiplist_remove_scomp(), which will go straight
 * to the last duplicate (using scomp) and then iterate on the previous elems
 * until pointers match.
 *
 * Providing uniqueness in the elem itself is the more clean/efficient option,
 * but if all you have is a unique pointer the pattern in the function may be
 * worth it ( or it's just a way to test several skiplist functionalities :)
 */
static void skiplist_remove_scomp(abts_case *tc, apr_skiplist *list, elem *n)
{
    elem *e;
    apr_skiplistnode *iter = NULL;
    e = apr_skiplist_last(list, n, &iter);
    while (e && e != n) {
        ABTS_INT_EQUAL(tc, 0, scomp(n, e));
        e = apr_skiplist_previous(list, &iter);
    }
    ABTS_PTR_EQUAL(tc, n, apr_skiplist_element(iter));
    apr_skiplist_remove_node(list, iter, NULL);
}

static void skiplist_test(abts_case *tc, void *data) {
    int test_elems = 10;
    int i = 0, j = 0;
    int *val = NULL;
    elem *val2 = NULL;
    apr_skiplist * list = NULL;
    apr_skiplist * list2 = NULL;
    apr_skiplist * list3 = NULL;
    apr_skiplist * list4 = NULL;
    int first_forty_two = 42,
        second_forty_two = 42;
    apr_array_header_t *array;
    elem t1, t2, t3, t4, t5;
    t1.a = 1; t1.b = 1;
    t2.a = 42; t2.b = 1;
    t3.a = 42; t3.b = 2;
    t4.a = 42; t4.b = 3;
    t5.a = 142; t5.b = 1;

    ABTS_INT_EQUAL(tc, APR_SUCCESS, apr_skiplist_init(&list, ptmp));
    apr_skiplist_set_compare(list, comp, comp);
    
    /* insert 10 objects */
    for (i = 0; i < test_elems; ++i){
        add_int_to_skiplist(tc, list, i);
    }

    /* remove all objects */
    while ((val = apr_skiplist_pop(list, NULL))){
        ABTS_INT_EQUAL(tc, *val, j++);
    }

    /* insert 10 objects again */
    for (i = test_elems; i < test_elems+test_elems; ++i){
        add_int_to_skiplist(tc, list, i);
    }

    j = test_elems;
    while ((val = apr_skiplist_pop(list, NULL))){
        ABTS_INT_EQUAL(tc, *val, j++);
    }

    /* empty */
    val = apr_skiplist_pop(list, NULL);
    ABTS_PTR_EQUAL(tc, val, NULL);

    add_int_to_skiplist(tc, list, 42);
    val = apr_skiplist_pop(list, NULL);
    ABTS_INT_EQUAL(tc, *val, 42); 

    /* empty */
    val = apr_skiplist_pop(list, NULL);
    ABTS_PTR_EQUAL(tc, val, NULL);

    ABTS_PTR_NOTNULL(tc, apr_skiplist_add(list, &first_forty_two));
    add_int_to_skiplist(tc, list, 1);
    add_int_to_skiplist(tc, list, 142);
    ABTS_PTR_NOTNULL(tc, apr_skiplist_add(list, &second_forty_two));
    val = apr_skiplist_peek(list);
    ABTS_INT_EQUAL(tc, *val, 1);
    val = apr_skiplist_pop(list, NULL);
    ABTS_INT_EQUAL(tc, *val, 1);
    val = apr_skiplist_peek(list);
    ABTS_PTR_EQUAL(tc, val, &first_forty_two);
    ABTS_INT_EQUAL(tc, *val, 42);
    val = apr_skiplist_pop(list, NULL);
    ABTS_PTR_EQUAL(tc, val, &first_forty_two);
    ABTS_INT_EQUAL(tc, *val, 42);
    val = apr_skiplist_pop(list, NULL);
    ABTS_PTR_EQUAL(tc, val, &second_forty_two);
    ABTS_INT_EQUAL(tc, *val, 42);
    val = apr_skiplist_peek(list);
    ABTS_INT_EQUAL(tc, *val, 142);

    ABTS_INT_EQUAL(tc, APR_SUCCESS, apr_skiplist_init(&list2, ptmp));
    apr_skiplist_set_compare(list2, scomp, scomp);
    add_elem_to_skiplist(tc, list2, t2);
    add_elem_to_skiplist(tc, list2, t1);
    add_elem_to_skiplist(tc, list2, t3);
    add_elem_to_skiplist(tc, list2, t5);
    add_elem_to_skiplist(tc, list2, t4);
    val2 = apr_skiplist_pop(list2, NULL);
    ABTS_INT_EQUAL(tc, val2->a, 1);
    val2 = apr_skiplist_pop(list2, NULL);
    ABTS_INT_EQUAL(tc, val2->a, 42);
    ABTS_INT_EQUAL(tc, val2->b, 1);
    val2 = apr_skiplist_pop(list2, NULL);
    ABTS_INT_EQUAL(tc, val2->a, 42);
    ABTS_INT_EQUAL(tc, val2->b, 2);
    val2 = apr_skiplist_pop(list2, NULL);
    ABTS_INT_EQUAL(tc, val2->a, 42);
    ABTS_INT_EQUAL(tc, val2->b, 3);
    val2 = apr_skiplist_pop(list2, NULL);
    ABTS_INT_EQUAL(tc, val2->a, 142);
    ABTS_INT_EQUAL(tc, val2->b, 1);

    ABTS_INT_EQUAL(tc, APR_SUCCESS, apr_skiplist_init(&list3, ptmp));
    apr_skiplist_set_compare(list3, ecomp, ecomp);
    array = apr_array_make(ptmp, 10, sizeof(elem *));
    for (i = 0; i < 10; ++i) {
        elem *e = apr_palloc(ptmp, sizeof *e);
        e->a = 4224;
        e->b = i;
        APR_ARRAY_PUSH(array, elem *) = e;
        ABTS_PTR_NOTNULL(tc, apr_skiplist_insert(list3, e));
    }
    for (i = 0; i < 5; ++i) {
        elem *e = APR_ARRAY_IDX(array, i, elem *);
        val2 = apr_skiplist_find(list3, e, NULL);
        ABTS_PTR_EQUAL(tc, e, val2);
        ABTS_TRUE(tc, apr_skiplist_remove(list3, e, NULL) != 0);
    }
    for (i = 0; i < 5; ++i) {
        elem *e = APR_ARRAY_IDX(array, 9 - i, elem *);
        val2 = apr_skiplist_find(list3, e, NULL);
        ABTS_PTR_EQUAL(tc, e, val2);
        ABTS_TRUE(tc, apr_skiplist_remove(list3, e, NULL) != 0);
    }

    ABTS_INT_EQUAL(tc, APR_SUCCESS, apr_skiplist_init(&list4, ptmp));
    apr_skiplist_set_compare(list4, scomp, scomp);
    for (i = 0; i < 5; ++i){
        add_elem_to_skiplist(tc, list4, t1);
    }
    for (i = 0; i < 5; ++i){
        add_elem_to_skiplist(tc, list4, t2);
    }
    apr_skiplist_add(list4, &t2);
    for (i = 0; i < 5; ++i){
        add_elem_to_skiplist(tc, list4, t2);
    }
    for (i = 0; i < 5; ++i){
        add_elem_to_skiplist(tc, list4, t3);
    }
    apr_skiplist_add(list4, &t3);
    for (i = 0; i < 5; ++i){
        add_elem_to_skiplist(tc, list4, t3);
    }
    for (i = 0; i < 5; ++i){
        add_elem_to_skiplist(tc, list4, t4);
    }
    apr_skiplist_add(list4, &t4);
    for (i = 0; i < 5; ++i){
        add_elem_to_skiplist(tc, list4, t4);
    }
    for (i = 0; i < 5; ++i){
        add_elem_to_skiplist(tc, list4, t5);
    }
    skiplist_remove_scomp(tc, list4, &t2);
    skiplist_remove_scomp(tc, list4, &t3);
    skiplist_remove_scomp(tc, list4, &t4);

    apr_pool_clear(ptmp);
}


abts_suite *testskiplist(abts_suite *suite)
{
    suite = ADD_SUITE(suite)

    apr_pool_create(&ptmp, p);

    abts_run_test(suite, skiplist_init, NULL);
    abts_run_test(suite, skiplist_find, NULL);
    abts_run_test(suite, skiplist_dontfind, NULL);
    abts_run_test(suite, skiplist_insert, NULL);
    abts_run_test(suite, skiplist_add, NULL);
    abts_run_test(suite, skiplist_replace, NULL);
    abts_run_test(suite, skiplist_destroy, NULL);
    abts_run_test(suite, skiplist_size, NULL);
    abts_run_test(suite, skiplist_remove, NULL);
    abts_run_test(suite, skiplist_random_loop, NULL);

    abts_run_test(suite, skiplist_test, NULL);

    apr_pool_destroy(ptmp);

    return suite;
}

