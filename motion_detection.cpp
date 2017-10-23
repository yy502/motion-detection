//
//  Created by Cedric Verstraeten on 18/02/14.
//  Copyright (c) 2014 Cedric Verstraeten. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>
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
using std::string;


// extract video file name from full path
string getFileName(const string& s)
{
   char sep = '/';
   size_t i = s.rfind(sep, s.length());
   if (i != string::npos) {
      return(s.substr(i+1, s.length() - i));
   }
   return("motion");
}

// Check if the directory exists, if not create it
// This function will create a new directory if the image is the first
// image taken for a specific day
inline void directoryExistsOrCreate(const char* pzPath)
{
    DIR *pDir;
    // directory doesn't exists -> create it
    if ( pzPath == NULL || (pDir = opendir (pzPath)) == NULL)
        mkdir(pzPath, 0777);
    // if directory exists we opened it and we
    // have to close the directory again.
    else if(pDir != NULL)
        (void) closedir (pDir);
}

// When motion is detected we write the image to disk
//    - Check if the directory exists where the image will be stored.
//    - Build the directory and image names.
inline bool saveImg(Mat image, const char *DIRECTORY, const char *PREFIX, int image_sequence, const string EXTENSION)
{
    stringstream ss;

    // Create name for the date directory
    ss.str("");
    ss << DIRECTORY;
    directoryExistsOrCreate(ss.str().c_str());

    // Create name for the image
    string path(PREFIX);
    ss << "/" << getFileName(path) << "_" << static_cast<int>(image_sequence) << EXTENSION;
    printf("Motion detected > %s\n",ss.str().c_str()); fflush(stdout);
    return imwrite(ss.str().c_str(), image);
}

// Check if there is motion in the result matrix
// count the number of changes and return.
inline int detectMotion(const Mat & motion, Mat & result,
                 int x_start, int x_stop, int y_start, int y_stop,
                 int max_deviation,
                 Scalar & box_color)
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
        if(frame_changes){
            //check if not out of bounds
            if(min_x-10 > 0) min_x -= 10;
            if(min_y-10 > 0) min_y -= 10;
            if(max_x+10 < result.cols-1) max_x += 10;
            if(max_y+10 < result.rows-1) max_y += 10;
            // draw rectangle round the changed pixel
            Point x(min_x,min_y);
            Point y(max_x,max_y);
            Rect rect(x,y);
            rectangle(result,rect,box_color,1);
        }
        return frame_changes;
    }
    return 0;
}

int main (int argc, char * const argv[])
{
    const string EXT = ".jpg"; // extension of the images

    if (argc<3) {
       fprintf(stderr, "Usage: motion INPUT_FILE_NAME OUTPUT_DIRECTORY [changes_threshold motion_deviation]\n");
       fprintf(stderr, "Default values: changes_threshold=5 motion_deviation=20\n");
       exit(1);
    }

    // Set up camera
    CvCapture * camera = cvCaptureFromFile(argv[1]);

    // Take images and convert them to gray
    Mat result, result_cropped;
    Mat prev_frame = result = cvQueryFrame(camera);
    Mat current_frame = cvQueryFrame(camera);
    Mat next_frame = cvQueryFrame(camera);

    cvtColor(current_frame, current_frame, CV_RGB2GRAY);
    cvtColor(prev_frame, prev_frame, CV_RGB2GRAY);
    cvtColor(next_frame, next_frame, CV_RGB2GRAY);
    
    // d1 and d2 for calculating the differences
    // result, the result of and operation, calculated on d1 and d2
    // frame_changes, the count of changes in the result matrix.
    // box_color, the color for drawing the rectangle around changed area
    Mat d1, d2, motion;
    int frame_changes, image_sequence = 0;
    Scalar mean_, box_color(0,255,255); // yellow
    

    int x_start = 0, x_stop = current_frame.cols;
    int y_start = 0, y_stop = current_frame.rows;


    // If more than 'motion_threshold' pixels are changed, we say there is motion
    // and store an image on disk
    int motion_threshold = 5;
    if (argc >= 4) motion_threshold = atoi(argv[3]);

    // Maximum deviation of the image, the higher the value, the more motion is allowed
    int max_deviation = 20;
    if (argc >= 5) max_deviation = atoi(argv[4]);

    // Erode kernel
    Mat kernel_ero = getStructuringElement(MORPH_RECT, Size(2,2));
    
    // All settings have been set, now go in endless loop and
    // take as many pictures you want..
    while (cvGrabFrame(camera)){
        // Take a new image
        prev_frame = current_frame;
        current_frame = next_frame;
        next_frame = cvRetrieveFrame(camera);
        result = next_frame;
        cvtColor(next_frame, next_frame, CV_RGB2GRAY);

        // Calc differences between the images and do AND-operation
        // threshold image, low differences are ignored (ex. contrast change due to sunlight)
        absdiff(prev_frame, next_frame, d1);
        absdiff(next_frame, current_frame, d2);
        bitwise_and(d1, d2, motion);
        threshold(motion, motion, 35, 255, CV_THRESH_BINARY);
        erode(motion, motion, kernel_ero);
        
        frame_changes = detectMotion(motion, result, x_start, x_stop, y_start, y_stop, max_deviation, box_color);
        if(frame_changes>=motion_threshold)
        {
            saveImg(result,argv[2],argv[1],image_sequence,EXT);
            image_sequence++;
        }
    }
    return 0;
}
