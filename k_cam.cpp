#include "opencv2/opencv.hpp"
#include <iostream>
#include "k_cam.hpp"

#define ERK cerr << __FUNCTION__ << " " << __LINE__ << endl
#define MAIN_WINDOW "wind waker"
#define CONTROL_PANEL "control_panel"

using namespace cv;
using namespace std;

//Class for getting pictures and parsing them for note data
KCam::KCam() {
    //Set up camera and window
    cap = VideoCapture(0);
    if(!cap.isOpened()) {
        cerr << "Camera not opening. Other jobs might be using camera." << endl;
        exit(1);
    }

    //Set up data structures
    color_data[VAL] = 10;
    color_data[SAT] = 0;
    color_data[HUE] = 0;
    color_data[VAL_T] = 30;
    color_data[SAT_T] = 40;
    color_data[HUE_T] = 50;
    cap >> sframe; 

    namedWindow(MAIN_WINDOW, 1); 
    namedWindow(CONTROL_PANEL, 0/*CV_WINDOW_AUTOSIZE | CV_GUI_NORMAL*/); 
    cvCreateTrackbar( "val", CONTROL_PANEL, &color_data[VAL_T], 100, NULL);
    cvCreateTrackbar( "sat", CONTROL_PANEL, &color_data[SAT_T], 100, NULL);
    cvCreateTrackbar( "hue", CONTROL_PANEL, &color_data[HUE_T], 100, NULL);

    
    //Cut out the buttons
    for (int c = 0; c < NUM_BUTTONS; c++) {
        frq_rect[c] = Rect(40 + 40 * c, sframe.rows - 130, 30, 20); //x,y,wid,hei
        frq_sframe[c] = sframe(frq_rect[c]);
        bi_frq_frame[c] = cv::Mat::zeros(frq_sframe[c].rows, frq_sframe[c].cols, CV_8U); 
    }
}

KCam::~KCam() {
    cap.release();
}

unsigned int KCam::getData(char command) {
    cap >> sframe; // get a new frame from camera
    flip(sframe, sframe, 1); //Orient video so it looks like a mirror

    //Make buttons VSH and apply the threshold
    for (int c = 0; c < NUM_BUTTONS; c++) {
        cvtColor(frq_sframe[c], frq_sframe[c], CV_BGR2HSV);
        filter(frq_sframe[c], bi_frq_frame[c]); 
    }

    //TODO find a better way to do this/ use the bars
    switch(command) {
        case 'c': {
            //sample color from first box
            Scalar temp = getAveColor(frq_sframe[0]);
            color_data[VAL] = (int)temp[0];
            color_data[SAT] = (int)temp[1];
            color_data[HUE] = (int)temp[2];
            displayColorData();
            break; }
    }

    //Display static objects
    //Button boxes
    for (int c = 0; c < NUM_BUTTONS; c++) {
        rectangle(sframe, frq_rect[c], Scalar(0,250,0,0), 2, 8, 0);
    }
    rectangle(sframe, Rect(50,50,50,50), Scalar(0,250,0,0), 2, 8, 0);

    double density = 0;
    unsigned int keys_played = 0;

    
    //Check if buttons are pressed
    for (int c = 0; c < NUM_BUTTONS; c++) {
        density = getDensity(bi_frq_frame[c]);
        keys_played = keys_played << 1;
        if (density > MIN_DENSITY) {  //Set right-most bit to indicate note being played or not
            keys_played += 1;
        }
        //Display density meters
        line(sframe, Point(frq_rect[c].x, frq_rect[c].y + frq_rect[c].height - density * frq_rect[c].height), 
                     Point(frq_rect[c].x, frq_rect[c].y + frq_rect[c].height),
                     Scalar(255,0,0,0), 4, 8, 0);

    }

    imshow(MAIN_WINDOW, sframe);

    //output the which notes are played
    return keys_played; 
}

inline void KCam::filter(Mat src, Mat dst, Scalar color, Scalar tolerance) {
    for (int y = 0; y < src.rows; y++) {
        unsigned char* row = src.ptr(y);
        unsigned char* row_dst = dst.ptr(y);
        for (int x = 0; x < src.cols * 3; x+=3) {
            if ((row[x    ] < color[0] + tolerance[0]) && 
                (row[x    ] > color[0] - tolerance[0]) && 
                (row[x + 1] < color[1] + tolerance[1]) && 
                (row[x + 1] > color[1] - tolerance[1]) && 
                (row[x + 2] < color[2] + tolerance[2]) && 
                (row[x + 2] > color[2] - tolerance[2]))
            {
                row_dst[x/3] = 255;
            } else {
                row_dst[x/3] = 0;
            }
        }
    }
}

inline void KCam::filter(Mat src, Mat dst) {
    for (int y = 0; y < src.rows; y++) {
        unsigned char* row = src.ptr(y);
        unsigned char* row_dst = dst.ptr(y);
        for (int x = 0; x < src.cols * 3; x+=3) {
            if ((row[x    ] < color_data[VAL] + color_data[VAL_T]) && 
                (row[x    ] > color_data[VAL] - color_data[VAL_T]) && 
                (row[x + 1] < color_data[SAT] + color_data[SAT_T]) && 
                (row[x + 1] > color_data[SAT] - color_data[SAT_T]) && 
                (row[x + 2] < color_data[HUE] + color_data[HUE_T]) && 
                (row[x + 2] > color_data[HUE] - color_data[HUE_T]))
            {
                row_dst[x/3] = 255;
            } else {
                row_dst[x/3] = 0;
            }
        }
    }
}

inline double KCam::getDensity(Mat & src) {
    double point_sum = 0;

    for (int y = 0; y < src.rows; y++) {
        unsigned char* row = src.ptr(y);
        for (int x = 0; x < src.cols; x++) {
            if (row[x] != 0) {
                point_sum ++;
            }
        }
    }
    return (point_sum / (src.rows * src.cols));
}

void KCam::makeBlue(Mat pic) {
    for (int y = 0; y < pic.rows; y++) {
        unsigned char* row = pic.ptr(y);
        for (int x = 0; x < pic.cols * 3; x+=3) {
            row[x] = 255;
        }
    }
}

Scalar KCam::getAveColor(Mat pic) {
    double b = 0;
    double g = 0;
    double r = 0;
    
    //unsigned char* data = pic.data;
    for (int y = 0; y < pic.rows; y++) {
        unsigned char* row = pic.ptr(y);
        //data++;
        for (int x = 0; x < pic.cols * 3; x+=3) {
            b += row[x];
            g += row[x+1];
            r += row[x+2];
        }
    }
    return Scalar(b / (pic.rows * pic.cols), g / (pic.rows * pic.cols), r / (pic.rows * pic.cols),0);
}

inline void KCam::displayColorData() {
    cout << " val:" << color_data[VAL];
    cout << " sat:" << color_data[SAT];
    cout << " hue:" << color_data[HUE];
}
