package com.teng.carn;

import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.graphics.Paint.Style;
import android.graphics.PixelFormat;
import android.graphics.PorterDuff.Mode;
import android.graphics.PorterDuffXfermode;
import android.graphics.Rect;
import android.util.AttributeSet;
import android.util.Log;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;

public class SVDraw extends SurfaceView implements Callback {

	protected SurfaceHolder sh;  
	private int mWidth;  
	private int mHeight; 

	public SVDraw(Context context, AttributeSet attrs) {
		super(context, attrs);
		// TODO Auto-generated constructor stub  
		sh = getHolder();  
		sh.addCallback(this);  
		sh.setFormat(PixelFormat.TRANSPARENT);  
		setZOrderOnTop(true); 
	}

	@Override
	public void surfaceChanged(SurfaceHolder arg0, int arg1, int w, int h) {
		// TODO Auto-generated method stub
		mWidth = w;  
		mHeight = h;
		 Log.i("jefry","w="+mWidth+",h="+mHeight);
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		// TODO Auto-generated method stub

	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		// TODO Auto-generated method stub
		sh = null;
	}
	
	void clearDraw()                             
	{       
		if(sh == null) return;
	    Canvas canvas = sh.lockCanvas();
	    canvas.drawColor(Color.TRANSPARENT);
	    Paint p = new Paint();
	    p.setXfermode(new PorterDuffXfermode(Mode.CLEAR));
        canvas.drawPaint(p);
        p.setXfermode(new PorterDuffXfermode(Mode.SRC));
	    sh.unlockCanvasAndPost(canvas);          
	}  
	public void drawRect(Rect r)         
	{
	    Canvas canvas = sh.lockCanvas();
	    canvas.drawColor(Color.TRANSPARENT);     
	    Paint p = new Paint();
	    
	    p.setXfermode(new PorterDuffXfermode(Mode.CLEAR));
        canvas.drawPaint(p);
        p.setXfermode(new PorterDuffXfermode(Mode.SRC));

	    p.setAntiAlias(true);                    
	    p.setColor(Color.WHITE);                   
	    p.setStyle(Style.STROKE); 
	    canvas.drawRect(r, p);
	    sh.unlockCanvasAndPost(canvas);
	}


}
