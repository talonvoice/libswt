/**********************************************************
 * C-based/Cached/Core Computer Vision Library
 * Liu Liu, 2010-02-01
 **********************************************************/

#ifndef GUARD_ccv_internal_h
#define GUARD_ccv_internal_h

static int _CCV_PRINT_COUNT __attribute__ ((unused)) = 0;
static int _CCV_PRINT_LOOP __attribute__ ((unused)) = 0;

/* simple utility functions */

#define ccv_descale(x, n) (((x) + (1 << ((n) - 1))) >> (n))
#define conditional_assert(x, expr) if ((x)) { assert(expr); }

#ifdef USE_DISPATCH
#define parallel_for(x, n) dispatch_apply(n, dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0), ^(size_t x) {
#define parallel_endfor });
#else
#define parallel_for(x, n) { int x; for (x = 0; x < n; x++) {
#define parallel_endfor } }
#endif

/* macro printf utilities */

#define PRINT(l, a, ...) \
	do { \
		if (CCV_CLI_OUTPUT_LEVEL_IS(l)) \
		{ \
			printf(a, ##__VA_ARGS__); \
			fflush(stdout); \
		} \
	} while (0) // using do while (0) to force ; line end


#define FLUSH(l, a, ...) \
	do { \
		if (CCV_CLI_OUTPUT_LEVEL_IS(l)) \
		{ \
			for (_CCV_PRINT_LOOP = 0; _CCV_PRINT_LOOP < _CCV_PRINT_COUNT; _CCV_PRINT_LOOP++) \
				printf("\b"); \
			for (_CCV_PRINT_LOOP = 0; _CCV_PRINT_LOOP < _CCV_PRINT_COUNT; _CCV_PRINT_LOOP++) \
				printf(" "); \
			for (_CCV_PRINT_LOOP = 0; _CCV_PRINT_LOOP < _CCV_PRINT_COUNT; _CCV_PRINT_LOOP++) \
				printf("\b"); \
			_CCV_PRINT_COUNT = printf(a, ##__VA_ARGS__); \
			fflush(stdout); \
		} \
	} while (0) // using do while (0) to force ; line end

/* the macros enable us to preserve state of the program at any point in a structure way, this is borrowed from coroutine idea */

#define ccv_function_state_reserve_field int line_no;
#define ccv_function_state_begin(reader, s, file) (reader)((file), &(s)); switch ((s).line_no) { case 0:;
#define ccv_function_state_resume(writer, s, file) do { (s).line_no = __LINE__; (writer)(&(s), (file)); case __LINE__:; } while (0)
#define ccv_function_state_finish() }

/* the factor used to provide higher accuracy in integer type (all integer computation in some cases) */
#define _ccv_get_32s_value(ptr, i, factor) (((int*)(ptr))[(i)] << factor)
#define _ccv_get_32f_value(ptr, i, factor) ((float*)(ptr))[(i)]
#define _ccv_get_64s_value(ptr, i, factor) (((int64_t*)(ptr))[(i)] << factor)
#define _ccv_get_64f_value(ptr, i, factor) ((double*)(ptr))[(i)]
#define _ccv_get_8u_value(ptr, i, factor) (((unsigned char*)(ptr))[(i)] << factor)

#define ccv_matrix_getter(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, _ccv_get_8u_value); } } }

#define ccv_matrix_getter_a(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, _ccv_get_8u_value); } } }

#define ccv_matrix_getter_b(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, _ccv_get_8u_value); } } }

#define ccv_matrix_getter_integer_only(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_get_32s_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_get_64s_value); break; } \
	case CCV_8U: { block(__VA_ARGS__, _ccv_get_8u_value); break; } \
	default: { assert((type & CCV_32S) || (type & CCV_64S) || (type & CCV_8U)); } } }

#define ccv_matrix_getter_integer_only_a(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_get_32s_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_get_64s_value); break; } \
	case CCV_8U: { block(__VA_ARGS__, _ccv_get_8u_value); break; } \
	default: { assert((type & CCV_32S) || (type & CCV_64S) || (type & CCV_8U)); } } }

#define ccv_matrix_getter_integer_only_b(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_get_32s_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_get_64s_value); break; } \
	case CCV_8U: { block(__VA_ARGS__, _ccv_get_8u_value); break; } \
	default: { assert((type & CCV_32S) || (type & CCV_64S) || (type & CCV_8U)); } } }

