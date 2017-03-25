#include "MarkingImg.h"

typedef struct _NumberElement
{
	IplImage *plate_number;
	int x;
	int y;
	int w;
	int h;
	string strWord;
} NumberElement;

CALL_BACK_SHOW_FUN pShowImage;

void setShowImgFun(CALL_BACK_SHOW_FUN f)
{
	pShowImage = f;
}

bool cmp(NumberElement a,NumberElement b)
{
	return a.x < b.x;
}

string  getSubtract(Mat&);
vector<string> m_vt;

vector<string> loadfile(const char* dirname)
{
		vector<string> vt;
		vt.clear();
    DIR* dp;
    struct dirent* dirp;
    struct stat st;
    //char tab[tabs + 1];

    /* open dirent directory */
    if((dp = opendir(dirname)) == NULL)
    {
        //perror("opendir");
        LOGD("not fond templat");
        return vt;
    }

    /* fill tab array with tabs */
    //memset(tab, '\t', tabs);
    //tab[tabs] = 0;

    /**
     * read all files in this dir
     **/
    while((dirp = readdir(dp)) != NULL)
    {
        char fullname[255];
        memset(fullname, 0, sizeof(fullname));

        /* ignore hidden files */
        if(dirp->d_name[0] == '.')
            continue;

        /* display file name with proper tab */
        //printf("%s%s\n", tab, dirp->d_name);

        strncpy(fullname, dirname, sizeof(fullname));
        strncat(fullname, "/", sizeof(fullname));
        strncat(fullname, dirp->d_name, sizeof(fullname));
        //printf("%s\n", fullname);
        vt.push_back(fullname);
        /* get dirent status */
        if(stat(fullname, &st) == -1)
            break;

        /* if dirent is a directory, call itself */
        if(S_ISDIR(st.st_mode) /*&& list_dir_name(fullname, tabs + 1) == -1*/)
            continue;
    }
    return vt;
}

void getPXSum(Mat &src, int &a)//获取所有像素点和
{ 
	threshold(src, src, 100, 255, CV_THRESH_BINARY);
	  a = 0;
	for (int i = 0; i < src.rows;i++)
	{
		for (int j = 0; j < src.cols; j++)
		{
			a += src.at <uchar>(i, j);
		}
	}
}

