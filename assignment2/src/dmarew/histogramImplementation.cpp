/* 
  Example use of openCV to generate a histogram of an image
  ---------------------------------------------------------
  Implementation file

  David Vernon
  19 May 2017

*/
 
#include "defectiveSweetsCounter.h"
 
/************************************************************************************************************/

/*
 * This code is provided as part of "A Practical Introduction to Computer Vision with OpenCV"
 * by Kenneth Dawson-Howe � Wiley & Sons Inc. 2014.  All rights reserved.
 */

//#include "Utilities.h"

class Histogram
{
protected:
	Mat mImage;
	int mNumberChannels;
	int* mChannelNumbers;
	int* mNumberBins;
	float mChannelRange[2];
public:
	Histogram( Mat image, int number_of_bins )
	{
		mImage = image;
		mNumberChannels = mImage.channels();
		mChannelNumbers = new int[mNumberChannels];
		mNumberBins = new int[mNumberChannels];
		mChannelRange[0] = 0.0;
		mChannelRange[1] = 255.0;
		for (int count=0; count<mNumberChannels; count++)
		{
			mChannelNumbers[count] = count;
			mNumberBins[count] = number_of_bins;
		}
		//ComputeHistogram();
	}
	virtual void ComputeHistogram()=0;
	virtual void NormaliseHistogram()=0;
	static void Draw1DHistogram( MatND histograms[], int number_of_histograms, Mat& display_image )
	{
		int number_of_bins = histograms[0].size[0];
		double max_value=0, min_value=0;
		double channel_max_value=0, channel_min_value=0;
		for (int channel=0; (channel < number_of_histograms); channel++)
		{
			minMaxLoc(histograms[channel], &channel_min_value, &channel_max_value, 0, 0);
			max_value = ((max_value > channel_max_value) && (channel > 0)) ? max_value : channel_max_value;
			min_value = ((min_value < channel_min_value) && (channel > 0)) ? min_value : channel_min_value;
		}
		float scaling_factor = ((float)256.0)/((float)number_of_bins);
		
		Mat histogram_image((int)(((float)number_of_bins)*scaling_factor)+1,(int)(((float)number_of_bins)*scaling_factor)+1,CV_8UC3,Scalar(255,255,255));
		display_image = histogram_image;
		line(histogram_image,Point(0,0),Point(0,histogram_image.rows-1),Scalar(0,0,0));
		line(histogram_image,Point(histogram_image.cols-1,histogram_image.rows-1),Point(0,histogram_image.rows-1),Scalar(0,0,0));
		int highest_point = static_cast<int>(0.9*((float)number_of_bins)*scaling_factor);
		for (int channel=0; (channel < number_of_histograms); channel++)
		{
			int last_height;
			for( int h = 0; h < number_of_bins; h++ )
			{
				float value = histograms[channel].at<float>(h);
				int height = static_cast<int>(value*highest_point/max_value);
				int where = (int)(((float)h)*scaling_factor);
				if (h > 0)
					line(histogram_image,Point((int)(((float)(h-1))*scaling_factor)+1,(int)(((float)number_of_bins)*scaling_factor)-last_height),
								         Point((int)(((float)h)*scaling_factor)+1,(int)(((float)number_of_bins)*scaling_factor)-height),
							             Scalar(channel==0?255:0,channel==1?255:0,channel==2?255:0));
				last_height = height;
			}
		}
	}
};



class HueHistogram : public Histogram
{
private:
	MatND mHistogram;
	int mMinimumSaturation, mMinimumValue, mMaximumValue;
#define DEFAULT_MIN_SATURATION 25
#define DEFAULT_MIN_VALUE 25
#define DEFAULT_MAX_VALUE 230
public:
	HueHistogram( Mat image, int number_of_bins, int min_saturation=DEFAULT_MIN_SATURATION, int min_value=DEFAULT_MIN_VALUE, int max_value=DEFAULT_MAX_VALUE ) :
	  Histogram( image, number_of_bins )
	{
		mMinimumSaturation = min_saturation;
		mMinimumValue = min_value;
		mMaximumValue = max_value;
		mChannelRange[1] = 180.0;
		ComputeHistogram();
	}
	void ComputeHistogram()
	{
		Mat hsv_image, hue_image, mask_image;
		cvtColor(mImage, hsv_image, CV_BGR2HSV);
		inRange( hsv_image, Scalar( 0, mMinimumSaturation, mMinimumValue ), Scalar( 180, 256, mMaximumValue ), mask_image );
		int channels[]={0,0};
		hue_image.create( mImage.size(), mImage.depth());
		mixChannels( &hsv_image, 1, &hue_image, 1, channels, 1 );
		const float* channel_ranges = mChannelRange;
		calcHist( &hue_image,1,0,mask_image,mHistogram,1,mNumberBins,&channel_ranges);
		
		Size gaussianKernelSize(7,7);
		GaussianBlur(mHistogram, mHistogram,gaussianKernelSize,0,0);
		Mat maskImage;
		double min,max;
		minMaxLoc(mHistogram,&min,&max);
		double minHistogramFactor = 0.05; //5% of the maximum
		inRange(mHistogram, Scalar(max*minHistogramFactor), Scalar(max),maskImage);
		maskImage.convertTo(maskImage,CV_32FC1);
		normalize(maskImage,maskImage,0,1,CV_MINMAX);
		multiply(mHistogram,maskImage,mHistogram);
		
		//cout<<mHistogram<<endl;

		/*int numberOfTimesToBlur = 3;
		Size gaussianKernelSize(5,5);
	    for(int i=0;i<numberOfTimesToBlur;i++)
			GaussianBlur(mHistogram, mHistogram,gaussianKernelSize,0,0);
		
		*/
		
	}
	void NormaliseHistogram()
	{
		normalize(mHistogram,mHistogram,0,255,CV_MINMAX);

	}
	Mat BackProject( Mat& image )
	{
		Mat& result = image.clone();
		const float* channel_ranges = mChannelRange;
		calcBackProject(&image,1,mChannelNumbers,mHistogram,result,&channel_ranges,255.0);
		return result;
	}
	MatND getHistogram()
	{
		return mHistogram;
	}
	void Draw( Mat& display_image )
	{
		
		Draw1DHistogram( &mHistogram, 1, display_image );

	}
	vector<colorTypes> get_color_types(){

		vector<colorTypes>colorHistogramMaxima;
		colorHistogramMaxima = get_local_maxima(mHistogram.clone(),8);
		return colorHistogramMaxima;
	}
};
vector<colorTypes> computeHueHistogramMaxima(Mat inputImage){
	bool debug  = true;
	Mat display_image;
	HueHistogram hh(inputImage,255,30,5,255);
	hh.NormaliseHistogram();
	hh.Draw(display_image);
	if(debug)imshow("hist image",display_image);
	
	return hh.get_color_types();
	
}
/************************************************************************************************************/

/* Example Code */
/* David Vernon */

/*=======================================================*/
/* Utility functions to prompt user to continue          */ 
/*=======================================================*/

void prompt_and_exit(int status) {
   printf("Press any key to continue and close terminal ... \n");
   getchar();
   exit(status);
}

void prompt_and_continue() {
   printf("Press any key to continue ... \n");
   getchar();
}