#define ccv_matrix_getter_float_only(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32F: { block(__VA_ARGS__, _ccv_get_32f_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_get_64f_value); break; } \
	default: { assert((type & CCV_32F) || (type & CCV_64F)); } } }

#define ccv_matrix_typeof(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int); break; } \
	case CCV_32F: { block(__VA_ARGS__, float); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t); break; } \
	case CCV_64F: { block(__VA_ARGS__, double); break; } \
	default: { block(__VA_ARGS__, unsigned char); } } }

#define ccv_matrix_typeof_a(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int); break; } \
	case CCV_32F: { block(__VA_ARGS__, float); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t); break; } \
	case CCV_64F: { block(__VA_ARGS__, double); break; } \
	default: { block(__VA_ARGS__, unsigned char); } } }

#define ccv_matrix_typeof_b(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int); break; } \
	case CCV_32F: { block(__VA_ARGS__, float); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t); break; } \
	case CCV_64F: { block(__VA_ARGS__, double); break; } \
	default: { block(__VA_ARGS__, unsigned char); } } }

#define _ccv_set_32s_value(ptr, i, value, factor) (((int*)(ptr))[(i)] = (int)(value) >> factor)
#define _ccv_set_32f_value(ptr, i, value, factor) (((float*)(ptr))[(i)] = (float)(value))
#define _ccv_set_64s_value(ptr, i, value, factor) (((int64_t*)(ptr))[(i)] = (int64_t)(value) >> factor)
#define _ccv_set_64f_value(ptr, i, value, factor) (((double*)(ptr))[(i)] = (double)(value))
#define _ccv_set_8u_value(ptr, i, value, factor) (((unsigned char*)(ptr))[(i)] = ccv_clamp((int)(value) >> factor, 0, 255))

#define ccv_matrix_setter(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_set_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, _ccv_set_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_set_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_set_64f_value); break; } \
	default: { block(__VA_ARGS__, _ccv_set_8u_value); } } }

#define ccv_matrix_setter_a(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_set_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, _ccv_set_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_set_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_set_64f_value); break; } \
	default: { block(__VA_ARGS__, _ccv_set_8u_value); } } }

#define ccv_matrix_setter_b(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_set_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, _ccv_set_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_set_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_set_64f_value); break; } \
	default: { block(__VA_ARGS__, _ccv_set_8u_value); } } }

#define ccv_matrix_setter_integer_only(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_set_32s_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_set_64s_value); break; } \
	case CCV_8U: { block(__VA_ARGS__, _ccv_set_8u_value); break; } \
	default: { assert((type & CCV_32S) || (type & CCV_64S) || (type & CCV_8U)); } } }

#define ccv_matrix_setter_float_only(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32F: { block(__VA_ARGS__, _ccv_set_32f_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_set_64f_value); break; } \
	default: { assert((type & CCV_32F) || (type & CCV_64F)); } } }

#define ccv_matrix_setter_getter(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_set_32s_value, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, _ccv_set_32f_value, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_set_64s_value, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_set_64f_value, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, _ccv_set_8u_value, _ccv_get_8u_value); } } }

#define ccv_matrix_setter_getter_a(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_set_32s_value, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, _ccv_set_32f_value, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_set_64s_value, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_set_64f_value, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, _ccv_set_8u_value, _ccv_get_8u_value); } } }

#define ccv_matrix_setter_getter_b(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_set_32s_value, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, _ccv_set_32f_value, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_set_64s_value, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_set_64f_value, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, _ccv_set_8u_value, _ccv_get_8u_value); } } }

#define ccv_matrix_setter_getter_integer_only(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, _ccv_set_32s_value, _ccv_get_32s_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, _ccv_set_64s_value, _ccv_get_64s_value); break; } \
	case CCV_8U: { block(__VA_ARGS__, _ccv_set_8u_value, _ccv_get_8u_value); break; } \
	default: { assert((type & CCV_32S) || (type & CCV_64S) || (type & CCV_8U)); } } }

#define ccv_matrix_setter_getter_float_only(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32F: { block(__VA_ARGS__, _ccv_set_32f_value, _ccv_get_32f_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, _ccv_set_64f_value, _ccv_get_64f_value); break; } \
	default: { assert((type & CCV_32F) || (type & CCV_64F)); } } }

#define ccv_matrix_typeof_getter(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, float, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, double, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, unsigned char, _ccv_get_8u_value); } } }

#define ccv_matrix_typeof_getter_a(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, float, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, double, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, unsigned char, _ccv_get_8u_value); } } }

