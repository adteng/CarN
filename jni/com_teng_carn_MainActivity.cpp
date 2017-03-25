#include "com_teng_carn_MainActivity.h"
#include "MarkingImg.h"

JavaVM *g_jvm;
jobject g_obj;
bool myAttachCurrentThread(void** env );
void ShowData(const unsigned char *pData,int iDataLen,int iFlag);

JNIEXPORT void JNICALL Java_com_teng_carn_MainActivity_setJNIEnv
  (JNIEnv *env, jobject)
{
        env->GetJavaVM(&g_jvm);
        setShowImgFun(&ShowData);
}

JNIEXPORT jstring JNICALL Java_com_teng_carn_MainActivity_getStringNumber
  (JNIEnv * env, jobject obj, jint width, jint height, jbyteArray yuv, jstring strDir)
{
	jbyte* _yuv =env->GetByteArrayElements(yuv,0);
   	const char *dir = env->GetStringUTFChars(strDir, 0);     	
   	 g_obj = env->NewGlobalRef(obj);
    string strWord = MarkingImg(width,height,(uchar *)_yuv,dir);
    LOGI("word:%s",strWord.c_str());
   	env->ReleaseByteArrayElements(yuv,_yuv,JNI_FALSE);
   	env->ReleaseStringUTFChars(strDir, dir); 
   	env->DeleteGlobalRef(g_obj);
   	jstring str = env->NewStringUTF(strWord.c_str());
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

void ShowData(const unsigned char *pData,int iDataLen,int iFlag)
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
        jbyteArray b = env->NewByteArray(iDataLen);
        env->SetByteArrayRegion(b,0,iDataLen,(jbyte *)pData);

        jclass cls = env->GetObjectClass(g_obj);
        jmethodID mDraw = env->GetMethodID(cls,"drawImage","([BII)V");
        env->CallVoidMethod(g_obj,mDraw,b,iDataLen,iFlag);
        env->DeleteLocalRef(cls);
        env->DeleteLocalRef(b);
        if(bAttach)
        if(g_jvm->DetachCurrentThread() != JNI_OK)
        {
                LOGE("%s: DetachCurrentThread() failed", __FUNCTION__);
        }
}

