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

CALL_BACK_SHOW_IMAGE_FUN pShowImage;
CALL_BACK_SHOW_MSG_FUN pShowMsg;

void setShowImgFun(CALL_BACK_SHOW_IMAGE_FUN f)
{
	pShowImage = f;
}
void setShowMsgFun(CALL_BACK_SHOW_MSG_FUN f)
{
	pShowMsg = f;
}

bool cmp(NumberElement a,NumberElement b)
{
	return a.x < b.x;
}

string  getSubtract(Mat&);
vector<string> m_vt;
Mat Operater(Mat &gray);
string separateCarStr(Mat &image);

int loadfile(const char* dirname)
{
	m_vt.clear();
    DIR* dp;
    struct dirent* dirp;
    struct stat st;
    //char tab[tabs + 1];

    /* open dirent directory */
    if((dp = opendir(dirname)) == NULL)
    {
        //perror("opendir");
        LOGD("not fond templat");
        return -1;
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
        m_vt.push_back(fullname);
        /* get dirent status */
        if(stat(fullname, &st) == -1)
            break;

        /* if dirent is a directory, call itself */
        if(S_ISDIR(st.st_mode) /*&& list_dir_name(fullname, tabs + 1) == -1*/)
            continue;
    }
    return m_vt.size();
}