#define ccv_matrix_typeof_getter_b(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, float, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, double, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, unsigned char, _ccv_get_8u_value); } } }

#define ccv_matrix_typeof_setter(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int, _ccv_set_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, float, _ccv_set_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t, _ccv_set_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, double, _ccv_set_64f_value); break; } \
	default: { block(__VA_ARGS__, unsigned char, _ccv_set_8u_value); } } }

#define ccv_matrix_typeof_setter_a(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int, _ccv_set_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, float, _ccv_set_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t, _ccv_set_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, double, _ccv_set_64f_value); break; } \
	default: { block(__VA_ARGS__, unsigned char, _ccv_set_8u_value); } } }

#define ccv_matrix_typeof_setter_b(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int, _ccv_set_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, float, _ccv_set_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t, _ccv_set_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, double, _ccv_set_64f_value); break; } \
	default: { block(__VA_ARGS__, unsigned char, _ccv_set_8u_value); } } }

#define ccv_matrix_typeof_setter_getter(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int, _ccv_set_32s_value, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, float, _ccv_set_32f_value, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t, _ccv_set_64s_value, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, double, _ccv_set_64f_value, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, unsigned char, _ccv_set_8u_value, _ccv_get_8u_value); } } }

#define ccv_matrix_typeof_setter_getter_a(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int, _ccv_set_32s_value, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, float, _ccv_set_32f_value, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t, _ccv_set_64s_value, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, double, _ccv_set_64f_value, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, unsigned char, _ccv_set_8u_value, _ccv_get_8u_value); } } }

#define ccv_matrix_typeof_setter_getter_b(type, block, ...) { switch (CCV_GET_DATA_TYPE(type)) { \
	case CCV_32S: { block(__VA_ARGS__, int, _ccv_set_32s_value, _ccv_get_32s_value); break; } \
	case CCV_32F: { block(__VA_ARGS__, float, _ccv_set_32f_value, _ccv_get_32f_value); break; } \
	case CCV_64S: { block(__VA_ARGS__, int64_t, _ccv_set_64s_value, _ccv_get_64s_value); break; } \
	case CCV_64F: { block(__VA_ARGS__, double, _ccv_set_64f_value, _ccv_get_64f_value); break; } \
	default: { block(__VA_ARGS__, unsigned char, _ccv_set_8u_value, _ccv_get_8u_value); } } }

/****************************************************************************************\

  Generic implementation of QuickSort algorithm.
  ----------------------------------------------
  Using this macro user can declare customized sort function that can be much faster
  than built-in qsort function because of lower overhead on elements
  comparison and exchange. The macro takes less_than (or LT) argument - a macro or function
  that takes 2 arguments returns non-zero if the first argument should be before the second
  one in the sorted sequence and zero otherwise.

  Example:

    Suppose that the task is to sort points by ascending of y coordinates and if
    y's are equal x's should ascend.

    The code is:
    ------------------------------------------------------------------------------
           #define cmp_pts( pt1, pt2 ) \
               ((pt1).y < (pt2).y || ((pt1).y < (pt2).y && (pt1).x < (pt2).x))

           [static] CV_IMPLEMENT_QSORT( icvSortPoints, CvPoint, cmp_pts )
    ------------------------------------------------------------------------------

    After that the function "void icvSortPoints( CvPoint* array, size_t total, int aux );"
    is available to user.

  aux is an additional parameter, which can be used when comparing elements.
  The current implementation was derived from *BSD system qsort():

    * Copyright (c) 1992, 1993
    *  The Regents of the University of California.  All rights reserved.
    *
    * Redistribution and use in source and binary forms, with or without
    * modification, are permitted provided that the following conditions
    * are met:
    * 1. Redistributions of source code must retain the above copyright
    *    notice, this list of conditions and the following disclaimer.
    * 2. Redistributions in binary form must reproduce the above copyright
    *    notice, this list of conditions and the following disclaimer in the
    *    documentation and/or other materials provided with the distribution.
    * 3. All advertising materials mentioning features or use of this software
    *    must display the following acknowledgement:
    *  This product includes software developed by the University of
    *  California, Berkeley and its contributors.
    * 4. Neither the name of the University nor the names of its contributors
    *    may be used to endorse or promote products derived from this software
    *    without specific prior written permission.
    *
    * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
    * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
    * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
    * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
    * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
    * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
    * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
    * SUCH DAMAGE.

\****************************************************************************************/

