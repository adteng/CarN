#include "com_teng_carn_MainActivity.h"
#include "MarkingImg.h"

JavaVM *g_jvm;
jobject g_obj;
bool myAttachCurrentThread(void** env );
void ShowData(const unsigned char *pData,int iDataLen,int iWidth,int iHeight);
void ShowMsg(const unsigned char *pData,int iDataLen);
void DrawRectangle(int x,int y,int width,int height);

JNIEXPORT void JNICALL Java_com_teng_carn_MainActivity_setJNIEnv
  (JNIEnv *env, jobject obj,jstring strDir)
{
        env->GetJavaVM(&g_jvm);
        setShowImgFun(&ShowData);
        setShowMsgFun(&ShowMsg);
        setDrawRectangleFun(&DrawRectangle);
        g_obj = env->NewGlobalRef(obj);
        const char *dir = env->GetStringUTFChars(strDir, 0);
        loadfile(dir);
        env->ReleaseStringUTFChars(strDir, dir); 
}
JNIEXPORT void JNICALL Java_com_teng_carn_MainActivity_breakRunning
  (JNIEnv *env, jobject obj)
{
	breakRunning();
}


JNIEXPORT jstring JNICALL Java_com_teng_carn_MainActivity_getStringNumber
  (JNIEnv * env, jobject obj, jint width, jint height, jbyteArray yuv, jstring strDir)
{
	jbyte* _yuv =env->GetByteArrayElements(yuv,0);
   	const char *dir = env->GetStringUTFChars(strDir, 0);
   	 //g_obj = env->NewGlobalRef(obj);
    string strWord = MarkingImg(width,height,(uchar *)_yuv,dir);
    LOGI("word:%s",strWord.c_str());
   	env->ReleaseByteArrayElements(yuv,_yuv,JNI_FALSE);
   	env->ReleaseStringUTFChars(strDir, dir); 
   	//env->DeleteGlobalRef(g_obj);
   	jstring str = env->NewStringUTF(strWord.c_str());
   	LOGI("111111111111");
   	return str;
}

bool myAttachCurrentThread(void** env )
{
    int status = 0;
    status = g_jvm->GetEnv(env,JNI_VERSION_1_4);
    if(status<0)
    {
        g_jvm->AttachCurrentThread((JNIEnv**)env, NULL);
        return true;
    }
    return false;
}

void ShowData(const unsigned char *pData,int iDataLen,int iWidth,int iHeight)
{
//      LOGI("helloooo");
        JNIEnv *env;
        /*
        if(g_jvm->AttachCurrentThread(&env, NULL) != JNI_OK)
        {
                LOGE("%s: AttachCurrentThread() failed", __FUNCTION__);
                return;
        }*/
        bool bAttach = myAttachCurrentThread((void**)&env);
        //jbyteArray b = env->NewByteArray(iDataLen);
        //env->SetByteArrayRegion(b,0,iDataLen,(jbyte *)pData);
       	jintArray carr = env->NewIntArray(iDataLen);
       	//jint *pColor = (jint *)calloc(iDataLen, sizeof(jint));
       	jint *pColor = env->GetIntArrayElements(carr, NULL);
        int k = 0;
       	for(int i = 0; i< iWidth * iHeight; i++)
        {
			pColor[i] = ( (uint32_t)pData[k] | (uint32_t)(pData[k+1] << 8) | (uint32_t)(pData[k+2]<<16) | (uint32_t)(255 << 24) );
			k+=iDataLen/(iWidth * iHeight);
		}
        //env->SetIntArrayRegion(carr,0,iDataLen,pColor);
        jclass cls = env->GetObjectClass(g_obj);
        jmethodID mDraw = env->GetMethodID(cls,"drawImage","([III)V");
        env->CallVoidMethod(g_obj,mDraw,carr,iWidth,iHeight);
        env->DeleteLocalRef(cls);
        env->ReleaseIntArrayElements(carr, pColor, 0);
        env->DeleteLocalRef(carr);
        //free(pColor);
          
        if(bAttach)
        if(g_jvm->DetachCurrentThread() != JNI_OK)
        {
                LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
        }
}

void ShowMsg(const unsigned char *pData,int iDataLen)
{
	JNIEnv *env;
	bool bAttach = myAttachCurrentThread((void**)&env);	
	jbyteArray b = env->NewByteArray(iDataLen);
    env->SetByteArrayRegion(b,0,iDataLen,(jbyte *)pData);
	jclass cls = env->GetObjectClass(g_obj);
	jmethodID mDraw = env->GetMethodID(cls,"showMsg","([BI)V");
 	env->CallVoidMethod(g_obj,mDraw,b,iDataLen);
    env->DeleteLocalRef(cls);
    env->DeleteLocalRef(b);
    if(bAttach)
        if(g_jvm->DetachCurrentThread() != JNI_OK)
        {
                LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
        }
}

void DrawRectangle(int x,int y,int width,int height)
{
	JNIEnv *env;
	bool bAttach = myAttachCurrentThread((void**)&env);	
	
	jclass cls = env->GetObjectClass(g_obj);
	jmethodID mDraw = env->GetMethodID(cls,"drawRect","(IIII)V");
 	env->CallVoidMethod(g_obj,mDraw,x,y,width,height);
    env->DeleteLocalRef(cls);
	
	
	if(bAttach)
        if(g_jvm->DetachCurrentThread() != JNI_OK)
        {
                LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
        }
}

