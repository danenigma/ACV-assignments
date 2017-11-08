/* 
  Example use of openCV to use the Hough Transform
  ------------------------------------------------
  Interface file

  David Vernon
  24 January 2017

*/

 

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include <ctype.h>
#include <iostream>
#include <string>
#include <conio.h>
#include <cmath>

//opencv
#include <cv.h>
#include <highgui.h>
#include <opencv2/opencv.hpp>

#define TRUE  1
#define FALSE 0
#define MAX_STRING_LENGTH 80
#define MAX_FILENAME_LENGTH 80

using namespace std;
using namespace cv;

/* function prototypes go here */

void CannyThreshold(int, void*);
void HoughThreshold(int, void*);
void prompt_and_exit(int status);
void get_orientation_and_center(vector<Vec2f> lines);
void get_center(vector<Vec2f> lines);
void create_histogram_of_lines(vector<float> lineOrientation);
int find_perpendicular(int indexOfCurrent,vector<Vec2f> lines,int parLineIndex,int distance,vector<Vec2f>&validLines);
float computeDistanceBetweenLines(Vec2f line1,Vec2f line2);
vector<Vec2f> get_the_four_main_lines(vector<Vec2f> lines);