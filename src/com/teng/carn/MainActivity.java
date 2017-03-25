package com.teng.carn;

import java.io.BufferedOutputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;

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

    private Camera mCamera;// Camera����
    private Button mButton;// �Ҳ����򣬵����������ͼ�����գ����¼�
    private SurfaceView mSurfaceView;// ��ʾͼ���surfaceView
    private SurfaceHolder holder;// SurfaceView�Ŀ�����
    private MyAutoFocusCallback mAutoFocusCallback = new MyAutoFocusCallback();// AutoFocusCallback�Զ��Խ��Ļص�����
    private ImageView sendImageIv;// ����ͼƬ��imageview��λ���Ҳ�����

    private String strCaptureFilePath = Environment
            .getExternalStorageDirectory() + "/DCIM/Camera/";// ����ͼ���·��
    
    private byte[] mBuffer = new byte[3224000];
    private int mCurrBufferLen = 0;
    private String m_strLock = "lock";
    
    private int m_iMaxZoom = 0;
    private int m_iZoom = 0;
    private boolean m_bTakingPicture = false; 
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		
	       if (checkCameraHardware(this)) {
	            Log.e("============", "����ͷ����");// ��֤����ͷ�Ƿ����
	        }
	        /* ����״̬�� */
	        this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
	                WindowManager.LayoutParams.FLAG_FULLSCREEN);
	        /* ���ر����� */
	        requestWindowFeature(Window.FEATURE_NO_TITLE);
	         
	        /* �趨��Ļ��ʾΪ���� */
	        // this.setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
	        setContentView(R.layout.activity_main);
	        
	        getWindow().setFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);		
			//getWindow().addFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN);
			
			getWindow().setSoftInputMode(
	                WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_HIDDEN);
	        
	        /* SurfaceHolder���� */
	        mSurfaceView = (SurfaceView) findViewById(R.id.mSurfaceView);
	        holder = mSurfaceView.getHolder();
	        holder.addCallback(this);
	        // holder.setType(SurfaceHolder.SURFACE_TYPE_PUSH_BUFFERS);
	        /* ��������Button��OnClick�¼����� */

	        mButton = (Button) findViewById(R.id.myButton);
	        mButton.setOnClickListener(new OnClickListener() {
	            @Override
	            public void onClick(View arg0) {
	                /* �Զ��Խ������� */
	                //mCamera.autoFocus(mAutoFocusCallback);// ����mCamera��
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
                mCamera = Camera.open();//Camera.open(0);//��������ڵͰ汾�ֻ��open�����������߼��汾����˷����������Ǿ��д򿪶��
                //������������������������Ϊ������ı��
                //��manifest���趨����С�汾��Ӱ�����﷽���ĵ��ã������С�汾�趨���󣨰汾���ͣ�����ide�ｫ�����������вε�
                //open����;
                //���ģ�����汾�ϸߵĻ����޲ε�open����������nullֵ!���Ծ���ʹ��ͨ�ð汾��ģ������API��
            } catch (Exception e) {
                Log.e("============", "����ͷ��ռ��");
            }
            if (mCamera == null) {
                Log.e("============", "�����Ϊ��");
                System.exit(0);
            }
            mCamera.setPreviewDisplay(holder);//������ʾ��������
            if(this.getResources().getConfiguration().orientation != Configuration.ORIENTATION_LANDSCAPE)
            	mCamera.setDisplayOrientation(90);
            else
            	mCamera.setDisplayOrientation(0);  
            android.hardware.Camera.Parameters params = mCamera.getParameters();
            m_iMaxZoom = params.getMaxZoom();

            //params.setPreviewFormat(ImageFormat.JPEG);
            params.setPreviewFormat(ImageFormat.NV21);  
            //params.setPreviewSize(320, 240);
            mCamera.setParameters(params);
            priviewCallBack pre = new priviewCallBack();//����Ԥ���ص�����
            mCamera.setPreviewCallback(pre); //����Ԥ���ص�����
            mCamera.startPreview();//��ʼԤ�����ⲽ��������Ҫ
   mCamera.autoFocus(mAutoFocusCallback);
        } catch (IOException exception) {
            mCamera.release();
            mCamera = null;
        }

        // ��������ʾ���Ĵ��룺
        /*
         * ������� mCamera = null; try { mCamera = Camera.open(0); } catch
         * (Exception e) { Log.e("============", "����ͷ��ռ��"); } if (mCamera ==
         * null) { Log.e("============", "���ؽ��Ϊ��"); System.exit(0); } //
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
	
	
	  /* ���յ�method */
    private void takePicture() {
        if (mCamera != null) {
            mCamera.takePicture(shutterCallback, rawCallback, jpegCallback);
        }
    }

    private ShutterCallback shutterCallback = new ShutterCallback() {
        public void onShutter() {
            /* ���¿���˲����������ĳ��� */
        	Log.w("============", "shutterCallback");
        }
    };

    private PictureCallback rawCallback = new PictureCallback() {
        public void onPictureTaken(byte[] _data, Camera _camera) {
            /* Ҫ����raw data?д?�� */
        	Log.w("============", "rawCallback");
        }
    };

    //��takepicture�е��õĻص�����֮һ������jpeg��ʽ��ͼ��
    private PictureCallback jpegCallback = new PictureCallback() {
        public void onPictureTaken(byte[] _data, Camera _camera) {

            /*
             * if (Environment.getExternalStorageState().equals(
             * Environment.MEDIA_MOUNTED)) // �ж�SD���Ƿ���ڣ����ҿ��Կ��Զ�д {
             * 
             * } else { Toast.makeText(EX07_16.this, "SD�������ڻ�д����",
             * Toast.LENGTH_LONG) .show(); }
             */
            // Log.w("============", _data[55] + "");
        	Log.w("============", strCaptureFilePath + "/1.jpg");
            try {
                /* ȡ����Ƭ */
                Bitmap bm = BitmapFactory.decodeByteArray(_data, 0,
                        _data.length);

                /* �����ļ� */
                File myCaptureFile = new File(strCaptureFilePath, "1.jpg");
                BufferedOutputStream bos = new BufferedOutputStream(
                        new FileOutputStream(myCaptureFile));
                /* ����ѹ��ת������ */
                bm.compress(Bitmap.CompressFormat.JPEG, 100, bos);

                /* ����flush()����������BufferStream */
                bos.flush();

                /* ����OutputStream */
                bos.close();

                /* ����Ƭ��ʾ3������������ */
                // Thread.sleep(2000);
                /* �����趨Camera */
                stopCamera();
                initCamera();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    };

    /* �Զ���class AutoFocusCallback */
    public final class MyAutoFocusCallback implements
            android.hardware.Camera.AutoFocusCallback {
        public void onAutoFocus(boolean focused, Camera camera) {
        	Log.w("11111111111111","111111111111111");
            /* �Ե��������� */
            if (focused && m_bTakingPicture) {
            	Log.w("2222222222222","22222222222222");
                //takePicture();
            	Size size = mCamera.getParameters().getPreviewSize(); //��ȡԤ����С
            	synchronized(m_strLock)
            	{
            		Log.i("jefry", "w="+size.width+"  h="+size.height);
            		String str = getStringNumber(size.width,size.height,mBuffer,"/storage/sdcard1/carnumber/65_car/65_car");
            		Log.i("result", str);	
            		TextView v = (TextView)findViewById(R.id.textView1);
            		v.setText(str);
            		m_bTakingPicture = false;
            		
            		FileInputStream fis =null;
					try {
						fis = new FileInputStream("/storage/emulated/0/data/morph.jpg");
					} catch (FileNotFoundException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
            		Bitmap bm=BitmapFactory.decodeStream(fis);
            		ImageView img = (ImageView)findViewById(R.id.send_image);
            		img.setImageBitmap(bm);		
            		
            	}
            	
            }
        }
    };

    /* �����ʼ����method */
    private void initCamera() {
        if (mCamera != null) {
            try {
                Camera.Parameters parameters = mCamera.getParameters();
                /*
                 * �趨��Ƭ��СΪ1024*768�� ��ʽΪJPG
                 */
                // parameters.setPictureFormat(PixelFormat.JPEG);
                //parameters.setPictureSize(1024, 768);
                mCamera.setParameters(parameters);
                /* ��Ԥ������ */
                mCamera.startPreview();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    /* ֹͣ�����method */
    private void stopCamera() {
        if (mCamera != null) {
            try {
                /* ֹͣԤ�� */
                mCamera.stopPreview();
                
                
                
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    // �������ͷ�Ƿ���ڵ�˽�з���
    private boolean checkCameraHardware(Context context) {
        if (context.getPackageManager().hasSystemFeature(
                PackageManager.FEATURE_CAMERA)) {
            // ����ͷ����
            return true;
        } else {
            // ����ͷ������
            return false;
        }
    }

    // ÿ��cam�ɼ�����ͼ��ʱ���õĻص�������ǰ���Ǳ��뿪��Ԥ��
    class priviewCallBack implements Camera.PreviewCallback {

        @Override
        public void onPreviewFrame(byte[] data, Camera camera) {
            // TODO Auto-generated method stub
            // Log.w("wwwwwwwww", data[5] + "");
            // Log.w("֧�ָ�ʽ", mCamera.getParameters().getPreviewFormat()+"");
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