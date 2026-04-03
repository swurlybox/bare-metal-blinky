#ifndef AUDIO_PLAYBACK_H
#define AUDIO_PLAYBACK_H

#include "device_drivers/minimp3_module/minimp3.h"
#include "device_drivers/fatfs_module/ff.h"

#define FREE    (0)
#define DATA_R  (1)
#define INPUT_BUF_SIZE  (16000) /* enough for ~10 mp3 frames. */

typedef struct pcm_t {
    short arr[MINIMP3_MAX_SAMPLES_PER_FRAME];   /* decoded mp3 pcm data */
    uint8_t status; /* indicate if arr is free or has data ready. */
} pcm_t;

typedef struct audio_player_t {
    mp3dec_t mp3d;  /* decoder structure */
    mp3dec_frame_info_t info;   /* info about decoded frame */
    FIL mp3_file;   /* open mp3 file */
    unsigned char input_buf[INPUT_BUF_SIZE];    /* raw chunk from mp3 */
    pcm_t pcm0; /* pcm0 and pcm1 as double buffer for continuous playback */
    pcm_t pcm1;
    pcm_t *dma_ptr; /* ptr to pcm block that dma is currently processing. */
} audio_player_t;

extern audio_player_t audio_player;

/* defines the audio-playback state. */
void audio_playback_main(void *);

#endif