string  getSubtract(Mat &src) //两张图片相减
{
	Mat img_result;
	int min = 1000000;
	string strVal;
	for(vector<string>::iterator it = m_vt.begin();it!=m_vt.end();it++)
	{
		string strFileName = (*it).c_str();
	
		Mat Template = imread(strFileName.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
		threshold(Template, Template, 100, 255, CV_THRESH_BINARY);
		threshold(src, src, 100, 255, CV_THRESH_BINARY);
		resize(src, src, Size(32, 48), 0, 0, CV_INTER_LINEAR);
		resize(Template, Template, Size(32, 48), 0, 0, CV_INTER_LINEAR);//调整尺寸
		//imshow(name, Template);
		absdiff(Template, src, img_result);//两个图片对应像素点值相减
		int diff = 0;
		getPXSum(img_result, diff);
		if (diff < min)
		{
			min = diff;
			//serieNum = i;
			int iPos = strFileName.rfind("/");
			strFileName.substr(iPos + 1);
			strVal = strFileName.substr(iPos + 1);
		}
	}
	//printf("最小距离是%d ", min);
	//printf("%s\n", strVal);
	return strVal.substr(0,strVal.length()-4);
}

string MarkingImg(int width,int height,uchar *_yuv,const char *dir)
{
	string strValues = "";
    Mat myuv(height+height/2, width, CV_8UC1,_yuv);
    Mat mbgr(height, width, CV_8UC3, cv::Scalar(0,0,255));
    cvtColor(myuv, mbgr, CV_YUV420sp2BGR);
    
    Mat t,o;
 	transpose(mbgr,t);//转90度
 	flip(t,o,1);
 	
 	IplImage f = IplImage(o);
 	IplImage *frame = &f; 	
int iret = cvSaveImage("/storage/emulated/0/data/frame.jpg",frame);
LOGI("save1 result:%d,w:%d,h:%d",iret,frame->width,frame->height);
(*pShowImage)((const uchar*)frame->imageData,frame->imageSize,0);

/*
const char * path = "/storage/sdcard1/carnumber/65_car/car.jpg";  
IplImage * frame = cvLoadImage(path);  
LOGI("load file:%s",path);
if(!frame)
{
	LOGD("not fond file");
	return "not fond file";
}
m_vt.clear();
m_vt = loadfile(dir);
LOGI("get template size:%d",m_vt.size());
if(m_vt.size() < 1)
{
	LOGI("load template err");
	return "load template err";	
}
*/ 	
 	//均值滤波  
	cvSmooth(frame, frame, CV_MEDIAN);  
	//cvSmooth(frame, frame, CV_GAUSSIAN, 3, 3); 
	//灰度图  
	IplImage * gray = cvCreateImage(cvGetSize(frame), frame->depth, 1);  
	cvCvtColor(frame, gray, CV_BGR2GRAY);  
	//cvNamedWindow("gray", 1);  
	//cvShowImage("gray", gray);  

	//边缘检测  
	IplImage * temp = cvCreateImage(cvGetSize(gray), IPL_DEPTH_16S,1);  
	//x方向梯度，垂直边缘  
	cvSobel(gray, temp, 2, 0, 3);  
	IplImage * sobel = cvCreateImage(cvGetSize(temp), IPL_DEPTH_8U,1);  
	cvConvertScale(temp, sobel, 1, 0);  
	//cvNamedWindow("sobel", 1);  
	//cvShowImage("sobel", sobel);  
 		
	//二值化
	IplImage * threshold = cvCreateImage(cvGetSize(sobel), gray->depth, 1);
	cvThreshold(sobel, threshold, 0, 255, CV_THRESH_BINARY|CV_THRESH_OTSU);
//cvNamedWindow("threshold", 1);
//cvShowImage("threshold", threshold);
iret = cvSaveImage("/storage/emulated/0/data/threshold.jpg",threshold);
LOGI("save2 result:%d",iret);
(*pShowImage)((const uchar*)threshold->imageData,threshold->imageSize,1);

	//形态学变化
	IplConvKernel * kernal;
	IplImage * morph = cvCreateImage(cvGetSize(threshold), threshold->depth, 1);
	//自定义 1x3 的核进行 x 方向的膨胀腐蚀	
	kernal = cvCreateStructuringElementEx(3, 1, 1, 0, CV_SHAPE_RECT);
	cvDilate(threshold, morph, kernal, 4);   //x 膨胀联通数字
	cvErode(morph, morph, kernal, 4);    //x 腐蚀去除碎片
	cvDilate(morph, morph, kernal, 4);   //x 膨胀回复形态
	cvReleaseStructuringElement(&kernal);
	//自定义 3x1 的核进行 y 方向的膨胀腐蚀
	kernal = cvCreateStructuringElementEx(1, 3, 0, 1, CV_SHAPE_RECT);
	cvErode(morph, morph, kernal, 1);    //y 腐蚀去除碎片
	cvDilate(morph, morph, kernal, 3);   //y 膨胀回复形态	
	cvReleaseStructuringElement(&kernal);

//cvNamedWindow("erode", 1);
//cvShowImage("erode", morph);
iret = cvSaveImage("/storage/emulated/0/data/morph.jpg",morph);
LOGI("save3 result:%d",iret);
(*pShowImage)((const uchar *)morph->imageData,morph->imageSize,2);

	//轮廓检测
	IplImage *plate_img = NULL; 
	IplImage * frame_draw = cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
	cvCopy(frame, frame_draw);
	CvMemStorage * storage = cvCreateMemStorage(0);  
	CvSeq * contour = 0;   
	int count = cvFindContours(morph, storage, &contour, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE );
	CvSeq * _contour = contour;   
	for( ; contour != 0; contour = contour->h_next )  
	{  		
		double tmparea = fabs(cvContourArea(contour));  		 
		CvRect aRect = cvBoundingRect( contour, 0 ); 
		//cout << "w=" << aRect.width << ",h=" << aRect.height << " x=" << aRect.x << " y=" <<  aRect.y << endl;
		if(tmparea > ((frame->height*frame->width)/10))   
		{  
			cvSeqRemove(contour,0); //删除面积小于设定值的轮廓,1/10
			continue;  
		} 
		if (aRect.width < (aRect.height*2))  
		{  
			cvSeqRemove(contour,0); //删除宽高比例小于设定值的轮廓   
			continue;  
		}
		if ((aRect.width/aRect.height) > 4 )
		{  
			cvSeqRemove(contour,0); //删除宽高比例小于设定值的轮廓   
			continue;  
		}
		if((aRect.height * aRect.width) < ((frame->height * frame->width)/100))
		{  
			cvSeqRemove(contour,0); //删除宽高比例小于设定值的轮廓   
			continue;  
		}
	
		if(aRect.width  < 100 || aRect.width > 150)
		{
			cvSeqRemove(contour,0); //删除宽小于设定值的轮廓
			continue;  
		}
		CvScalar color = CV_RGB( 255, 0, 0); 
		cvDrawContours(frame_draw, contour, color, color, 0, 1, 8 );//绘制外部和内部的轮廓
		//cout << "last w=" << aRect.width << ",h=" << aRect.height << " x=" << aRect.x << " y=" <<  aRect.y << endl;
	
		cvSetImageROI(frame,  aRect);
		plate_img = cvCreateImage( cvSize(aRect.width,aRect.height), frame->depth, frame->nChannels);
		cvCopy(frame,plate_img);
		cvResetImageROI(frame);
		break;
	}
	cvReleaseMemStorage(&storage);
//cvNamedWindow("frame_draw", 1);
//cvShowImage("frame_draw", frame_draw);

	if(plate_img == NULL)
	{
		//cout << "not fond" << endl;
		LOGD("not found");
		//cvWaitKey(65000);
		
		cvReleaseImage(&gray);
		cvReleaseImage(&temp);
		cvReleaseImage(&threshold);
		cvReleaseImage(&sobel);
		cvReleaseImage(&morph);
		cvReleaseImage(&frame_draw);
		
		return strValues;
	}

	//cvSaveImage("1.bmp",plate_img);
	
	//灰度图  
	IplImage * grayPlate = cvCreateImage(cvGetSize(plate_img), plate_img->depth, 1);  
	cvCvtColor(plate_img, grayPlate, CV_BGR2GRAY);
	
	//二值化
	IplImage * thresholdPlate = cvCreateImage(cvGetSize(grayPlate), grayPlate->depth, 1);
	cvThreshold(grayPlate, thresholdPlate, 0, 255, CV_THRESH_BINARY|CV_THRESH_OTSU);
	IplImage * frame_bi = cvCreateImage(cvGetSize(thresholdPlate), thresholdPlate->depth, thresholdPlate->nChannels);
	cvCopy(thresholdPlate, frame_bi);
	
	//cvSmooth(thresholdPlate, thresholdPlate, CV_MEDIAN, 3, 0, 0, 0); //中值滤波，消除小的噪声；
	//cvDilate(thresholdPlate, thresholdPlate, 0, 1);
	//cvErode(thresholdPlate, thresholdPlate, 0, 2);

//cvNamedWindow("轮廓", 1);
//cvShowImage("轮廓", thresholdPlate);
iret = cvSaveImage("/storage/emulated/0/data/thresholdPlate.jpg",morph);
LOGI("save4 result:%d",iret);

	vector<NumberElement> numberVector;
	numberVector.clear();
	CvMemStorage * stg = cvCreateMemStorage(0);
	CvSeq * seq = 0; 
	int sum = cvFindContours(thresholdPlate,stg, &seq, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	for( ; seq != 0; seq = seq->h_next )
	{
		double tmparea = fabs(cvContourArea(seq));  		 
		CvRect aRect = cvBoundingRect( seq, 0 ); 
		if (aRect.height < 10)
		{
			cvSeqRemove(seq,0); //删除宽小于设定值的轮廓
			continue;
		}
		cvSetImageROI(frame_bi,  aRect);
		IplImage *plate_number = cvCreateImage( cvSize(aRect.width,aRect.height), frame_bi->depth, frame_bi->nChannels);
		cvCopy(frame_bi,plate_number);
		cvResetImageROI(frame_bi);
		Mat src = cv::cvarrToMat(plate_number);  //(plate_number);
		NumberElement ne;
		ne.plate_number = plate_number;
		ne.x = aRect.x;
		ne.y = aRect.y;
		ne.w = aRect.width;
		ne.h = aRect.height;
		ne.strWord = getSubtract(src);
		numberVector.push_back(ne);
	
	}
	cvReleaseMemStorage(&stg);
	sort(numberVector.begin(),numberVector.end(),cmp);
	if(numberVector.size() < 7)
	{
		vector<NumberElement>::iterator it = numberVector.begin();
		CvRect aRect = cvRect(it->x - it->w - 3,it->y,it->w+2,it->h);
		cvSetImageROI(frame_bi,  aRect);
		IplImage *plate_number = cvCreateImage( cvSize(aRect.width,aRect.height), frame_bi->depth, frame_bi->nChannels);
		cvCopy(frame_bi,plate_number);
		cvResetImageROI(frame_bi);
		Mat src = cv::cvarrToMat(plate_number);
		NumberElement ne;
		ne.plate_number = plate_number;
		ne.x = aRect.x;
		ne.y = aRect.y;
		ne.w = aRect.width;
		ne.h = aRect.height;
		ne.strWord = getSubtract(src);
		numberVector.push_back(ne);
	}
	sort(numberVector.begin(),numberVector.end(),cmp);
	//	cvNamedWindow("京", 1);
	//	cvShowImage("京", numberVector.begin()->plate_number);
	
	for(vector<NumberElement>::iterator it = numberVector.begin();it!=numberVector.end();it++)
	{
		char str[4] = {0};
		strcpy(str,it->strWord.c_str());
		strValues += it->strWord;
		cvReleaseImage(&it->plate_number);
	}
	
	cvReleaseImage(&gray);
	cvReleaseImage(&temp);
	cvReleaseImage(&threshold);
	cvReleaseImage(&sobel);
	cvReleaseImage(&morph);
	cvReleaseImage(&frame_draw);
	cvReleaseImage(&plate_img);
	cvReleaseImage(&grayPlate);
	cvReleaseImage(&thresholdPlate);
	cvReleaseImage(&frame_bi);
	
	return strValues;
}
