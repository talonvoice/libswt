#ifndef LIBSWT_H
#define LIBSWT_H

// copied from ccv.h
typedef struct {
    int interval;               /* Intervals for scale invariant option. */
    int min_neighbors;          /* Minimal neighbors to make a detection valid, this is for scale-invariant version. */
    int scale_invariant;        /* Enable scale invariant swt (to scale to different sizes and then combine the results) */
    int direction;              /* SWT direction. (black to white or white to black). */
    double same_word_thresh[2]; /* Overlapping more than 0.1 of the bigger one (0), and 0.9 of the smaller one (1) */

    // Canny parameters
    int size;        /* Parameters for Canny edge detector. */
    int low_thresh;  /* Parameters for Canny edge detector. */
    int high_thresh; /* Parameters for Canny edge detector. */

    // Geometry filtering parameters
    int max_height;         /* The maximum height for a letter. */
    int min_height;         /* The minimum height for a letter. */
    int min_area;           /* The minimum occupied area for a letter. */
    int letter_occlude_thresh;
    double aspect_ratio;    /* The maximum aspect ratio for a letter. */
    double std_ratio;       /* The inner-class standard derivation when grouping letters. */

    // Grouping parameters
    double thickness_ratio; /* The allowable thickness variance when grouping letters. */
    double height_ratio;    /* The allowable height variance when grouping letters. */
    int intensity_thresh;   /* The allowable intensity variance when grouping letters. */
    double distance_ratio;  /* The allowable distance variance when grouping letters. */
    double intersect_ratio; /* The allowable intersect variance when grouping letters. */
    double elongate_ratio;  /* The allowable elongate variance when grouping letters. */
    int letter_thresh;      /* The allowable letter threshold. */

    // Break text line into words
    int breakdown;          /* Whether to break text lines into words. */
    double breakdown_ratio; /* Apply OSTU and if inter-class variance above the threshold, it will be broken down into words. */
} swt_params;

typedef struct {
    int x;
    int y;
    int width;
    int height;
} swt_rect;

typedef struct swt_array swt_array;

swt_array *swt_detect(const unsigned char *data, int cols, int rows, swt_params *params);
int swt_len(swt_array *a);
swt_rect *swt_get(swt_array *a, int i);
void swt_free(swt_array *a);

#endif
