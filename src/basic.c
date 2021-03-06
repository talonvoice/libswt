#include "ccv.h"
#include "ccv_internal.h"
#if defined(HAVE_SSE2)
#include <xmmintrin.h>
#elif defined(HAVE_NEON)
#include <arm_neon.h>
#endif

/* sobel filter is fundamental to many other high-level algorithms,
 * here includes 2 special case impl (for 1x3/3x1, 3x3) and one general impl */
void ccv_sobel(ccv_dense_matrix_t* a, ccv_dense_matrix_t** b, int type, int dx, int dy)
{
    type = (type == 0) ? CCV_32S | CCV_GET_CHANNEL(a->type) : CCV_GET_DATA_TYPE(type) | CCV_GET_CHANNEL(a->type);
	ccv_dense_matrix_t* db = *b = ccv_dense_matrix_renew(*b, a->rows, a->cols, CCV_GET_CHANNEL(a->type) | CCV_ALL_DATA_TYPE, type);
	int i, j, k, c, ch = CCV_GET_CHANNEL(a->type);
	unsigned char* a_ptr = a->data.u8;
	unsigned char* b_ptr = db->data.u8;
	if (dx == 1 && dy == 0)
	{
		assert(a->cols >= 3);
		/* special case 1: 1x3 or 3x1 window */
#define for_block(_for_get, _for_set) \
		for (i = 0; i < a->rows; i++) \
		{ \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, k, 2 * (_for_get(a_ptr, ch + k, 0) - _for_get(a_ptr, k, 0)), 0); \
			for (j = 1; j < a->cols - 1; j++) \
				for (k = 0; k < ch; k++) \
					_for_set(b_ptr, j * ch + k, _for_get(a_ptr, (j + 1) * ch + k, 0) - _for_get(a_ptr, (j - 1) * ch + k, 0), 0); \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, (a->cols - 1) * ch + k, 2 * (_for_get(a_ptr, (a->cols - 1) * ch + k, 0) - _for_get(a_ptr, (a->cols - 2) * ch + k, 0)), 0); \
			b_ptr += db->step; \
			a_ptr += a->step; \
		}
		ccv_matrix_getter(a->type, ccv_matrix_setter, db->type, for_block);
#undef for_block
	} else if (dx == 0 && dy == 1) {
		assert(a->rows >= 3);
		/* special case 1: 1x3 or 3x1 window */
#define for_block(_for_get, _for_set) \
		for (j = 0; j < a->cols; j++) \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, j * ch + k, 2 * (_for_get(a_ptr + a->step, j * ch + k, 0) - _for_get(a_ptr, j * ch + k, 0)), 0); \
		a_ptr += a->step; \
		b_ptr += db->step; \
		for (i = 1; i < a->rows - 1; i++) \
		{ \
			for (j = 0; j < a->cols; j++) \
				for (k = 0; k < ch; k++) \
					_for_set(b_ptr, j * ch + k, _for_get(a_ptr + a->step, j * ch + k, 0) - _for_get(a_ptr - a->step, j * ch + k, 0), 0); \
			a_ptr += a->step; \
			b_ptr += db->step; \
		} \
		for (j = 0; j < a->cols; j++) \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, j * ch + k, 2 * (_for_get(a_ptr, j * ch + k, 0) - _for_get(a_ptr - a->step, j * ch + k, 0)), 0);
		ccv_matrix_getter(a->type, ccv_matrix_setter, db->type, for_block);