void getPXSum(Mat &src, int &a)//获取所有像素点和
{ 
	threshold(src, src, 0, 255, CV_THRESH_BINARY |CV_THRESH_OTSU);
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
	int min = 1000000;
	string strVal;
	for(vector<string>::iterator it = m_vt.begin();it!=m_vt.end();it++)
	{
		string strFileName = (*it).c_str();
	
		Mat Template = imread(strFileName.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
		threshold(Template, Template, 0, 255, CV_THRESH_BINARY |CV_THRESH_OTSU);
		threshold(src, src, 0, 255, CV_THRESH_BINARY |CV_THRESH_OTSU);
		resize(src, src, Size(20, 40), 0, 0, CV_INTER_LINEAR);
		//resize(Template, Template, Size(32, 48), 0, 0, CV_INTER_LINEAR);//调整尺寸
		//imshow(name, Template);
		Mat img_result;
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
/*
string MarkingImg1(int width,int height,uchar *_yuv,const char *dir)
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
//int iret = cvSaveImage("/storage/emulated/0/data/frame.jpg",frame);
//LOGI("save1 result:%d,w:%d,h:%d",iret,frame->width,frame->height);
//(*pShowImage)((const uchar*)frame->imageData,frame->imageSize,0);

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
//iret = cvSaveImage("/storage/emulated/0/data/morph.jpg",morph);
//LOGI("save3 result:%d",iret);
//(*pShowImage)((const uchar *)morph->imageData,morph->imageSize,2);

	//轮廓检测
	IplImage *plate_img = NULL; 
	IplImage * frame_draw = cvCreateImage(cvGetSize(frame), frame->depth, frame->nChannels);
	cvCopy(frame, frame_draw);
	CvMemStorage * storage = cvCreateMemStorage(0);  
	CvSeq * contour = 0;   
	int count = cvFindContours(morph, storage, &contour, sizeof(CvContour), CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE );
	CvSeq * _contour = contour;   
	for( ; contour != 0; contour = contour->h_next )  
	{  		
		double tmparea = fabs(cvContourArea(contour));
		CvRect aRect = cvBoundingRect( contour, 0 );
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
		//(aRect.x + aRect.width/2,aRect.y + aRect.height/2)   (morph.width/2,morph.height/2)
		double l = sqrt((aRect.x + aRect.width/2 - morph->width/2) * (aRect.x + aRect.width/2 - morph->width/2) + (aRect.y + aRect.height/2-morph->height/2) * (aRect.y + aRect.height/2-morph->height/2));
		if( l > 100.0 ) 
		{
			cvSeqRemove(contour,0); 
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

	if(plate_img == NULL)
	{
		LOGD("not found");		
		cvReleaseImage(&gray);
		cvReleaseImage(&temp);
		cvReleaseImage(&threshold);
		cvReleaseImage(&sobel);
		cvReleaseImage(&morph);
		cvReleaseImage(&frame_draw);
		return strValues;
	}
	cvSaveImage("/storage/emulated/0/data/morph.jpg",plate_img);
	
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
int iret = cvSaveImage("/storage/emulated/0/data/thresholdPlate.jpg",morph);
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
string MarkingImg2(int width,int height,uchar *_yuv,const char *dir)
{
	string strValues = "";
    Mat myuv(height+height/2, width, CV_8UC1,_yuv);
    Mat mbgr(height, width, CV_8UC3, cv::Scalar(0,0,255));
    cvtColor(myuv, mbgr, CV_YUV420sp2BGR);
    
    Mat t,oriMat;
 	transpose(mbgr,t);//转90度
 	flip(t,oriMat,1);
 	
 	Mat w_mat = oriMat.clone();
 	Mat gray;
	cvtColor(w_mat,gray,CV_BGR2GRAY);
	
	Mat gray_bi = Operater(gray);
	if(string(dir) == "true")
		imwrite("/storage/emulated/0/data/morph.jpg",gray_bi);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours( gray_bi, contours, hierarchy, 
	  CV_RETR_EXTERNAL,//CV_RETR_TREE,CV_RETR_LIST,CV_RETR_CCOMP,CV_RETR_EXTERNAL
	  CV_CHAIN_APPROX_SIMPLE, 
	  Point(0, 0) );
	 
	int i=0;
	string str = "";
	vector<vector<Point> >::iterator itc= contours.begin();
	while (itc!=contours.end()) 
	{	 
		double tmparea = fabs(contourArea(*itc));//面积
		double contLenth =  arcLength(*itc,true);//周长
		double Afa = (4 * CV_PI *  tmparea)/(contLenth * contLenth);//与圆的近似度
		
		RotatedRect minRect = minAreaRect(*itc);  
		Point2f vertices[4];  
		minRect.points(vertices); //获得最小外接矩形4个点
		double L1 = sqrt((vertices[0].x-vertices[1].x) * (vertices[0].x-vertices[1].x) + (vertices[0].y-vertices[1].y) * (vertices[0].y-vertices[1].y));
		double L2 = sqrt((vertices[2].x-vertices[1].x) * (vertices[2].x-vertices[1].x) + (vertices[2].y-vertices[1].y) * (vertices[2].y-vertices[1].y));
		float angle;
		if(L1 > L2) 
		{
			int T = L2;
			L2 = L1;
			L1 = T;
			angle = atan2((vertices[0].y-vertices[1].y),(vertices[0].x-vertices[1].x)) * 180.0/CV_PI;
		}
		else
			angle = atan2((vertices[2].y-vertices[1].y),(vertices[2].x-vertices[1].x)) * 180.0/CV_PI;
		
		//最小外接圆
		Point2f center;//圆心  
		float radius;//半径  
		minEnclosingCircle(*itc, center, radius);
		
		Rect rt = boundingRect(*itc);//包含轮廓的矩形
		double l = sqrt((center.x - gray_bi.size().width/2) * (center.x - gray_bi.size().width/2) + (center.y - gray_bi.size().height/2) * (center.y - gray_bi.size().height/2));
		if(l > 20)
		{
			itc = contours.erase(itc);
		}
		else if(Afa < 0.08)
			itc = contours.erase(itc);
		else
		{
			char sTmp[64] = {0};
			sprintf(sTmp,"%d,%d,%d,%d,",rt.x,rt.y,rt.x + rt.width,rt.y + rt.height);
			str += sTmp;
			i++;
			itc++;
		}
	}
	char *pRetBuffer = new char[str.length() + 4];
	memset(pRetBuffer,0,str.length() + 3);
	sprintf(pRetBuffer,"%d,%s",i,str.c_str());
	str = pRetBuffer;
	delete [] pRetBuffer;
	return str;
}
*/
string MarkingImg(int width,int height,uchar *_yuv,const char *dir)
{
	string strValues = "";
    Mat myuv(height+height/2, width, CV_8UC1,_yuv);
    Mat mbgr(height, width, CV_8UC3, cv::Scalar(0,0,255));
    cvtColor(myuv, mbgr, CV_YUV420sp2BGR);
    
    Mat t,oriMat;
 	transpose(mbgr,t);//转90度
 	flip(t,oriMat,1);

//(*pShowImage)(oriMat.data,oriMat.step[0]*oriMat.rows,oriMat.cols,oriMat.rows);

 	Mat w_mat = oriMat.clone();
 	Mat imgHSV;
 	vector<Mat> hsvSplit;
	cvtColor(w_mat,imgHSV,COLOR_BGR2HSV);
	split(imgHSV,hsvSplit);
	equalizeHist(hsvSplit[2],hsvSplit[2]);
	merge(hsvSplit,imgHSV);
	Mat imgThresholded;
	inRange(imgHSV, Scalar(100, 50, 50), Scalar(170, 255, 255), imgThresholded); //Threshold the image
	
	Mat gray_bi = Operater(imgThresholded);
//(*pShowImage)(gray_bi.data,gray_bi.step[0]*gray_bi.rows,gray_bi.cols,gray_bi.rows);
	/*
	if(string(dir) == "true")
		imwrite("/storage/emulated/0/data/morph.jpg",gray_bi);	
	*/
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours( gray_bi, contours, hierarchy, 
	  CV_RETR_EXTERNAL,//CV_RETR_TREE,CV_RETR_LIST,CV_RETR_CCOMP,CV_RETR_EXTERNAL
	  CV_CHAIN_APPROX_SIMPLE, 
	  Point(0, 0) );
	 
	int i=0;
	string str = "";
	vector<vector<Point> >::iterator itc= contours.begin();
	while (itc!=contours.end()) 
	{	 
		double tmparea = fabs(contourArea(*itc));//面积
		double contLenth =  arcLength(*itc,true);//周长
		double Afa = (4 * CV_PI *  tmparea)/(contLenth * contLenth);//与圆的近似度
		
		RotatedRect minRect = minAreaRect(*itc);  
		Point2f vertices[4];  
		minRect.points(vertices); //获得最小外接矩形4个点
		double L1 = sqrt((vertices[0].x-vertices[1].x) * (vertices[0].x-vertices[1].x) + (vertices[0].y-vertices[1].y) * (vertices[0].y-vertices[1].y));
		double L2 = sqrt((vertices[2].x-vertices[1].x) * (vertices[2].x-vertices[1].x) + (vertices[2].y-vertices[1].y) * (vertices[2].y-vertices[1].y));
		float angle;
		if(L1 > L2) 
		{
			int T = L2;
			L2 = L1;
			L1 = T;
			angle = atan2((vertices[0].y-vertices[1].y),(vertices[0].x-vertices[1].x)) * 180.0/CV_PI;
		}
		else
			angle = atan2((vertices[2].y-vertices[1].y),(vertices[2].x-vertices[1].x)) * 180.0/CV_PI;
		
		//最小外接圆
		Point2f center;//圆心  
		float radius;//半径 
		minEnclosingCircle(*itc, center, radius);
		
		Rect rt = boundingRect(*itc);//包含轮廓的矩形
		double l = sqrt((center.x - gray_bi.size().width/2) * (center.x - gray_bi.size().width/2) + (center.y - gray_bi.size().height/2) * (center.y - gray_bi.size().height/2));
		if(l > 50)
		{
			itc = contours.erase(itc);
		}
		else if(Afa < 0.08)
			itc = contours.erase(itc);
		else if(rt.width < oriMat.cols/8)
			itc = contours.erase(itc);
		else
		{
			char sTmp[64] = {0};
			sprintf(sTmp,"%d,%d,%d,%d,",rt.x,rt.y,rt.x + rt.width,rt.y + rt.height);
			str += sTmp;
			i++;
			itc++;
			Mat image_roi = oriMat(rt).clone();
			(*pShowImage)(image_roi.data,image_roi.step[0]*image_roi.rows,image_roi.cols,image_roi.rows);
			string strNum = separateCarStr(image_roi);
imwrite("/storage/emulated/0/data/car_sno.jpg",image_roi);	
		}
	}
	char *pRetBuffer = new char[str.length() + 4];
	memset(pRetBuffer,0,str.length() + 3);
	sprintf(pRetBuffer,"%d,%s",i,str.c_str());
	str = pRetBuffer;
	delete [] pRetBuffer;
	return str;
}

Mat Operater(Mat &gray)
{
	//高斯滤波器滤波去噪（可选）
	int ksize = 3;
	Mat g_gray;
	Mat G_kernel = getGaussianKernel(ksize,0.3*((ksize-1)*0.5-1)+0.8);
	filter2D(gray,g_gray,-1,G_kernel);

/*	//Sobel算子（x方向和y方向）
	Mat sobel_x,sobel_y;
	Sobel(g_gray,sobel_x,CV_16S,1,0,3);
	Sobel(g_gray,sobel_y,CV_16S,0,1,3); 
	Mat abs_x,abs_y;
	convertScaleAbs(sobel_x,abs_x);
	convertScaleAbs(sobel_y,abs_y);
	Mat grad;
	addWeighted(abs_x,0.5,abs_y,0.5,0,grad);
	*/
	Mat detected_edges;
	blur( g_gray, detected_edges, Size(3,3) );
	/// 运行Canny算子
	Canny( detected_edges, detected_edges, 80, 80*2.5, 3 );
	/// 使用 Canny算子输出边缘作为掩码显示原图像
	/*
	dst = Scalar::all(0);
	src.copyTo( dst, detected_edges);
	imshow( window_name, dst );
	*/
	//return detected_edges;
	Mat img_bin;
	threshold(detected_edges,img_bin,0,255,CV_THRESH_BINARY |CV_THRESH_OTSU);
    Mat elementX = getStructuringElement(MORPH_RECT, Size(3, 3),Point(-1,-1));
    Mat m_ResImg;
    dilate(img_bin, m_ResImg,elementX,Point(-1,-1),1);
	//erode(m_ResImg, m_ResImg,elementX,Point(-1,-1),1);
	//dilate(m_ResImg, m_ResImg,elementX,Point(-1,-1),1);	
	return m_ResImg;
}

string separateCarStr(Mat &image)
{
	Mat detected_edges,gray;
	cvtColor(image,gray,COLOR_BGR2GRAY);
	blur(gray, gray, Size(3,3) );
	Canny(gray, detected_edges, 80, 80*2.5, 3 );
	Mat ori = image.clone();
	//image.copyTo(ori, detected_edges);

	threshold(detected_edges,detected_edges,0,255,CV_THRESH_BINARY |CV_THRESH_OTSU);
	
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours( detected_edges, contours, hierarchy, 
	  CV_RETR_EXTERNAL,//CV_RETR_TREE,CV_RETR_LIST,CV_RETR_CCOMP,CV_RETR_EXTERNAL
	  CV_CHAIN_APPROX_SIMPLE, 
	  Point(0, 0) );
	vector<NumberElement> vlist;vlist.clear();
	vector<vector<Point> >::iterator itc= contours.begin();
	while (itc!=contours.end()) 
	{	 
		double tmparea = fabs(contourArea(*itc));//面积	
			
		RotatedRect minRect = minAreaRect(*itc);
		Point2f vertices[4];  
		minRect.points(vertices); //获得最小外接矩形4个点
		Rect rt = boundingRect(*itc);
		if(rt.height < image.rows /4.0)
			itc = contours.erase(itc);
		else if(rt.width > rt.height)
			itc = contours.erase(itc);
		else if(rt.height > 5 * rt.width)
			itc = contours.erase(itc);
		else
		{
			NumberElement ne;
			ne.x = rt.x;
			ne.y = rt.y;
			ne.w = rt.width;
			ne.h = rt.height;
	
			Mat m = ori(rt).clone();
			cvtColor(m,m,COLOR_BGR2GRAY);
			//blur(m, m, Size(3,3));
			//threshold(m,m,0,255,CV_THRESH_BINARY |CV_THRESH_OTSU);
			ne.strWord = getSubtract(m);
			vlist.push_back(ne);
//cout << "seq:" << itc-contours.begin() + 1 << " tmparea:" << tmparea << " rt_width:" << rt.width << " rt_height:" << rt.height  << endl;
			++itc;
		}
	}
	string str = "";
	int iBeforeX = 0;
	sort(vlist.begin(),vlist.end(),cmp);
	vector<NumberElement>::iterator it=vlist.begin();
	if(it == vlist.end()||it->strWord.length() < 2) return "0";
	for(;it!=vlist.end();it++)
	{
		if(it->x == iBeforeX)
			continue;
		iBeforeX = it->x;
		str += it->strWord;
	}

	(*pShowMsg)((uchar *)str.c_str(),str.length());
	return str;
}
/*
string separateCarStr(Mat &image)
{
	Mat gray;
	cvtColor(image,gray,COLOR_BGR2GRAY);
	blur( gray, gray, Size(3,3) );
	Canny( gray, gray, 80, 80*2.5, 3 );
	Mat ori = gray.clone();
	threshold(gray,gray,0,255,CV_THRESH_BINARY |CV_THRESH_OTSU);
	vector<vector<Point> > contours;
	vector<Vec4i> hierarchy;
	findContours( gray, contours, hierarchy, 
	  CV_RETR_CCOMP,//CV_RETR_TREE,CV_RETR_LIST,CV_RETR_CCOMP,CV_RETR_EXTERNAL
	  CV_CHAIN_APPROX_SIMPLE, 
	  Point(0, 0) );
	string str = "";
	vector<vector<Point> >::iterator itc= contours.begin();
	while (itc!=contours.end()) 
	{	 
		double tmparea = fabs(contourArea(*itc));//面积	
			
		RotatedRect minRect = minAreaRect(*itc);
		Point2f vertices[4];  
		minRect.points(vertices); //获得最小外接矩形4个点
		Rect rt = boundingRect(*itc);
		if(rt.height < image.rows /4.0)
			itc = contours.erase(itc);
		else if(rt.width > rt.height)
			itc = contours.erase(itc);
		else if(rt.height > 5 * rt.width)
			itc = contours.erase(itc);
		else
		{
			Mat m = ori(rt).clone();
			str += getSubtract(m);
			++itc;
		}
	}
	(*pShowMsg)((uchar *)str.c_str(),str.length());
	return str;
}

*/
