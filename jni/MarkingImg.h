#ifndef _MARKING_IMG_H
#define _MARKING_IMG_H
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <android/log.h>

#include <math.h>
#include <vector>
#include <list>
#include <dirent.h>
#include <sys/stat.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv/cv.h>

using namespace std;
using namespace cv;

#define  LOG_TAG    "opencv_face_detect" 
#define  LOGD(...) __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#define  LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)  
#define  LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__) 

typedef  void(*CALL_BACK_SHOW_IMAGE_FUN)(const unsigned char*,int,int,int);
typedef  void(*CALL_BACK_SHOW_MSG_FUN)(const unsigned char*,int);
typedef  void(*CALL_BACK_DRAW_RECT_FUN)(int,int,int,int);

void setShowImgFun(CALL_BACK_SHOW_IMAGE_FUN);
void setShowMsgFun(CALL_BACK_SHOW_MSG_FUN);
void setDrawRectangleFun(CALL_BACK_DRAW_RECT_FUN);
void breakRunning();
string MarkingImg(int,int,uchar *,const char*);
//string MarkingImg1(int,int,uchar *,const char*);
int loadfile(const char* dirname);

#endif