#undef for_block
	} else if ((dx == 1 && dy == 1) || (dx == -1 && dy == -1)) {
		/* special case 2: 3x3 window with diagonal direction */
		assert(a->rows >= 3 && a->cols >= 3);
#define for_block(_for_get, _for_set) \
		for (j = 0; j < a->cols - 1; j++) \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, j * ch + k, 2 * (_for_get(a_ptr + a->step, (j + 1) * ch + k, 0) - _for_get(a_ptr, j * ch + k, 0)), 0); \
		for (k = 0; k < ch; k++) \
			_for_set(b_ptr, (a->cols - 1) * ch + k, 2 * (_for_get(a_ptr + a->step, (a->cols - 1) * ch + k, 0) - _for_get(a_ptr, (a->cols - 1) * ch + k, 0)), 0); \
		a_ptr += a->step; \
		b_ptr += db->step; \
		for (i = 1; i < a->rows - 1; i++) \
		{ \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, k, 2 * (_for_get(a_ptr + a->step, ch + k, 0) - _for_get(a_ptr, k, 0)), 0); \
			for (j = 1; j < a->cols - 1; j++) \
				for (k = 0; k < ch; k++) \
					_for_set(b_ptr, j * ch + k, _for_get(a_ptr + a->step, (j + 1) * ch + k, 0) - _for_get(a_ptr - a->step, (j - 1) * ch + k, 0), 0); \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, (a->cols - 1) * ch + k, 2 * (_for_get(a_ptr, (a->cols - 1) * ch + k, 0) - _for_get(a_ptr - a->step, (a->cols - 2) * ch + k, 0)), 0); \
			a_ptr += a->step; \
			b_ptr += db->step; \
		} \
		for (k = 0; k < ch; k++) \
			_for_set(b_ptr, k, 2 * (_for_get(a_ptr, k, 0) - _for_get(a_ptr - a->step, k, 0)), 0); \
		for (j = 1; j < a->cols; j++) \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, j * ch + k, 2 * (_for_get(a_ptr, j * ch + k, 0) - _for_get(a_ptr - a->step, (j - 1) * ch + k, 0)), 0);
		ccv_matrix_getter(a->type, ccv_matrix_setter, db->type, for_block);
#undef for_block
	} else if ((dx == 1 && dy == -1) || (dx == -1 && dy == 1)) {
		/* special case 2: 3x3 window with diagonal direction */
		assert(a->rows >= 3 && a->cols >= 3);
#define for_block(_for_get, _for_set) \
		for (k = 0; k < ch; k++) \
			_for_set(b_ptr, k, 2 * (_for_get(a_ptr + a->step, k, 0) - _for_get(a_ptr, k, 0)), 0); \
		for (j = 1; j < a->cols; j++) \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, j * ch + k, 2 * (_for_get(a_ptr + a->step, (j - 1) * ch + k, 0) - _for_get(a_ptr, j * ch + k, 0)), 0); \
		a_ptr += a->step; \
		b_ptr += db->step; \
		for (i = 1; i < a->rows - 1; i++) \
		{ \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, k, 2 * (_for_get(a_ptr, k, 0) - _for_get(a_ptr - a->step, ch + k, 0)), 0); \
			for (j = 1; j < a->cols - 1; j++) \
				for (k = 0; k < ch; k++) \
					_for_set(b_ptr, j * ch + k, _for_get(a_ptr + a->step, (j - 1) * ch + k, 0) - _for_get(a_ptr - a->step, (j + 1) * ch + k, 0), 0); \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, (a->cols - 1) * ch + k, 2 * (_for_get(a_ptr + a->step, (a->cols - 2) * ch + k, 0) - _for_get(a_ptr, (a->cols - 1) * ch + k, 0)), 0); \
			a_ptr += a->step; \
			b_ptr += db->step; \
		} \
		for (j = 0; j < a->cols - 1; j++) \
			for (k = 0; k < ch; k++) \
				_for_set(b_ptr, j * ch + k, 2 * (_for_get(a_ptr, j * ch + k, 0) - _for_get(a_ptr - a->step, (j + 1) * ch + k, 0)), 0); \
		for (k = 0; k < ch; k++) \
			_for_set(b_ptr, (a->cols - 1) * ch + k, 2 * (_for_get(a_ptr, (a->cols - 1) * ch + k, 0) - _for_get(a_ptr - a->step, (a->cols - 1) * ch + k, 0)), 0);
		ccv_matrix_getter(a->type, ccv_matrix_setter, db->type, for_block);
