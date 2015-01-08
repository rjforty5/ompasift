package com.testASIFT;

public class LibASIFT {
	static {
        System.loadLibrary("ASIFT");
    }

   /**
    * @param width the current view width
    * @param height the current view height
    */
    public static native void initZoomSize(int width, int height);
    public static native int[] GetZoomSize();
    public static native void initImage1(int[] iarr, int w, int h, int num_of_tilts);
    public static native int Match2ImageForNum(int[] iarr, int w2, int h2,int num_of_tilts2);
    public static native float GetPixel(int index);
    public static native int[] GetMatchedImageSize();
    public static native  int[] Match2ImageForImg(int[] iarr, int w2, int h2,int num_of_tilts2);
    public static native int[] GetMatchingPoints();
   
}
