/*--------------------------- demo_ASIFT  -------------------------*/
// Detect corresponding points in two images with the ASIFT method. 
// Copyright, Jean-Michel Morel, Guoshen Yu, 2008. 
// Please report bugs and/or send comments to Guoshen Yu yu@cmap.polytechnique.fr
// 2010.08.12
// 
// Reference: J.M. Morel and G.Yu, ASIFT: A New Framework for Fully Affine Invariant Image 
//            Comparison, SIAM Journal on Imaging Sciences, vol. 2, issue 2, pp. 438-469, 2009. 
// Reference: ASIFT online demo (You can try ASIFT with your own images online.) 
//			  http://www.ipol.im/pub/algo/my_affine_sift/
/*---------------------------------------------------------------------------*/
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <vector>
#include "demo_lib_sift.h"
#include "library.h"
#include "frot.h"
#include "fproj.h"
#include "compute_asift_keypoints.h"
#include "compute_asift_matches.h"
using namespace std;

#ifdef _OPENMP
#include <omp.h>
#endif

static int IM_X = 320;
static int IM_Y = 240;
static vector<vector<keypointslist> > keys1;
std::vector<float> ipixels1;
static int wS1, hS1, w1, h1, wo, ho;
int num_matching_pts;

vector<vector<keypointslist> > LoadKeyPoints(std::vector<float> ipixels, int w,
		int h, int num_of_tilts, int * out_WS, int *out_HS);
matchingslist MatchKeyPoints(int num_of_tilts, int wS1, int hS1, int wS2,
		int hS2, vector<vector<keypointslist> > keys1,
		vector<vector<keypointslist> > keys2);
void PixelToVector(jint *cbuf, int w, int h, std::vector<float> *ipixels);

extern "C" {
JNIEXPORT void JNICALL Java_com_testASIFT_LibASIFT_initZoomSize( JNIEnv* env,jobject obj,jint w,jint h);
JNIEXPORT jintArray JNICALL Java_com_testASIFT_LibASIFT_GetZoomSize(JNIEnv* env,
		jobject obj);
JNIEXPORT void JNICALL Java_com_testASIFT_LibASIFT_initImage1(JNIEnv* env,
		jobject obj, jintArray iarr, int w, int h, int num_of_tilts);
JNIEXPORT jint JNICALL Java_com_testASIFT_LibASIFT_Match2ImageForNum(
		JNIEnv* env, jobject obj, jintArray iarr, int w2, int h2,
		int num_of_tilts2);
JNIEXPORT jintArray JNICALL Java_com_testASIFT_LibASIFT_GetMatchedImageSize(
		JNIEnv* env, jobject obj);
JNIEXPORT jintArray JNICALL Java_com_testASIFT_LibASIFT_Match2ImageForImg(
		JNIEnv* env, jobject obj, jintArray iarr, int w2, int h2,
		int num_of_tilts2);
JNIEXPORT jint JNICALL Java_com_testASIFT_LibASIFT_getmatchingPoints(
		JNIEnv* env, jobject obj);

}
;

/**
 * 取得放大/缩小之后的图像大小
 */
JNIEXPORT jintArray JNICALL Java_com_testASIFT_LibASIFT_GetZoomSize(JNIEnv* env,
		jobject obj) {
	jint arrint[2];
	arrint[0] = IM_X;
	arrint[1] = IM_Y;
	jintArray result = env->NewIntArray(2);
	env->SetIntArrayRegion(result, 0, 2, arrint);
	return result;
}

/**
 * 初始化放大/缩小之后的图像大小
 */
JNIEXPORT void JNICALL Java_com_testASIFT_LibASIFT_initZoomSize(JNIEnv* env,jobject obj,jint w,jint h)
{
	IM_X=w;
	IM_Y=h;
}

/**
 * 计算出图像1的特征点集合，并保存在静态变量
 */
JNIEXPORT void JNICALL Java_com_testASIFT_LibASIFT_initImage1(JNIEnv* env,
		jobject obj, jintArray iarr, int w, int h, int num_of_tilts) {
	jint *cbuf;
	cbuf = env->GetIntArrayElements(iarr, false);
	ipixels1.clear();
	PixelToVector(cbuf, w, h, &ipixels1);
	env->ReleaseIntArrayElements(iarr, cbuf, 0);

	w1 = w;
	h1 = h;
	keys1 = LoadKeyPoints(ipixels1, w, h, num_of_tilts, &wS1, &hS1);
}

/**
 * 计算出图象2的特征点集合，并与图像1的特征点集合做匹配，返回类似特征的数量
 */
