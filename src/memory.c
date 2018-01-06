#include "ccv.h"
#include "ccv_internal.h"

#ifdef __APPLE__
#include "TargetConditionals.h"
#if (TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR)
// Temporary fix: __thread is not supported on iOS so define it to nothing.
#define __thread
#endif
#endif

/**
 * For new typed cache object:
 * ccv_dense_matrix_t: type 0
 * ccv_array_t: type 1
 **/

ccv_dense_matrix_t* ccv_dense_matrix_new(int rows, int cols, int type, void* data)
{
	ccv_dense_matrix_t* mat;
	if (type & CCV_NO_DATA_ALLOC)
	{
		mat = (ccv_dense_matrix_t*)ccmalloc(sizeof(ccv_dense_matrix_t));
		mat->type = (CCV_GET_CHANNEL(type) | CCV_GET_DATA_TYPE(type) | CCV_MATRIX_DENSE | CCV_NO_DATA_ALLOC);
		mat->data.u8 = data;
	} else {
		mat = (ccv_dense_matrix_t*)(data ? data : ccmalloc(ccv_compute_dense_matrix_size(rows, cols, type)));
		mat->type = (CCV_GET_CHANNEL(type) | CCV_GET_DATA_TYPE(type) | CCV_MATRIX_DENSE);
		mat->type |= data ? CCV_UNMANAGED : 0; // it still could be reusable because the signature could be derived one.
		mat->data.u8 = (unsigned char*)(mat + 1);
	}
	mat->rows = rows;
	mat->cols = cols;
	mat->step = (cols * CCV_GET_DATA_TYPE_SIZE(type) * CCV_GET_CHANNEL(type) + 3) & -4;
	mat->refcount = 1;
	return mat;
}

ccv_dense_matrix_t* ccv_dense_matrix_renew(ccv_dense_matrix_t* x, int rows, int cols, int types, int prefer_type)
{
	if (x != 0)
	{
		assert(x->rows == rows && x->cols == cols && (CCV_GET_DATA_TYPE(x->type) & types) && (CCV_GET_CHANNEL(x->type) == CCV_GET_CHANNEL(types)));
		prefer_type = CCV_GET_DATA_TYPE(x->type) | CCV_GET_CHANNEL(x->type);
	}
	if (x == 0)
	{
		x = ccv_dense_matrix_new(rows, cols, prefer_type, 0);
	}
	return x;
}

ccv_dense_matrix_t ccv_dense_matrix(int rows, int cols, int type, void* data)
{
	ccv_dense_matrix_t mat;
	mat.type = (CCV_GET_CHANNEL(type) | CCV_GET_DATA_TYPE(type) | CCV_MATRIX_DENSE | CCV_UNMANAGED);
	mat.rows = rows;
	mat.cols = cols;
	mat.step = (cols * CCV_GET_DATA_TYPE_SIZE(type) * CCV_GET_CHANNEL(type) + 3) & -4;
	mat.refcount = 1;
	mat.data.u8 = (unsigned char*)data;
	return mat;
}

ccv_sparse_matrix_t* ccv_sparse_matrix_new(int rows, int cols, int type, int major)
{
	ccv_sparse_matrix_t* mat;
	mat = (ccv_sparse_matrix_t*)ccmalloc(sizeof(ccv_sparse_matrix_t));
	mat->rows = rows;
	mat->cols = cols;
	mat->type = type | CCV_MATRIX_SPARSE | ((type & CCV_DENSE_VECTOR) ? CCV_DENSE_VECTOR : CCV_SPARSE_VECTOR);
	mat->major = major;
	mat->prime = 0;
	mat->load_factor = 0;
	mat->refcount = 1;
	mat->vector = (ccv_dense_vector_t*)ccmalloc(CCV_GET_SPARSE_PRIME(mat->prime) * sizeof(ccv_dense_vector_t));
	int i;
	for (i = 0; i < CCV_GET_SPARSE_PRIME(mat->prime); i++)
	{
		mat->vector[i].index = -1;
		mat->vector[i].length = 0;
		mat->vector[i].next = 0;
	}
	return mat;
}

void ccv_matrix_free_immediately(ccv_matrix_t* mat)
{
	int type = *(int*)mat;
	assert(!(type & CCV_UNMANAGED));
	if (type & CCV_MATRIX_DENSE)
	{
		ccv_dense_matrix_t* dmt = (ccv_dense_matrix_t*)mat;
		dmt->refcount = 0;
		ccfree(dmt);
	} else if (type & CCV_MATRIX_SPARSE) {
		ccv_sparse_matrix_t* smt = (ccv_sparse_matrix_t*)mat;
		int i;
		for (i = 0; i < CCV_GET_SPARSE_PRIME(smt->prime); i++)
			if (smt->vector[i].index != -1)
			{
				ccv_dense_vector_t* iter = &smt->vector[i];
				ccfree(iter->data.u8);
				iter = iter->next;
				while (iter != 0)
				{
					ccv_dense_vector_t* iter_next = iter->next;
					ccfree(iter->data.u8);
					ccfree(iter);
					iter = iter_next;
				}
			}
		ccfree(smt->vector);
		ccfree(smt);
	} else if ((type & CCV_MATRIX_CSR) || (type & CCV_MATRIX_CSC)) {
		ccv_compressed_sparse_matrix_t* csm = (ccv_compressed_sparse_matrix_t*)mat;
		csm->refcount = 0;
		ccfree(csm);
	}
}

void ccv_matrix_free(ccv_matrix_t* mat)
{
	int type = *(int*)mat;
	assert(!(type & CCV_UNMANAGED));
	if (type & CCV_MATRIX_DENSE)
	{
		ccv_dense_matrix_t* dmt = (ccv_dense_matrix_t*)mat;
		dmt->refcount = 0;
        ccfree(dmt);
	} else if (type & CCV_MATRIX_SPARSE) {
		ccv_sparse_matrix_t* smt = (ccv_sparse_matrix_t*)mat;
		int i;
		for (i = 0; i < CCV_GET_SPARSE_PRIME(smt->prime); i++)
			if (smt->vector[i].index != -1)
			{
				ccv_dense_vector_t* iter = &smt->vector[i];
				ccfree(iter->data.u8);
				iter = iter->next;
				while (iter != 0)
				{
					ccv_dense_vector_t* iter_next = iter->next;
					ccfree(iter->data.u8);
					ccfree(iter);
					iter = iter_next;
				}
			}
		ccfree(smt->vector);
		ccfree(smt);
	} else if ((type & CCV_MATRIX_CSR) || (type & CCV_MATRIX_CSC)) {
		ccv_compressed_sparse_matrix_t* csm = (ccv_compressed_sparse_matrix_t*)mat;
		csm->refcount = 0;
		ccfree(csm);
	}
}

ccv_array_t* ccv_array_new(int rsize, int rnum)
{
	ccv_array_t* array;
	array = (ccv_array_t*)ccmalloc(sizeof(ccv_array_t));
	array->type = 0;
	array->rnum = 0;
	array->rsize = rsize;
	array->size = ccv_max(rnum, 2 /* allocate memory for at least 2 items */);
	array->data = ccmalloc((size_t)array->size * (size_t)rsize);
	return array;
}

void ccv_array_free_immediately(ccv_array_t* array)
{
	array->refcount = 0;
	ccfree(array->data);
	ccfree(array);
}

void ccv_array_free(ccv_array_t* array)
{
    array->refcount = 0;
    ccfree(array->data);
    ccfree(array);
}
