#include <CoreGraphics/CoreGraphics.h>
  
#include "swt.h"

swt_array *swtshot(CGRect rect) {
    CGImageRef ss = CGWindowListCreateImage(rect, kCGWindowListOptionOnScreenOnly, kCGNullWindowID, kCGWindowImageDefault);
    size_t width = CGImageGetWidth(ss);
    size_t height = CGImageGetHeight(ss);

    CGColorSpaceRef csg = CGColorSpaceCreateDeviceGray();
    CGContextRef ctx = CGBitmapContextCreate(NULL, width, height, 8, 0, csg, 0);
    CGContextDrawImage(ctx, CGRectMake(0, 0, width, height), ss);
    CGImageRef grayss = CGBitmapContextCreateImage(ctx);
    CFDataRef data = CGDataProviderCopyData(CGImageGetDataProvider(grayss));
    const uint8_t *pixels = CFDataGetBytePtr(data);

    swt_array *words = swt_detect(pixels, width, height, NULL);

    CFRelease(data);
    CFRelease(grayss);
    CFRelease(ctx);
    CFRelease(ss);
    return words;
}

int main(int argc, char **argv) {
    swt_array *words = swtshot(CGRectInfinite);
    if (words) {
        for (int i = 0; i < swt_len(words); i++) {
            swt_rect *rect = swt_get(words, i);
            printf("%d %d %d %d\n", rect->x, rect->y, rect->width, rect->height);
        }
        swt_free(words);
    }
    return 0;
}
