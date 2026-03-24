#include "audio_playback.h"
#include "gbl_ctx.h"

#define MINIMP3_ONLY_MP3
#define MINIMP3_IMPLEMENTATION
#include "device_drivers/minimp3_module/minimp3.h"
#include "device_drivers/fatfs_module/ff.h"

#include <stdio.h>

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

static audio_player_t audio_player;

void audio_playback_main(void *args) {
    (void) args;

    /* do any initializations of important audio-playback data structures. */    
    if (ctx.status & 1U) {
        /* TEST: assumption, file is at root. open our test mp3 file. */
        FRESULT res;
        res=f_open(&audio_player.mp3_file, "the_moving_fortress.mp3", FA_READ);
        if (res != FR_OK) {
            printf("couldn't open file\r\n");
            return;
        }
        mp3dec_init(&audio_player.mp3d);
        printf("audio player struct initialized\r\n");
        ctx.status &= (uint8_t)~(1U);
    }

    /* TODO: implement the audio processing pipeline. but first, initialize
        all the necessary peripherals beforehand. */

}
