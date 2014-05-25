#ifndef K_SOUND_HPP
#define K_SOUND_HPP

#define ALSA_PCM_NEW_HW_PARAMS_API /* Use the newer ALSA API */

#include <alsa/asoundlib.h>
#include <iostream>
#include <math.h>

using namespace std;

class KAudioPlayer {
    //device handles
    snd_pcm_t *pcm_handle;

    //KAudioPlayer settings. Subject to change in constructor
    snd_pcm_uframes_t buffer_size;
    snd_pcm_uframes_t period_size;
    unsigned int sample_rate_bps; 

    char *buffer;
    unsigned int period_time;
    //static KAudioPlayer alsaplayer; 

    //Counters to keep track of which sample it is on
    int period;

    //Utility methods
    inline short sin_sample(double _freq, int _time);
    inline double note_to_freq(unsigned int note);

public:
    KAudioPlayer();
    void play(unsigned int notes);
};

#endif //K_SOUND_HPP
