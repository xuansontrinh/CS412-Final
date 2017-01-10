// Main.h

#ifndef MY_MAIN         // used MY_MAIN for this include guard rather than MAIN just in case some compilers or environments #define MAIN already
#define MY_MAIN

#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include "DetectPlates.h"
#include "PossiblePlate.h"
#include "DetectChars.h"
#include <vector>
#include <string>
#include <Windows.h>
#include <boost\filesystem.hpp>
#include <iostream>
#include <fstream>
#include <locale> 
using namespace std;

//#define SHOW_STEPS            // un-comment or comment this line to show steps or not

// global constants ///////////////////////////////////////////////////////////////////////////////
const cv::Scalar SCALAR_BLACK = cv::Scalar(0.0, 0.0, 0.0);
const cv::Scalar SCALAR_WHITE = cv::Scalar(255.0, 255.0, 255.0);
const cv::Scalar SCALAR_YELLOW = cv::Scalar(0.0, 255.0, 255.0);
const cv::Scalar SCALAR_GREEN = cv::Scalar(0.0, 255.0, 0.0);
const cv::Scalar SCALAR_RED = cv::Scalar(0.0, 0.0, 255.0);

// function prototypes ////////////////////////////////////////////////////////////////////////////
int main(void);
void drawRedRectangleAroundPlate(cv::Mat &imgOriginalScene, PossiblePlate &licPlate);
void writeLicensePlateCharsOnImage(cv::Mat &imgOriginalScene, PossiblePlate &licPlate);
std::vector<std::string> get_file_list(const std::string& path);
void create_folders(const std::vector<string>& folders);
std::string toLower(string s);
string removeExtension(string s);
string refineString(string s);
int compareString(string s, string t);
# endif	// MAIN

