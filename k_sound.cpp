#define ALSA_PCM_NEW_HW_PARAMS_API /* Use the newer ALSA API */

#include "k_sound.hpp"
#include <alsa/asoundlib.h>
#include <iostream>
#include <math.h>
using namespace std;

#define LARGEST_SIGNED_SHORT 32767

KAudioPlayer::KAudioPlayer() {
    //initialize variables
    sample_rate_bps = 44100; // 44100 bits/second sampling rate (CD quality)
    buffer_size = 512;
    period_size = 0;
    int err = 0;
    period = 0;

    // Open PCM device for playback. 
    pcm_handle = NULL;
    err = snd_pcm_open (&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
    printf("open pcm                       %d\n", err);

    //Make and send hardware paremeters
    snd_pcm_hw_params_t *hw_params = NULL;
    err = snd_pcm_hw_params_malloc(&hw_params);
    printf("hw_params_malloc               %d\n", err);
    err = snd_pcm_hw_params_any(pcm_handle, hw_params);
    printf("default hw_params              %d\n", err);
    err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
    printf("hw_params_set_access           %d\n", err);
    err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_BE);
    printf("hw_params_set_format (S16_BE)  %d\n", err);
    err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &sample_rate_bps, NULL);
    printf("hw_params_set_rate_near        %d\n", err);
    err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 1);
    printf("hw_params_set_channels         %d\n", err);
    err = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, hw_params, &buffer_size);
    printf("hw_params_set_buffer_size_near %d\n", err);
    err = snd_pcm_hw_params_set_period_size_first(pcm_handle, hw_params, &period_size, NULL);
    printf("hw_params_set_period_size      %d\n", err);
    err = snd_pcm_hw_params(pcm_handle, hw_params);
    printf("hw_params_send                 %d\n", err);
    snd_pcm_hw_params_free(hw_params);

    //Make and send software paremeters
    /*
    snd_pcm_sw_params_t *sw_params = NULL;
    err = snd_pcm_sw_params_malloc(&sw_params);
    printf("sw_params_malloc               %d\n", err);
    err = snd_pcm_sw_params_current(pcm_handle, sw_params);
    printf("sw_params_curret (default)     %d\n", err);
    err = snd_pcm_sw_params_set_start_threshold(pcm_handle, sw_params, buffer_size - period_size); //when to start playing
    printf("sw_params_set_start_threshold  %d\n", err);
    err = snd_pcm_sw_params_set_avail_min(pcm_handle, sw_params, period_size);
    printf("sw_params_set_avail_min        %d\n", err);
    err = snd_pcm_sw_params(pcm_handle, sw_params);
    printf("sw_params send                 %d\n", err);
    snd_pcm_sw_params_free(sw_params);
    */

    // Use a buffer large enough to hold one period
    snd_pcm_hw_params_get_period_size(hw_params, &period_size, NULL);
    //buffer_size = period_size * 2; // 2 bytes/sample, 1 channels
    buffer = (char *) malloc(buffer_size);

    snd_pcm_hw_params_get_period_time(hw_params, &period_time, NULL);
    /*
    snd_pcm_prepare (pcm_handle);
    snd_pcm_start(pcm_handle);
    */
}

void KAudioPlayer::play(unsigned int notes) {
    period++;
    int play_time = 0;
    short sample = 0;
    double freq = 0; 

    //assign frequency
    //TODO polyphony
    int num_playing = 0;
    double freqs[12]; 
    freqs[0] = 0; //great error, gotta tell someone

    for (int c = 0; notes != 0 && c < 12; c++) { //TODO make 12 a var/const
        if (notes & 1) { //Pull of last bit
            //freq = note_to_freq(c);
            freqs[num_playing] = note_to_freq(11-c);
            num_playing ++;
        } 
        notes = notes >> 1; //Shift next bit into possition
    }

    double temp = 0;
    //Fill new buffer
    for (int frame = 0; frame < period_size; frame++) {
        play_time = ((frame * period_time) / period_size) + (period * period_time);
        //for (int c = 0; c < num_playing; c++) {
            //temp = sin_sample(freqs[c], play_time);
            //cout << " " << freqs[c];
        //}
        sample = sin_sample(freqs[0], play_time);
        //sample = (short)(temp / num_playing);
        //cout << sample << " ";

        buffer[frame * 2] = (char)(sample>>8);
        buffer[(frame * 2) + 1] = (char)((sample<<8)>>8);
    }

    //Send buffer
    int err = snd_pcm_writei(pcm_handle, buffer, period_size);

    //Handle errors
    if (err == -EPIPE) { 
        cerr << "underrun occurred" << endl;
        snd_pcm_prepare(pcm_handle);
    } else if (err < 0) 
        cerr << "error from writei: " << snd_strerror(err) << endl;
    else if (err != (int)period_size) 
        cerr << "short write, write " << err << " frames." << endl;
}

inline short KAudioPlayer::sin_sample(double _freq, int _time) {
    return LARGEST_SIGNED_SHORT * sin((_freq * 2 * M_PI * _time) / 1000000);
}

//TODO get a regular scale, use stuff from 109A
inline double KAudioPlayer::note_to_freq(unsigned int note) { 
    double frequencies[] = {392.0, 440.0, 493.88, //4 octave
                     523.25, 587.33, 659.26, 783.99, 880.0, 987.77, //5
                     1046.5, 1174.7, 1568.0}; //6
    return frequencies[note];
}
//inline short KAudioPlayer::note_to_freq(unsigned int note) {
