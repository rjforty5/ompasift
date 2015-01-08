package com.testASIFT;

import android.app.Activity;
import android.graphics.Bitmap;
import android.graphics.Bitmap.Config;
import android.graphics.drawable.BitmapDrawable;
import android.os.Bundle;
import android.util.Log;
import android.widget.ImageView;
import android.widget.TextView;

public class testASIFT extends Activity {
    /** Called when the activity is first created. */
	ImageView imgView;
	
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        this.setTitle("Android…œ π”√ASIFT---hellogv");
        imgView=(ImageView)this.findViewById(R.id.ImageView01);
        
        LibASIFT.initZoomSize(320, 480); // scaling the target's size  
        int []size=LibASIFT.GetZoomSize();// determine whether the setting is successful  
        Log.e(String.valueOf(size[0]),String.valueOf(size[1]));
        
        Bitmap img1=((BitmapDrawable) getResources().getDrawable(R.drawable.adam1)).getBitmap();
        int w1=img1.getWidth(),h1=img1.getHeight();
        int[] pix1 = new int[w1 * h1];
        img1.getPixels(pix1, 0, w1, 0, 0, w1, h1);

     // Extract the first picture of the feature points  
        LibASIFT.initImage1(pix1, w1, h1, 2);

        Bitmap img2=((BitmapDrawable) getResources().getDrawable(R.drawable.adam5)).getBitmap();
        int w2=img2.getWidth(),h2=img2.getHeight();
        int[] pix2 = new int[w2 * h2];
        img2.getPixels(pix2, 0, w2, 0, 0, w2, h2);
        int[] imgPixels=LibASIFT.Match2ImageForImg(pix2, w2, h2, 2);// two graph matching  
        int[] imgSize=LibASIFT.GetMatchedImageSize(); // matches the size of the result
        int[] numMatch = LibASIFT.GetMatchingPoints();
        
        Log.e("NUM_MATCH", numMatch[0]+"");
        
        for(int i=0;i < imgSize.length; i++){
        	Log.e("Matched Size", imgSize[i] + "");
        }
        

        Bitmap imgResult=Bitmap.createBitmap(imgSize[0], imgSize[1], Config.RGB_565);
        imgResult.setPixels(imgPixels, 0, imgResult.getWidth(), 0, 0, imgResult.getWidth(), imgResult.getHeight());
        imgView.setImageBitmap(imgResult); // display the results
        
        
    }
  
}