JNIEXPORT jint JNICALL Java_com_testASIFT_LibASIFT_Match2ImageForNum(
		JNIEnv* env, jobject obj, jintArray iarr, int w2, int h2,
		int num_of_tilts2) {
	int wS2, hS2;
	std::vector<float> ipixels2;
	//转化为灰度图
	jint *cbuf;
	cbuf = env->GetIntArrayElements(iarr, false);
	PixelToVector(cbuf, w2, h2, &ipixels2);
	env->ReleaseIntArrayElements(iarr, cbuf, 0);

	//取得图片的特征点集合
	vector < vector<keypointslist> > keys2 = LoadKeyPoints(ipixels2, w2, h2,
			num_of_tilts2, &wS2, &hS2);

	//匹配两张图片的特征点
	matchingslist matchings = MatchKeyPoints(num_of_tilts2, wS1, hS1, wS2, hS2,
			keys1, keys2);
	return matchings.size();
}

JNIEXPORT jintArray JNICALL Java_com_testASIFT_LibASIFT_GetMatchingPoints(
		JNIEnv* env, jobject obj) {
	jint arrint[2];
	arrint[0] = num_matching_pts;
	arrint[1] = 0;
	jintArray result = env->NewIntArray(2);
	env->SetIntArrayRegion(result, 0, 2, arrint);
	return result;
}

/**
 * 返回匹配后图像的大小 jintArray[0]为width， jintArray[1]为height
 */
JNIEXPORT jintArray JNICALL Java_com_testASIFT_LibASIFT_GetMatchedImageSize(
		JNIEnv* env, jobject obj) {
	jint arrint[2];
	arrint[0] = wo;
	arrint[1] = ho;
	jintArray result = env->NewIntArray(2);
	env->SetIntArrayRegion(result, 0, 2, arrint);
	return result;
}

/**
 *计算出图象2的特征点集合，并与图像1的特征点集合做匹配，返回两图对比之后的图像像素数组
 */
JNIEXPORT jintArray JNICALL Java_com_testASIFT_LibASIFT_Match2ImageForImg(
		JNIEnv* env, jobject obj, jintArray iarr, int w2, int h2,
		int num_of_tilts2) {
	int wS2, hS2;
	std::vector<float> ipixels2;
	//转化为灰度图
	jint *cbuf;
	cbuf = env->GetIntArrayElements(iarr, false);
	PixelToVector(cbuf, w2, h2, &ipixels2);
	env->ReleaseIntArrayElements(iarr, cbuf, 0);

	//取得图片的特征点集合
	vector < vector<keypointslist> > keys2 = LoadKeyPoints(ipixels2, w2, h2,
			num_of_tilts2, &wS2, &hS2);

	//匹配两张图片的特征点
	matchingslist matchings = MatchKeyPoints(num_of_tilts2, wS1, hS1, wS2, hS2,
			keys1, keys2);

	///////////////// Output image containing line matches (the two images are concatenated one above the other)
	int band_w = 10; // insert a black band of width band_w between the two images for better visibility

	wo = MAX(w1, w2);
	ho = h1 + h2 + band_w;

	float areaS = IM_X * IM_Y;
	float zoom1 = sqrt((w1 * h1) / areaS);
	float zoom2 = sqrt((w2 * h2) / areaS);
	float *opixelsASIFT = new float[wo * ho];

	for (int j = 0; j < (int) ho; j++)
		for (int i = 0; i < (int) wo; i++)
			opixelsASIFT[j * wo + i] = 255;

	/////////////////////////////////////////////////////////////////// Copy both images to output
	for (int j = 0; j < (int) h1; j++)
		for (int i = 0; i < (int) w1; i++)
			opixelsASIFT[j * wo + i] = ipixels1[j * w1 + i];

	for (int j = 0; j < (int) h2; j++)
		for (int i = 0; i < (int) (int) w2; i++)
			opixelsASIFT[(h1 + band_w + j) * wo + i] = ipixels2[j * w2 + i];

	//////////////////////////////////////////////////////////////////// Draw matches
	matchingslist::iterator ptr = matchings.begin();
	for (int i = 0; i < (int) matchings.size(); i++, ptr++) {
		draw_line(opixelsASIFT, (int) (zoom1 * ptr->first.x),
				(int) (zoom1 * ptr->first.y), (int) (zoom2 * ptr->second.x),
				(int) (zoom2 * ptr->second.y) + h1 + band_w, 255.0f, wo, ho);
	}

	jintArray result = env->NewIntArray(wo * ho);
	jint *cResult;
	cResult = env->GetIntArrayElements(result, false);
	int alpha = 0xFF << 24;
	for (int i = 0; i < ho; i++) {
		for (int j = 0; j < wo; j++) {
			// 获得像素的颜色
			int color = (int) opixelsASIFT[wo * i + j];
			color = alpha | (color << 16) | (color << 8) | color;
			cResult[wo * i + j] = color;
		}
	}

	env->ReleaseIntArrayElements(result, cResult, 0);
	return result;
}

