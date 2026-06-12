// xmp-ssf.cpp — XMPlay input plugin for Highly_Theoretical (SSF/DSF)
// 最小構成版：タグなし・設定なし・3分固定・XMPlay 専用

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "xmpin.h"

#include "../Core/sega.h"
#include "../Core/satsound.h"
#include "../Core/dcsound.h"

/* ============================================================
 * XMPlay 固有のデフォルト設定
 * ============================================================ */

static const int DEFAULT_LENGTH_MS      = 180000;   // 3分固定
static const int DEFAULT_RESAMPLE_RATE  = 44100;    // 44.1kHz 固定

/* ============================================================
 * Global state
 * ============================================================ */

static void*   sega_state       = NULL;
static int     sega_version     = 1;      // 1 = SSF (Saturn), 2 = DSF (Dreamcast)
static int     library_inited   = 0;

static int     is_playing       = 0;
static uint32_t played_samples  = 0;

static char    current_filename[MAX_PATH] = {0};

/* ============================================================
 * 対応拡張子
 * ============================================================ */

static const char* ssf_supported_ext[] = {
    ".ssf", ".dsf", ".minissf", ".minidsf", NULL
};

static bool has_supported_ext(const char* fn)
{
    const char* ext = strrchr(fn, '.');
    if (!ext) return false;

    for (int i = 0; ssf_supported_ext[i]; i++) {
        if (_stricmp(ext, ssf_supported_ext[i]) == 0)
            return true;
    }
    return false;
}

/* ============================================================
 * Core 初期化ヘルパ
 * ============================================================ */

static int ensure_sega_library_inited()
{
    if (library_inited)
        return 0;

    sint32 r = sega_init();
    if (r != 0)
        return -1;

    library_inited = 1;
    return 0;
}

static int create_sega_state_for_file(const char* filename)
{
    const char* ext = strrchr(filename, '.');
    if (ext && (_stricmp(ext, ".dsf") == 0 || _stricmp(ext, ".minidsf") == 0)) {
        sega_version = 2; // Dreamcast
    } else {
        sega_version = 1; // Saturn
    }

    if (sega_state) {
        free(sega_state);
        sega_state = NULL;
    }

    uint32_t size = sega_get_state_size((uint8)sega_version);
    sega_state = malloc(size);
    if (!sega_state)
        return -1;

    sega_clear_state(sega_state, (uint8)sega_version);
    return 0;
}

/* ============================================================
 * XMPlay: サブソング情報
 * ============================================================ */

static int ssf_get_length_ms(const char* filename)
{
    (void)filename;
    return DEFAULT_LENGTH_MS;
}

static BOOL WINAPI xmp_getsubsongs(char* fn, int* length)
{
    if (!has_supported_ext(fn))
        return FALSE;

    length[0] = 1;                         // サブソング数
    length[1] = ssf_get_length_ms(fn);     // 1曲目の長さ
    return TRUE;
}

/* ============================================================
 * XMPlay: ファイル情報取得
 * ============================================================ */

static BOOL WINAPI xmp_getinfo(char* fn, xmp_info_struct* info)
{
    if (!has_supported_ext(fn))
        return FALSE;

    memset(info, 0, sizeof(*info));

    info->rate   = DEFAULT_RESAMPLE_RATE;
    info->chan   = 2;
    info->format = XMP_FORMAT_INT16;
    info->length = ssf_get_length_ms(fn);

    return TRUE;
}

/* ============================================================
 * XMPlay: 再生開始
 * ============================================================ */

static BOOL WINAPI xmp_play(char* fn, int subsong)
{
    (void)subsong;

    if (!has_supported_ext(fn))
        return FALSE;
    if (!fn)
        return FALSE;

    if (ensure_sega_library_inited() != 0)
        return FALSE;

    if (create_sega_state_for_file(fn) != 0)
        return FALSE;

    strncpy_s(current_filename, sizeof(current_filename), fn, MAX_PATH - 1);
    is_playing = 1;
    played_samples = 0;

    return TRUE;
}

/* ============================================================
 * XMPlay: PCM データ供給
 * ============================================================ */

static void WINAPI xmp_getdata(char* buf, int length)
{
    if (!is_playing || !sega_state) {
        memset(buf, 0, length);
        return;
    }

    int bytes_per_sample = sizeof(sint16) * 2; // 16bit stereo
    int samples = length / bytes_per_sample;

    if (samples <= 0) {
        memset(buf, 0, length);
        return;
    }

    uint32 req = (uint32)samples;
    sint32 cycles = 0x1000000;

    if (sega_version == 2) {
        void* dc = sega_get_dcsound_state(sega_state);
        if (!dc || dcsound_execute(dc, cycles, (sint16*)buf, &req) < 0 || req == 0) {
            memset(buf, 0, length);
            is_playing = 0;
            return;
        }
    } else {
        void* sat = sega_get_satsound_state(sega_state);
        if (!sat || satsound_execute(sat, cycles, (sint16*)buf, &req) < 0 || req == 0) {
            memset(buf, 0, length);
            is_playing = 0;
            return;
        }
    }

    if (req < (uint32)samples) {
        size_t done_bytes = req * bytes_per_sample;
        memset(buf + done_bytes, 0, length - done_bytes);
        is_playing = 0;
    }

    played_samples += req;
}

/* ============================================================
 * XMPlay に公開する構造体
 * ============================================================ */

__declspec(dllexport)
XMPIN xmpin = {
    XMPIN_FLAG_SUBSONGS,
    "Highly Theoretical (SSF/DSF) [XMPlay minimal]",
    xmp_getsubsongs,
    xmp_getinfo,
    xmp_play,
    xmp_getdata
};
