#ifndef XMPIN_H
#define XMPIN_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#define XMP_FORMAT_INT16   1
#define XMP_FORMAT_FLOAT   2

typedef struct {
    int rate;
    int chan;
    int format;
    int length;
} xmp_info_struct;

#define XMPIN_FLAG_SUBSONGS   1

typedef struct {
    unsigned int flags;
    const char *description;

    BOOL (WINAPI *GetSubSongs)(char *filename, int *length);
    BOOL (WINAPI *GetInfo)(char *filename, xmp_info_struct *info);
    BOOL (WINAPI *Play)(char *filename, int subsong);
    void (WINAPI *GetData)(char *buffer, int length);

} XMPIN;

#ifdef __cplusplus
}
#endif

#endif