#define CCV_SWAP(a,b,t) ((t) = (a), (a) = (b), (b) = (t))

#define CCV_IMPLEMENT_QSORT_EX(func_name, T, LT, swap_func, user_data_type)                     \
void func_name(T *array, size_t total, user_data_type aux)                                      \
{                                                                                               \
    int isort_thresh = 7;                                                                       \
    T t;                                                                                        \
    int sp = 0;                                                                                 \
                                                                                                \
    struct                                                                                      \
    {                                                                                           \
        T *lb;                                                                                  \
        T *ub;                                                                                  \
    }                                                                                           \
    stack[48];                                                                                  \
                                                                                                \
    if( total <= 1 )                                                                            \
        return;                                                                                 \
                                                                                                \
    stack[0].lb = array;                                                                        \
    stack[0].ub = array + (total - 1);                                                          \
                                                                                                \
    while( sp >= 0 )                                                                            \
    {                                                                                           \
        T* left = stack[sp].lb;                                                                 \
        T* right = stack[sp--].ub;                                                              \
                                                                                                \
        for(;;)                                                                                 \
        {                                                                                       \
            int i, n = (int)(right - left) + 1, m;                                              \
            T* ptr;                                                                             \
            T* ptr2;                                                                            \
                                                                                                \
            if( n <= isort_thresh )                                                             \
            {                                                                                   \
            insert_sort:                                                                        \
                for( ptr = left + 1; ptr <= right; ptr++ )                                      \
                {                                                                               \
                    for( ptr2 = ptr; ptr2 > left && LT(ptr2[0],ptr2[-1], aux); ptr2--)          \
                        swap_func( ptr2[0], ptr2[-1], array, aux, t );                          \
                }                                                                               \
                break;                                                                          \
            }                                                                                   \
            else                                                                                \
            {                                                                                   \
                T* left0;                                                                       \
                T* left1;                                                                       \
                T* right0;                                                                      \
                T* right1;                                                                      \
                T* pivot;                                                                       \
                T* a;                                                                           \
                T* b;                                                                           \
                T* c;                                                                           \
                int swap_cnt = 0;                                                               \
                                                                                                \
                left0 = left;                                                                   \
                right0 = right;                                                                 \
                pivot = left + (n/2);                                                           \
                                                                                                \
                if( n > 40 )                                                                    \
                {                                                                               \
                    int d = n / 8;                                                              \
                    a = left, b = left + d, c = left + 2*d;                                     \
                    left = LT(*a, *b, aux) ? (LT(*b, *c, aux) ? b : (LT(*a, *c, aux) ? c : a))  \
                                      : (LT(*c, *b, aux) ? b : (LT(*a, *c, aux) ? a : c));      \
                                                                                                \
                    a = pivot - d, b = pivot, c = pivot + d;                                    \
                    pivot = LT(*a, *b, aux) ? (LT(*b, *c, aux) ? b : (LT(*a, *c, aux) ? c : a)) \
                                      : (LT(*c, *b, aux) ? b : (LT(*a, *c, aux) ? a : c));      \
                                                                                                \
                    a = right - 2*d, b = right - d, c = right;                                  \
                    right = LT(*a, *b, aux) ? (LT(*b, *c, aux) ? b : (LT(*a, *c, aux) ? c : a)) \
                                      : (LT(*c, *b, aux) ? b : (LT(*a, *c, aux) ? a : c));      \
                }                                                                               \
                                                                                                \
                a = left, b = pivot, c = right;                                                 \
                pivot = LT(*a, *b, aux) ? (LT(*b, *c, aux) ? b : (LT(*a, *c, aux) ? c : a))     \
                                   : (LT(*c, *b, aux) ? b : (LT(*a, *c, aux) ? a : c));         \
                if( pivot != left0 )                                                            \
                {                                                                               \
                    swap_func( *pivot, *left0, array, aux, t );                                 \
                    pivot = left0;                                                              \
                }                                                                               \
                left = left1 = left0 + 1;                                                       \
                right = right1 = right0;                                                        \
                                                                                                \
                for(;;)                                                                         \
                {                                                                               \
                    while( left <= right && !LT(*pivot, *left, aux) )                           \
                    {                                                                           \
                        if( !LT(*left, *pivot, aux) )                                           \
                        {                                                                       \
                            if( left > left1 )                                                  \
                                swap_func( *left1, *left, array, aux, t );                      \
                            swap_cnt = 1;                                                       \
                            left1++;                                                            \
                        }                                                                       \
                        left++;                                                                 \
                    }                                                                           \
                                                                                                \
                    while( left <= right && !LT(*right, *pivot, aux) )                          \
                    {                                                                           \
                        if( !LT(*pivot, *right, aux) )                                          \
                        {                                                                       \
                            if( right < right1 )                                                \
                                swap_func( *right1, *right, array, aux, t );                    \
                            swap_cnt = 1;                                                       \
                            right1--;                                                           \
                        }                                                                       \
                        right--;                                                                \
                    }                                                                           \
                                                                                                \
                    if( left > right )                                                          \
                        break;                                                                  \
                    swap_func( *left, *right, array, aux, t );                                  \
                    swap_cnt = 1;                                                               \
                    left++;                                                                     \
                    right--;                                                                    \
                }                                                                               \
                                                                                                \
                if( swap_cnt == 0 )                                                             \
                {                                                                               \
                    left = left0, right = right0;                                               \
                    goto insert_sort;                                                           \
                }                                                                               \
                                                                                                \
                n = ccv_min( (int)(left1 - left0), (int)(left - left1) );                       \
                for( i = 0; i < n; i++ )                                                        \
                    swap_func( left0[i], left[i-n], array, aux, t );                            \
                                                                                                \
                n = ccv_min( (int)(right0 - right1), (int)(right1 - right) );                   \
                for( i = 0; i < n; i++ )                                                        \
                    swap_func( left[i], right0[i-n+1], array, aux, t );                         \
                n = (int)(left - left1);                                                        \
                m = (int)(right1 - right);                                                      \
                if( n > 1 )                                                                     \
                {                                                                               \
                    if( m > 1 )                                                                 \
                    {                                                                           \
                        if( n > m )                                                             \
                        {                                                                       \
                            stack[++sp].lb = left0;                                             \
                            stack[sp].ub = left0 + n - 1;                                       \
                            left = right0 - m + 1, right = right0;                              \
                        }                                                                       \
                        else                                                                    \
                        {                                                                       \
                            stack[++sp].lb = right0 - m + 1;                                    \
                            stack[sp].ub = right0;                                              \
                            left = left0, right = left0 + n - 1;                                \
                        }                                                                       \
                    }                                                                           \
                    else                                                                        \
                        left = left0, right = left0 + n - 1;                                    \
                }                                                                               \
                else if( m > 1 )                                                                \
                    left = right0 - m + 1, right = right0;                                      \
                else                                                                            \
                    break;                                                                      \
            }                                                                                   \
        }                                                                                       \
    }                                                                                           \
}