#undef for_block
	} else if (dx == 3 && dy == 0) {
		assert(a->rows >= 3 && a->cols >= 3);
		/* special case 3: 3x3 window, corresponding sigma = 0.85 */
		unsigned char* buf = (unsigned char*)alloca(db->step);
#define for_block(_for_get, _for_set_b, _for_get_b) \
		for (j = 0; j < a->cols; j++) \
			for (k = 0; k < ch; k++) \
				_for_set_b(b_ptr, j * ch + k, _for_get(a_ptr + a->step, j * ch + k, 0) + 3 * _for_get(a_ptr, j * ch + k, 0), 0); \
		a_ptr += a->step; \
		b_ptr += db->step; \
		for (i = 1; i < a->rows - 1; i++) \
		{ \
			for (j = 0; j < a->cols; j++) \
				for (k = 0; k < ch; k++) \
					_for_set_b(b_ptr, j * ch + k, _for_get(a_ptr + a->step, j * ch + k, 0) + 2 * _for_get(a_ptr, j * ch + k, 0) + _for_get(a_ptr - a->step, j * ch + k, 0), 0); \
			a_ptr += a->step; \
			b_ptr += db->step; \
		} \
		for (j = 0; j < a->cols; j++) \
			for (k = 0; k < ch; k++) \
				_for_set_b(b_ptr, j * ch + k, 3 * _for_get(a_ptr, j * ch + k, 0) + _for_get(a_ptr - a->step, j * ch + k, 0), 0); \
		b_ptr = db->data.u8; \
		for (i = 0; i < a->rows; i++) \
		{ \
			for (k = 0; k < ch; k++) \
				_for_set_b(buf, k, _for_get_b(b_ptr, ch + k, 0) - _for_get_b(b_ptr, k, 0), 0); \
			for (j = 1; j < a->cols - 1; j++) \
				for (k = 0; k < ch; k++) \
					_for_set_b(buf, j * ch + k, _for_get_b(b_ptr, (j + 1) * ch + k, 0) - _for_get_b(b_ptr, (j - 1) * ch + k, 0), 0); \
			for (k = 0; k < ch; k++) \
				_for_set_b(buf, (a->cols - 1) * ch + k, _for_get_b(b_ptr, (a->cols - 1) * ch + k, 0) - _for_get_b(b_ptr, (a->cols - 2) * ch + k, 0), 0); \
			memcpy(b_ptr, buf, db->step); \
			b_ptr += db->step; \
		}
		ccv_matrix_getter(a->type, ccv_matrix_setter_getter, db->type, for_block);
#undef for_block
	} else if (dx == 0 && dy == 3) {
		assert(a->rows >= 3 && a->cols >= 3);
		/* special case 3: 3x3 window, corresponding sigma = 0.85 */
		unsigned char* buf = (unsigned char*)alloca(db->step);
#define for_block(_for_get, _for_set_b, _for_get_b) \
		for (j = 0; j < a->cols; j++) \
			for (k = 0; k < ch; k++) \
				_for_set_b(b_ptr, j * ch + k, _for_get(a_ptr + a->step, j * ch + k, 0) - _for_get(a_ptr, j * ch + k, 0), 0); \
		a_ptr += a->step; \
		b_ptr += db->step; \
		for (i = 1; i < a->rows - 1; i++) \
		{ \
			for (j = 0; j < a->cols; j++) \
				for (k = 0; k < ch; k++) \
					_for_set_b(b_ptr, j * ch + k, _for_get(a_ptr + a->step, j * ch + k, 0) - _for_get(a_ptr - a->step, j * ch + k, 0), 0); \
			a_ptr += a->step; \
			b_ptr += db->step; \
		} \
		for (j = 0; j < a->cols; j++) \
			for (k = 0; k < ch; k++) \
				_for_set_b(b_ptr, j * ch + k, _for_get(a_ptr, j * ch + k, 0) - _for_get(a_ptr - a->step, j * ch + k, 0), 0); \
		b_ptr = db->data.u8; \
		for (i = 0; i < a->rows; i++) \
		{ \
			for (k = 0; k < ch; k++) \
				_for_set_b(buf, k, _for_get_b(b_ptr, ch + k, 0) + 3 * _for_get_b(b_ptr, k, 0), 0); \
			for (j = 1; j < a->cols - 1; j++) \
				for (k = 0; k < ch; k++) \
					_for_set_b(buf, j * ch + k, _for_get_b(b_ptr, (j + 1) * ch + k, 0) + 2 * _for_get_b(b_ptr, j * ch + k, 0) + _for_get_b(b_ptr, (j - 1) * ch + k, 0), 0); \
			for (k = 0; k < ch; k++) \
				_for_set_b(buf, (a->cols - 1) * ch + k, _for_get_b(b_ptr, (a->cols - 2) * ch + k, 0) + 3 * _for_get_b(b_ptr, (a->cols - 1) * ch + k, 0), 0); \
			memcpy(b_ptr, buf, db->step); \
			b_ptr += db->step; \
		}
		ccv_matrix_getter(a->type, ccv_matrix_setter_getter, db->type, for_block);
#undef for_block
	} else {
		/* general case: in this case, I will generate a separable filter, and do the convolution */
		int fsz = ccv_max(dx, dy);
		assert(fsz % 2 == 1);
		int hfz = fsz / 2;
		unsigned char* df = (unsigned char*)alloca(sizeof(double) * fsz);
		unsigned char* gf = (unsigned char*)alloca(sizeof(double) * fsz);
		/* the sigma calculation is linear derviation of 3x3 - 0.85, 5x5 - 1.32 */
		double sigma = ((fsz - 1) / 2) * 0.47 + 0.38;
		double sigma2 = (2.0 * sigma * sigma);
		/* 2.5 is the factor to make the kernel "visible" in integer setting */
		double psigma3 = 2.5 / sqrt(sqrt(2 * CCV_PI) * sigma * sigma * sigma);
		for (i = 0; i < fsz; i++)
		{
			((double*)df)[i] = (i - hfz) * exp(-((i - hfz) * (i - hfz)) / sigma2) * psigma3;
			((double*)gf)[i] = exp(-((i - hfz) * (i - hfz)) / sigma2) * psigma3;
		}
		if (db->type & CCV_32S)
		{
			for (i = 0; i < fsz; i++)
			{
				// df could be negative, thus, (int)(x + 0.5) shortcut will not work
				((int*)df)[i] = (int)round(((double*)df)[i] * 256.0);
				((int*)gf)[i] = (int)(((double*)gf)[i] * 256.0 + 0.5);
			}
		} else {
			for (i = 0; i < fsz; i++)
			{
				ccv_set_value(db->type, df, i, ((double*)df)[i], 0);
				ccv_set_value(db->type, gf, i, ((double*)gf)[i], 0);
			}
		}
		if (dx < dy)
		{
			unsigned char* tf = df;
			df = gf;
			gf = tf;
		}
		unsigned char* buf = (unsigned char*)alloca(sizeof(double) * ch * (fsz + ccv_max(a->rows, a->cols)));
#define for_block(_for_get, _for_type_b, _for_set_b, _for_get_b) \
		for (i = 0; i < a->rows; i++) \
		{ \
			for (j = 0; j < hfz; j++) \
				for (k = 0; k < ch; k++) \
					_for_set_b(buf, j * ch + k, _for_get(a_ptr, k, 0), 0); \
			for (j = 0; j < a->cols; j++) \
				for (k = 0; k < ch; k++) \
					_for_set_b(buf, (j + hfz) * ch + k, _for_get(a_ptr, j * ch + k, 0), 0); \
			for (j = a->cols; j < a->cols + hfz; j++) \
				for (k = 0; k < ch; k++) \
					_for_set_b(buf, (j + hfz) * ch + k, _for_get(a_ptr, (a->cols - 1) * ch + k, 0), 0); \
			for (j = 0; j < a->cols; j++) \
			{ \
				for (c = 0; c < ch; c++) \
				{ \
					_for_type_b sum = 0; \
					for (k = 0; k < fsz; k++) \
						sum += _for_get_b(buf, (j + k) * ch + c, 0) * _for_get_b(df, k, 0); \
					_for_set_b(b_ptr, j * ch + c, sum, 8); \
				} \
			} \
			a_ptr += a->step; \
			b_ptr += db->step; \
		} \
		b_ptr = db->data.u8; \
		for (i = 0; i < a->cols; i++) \
		{ \
			for (j = 0; j < hfz; j++) \
				for (k = 0; k < ch; k++) \
					_for_set_b(buf, j * ch + k, _for_get_b(b_ptr, i * ch + k, 0), 0); \
			for (j = 0; j < a->rows; j++) \
				for (k = 0; k < ch; k++) \
					_for_set_b(buf, (j + hfz) * ch + k, _for_get_b(b_ptr + j * db->step, i * ch + k, 0), 0); \
			for (j = a->rows; j < a->rows + hfz; j++) \
				for (k = 0; k < ch; k++) \
					_for_set_b(buf, (j + hfz) * ch + k, _for_get_b(b_ptr + (a->rows - 1) * db->step, i * ch + k, 0), 0); \
			for (j = 0; j < a->rows; j++) \
			{ \
				for (c = 0; c < ch; c++) \
				{ \
					_for_type_b sum = 0; \
					for (k = 0; k < fsz; k++) \
						sum += _for_get_b(buf, (j + k) * ch + c, 0) * _for_get_b(gf, k, 0); \
					_for_set_b(b_ptr + j * db->step, i * ch + c, sum, 8); \
				} \
			} \
		}
		ccv_matrix_getter(a->type, ccv_matrix_typeof_setter_getter, db->type, for_block);
#undef for_block
	}
}

