#ifndef K_CAM_HPP
#define K_CAM_HPP

#include "opencv2/opencv.hpp"
#include <iostream>

#define NUM_BUTTONS 12
#define MIN_DENSITY 0.3
using namespace cv;
using namespace std;

class KCam {
    //Set up camera and window
    VideoCapture cap;

    //Set up data structures
    Mat sframe;
    
    enum color_data_key {VAL, SAT, HUE, VAL_T, SAT_T, HUE_T};
    int color_data[6];

    Rect frq_rect[NUM_BUTTONS];
    Mat frq_sframe[NUM_BUTTONS];
    Mat bi_frq_frame[NUM_BUTTONS];

    void makeBlue(Mat pic);
    Scalar getAveColor(Mat pic);
    inline void filter(Mat src, Mat dst, Scalar color, Scalar tolerance);
    inline void filter(Mat src, Mat dst);
    inline double getDensity(Mat & src);
    inline void displayColorData();

public:
    KCam();
    ~KCam();
    unsigned int getData(char command);
};

#endif //K_CAM_HPP
//enum color_data_key {VAL, SAT, HUE, VAL_T, SAT_T, HUE_T};
//int color_data[6];
