#define MINIMP3_ONLY_MP3
#define MINIMP3_IMPLEMENTATION

#include "audio_playback.h"
#include "gbl_ctx.h"
#include "user_input/buttons.h"
#include "peripherals/spi.h"    // for i2s3_start and stop
#include "device_drivers/SSD1306/graphics_lib.h"
#include "device_drivers/SSD1306/ssd1306_driver.h"

/* NOTE: debugging */
#include "peripherals/dma.h"    

#include <stdio.h>

#define FREE    (0)
#define DATA_R  (1)

#define PLAYING (0)
#define PAUSED  (1)

audio_player_t audio_player;
static uint8_t play_state = PLAYING;

/* pauses/plays audio playback */
static void pause_play(void *);

/* navigates the gui to select a particular function 
(though that would mean our select button would have multi-functionality) 
NOTE: for now ignore this. */
static void cycle_up(void *);
static void cycle_down(void *);

/* go back to ff_nav. 
NOTE: when this is invoked, just set a flag. Then within the audio_playback
function, it can do the necessary cleanup in sequence, then switch the function
pointer to ff_nav_main. */
static void exit_state(void *);

static void cycle_up(void *args) {
    (void) args;
}

static void cycle_down(void *args) {
    (void) args;
}

static void exit_state(void *args) {
    (void) args;
}

static void pause_play(void *args) {
    (void) args;

    /* draw a pause or play indicator on the screen, to simply visualize
    the state of the media player. */
    if (play_state == PLAYING) {
        printf("paused\r\n");
        play_state = PAUSED;
        i2s3_stop();
        
        graphics_clear();
        graphics_draw_chars("PAUSED");
        display_update();
        printf("I2S3->I2SCFGR: %lx\r\n", I2S3->I2SCFGR);
    } else {
        printf("playing\r\n");
        play_state = PLAYING;
        i2s3_start();
        
        graphics_clear();
        graphics_draw_chars("PLAYING");
        display_update();
        printf("I2S3->I2SCFGR: %lx\r\n", I2S3->I2SCFGR);
    }
}

void audio_playback_main(void *args) {
    (void) args;
    static pcm_t *ap_ptr = &audio_player.pcm0;

    /* do any initializations of important audio-playback data structures. */    
    if (ctx.status & 1U) {
        /* TEST: assumption, file is at root. open our test mp3 file. */
        FRESULT res;
        /* TODO: Get filename from cwd */
        res=f_open(&audio_player.mp3_file, "the_moving_fortress.mp3", FA_READ);
        if (res != FR_OK) {
            printf("couldn't open file\r\n");
            return;
        }
        mp3dec_init(&audio_player.mp3d);
        printf("audio player struct initialized\r\n");

        /* hook buttons to pause/play */
        button_arr[SELECT].release = pause_play;
        /* NOTE: these 3 don't do anything yet.*/
        button_arr[CANCEL].release = exit_state;
        button_arr[UP].release = cycle_up;
        button_arr[DOWN].release = cycle_down;
      
        /* initialize audio_player*/
        audio_player.pcm0.status = DATA_R;
        audio_player.pcm1.status = DATA_R;
        audio_player.dma_ptr = &audio_player.pcm0;

        /* since our context can only be in a single state (just the way
            I've set this up right now), we may have to interweave the media
            player gui display with the audio processing pipeline.

           or rather, we can consider them separate components, since the
           state of the music display really depends on the buttons pressed. */
        play_state = PAUSED;
        i2s3_stop();

        ctx.status &= (uint8_t)~(1U);
    }

    /* not giving the buttons enough time to debounce, since the audio
        processing pipeline takes very long. */
    buttons_listen();
    if (button_arr[SELECT].state == BOUNCING) {
        return; /* priority to handle the button first */
    }

    /* TODO: implement the audio processing pipeline. but first, initialize
        all the necessary peripherals beforehand. */
    
    /* dma_ptr is the block that DMA (the consumer) is currently processing.*/
    /* audio_ptr is the block that the pipeline (the producer) is currently
        processing. */
    if (play_state == PAUSED) {
        return;
    }


    /* take the open file and read in a chunk to the input buf, keep 
        track of the original position. */
    unsigned long pos = 0;
    unsigned int count = 0;

    /* if the chunk that ap_ptr is currently pointing to is still in use,
        skip. */
    if(ap_ptr->status == DATA_R) {
        return;
    }
    
    //printf("start read data\r\n");

    /* reading a chunk from the file into input buf */
    pos = f_tell(&audio_player.mp3_file);
    if (f_read(&audio_player.mp3_file, audio_player.input_buf, INPUT_BUF_SIZE, 
        &count) != FR_OK) {
        printf("failed to read chunk from file\r\n");
        return;
    };

    //printf("end read data\r\n");

    if (count == 0) {
        /* probably EOF, so we want to pause our DMA and set file pointer
            back to 0. */
        pause_play(NULL);
        f_lseek(&audio_player.mp3_file, 0);
        return;
    }
    //printf("start decoding\r\n");

    /* decode the chunk and place it in whatever the producer is pointing to */
    int samples = mp3dec_decode_frame(&audio_player.mp3d,
        audio_player.input_buf, INPUT_BUF_SIZE, ap_ptr->arr,
        &audio_player.info);
    //printf("end decoding\r\n");
    if (samples > 0 && audio_player.info.frame_bytes > 0) {
        /* successful decode: set the current block as DATA_R, then switch
            the pointer to the other block. */
        //printf("successful decode\r\n");

        /* advance pointer by number of bytes read. */
        ap_ptr->status = DATA_R;
        if (ap_ptr == &audio_player.pcm0) {
            ap_ptr = &audio_player.pcm1;
        } else {
            ap_ptr = &audio_player.pcm0;
        }
        f_lseek(&audio_player.mp3_file, pos + (unsigned long)
            audio_player.info.frame_bytes);
    }
    else if (samples == 0 && audio_player.info.frame_bytes > 0) {
        /* no valid samples, probably id3 metadata, skip it. */
        // printf("metadata\r\n");
        f_lseek(&audio_player.mp3_file, pos + (unsigned long) 
            audio_player.info.frame_bytes);
    } else {
        printf("insufficient data\r\n");
        /* samples > 0 && frame_bytes == 0 or samples == 0 && frame_bytes ==0*/
        /* insufficient data */
        return;
    }

    /* TODO: Test DMA interrupts?*/

    /* on pause, disable i2s. */
}
