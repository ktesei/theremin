#include "opencv2/opencv.hpp"
#include "k_sound.hpp"
#include "k_cam.hpp"
#include <iostream>
#include <pthread.h>

using namespace std;

unsigned int notes;
pthread_mutex_t mutex_notes = PTHREAD_MUTEX_INITIALIZER;

//Thread for playing sound
void * soundManaging(void * param) {
    KAudioPlayer ap; 

    cout << "starting audio" << endl;

    unsigned int last_notes = 0;
    
    while (true) {
        if(pthread_mutex_trylock(&mutex_notes) != EBUSY) {
            last_notes = notes;
            pthread_mutex_unlock(&mutex_notes);
        }
        ap.play(last_notes); 
    }
    return param;
}

//Main thread, does video processing
int main(int argc, char* argv[]) {
    //initialize shared data and sound managing thread
    pthread_mutex_init(&mutex_notes, NULL);
    pthread_t sound_managing_thread_id;
    pthread_create(&sound_managing_thread_id, NULL, soundManaging, 0);

    //make video processing stuff
    KCam cam;

    char command;

    cout << "starting video processing" << endl;


    while (1) {
        command = waitKey(1);
        unsigned int temp_notes = cam.getData(command);
        pthread_mutex_lock(&mutex_notes);
        notes = temp_notes;
        pthread_mutex_unlock(&mutex_notes);
         
        switch(command) {
            case 'q': {
                pthread_cancel(sound_managing_thread_id);
                cout << "Goodbye!" << endl;
                return 0;
            }
        }
    }
    return 0;
}
