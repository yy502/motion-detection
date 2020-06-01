//
//  Created by Cedric Verstraeten on 18/02/14.
//  Copyright (c) 2014 Cedric Verstraeten. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <time.h>
#include <dirent.h>
#include <sstream>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;
using namespace cv;


// Check if there is motion in the result matrix
// count the number of changes and return.
inline int detectMotion(const Mat & motion, Mat & result,
                 int x_start, int x_stop, int y_start, int y_stop,
                 int max_deviation)
{
    // calculate the standard deviation
    Scalar mean, stddev;
    meanStdDev(motion, mean, stddev);
    // if not to much changes then the motion is real (neglect agressive snow, temporary sunlight)
    if(stddev[0] < max_deviation)
    {
        int frame_changes = 0;
        int min_x = motion.cols, max_x = 0;
        int min_y = motion.rows, max_y = 0;
        // loop over image and detect changes
        for(int j = y_start; j < y_stop; j+=2){ // height
            for(int i = x_start; i < x_stop; i+=2){ // width
                // check if at pixel (j,i) intensity is equal to 255
                // this means that the pixel is different in the sequence
                // of images (prev_frame, current_frame, next_frame)
                if(static_cast<int>(motion.at<uchar>(j,i)) == 255)
                {
                    frame_changes++;
                    if(min_x>i) min_x = i;
                    if(max_x<i) max_x = i;
                    if(min_y>j) min_y = j;
                    if(max_y<j) max_y = j;
                }
            }
        }

        return frame_changes;
    }
    return 0;
}

int main (int argc, char * const argv[])
{

    if (argc<2) {
       fprintf(stderr, "Usage: motion-seeker INPUT_FILE_NAME [changes_threshold motion_deviation]\n");
       fprintf(stderr, "Prints 1 to stdout if motion is detected in the video; 0 for no motion.\n");
       fprintf(stderr, "Default values: changes_threshold=20 motion_deviation=50\n");
       exit(1);
    }

    // Set up camera
    VideoCapture camera;
    camera.open(argv[1]);

    // Take images and convert them to gray
    Mat result;
    Mat prev_frame;
    Mat current_frame;
    Mat next_frame;

    camera.read(prev_frame);
    result = prev_frame;
    camera.read(current_frame);
    camera.read(next_frame);

    cvtColor(current_frame, current_frame, COLOR_RGB2GRAY);
    cvtColor(prev_frame, prev_frame, COLOR_RGB2GRAY);
    cvtColor(next_frame, next_frame, COLOR_RGB2GRAY);
    
    // d1 and d2 for calculating the differences
    // result, the result of and operation, calculated on d1 and d2
    // frame_changes, the count of changes in the result matrix.
    // box_color, the color for drawing the rectangle around changed area
    Mat d1, d2, motion;
    int frame_changes = 0;

    int x_start = 0, x_stop = current_frame.cols;
    int y_start = 0, y_stop = current_frame.rows;


    // If more than 'changes_threshold' pixels are changed, we say there is motion
    // and store an image on disk
    int changes_threshold = 20;
    if (argc >= 3) changes_threshold = atoi(argv[2]);

    // Maximum deviation of the image, the higher the value, the more motion is allowed
    int max_deviation = 50;
    if (argc == 4) max_deviation = atoi(argv[3]);

    // Erode kernel
    Mat kernel_ero = getStructuringElement(MORPH_RECT, Size(2,2));
    
    // All settings have been set, now go in endless loop and
    // take as many pictures you want..
    while (camera.isOpened()){
        // Take a new image
        prev_frame = current_frame;
        current_frame = next_frame;

	if (!camera.read(next_frame))
		break;
        result = next_frame;
        cvtColor(next_frame, next_frame, COLOR_RGB2GRAY);

        // Calc differences between the images and do AND-operation
        // threshold image, low differences are ignored (ex. contrast change due to sunlight)
        absdiff(prev_frame, next_frame, d1);
        absdiff(next_frame, current_frame, d2);
        bitwise_and(d1, d2, motion);
        threshold(motion, motion, 35, 255, THRESH_BINARY);
        erode(motion, motion, kernel_ero);
        
        frame_changes = detectMotion(motion, result, x_start, x_stop, y_start, y_stop, max_deviation);
        if(frame_changes>=changes_threshold)
        {
            fprintf(stdout, "1\n");
            return 0;
        }
    }
    fprintf(stdout, "0\n");
    return 0;
}