void PixelToVector(jint *cbuf, int w, int h, std::vector<float> *ipixels) {
	for (int i = 0; i < h; i++) {
		for (int j = 0; j < w; j++) {
			// 获得像素的颜色
			int color = cbuf[w * i + j];
			int red = ((color & 0x00FF0000) >> 16);
			int green = ((color & 0x0000FF00) >> 8);
			int blue = color & 0x000000FF;
			color = (red + green + blue) / 3;
			ipixels->push_back(color); //保存灰度值
		}
	}
}

matchingslist MatchKeyPoints(int num_of_tilts, int wS1, int hS1, int wS2,
		int hS2, vector<vector<keypointslist> > keys1,
		vector<vector<keypointslist> > keys2) {
	//// Match ASIFT keypoints
	int num_matchings;
	matchingslist matchings;

	siftPar siftparameters;
	default_sift_parameters(siftparameters);

	cout << "Matching the keypoints..." << endl;

#ifdef _WIN32_WCE
	/////////////////
#else
	time_t tstart, tend;
	tstart = time(0);
#endif /* _WIN32_WCE */

	num_matchings = compute_asift_matches(num_of_tilts, num_of_tilts, wS1, hS1,
			wS2, hS2, 0, keys1, keys2, matchings, siftparameters);
	num_matching_pts = num_matchings;
#ifdef _WIN32_WCE
	//////////
#else
	tend = time(0);
	cout << " Keypoints matching accomplished in " << difftime(tend, tstart)
			<< " seconds." << endl;
#endif /* _WIN32_WCE */
	return matchings;
}

int getmatchingPoints() {
	return num_matching_pts;
}

vector<vector<keypointslist> > LoadKeyPoints(std::vector<float> ipixels, int w,
		int h, int num_of_tilts, int * out_WS, int *out_HS) {
	///// Resize the images to area IM_X*hW in remaining the apsect-ratio	
	///// Resize if the resize flag is not set or if the flag is set unequal to 0
	float zoom = 0;
	int wS = 0, hS = 0;
	vector<float> ipixels_zoom;

	float InitSigma_aa = 1.6;

	float fproj_p, fproj_bg;
	char fproj_i;
	float *fproj_x4, *fproj_y4;
	int fproj_o;

	fproj_o = 3;
	fproj_p = 0;
	fproj_i = 0;
	fproj_bg = 0;
	fproj_x4 = 0;
	fproj_y4 = 0;

	float areaS = IM_X * IM_Y;

	// Resize image 1 
	float area = w * h;
	zoom = sqrt(area / areaS);

	wS = (int) (w / zoom);
	hS = (int) (h / zoom);

	int fproj_sx = wS;
	int fproj_sy = hS;

	float fproj_x1 = 0;
	float fproj_y1 = 0;
	float fproj_x2 = wS;
	float fproj_y2 = 0;
	float fproj_x3 = 0;
	float fproj_y3 = hS;

	/* Anti-aliasing filtering along vertical direction */
	if (zoom > 1) {
		float sigma_aa = InitSigma_aa * zoom / 2;
		GaussianBlur1D(ipixels, w, h, sigma_aa, 1);
		GaussianBlur1D(ipixels, w, h, sigma_aa, 0);
	}

	// simulate a tilt: subsample the image along the vertical axis by a factor of t.
	ipixels_zoom.resize(wS * hS);
	fproj(ipixels, ipixels_zoom, w, h, &fproj_sx, &fproj_sy, &fproj_bg,
			&fproj_o, &fproj_p, &fproj_i, fproj_x1, fproj_y1, fproj_x2,
			fproj_y2, fproj_x3, fproj_y3, fproj_x4, fproj_y4);

	///// Compute ASIFT keypoints
	// Define the SIFT parameters
	siftPar siftparameters;
	default_sift_parameters(siftparameters);

	vector < vector<keypointslist> > keys;

	int num_keys = 0;

	cout << "Computing keypoints on the two images..." << endl;
#ifdef _WIN32_WCE
	//////////
#else
	time_t tstart, tend;
	tstart = time(0);
#endif /* _WIN32_WCE */

	num_keys = compute_asift_keypoints(ipixels_zoom, wS, hS, num_of_tilts, 0,
			keys, siftparameters);

#ifdef _WIN32_WCE
	//////////
#else
	tend = time(0);
	cout << "Keypoints computation accomplished in " << difftime(tend, tstart)
			<< " seconds." << endl;
#endif /* _WIN32_WCE */
	*out_WS = wS;
	*out_HS = hS;

	return keys;
}

