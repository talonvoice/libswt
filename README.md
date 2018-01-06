libswt
--------

A tiny library derived from libccv's SWT (Stroke Width Transform) code.

```
swt_array_t *swt_detect(const unsigned char *data, int cols, int rows, swt_param_t *params);
int swt_len(swt_array_t *a);
swt_rect_t *swt_get(swt_array_t *a, int i);
void swt_free(swt_array_t *a);

// data is a dense greyscale image
swt_array *words = swt_detect(data, width, height);
for (int i = 0; i < swt_len(words); i++) {
    swt_rect *rect = swt_get(words, i);
    printf("%d %d %d %d\n", rect->x, rect->y, rect->width, rect->height);
}
```
