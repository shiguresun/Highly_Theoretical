#ifndef XMPIN_H
#define XMPIN_H

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ------------------------------------------------------------
 * PCM format constants
 * ------------------------------------------------------------ */
#define XMP_FORMAT_INT16   1
#define XMP_FORMAT_FLOAT   2

/* ------------------------------------------------------------
 * File info structure
 * ------------------------------------------------------------ */
typedef struct {
    int rate;       /* sample rate (Hz) */
    int chan;       /* number of channels */
    int format;     /* XMP_FORMAT_xxx */
    int length;     /* length in ms (or -1 if unknown) */
} xmp_info_struct;

/* ------------------------------------------------------------
 * Plugin flags
 * ------------------------------------------------------------ */
#define XMPIN_FLAG_SUBSONGS   1   /* plugin supports subsongs */

/* ------------------------------------------------------------
 * XMPlay input plugin structure
 * ------------------------------------------------------------ */
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

#endif /* XMPIN_H */
