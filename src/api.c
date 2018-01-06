#include "ccv.h"

ccv_array_t *swt_detect(const unsigned char *data, int cols, int rows, const ccv_swt_param_t *params) {
    if (!params) params = &ccv_swt_default_params;

    ccv_dense_matrix_t *mat = ccv_dense_matrix_new(rows, cols, CCV_8U | CCV_C1 | CCV_NO_DATA_ALLOC, (void *)data);
    ccv_array_t *words = ccv_swt_detect_words(mat, *params);
    ccv_matrix_free(mat);
    return words;
}

int swt_len(ccv_array_t *a) {
    return a->rnum;
}

ccv_rect_t *swt_get(ccv_array_t *a, int i) {
    return (ccv_rect_t *)ccv_array_get(a, i);
}

void swt_free(ccv_array_t *a) {
    ccv_array_free(a);
}
