package com.teng.carn;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.ImageFormat;
import android.graphics.Rect;
import android.graphics.YuvImage;
import android.hardware.Camera;
import android.hardware.Camera.PictureCallback;
import android.hardware.Camera.ShutterCallback;
import android.hardware.Camera.Size;
import android.os.Bundle;
import android.os.Environment;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.Configuration;
import android.util.Log;
import android.view.Menu;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.ZoomControls;

public class MainActivity extends Activity  implements SurfaceHolder.Callback {

    private Camera mCamera;// Camera对象
    private Button mButton;// 右侧条框，点击出发保存图像（拍照）的事件
    private SurfaceView mSurfaceView;// 显示图像的surfaceView
    private SurfaceHolder holder;// SurfaceView的控制器
    private MyAutoFocusCallback mAutoFocusCallback = new MyAutoFocusCallback();// AutoFocusCallback自动对焦的回调对象
    private ImageView sendImageIv;// 发送图片的imageview，位于右侧条框

    private String strCaptureFilePath = Environment
            .getExternalStorageDirectory() + "/DCIM/Camera/";// 保存图像的路径
    
    private byte[] mBuffer = new byte[3224000];
    private int mCurrBufferLen = 0;
    private String m_strLock = "lock";
    
    private int m_iMaxZoom = 0;
    private int m_iZoom = 0;
    private boolean m_bTakingPicture = false; 
    private boolean m_bFocus = false;
    private SVDraw  mSVDraw = null;
    Thread m_setFocusThread;	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		
	       if (checkCameraHardware(this)) {
	            Log.e("============", "摄像头存在");// 验证摄像头是否存在
	        }
	        /* 隐藏状态栏 */
	        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
	                WindowManager.LayoutParams.FLAG_FULLSCREEN);
	        /* 隐藏标题栏 */
	        requestWindowFeature(Window.FEATURE_NO_TITLE);
	         
	        /* 设定屏幕显示为横向 */
	        // this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
	        setContentView(R.layout.activity_main);
	        
	        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);		
			//getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
			
			getWindow().setSoftInputMode(
	                WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
	        
	        /* SurfaceHolder设置 */
	        mSurfaceView = (SurfaceView) findViewById(R.id.mSurfaceView);
	        holder = mSurfaceView.getHolder();
	        holder.addCallback(this);
	        // holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	        /* 设置拍照Button的OnClick事件处理 */

	        mButton = (Button) findViewById(R.id.myButton);
	        mButton.setOnClickListener(new OnClickListener() {
	            @Override
	            public void onClick(View arg0) {
	                /* 自动对焦后拍照 */
	                //mCamera.autoFocus(mAutoFocusCallback);// 调用mCamera的
	                //takePicture();
	            	m_bTakingPicture = true;
	            	mCamera.autoFocus(mAutoFocusCallback);
	            	
	            }
	        });
	       

	        sendImageIv = (ImageView) findViewById(R.id.send_image);
	        sendImageIv.setOnClickListener(new OnClickListener() {
	            @Override
	            public void onClick(View v) {
	                Intent i = new Intent();
	                i.setType("image/*");
	                i.setAction(Intent.ACTION_GET_CONTENT);
	                startActivityForResult(i, Activity.DEFAULT_KEYS_SHORTCUT);
	            }
	        });
	        
	        
	        
	        ZoomControls zoomControls = (ZoomControls) findViewById(R.id.zoomControls1);
            zoomControls.setOnZoomInClickListener (new View.OnClickListener() {
                       @Override
                       public void onClick(View v) {
                               if(m_iZoom < m_iMaxZoom)
                               {
                                       m_iZoom = m_iZoom + 1;
                                       android.hardware.Camera.Parameters params = mCamera.getParameters();
                                       params.setZoom(m_iZoom);
                                       mCamera.setParameters(params);
                                       mCamera.autoFocus(mAutoFocusCallback);
                               }
                       }
                      });
            zoomControls.setOnZoomOutClickListener(new View.OnClickListener() {

                       @Override
                       public void onClick(View v) {
                               if(m_iZoom > 0)
                               {
                                       m_iZoom = m_iZoom - 1;
                                       android.hardware.Camera.Parameters params = mCamera.getParameters();
                                       params.setZoom(m_iZoom);
                                       mCamera.setParameters(params);
                                       mCamera.autoFocus(mAutoFocusCallback);
                               }
                       }

                      });
            
            
           mSVDraw = (SVDraw)findViewById(R.id.mDraw);
           mSVDraw.setVisibility(View.VISIBLE);  
           m_setFocusThread = new Thread(){
        	   	
        	   	public void run()
        	   	{
        	   		try {
						Thread.sleep(5000);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
        	   		while(mCamera != null)
        	   		{
        	   			if(!m_bFocus)
        	   			{
        	   				mSVDraw.clearDraw();
        	   				m_bFocus = true;
        	   				mCamera.autoFocus(mAutoFocusCallback);
        	   			}
        	   			try 
        	   			{
        	   				Thread.sleep(3000);
        	   			} 
        	   			catch (InterruptedException e) 
        	   			{
        	   				// TODO Auto-generated catch block
        	   				e.printStackTrace();
        	   			}
        	   		}
        	   	}
           };
            
           setJNIEnv();
	}


	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		// TODO Auto-generated method stub
		//Log.i("jefry", "w="+width+"  h="+height);
		//Log.i("jefry", "1111111111111111111111111111111111111111111111111");	
		//initCamera();
		
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO Auto-generated method stub
        try {
            mCamera = null;
            try {
                mCamera = Camera.open();//Camera.open(0);//打开相机；在低版本里，只有open（）方法；高级版本加入此方法的意义是具有打开多个
                //摄像机的能力，其中输入参数为摄像机的编号
                //在manifest中设定的最小版本会影响这里方法的调用，如果最小版本设定有误（版本过低），在ide里将不允许调用有参的
                //open方法;
                //如果模拟器版本较高的话，无参的open方法将会获得null值!所以尽量使用通用版本的模拟器和API；
            } catch (Exception e) {
                Log.e("============", "摄像头被占用");
            }
            if (mCamera == null) {
                Log.e("============", "摄像机为空");
                System.exit(0);
            }
            mCamera.setPreviewDisplay(holder);//设置显示面板控制器
            if(this.getResources().getConfiguration().orientation != Configuration.ORIENTATION_LANDSCAPE)
            	mCamera.setDisplayOrientation(90);
            else
            	mCamera.setDisplayOrientation(0);  
            android.hardware.Camera.Parameters params = mCamera.getParameters();
            m_iMaxZoom = params.getMaxZoom();

            //params.setPreviewFormat(ImageFormat.JPEG);
            params.setPreviewFormat(ImageFormat.NV21);  
            //params.setPictureSize(480,640);
            params.setPreviewSize(320, 240);
            
            /*List<String> focusModes = params.getSupportedFocusModes();  
            if(focusModes.contains("continuous-video")){  
            	params.setFocusMode(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO);  
            }*/
            mCamera.setParameters(params);
            priviewCallBack pre = new priviewCallBack();//建立预览回调对象
            mCamera.setPreviewCallback(pre); //设置预览回调对象
            mCamera.startPreview();//开始预览，这步操作很重要
            //mCamera.autoFocus(mAutoFocusCallback);
            m_setFocusThread.start();
        } catch (IOException exception) {
            mCamera.release();
            mCamera = null;
        }

        // 不添加显示面板的代码：
        /*
         * 打开相机， mCamera = null; try { mCamera = Camera.open(0); } catch
         * (Exception e) { Log.e("============", "摄像头被占用"); } if (mCamera ==
         * null) { Log.e("============", "返回结果为空"); System.exit(0); } //
         * mCamera.setPreviewDisplay(holder); priviewCallBack pre = new
         * priviewCallBack(); mCamera.setPreviewCallback(pre); Log.w("wwwwwwww",
         * mCamera.getParameters().getPreviewFormat() + "");
         * mCamera.startPreview();
         */
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		Log.i("jefry", "2222222222222222222222222222222222222222222222222222");
        stopCamera();
      //camera.stopPreview();
        mCamera.setPreviewCallback(null) ;
        mCamera.release();
        mCamera = null;
	}
	/* 拍照的method */
    private void takePicture() {
        if (mCamera != null) {
            mCamera.takePicture(shutterCallback, rawCallback, jpegCallback);
        }
    }

    private ShutterCallback shutterCallback = new ShutterCallback() {
        public void onShutter() {
            /* 按下快门瞬间会调用这里的程序 */
        	Log.w("jefry", "shutterCallback");
        }
    };

    private PictureCallback rawCallback = new PictureCallback() {
        public void onPictureTaken(byte[] _data, Camera _camera) {
            /* 要处理raw data?写?否 */
        	Log.w("============", "rawCallback");
        }
    };

    //在takepicture中调用的回调方法之一，接收jpeg格式的图像
    private PictureCallback jpegCallback = new PictureCallback() {
        public void onPictureTaken(byte[] _data, Camera _camera) {

            /*
             * if (Environment.getExternalStorageState().equals(
             * Environment.MEDIA_MOUNTED)) // 判断SD卡是否存在，并且可以可以读写 {
             * 
             * } else { Toast.makeText(EX07_16.this, "SD卡不存在或写保护",
             * Toast.LENGTH_LONG) .show(); }
             */
            // Log.w("============", _data[55] + "");
        	Log.w("============", strCaptureFilePath + "/1.jpg");
            try {
                /* 取得相片 */
                Bitmap bm = BitmapFactory.decodeByteArray(_data, 0,
                        _data.length);

                /* 创建文件 */
                File myCaptureFile = new File(strCaptureFilePath, "1.jpg");
                BufferedOutputStream bos = new BufferedOutputStream(
                        new FileOutputStream(myCaptureFile));
                /* 采用压缩转档方法 */
                bm.compress(Bitmap.CompressFormat.JPEG, 100, bos);

                /* 调用flush()方法，更新BufferStream */
                bos.flush();

                /* 结束OutputStream */
                bos.close();

                /* 让相片显示3秒后圳重设相机 */
                // Thread.sleep(2000);
                /* 重新设定Camera */
                stopCamera();
                initCamera();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    };

    /* 自定义class AutoFocusCallback */
    public final class MyAutoFocusCallback implements
            android.hardware.Camera.AutoFocusCallback {
        public void onAutoFocus(boolean focused, Camera camera) {
        	Log.w("jefry","111111111111111");
            /* 对到焦点拍照 */
            if (focused && m_bFocus) {
            	Log.w("2222222222222","22222222222222");
                //takePicture();
            	Size size = mCamera.getParameters().getPreviewSize(); //获取预览大小
            	synchronized(m_strLock)
            	{
            		Log.i("jefry", "w="+size.width+"  h="+size.height);
            		String strOP;
            		if(m_bTakingPicture)
            			strOP = "true";
            		else
            			strOP = "false";
            		String str = getStringNumber(size.width,size.height,mBuffer,strOP/*"/storage/sdcard1/carnumber/65_car/65_car"*/);
            		String[] s = str.split(",");
            		int iSum = Integer.parseInt(s[0]);
            		if(iSum > 0 && s.length > 4)
            		{
            			Rect r = new Rect(Integer.parseInt(s[1])*2,Integer.parseInt(s[2])*2,Integer.parseInt(s[3])*2,Integer.parseInt(s[4])*2);
            			mSVDraw.drawRect(r);
            		}
            		else
            			mSVDraw.clearDraw();
            		Log.i("result", str);	
            		TextView v = (TextView)findViewById(R.id.textView1);
            		v.setText(str);
            		if(m_bTakingPicture)
            		{
            			m_bTakingPicture = false;
            			FileInputStream fis =null;
            			try {
            				fis = new FileInputStream("/storage/emulated/0/data/morph.jpg");
            			} 
            			catch (FileNotFoundException e) 
            			{
            				// TODO Auto-generated catch block
            				e.printStackTrace();
            			}
            			Bitmap bm=BitmapFactory.decodeStream(fis);
            			ImageView img = (ImageView)findViewById(R.id.send_image);
            			img.setImageBitmap(bm);		
            		}
            	}
            	
            }
            m_bFocus = false;
        }
    };

    /* 相机初始化的method */
    private void initCamera() {
        if (mCamera != null) {
            try {
                Camera.Parameters parameters = mCamera.getParameters();
                /*
                 * 设定相片大小为1024*768， 格式为JPG
                 */
                // parameters.setPictureFormat(PixelFormat.JPEG);
                //parameters.setPictureSize(1024, 768);
                mCamera.setParameters(parameters);
                /* 打开预览画面 */
                mCamera.startPreview();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /* 停止相机的method */
    private void stopCamera() {
        if (mCamera != null) {
            try {
                /* 停止预览 */
                mCamera.stopPreview();
                
                
                
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    // 检测摄像头是否存在的私有方法
    private boolean checkCameraHardware(Context context) {
        if (context.getPackageManager().hasSystemFeature(
                PackageManager.FEATURE_CAMERA)) {
            // 摄像头存在
            return true;
        } else {
            // 摄像头不存在
            return false;
        }
    }

    // 每次cam采集到新图像时调用的回调方法，前提是必须开启预览
    class priviewCallBack implements Camera.PreviewCallback {

        @Override
        public void onPreviewFrame(byte[] data, Camera camera) {
            // TODO Auto-generated method stub
            // Log.w("wwwwwwwww", data[5] + "");
            // Log.w("支持格式", mCamera.getParameters().getPreviewFormat()+"");
            //decodeToBitMap(data, camera);
        	synchronized(m_strLock)
        	{
        		mCurrBufferLen = data.length;
        		System.arraycopy(data, 0, mBuffer, 0,mCurrBufferLen);
        	}
        }
    }

    public void decodeToBitMap(byte[] data, Camera _camera) {
        Size size = mCamera.getParameters().getPreviewSize();
        try {
            YuvImage image = new YuvImage(data, ImageFormat.NV21, size.width,
                    size.height, null);
            Log.w("wwwwwwwww", size.width + " " + size.height);
            if (image != null) {
                ByteArrayOutputStream stream = new ByteArrayOutputStream();
                image.compressToJpeg(new Rect(0, 0, size.width, size.height),
                        80, stream);
                Bitmap bmp = BitmapFactory.decodeByteArray(
                        stream.toByteArray(), 0, stream.size());
                Log.w("wwwwwwwww", bmp.getWidth() + " " + bmp.getHeight());
                Log.w("wwwwwwwww",
                        (bmp.getPixel(100, 100) & 0xff) + "  "
                                + ((bmp.getPixel(100, 100) >> 8) & 0xff) + "  "
                                + ((bmp.getPixel(100, 100) >> 16) & 0xff));

                stream.close();
            }
        } catch (Exception ex) {
            Log.e("Sys", "Error:" + ex.getMessage());
        }
    }
	
	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

    public void drawImage(byte[] imageBuff,int iTotalLen,int iFlag)
    {
            System.out.println("1111111111len:"+iTotalLen + " len2:" + imageBuff.length);
            Bitmap bm = null;
            try
            {
                bm = BitmapFactory.decodeByteArray(imageBuff, 0, iTotalLen);
            }
            catch (java.lang.ArrayIndexOutOfBoundsException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
            }
            if(bm == null)
            {
            	System.out.println("1111111111l not nullnull");
            	return;
            }         
            switch(iFlag)
            {
            case 0:
            	ImageView v = (ImageView)findViewById(R.id.send_image);
            	v.setImageBitmap(bm);
            	
            	break;
            case 1:
            	break;
            case 2:
            	break;
            default:
            	break;
            }
    }
	
	private native String getStringNumber(int w,int h,byte[] yuv,String strTemplatePath);
	private native void setJNIEnv();
	static
	{
		System.loadLibrary("MarkingImg");
	}
	
	

}