/* the fast arctan function adopted from OpenCV */
static void _ccv_atan2(float* x, float* y, float* angle, float* mag, int len)
{
	int i = 0;
	float scale = (float)(180.0 / CCV_PI);
#ifdef HAVE_SSE2
#ifndef _WIN32
	union { int i; float fl; } iabsmask; iabsmask.i = 0x7fffffff;
	__m128 eps = _mm_set1_ps((float)1e-6), absmask = _mm_set1_ps(iabsmask.fl);
	__m128 _90 = _mm_set1_ps((float)(3.141592654 * 0.5)), _180 = _mm_set1_ps((float)3.141592654), _360 = _mm_set1_ps((float)(3.141592654 * 2));
	__m128 zero = _mm_setzero_ps(), _0_28 = _mm_set1_ps(0.28f), scale4 = _mm_set1_ps(scale);
	
	for(; i <= len - 4; i += 4)
	{
		__m128 x4 = _mm_loadu_ps(x + i), y4 = _mm_loadu_ps(y + i);
		__m128 xq4 = _mm_mul_ps(x4, x4), yq4 = _mm_mul_ps(y4, y4);
		__m128 xly = _mm_cmplt_ps(xq4, yq4);
		__m128 z4 = _mm_div_ps(_mm_mul_ps(x4, y4), _mm_add_ps(_mm_add_ps(_mm_max_ps(xq4, yq4), _mm_mul_ps(_mm_min_ps(xq4, yq4), _0_28)), eps));

		// a4 <- x < y ? 90 : 0;
		__m128 a4 = _mm_and_ps(xly, _90);
		// a4 <- (y < 0 ? 360 - a4 : a4) == ((x < y ? y < 0 ? 270 : 90) : (y < 0 ? 360 : 0))
		__m128 mask = _mm_cmplt_ps(y4, zero);
		a4 = _mm_or_ps(_mm_and_ps(_mm_sub_ps(_360, a4), mask), _mm_andnot_ps(mask, a4));
		// a4 <- (x < 0 && !(x < y) ? 180 : a4)
		mask = _mm_andnot_ps(xly, _mm_cmplt_ps(x4, zero));
		a4 = _mm_or_ps(_mm_and_ps(_180, mask), _mm_andnot_ps(mask, a4));
		
		// a4 <- (x < y ? a4 - z4 : a4 + z4)
		a4 = _mm_mul_ps(_mm_add_ps(_mm_xor_ps(z4, _mm_andnot_ps(absmask, xly)), a4), scale4);
		__m128 m4 = _mm_sqrt_ps(_mm_add_ps(xq4, yq4));
		_mm_storeu_ps(angle + i, a4);
		_mm_storeu_ps(mag + i, m4);
	}
#endif
#endif
	for(; i < len; i++)
	{
		float xf = x[i], yf = y[i];
		float a, x2 = xf * xf, y2 = yf * yf;
		if(y2 <= x2)
			a = xf * yf / (x2 + 0.28f * y2 + (float)1e-6) + (float)(xf < 0 ? CCV_PI : yf >= 0 ? 0 : CCV_PI * 2);
		else
			a = (float)(yf >= 0 ? CCV_PI * 0.5 : CCV_PI * 1.5) - xf * yf / (y2 + 0.28f * x2 + (float)1e-6);
		angle[i] = a * scale;
		mag[i] = sqrtf(x2 + y2);
	}
}

void ccv_gradient(ccv_dense_matrix_t* a, ccv_dense_matrix_t** theta, int ttype, ccv_dense_matrix_t** m, int mtype, int dx, int dy)
{
	int ch = CCV_GET_CHANNEL(a->type);
	ccv_dense_matrix_t* dtheta = *theta = ccv_dense_matrix_renew(*theta, a->rows, a->cols, CCV_32F | ch, CCV_32F | ch);
	ccv_dense_matrix_t* dm = *m = ccv_dense_matrix_renew(*m, a->rows, a->cols, CCV_32F | ch, CCV_32F | ch);
	assert(dtheta && dm);
	ccv_dense_matrix_t* tx = 0;
	ccv_dense_matrix_t* ty = 0;
	ccv_sobel(a, &tx, CCV_32F | ch, dx, 0);
	ccv_sobel(a, &ty, CCV_32F | ch, 0, dy);
	_ccv_atan2(tx->data.f32, ty->data.f32, dtheta->data.f32, dm->data.f32, ch * a->rows * a->cols);
	ccv_matrix_free(tx);
	ccv_matrix_free(ty);
}