#define _ccv_qsort_default_swap(a, b, array, aux, t) CCV_SWAP((a), (b), (t))

#define CCV_IMPLEMENT_QSORT(func_name, T, cmp) \
    CCV_IMPLEMENT_QSORT_EX(func_name, T, cmp, _ccv_qsort_default_swap, int)

#define CCV_IMPLEMENT_MEDIAN(func_name, T) \
T func_name(T* buf, int low, int high) \
{                                                    \
	T w;                                             \
	int middle, ll, hh;                              \
	int median = (low + high) / 2;                   \
	for (;;)                                         \
	{                                                \
		if (high <= low)                             \
			return buf[median];                      \
		if (high == low + 1)                         \
		{                                            \
			if (buf[low] > buf[high])                \
				CCV_SWAP(buf[low], buf[high], w);    \
			return buf[median];                      \
		}                                            \
		middle = (low + high) / 2;                   \
		if (buf[middle] > buf[high])                 \
			CCV_SWAP(buf[middle], buf[high], w);     \
		if (buf[low] > buf[high])                    \
			CCV_SWAP(buf[low], buf[high], w);        \
		if (buf[middle] > buf[low])                  \
			CCV_SWAP(buf[middle], buf[low], w);      \
		CCV_SWAP(buf[middle], buf[low + 1], w);      \
		ll = low + 1;                                \
		hh = high;                                   \
		for (;;)                                     \
		{                                            \
			do ll++; while (buf[low] > buf[ll]);     \
			do hh--; while (buf[hh] > buf[low]);     \
			if (hh < ll)                             \
				break;                               \
			CCV_SWAP(buf[ll], buf[hh], w);           \
		}                                            \
		CCV_SWAP(buf[low], buf[hh], w);              \
		if (hh <= median)                            \
			low = ll;                                \
		else if (hh >= median)                       \
			high = hh - 1;                           \
	}                                                \
}

#endif
