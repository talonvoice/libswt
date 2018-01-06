#include "ccv.h"
#include "ccv_internal.h"

/* it is a supposely cleaner and faster implementation than original OpenCV (ccv_canny_deprecated,
 * removed, since the newer implementation achieve bit accuracy with OpenCV's), after a lot
 * profiling, the current implementation still uses integer to speed up */
void ccv_canny(ccv_dense_matrix_t* a, ccv_dense_matrix_t** b, int type, int size, double low_thresh, double high_thresh)
{
	assert(CCV_GET_CHANNEL(a->type) == CCV_C1);
	type = (type == 0) ? CCV_8U | CCV_C1 : CCV_GET_DATA_TYPE(type) | CCV_C1;
	ccv_dense_matrix_t* db = *b = ccv_dense_matrix_renew(*b, a->rows, a->cols, CCV_C1 | CCV_ALL_DATA_TYPE, type);
	if ((a->type & CCV_8U) || (a->type & CCV_32S))
	{
		ccv_dense_matrix_t* dx = 0;
		ccv_dense_matrix_t* dy = 0;
		ccv_sobel(a, &dx, 0, size, 0);
		ccv_sobel(a, &dy, 0, 0, size);
		/* special case, all integer */
		int low = (int)(low_thresh + 0.5);
		int high = (int)(high_thresh + 0.5);
		int* dxi = dx->data.i32;
		int* dyi = dy->data.i32;
		int i, j;
		int* mbuf = (int*)alloca(3 * (a->cols + 2) * sizeof(int));
		memset(mbuf, 0, 3 * (a->cols + 2) * sizeof(int));
		int* rows[3];
		rows[0] = mbuf + 1;
		rows[1] = mbuf + (a->cols + 2) + 1;
		rows[2] = mbuf + 2 * (a->cols + 2) + 1;
		for (j = 0; j < a->cols; j++)
			rows[1][j] = abs(dxi[j]) + abs(dyi[j]);
		dxi += a->cols;
		dyi += a->cols;
		int* map = (int*)ccmalloc(sizeof(int) * (a->rows + 2) * (a->cols + 2));
		int map_cols = a->cols + 2;
		memset(map, 0, sizeof(int) * map_cols);
		int* map_ptr = map + map_cols + 1;
		int** stack = (int**)ccmalloc(sizeof(int*) * a->rows * a->cols);
		int** stack_top = stack;
		int** stack_bottom = stack;
		for (i = 1; i <= a->rows; i++)
		{
			/* the if clause should be unswitched automatically, no need to manually do so */
			if (i == a->rows)
				memset(rows[2], 0, sizeof(int) * a->cols);
			else
				for (j = 0; j < a->cols; j++)
					rows[2][j] = abs(dxi[j]) + abs(dyi[j]);
			int* _dx = dxi - a->cols;
			int* _dy = dyi - a->cols;
			map_ptr[-1] = 0;
			int suppress = 0;
			for (j = 0; j < a->cols; j++)
			{
				int f = rows[1][j];
				if (f > low)
				{
					int x = abs(_dx[j]);
					int y = abs(_dy[j]);
					int s = _dx[j] ^ _dy[j];
					/* x * tan(22.5) */
					int tg22x = x * (int)(0.4142135623730950488016887242097 * (1 << 15) + 0.5);
					/* x * tan(67.5) == 2 * x + x * tan(22.5) */
					int tg67x = tg22x + ((x + x) << 15);
					y <<= 15;
					/* it is a little different from the Canny original paper because we adopted the coordinate system of
					 * top-left corner as origin. Thus, the derivative of y convolved with matrix:
					 * |-1 -2 -1|
					 * | 0  0  0|
					 * | 1  2  1|
					 * actually is the reverse of real y. Thus, the computed angle will be mirrored around x-axis.
					 * In this case, when angle is -45 (135), we compare with north-east and south-west, and for 45,
					 * we compare with north-west and south-east (in traditional coordinate system sense, the same if we
					 * adopt top-left corner as origin for "north", "south", "east", "west" accordingly) */
#define high_block \
					{ \
						if (f > high && !suppress && map_ptr[j - map_cols] != 2) \
						{ \
							map_ptr[j] = 2; \
							suppress = 1; \
							*(stack_top++) = map_ptr + j; \
						} else { \
							map_ptr[j] = 1; \
						} \
						continue; \
					}
					/* sometimes, we end up with same f in integer domain, for that case, we will take the first occurrence
					 * suppressing the second with flag */
					if (y < tg22x)
					{
						if (f > rows[1][j - 1] && f >= rows[1][j + 1])
							high_block;
					} else if (y > tg67x) {
						if (f > rows[0][j] && f >= rows[2][j])
							high_block;
					} else {
						s = s < 0 ? -1 : 1;
						if (f > rows[0][j - s] && f > rows[2][j + s])
							high_block;
					}
#undef high_block
				}
				map_ptr[j] = 0;
				suppress = 0;
			}
			map_ptr[a->cols] = 0;
			map_ptr += map_cols;
			dxi += a->cols;
			dyi += a->cols;
			int* row = rows[0];
			rows[0] = rows[1];
			rows[1] = rows[2];
			rows[2] = row;
		}
		memset(map_ptr - 1, 0, sizeof(int) * map_cols);
		int dr[] = {-1, 1, -map_cols - 1, -map_cols, -map_cols + 1, map_cols - 1, map_cols, map_cols + 1};
		while (stack_top > stack_bottom)
		{
			map_ptr = *(--stack_top);
			for (i = 0; i < 8; i++)
				if (map_ptr[dr[i]] == 1)
				{
					map_ptr[dr[i]] = 2;
					*(stack_top++) = map_ptr + dr[i];
				}
		}
		map_ptr = map + map_cols + 1;
		unsigned char* b_ptr = db->data.u8;
#define for_block(_, _for_set) \
		for (i = 0; i < a->rows; i++) \
		{ \
			for (j = 0; j < a->cols; j++) \
				_for_set(b_ptr, j, (map_ptr[j] == 2), 0); \
			map_ptr += map_cols; \
			b_ptr += db->step; \
		}
		ccv_matrix_setter(db->type, for_block);
#undef for_block
		ccfree(stack);
		ccfree(map);
		ccv_matrix_free(dx);
		ccv_matrix_free(dy);
	} else {
		/* general case, use all ccv facilities to deal with it */
		ccv_dense_matrix_t* mg = 0;
		ccv_dense_matrix_t* ag = 0;
		ccv_gradient(a, &ag, 0, &mg, 0, size, size);
		ccv_matrix_free(ag);
		ccv_matrix_free(mg);
		/* FIXME: Canny implementation for general case */
	}
}

void ccv_close_outline(ccv_dense_matrix_t* a, ccv_dense_matrix_t** b, int type)
{
	assert((CCV_GET_CHANNEL(a->type) == CCV_C1) && ((a->type & CCV_8U) || (a->type & CCV_32S) || (a->type & CCV_64S)));
	type = ((type == 0) || (type & CCV_32F) || (type & CCV_64F)) ? CCV_GET_DATA_TYPE(a->type) | CCV_C1 : CCV_GET_DATA_TYPE(type) | CCV_C1;
	ccv_dense_matrix_t* db = *b = ccv_dense_matrix_renew(*b, a->rows, a->cols, CCV_C1 | CCV_ALL_DATA_TYPE, type);
	int i, j;
	unsigned char* a_ptr = a->data.u8;
	unsigned char* b_ptr = db->data.u8;
	ccv_zero(db);
#define for_block(_for_get, _for_set_b, _for_get_b) \
	for (i = 0; i < a->rows - 1; i++) \
	{ \
		for (j = 0; j < a->cols - 1; j++) \
		{ \
			if (!_for_get_b(b_ptr, j, 0)) \
				_for_set_b(b_ptr, j, _for_get(a_ptr, j, 0), 0); \
			if (_for_get(a_ptr, j, 0) && _for_get(a_ptr + a->step, j + 1, 0)) \
			{ \
				_for_set_b(b_ptr + a->step, j, 1, 0); \
				_for_set_b(b_ptr, j + 1, 1, 0); \
			} \
			if (_for_get(a_ptr + a->step, j, 0) && _for_get(a_ptr, j + 1, 0)) \
			{ \
				_for_set_b(b_ptr, j, 1, 0); \
				_for_set_b(b_ptr + a->step, j + 1, 1, 0); \
			} \
		} \
		if (!_for_get_b(b_ptr, a->cols - 1, 0)) \
			_for_set_b(b_ptr, a->cols - 1, _for_get(a_ptr, a->cols - 1, 0), 0); \
		a_ptr += a->step; \
		b_ptr += db->step; \
	} \
	for (j = 0; j < a->cols; j++) \
	{ \
		if (!_for_get_b(b_ptr, j, 0)) \
			_for_set_b(b_ptr, j, _for_get(a_ptr, j, 0), 0); \
	}
	ccv_matrix_getter_integer_only(a->type, ccv_matrix_setter_getter_integer_only, db->type, for_block);
#undef for_block
}

int ccv_otsu(ccv_dense_matrix_t* a, double* outvar, int range)
{
	assert((a->type & CCV_32S) || (a->type & CCV_8U));
	int* histogram = (int*)alloca(range * sizeof(int));
	memset(histogram, 0, sizeof(int) * range);
	int i, j;
	unsigned char* a_ptr = a->data.u8;
#define for_block(_, _for_get) \
	for (i = 0; i < a->rows; i++) \
	{ \
		for (j = 0; j < a->cols; j++) \
			histogram[ccv_clamp((int)_for_get(a_ptr, j, 0), 0, range - 1)]++; \
		a_ptr += a->step; \
	}
	ccv_matrix_getter(a->type, for_block);
#undef for_block
	double sum = 0, sumB = 0;
	for (i = 0; i < range; i++)
		sum += i * histogram[i];
	int wB = 0, wF = 0, total = a->rows * a->cols;
	double maxVar = 0;
	int threshold = 0;
	for (i = 0; i < range; i++)
	{
		wB += histogram[i];
		if (wB == 0)
			continue;
		wF = total - wB;
		if (wF == 0)
			break;
		sumB += i * histogram[i];
		double mB = sumB / wB;
		double mF = (sum - sumB) / wF;
		double var = wB * wF * (mB - mF) * (mB - mF);
		if (var > maxVar)
		{
			maxVar = var;
			threshold = i;
		}
	}
	if (outvar != 0)
		*outvar = maxVar / total / total;
	return threshold;
}
