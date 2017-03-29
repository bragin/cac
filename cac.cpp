#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>
#include <random>
#include <cmath>
#include <fstream>
#include <armadillo>

using namespace std;
using namespace cv;

#include "input.h"

void findEllipses(vector<Mat> channels, int k, Mat &sc, Mat &res, vector<RotatedRect> &minEllipse) {
	vector<Vec4i> hierarchy;
	vector<vector<Point>> contours;

	// Blur the image before binarization
	blur(channels[k], sc, Size(3, 3));

	//Mat img_copy(sc.rows, sc.cols, CV_8UC3, Scalar(255, 255, 255));

	// Binarize it and find contours
	threshold(sc, sc, 105, 255, THRESH_BINARY);
	findContours(sc, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0));

	vector<RotatedRect> minRect_red(contours.size());

	for (int i = 0; i < contours.size(); i++) {
		minRect_red[i] = minAreaRect(Mat(contours[i]));
		//printf("contour %d, size %ld, %ld x %ld\n", i, contours[i].size(), minRect_red[i].size.width, minRect_red[i].size.height);
		if (contours[i].size() > 5 && minRect_red[i].size.width > 60.0){
			minEllipse.push_back(fitEllipse(Mat(contours[i])));
			//{ minEllipse_red[i] = fitEllipse( Mat(contours[i]) ); }
		}
	}

	// Draw the resulting ellipses
	for (int i = 0; i< minEllipse.size(); i++) {
		Scalar color = Scalar(rand() % 255, rand() % 255, rand() % 255);
		// contour
		drawContours(res, contours, i, color, 1, 8, vector<Vec4i>(), 0, Point());
		// ellipse
		ellipse(res, minEllipse[i], color, 2, 8);
		// rotated rectangle
		Point2f rect_points[4];
		minRect_red[i].points(rect_points);
		for (int j = 0; j < 4; j++)
			line(res, rect_points[j], rect_points[(j + 1) % 4], color, 1, 8);
	}
}

void generatePattern() {
    // 300 dpi (print) = 2480 X 3508 pixels (This is "A4" as I know it, i.e. "210mm X 297mm @ 300 dpi")
    // 600 dpi = 
    //unsigned int height = 1123, width = 1587;
    unsigned int height = 2480 - 100, width = 3508 - 100;
    Mat image(height, width, CV_8UC3, Scalar(255, 255, 255));
    int x = 90, y = 55, radius = 30;
    while (x + 15 < width){
        while (y + 15 < height) {
            circle(image, Point(x, y), radius, Scalar(0, 0, 0), CV_FILLED, 4);
            y += 90;
        }
        y = 55;
        x += 90;

    }

    imwrite("picture.png", image);
}

int main(int argc, char *argv[]) {

    // Parse input options
    InputParser input(argc, argv);
    if(input.cmdOptionExists("-g")){
        // Generate image with a test pattern to photo
        cout << "Generating test pattern" << endl;
        generatePattern();
        return 0;
    }

    const std::string &filename = input.getCmdOption("-f");
    if (!filename.empty()){
        // Source image file name
    }

	// Load photo of the template
	Mat img = imread("src4.png", 1);

	// Create images for the channels
	Mat img_resR(img.rows, img.cols, CV_8UC3, Scalar(255, 255, 255));
	Mat img_resG(img.rows, img.cols, CV_8UC3, Scalar(255, 255, 255));
	Mat img_resB(img.rows, img.cols, CV_8UC3, Scalar(255, 255, 255));

	// Create images for the offsets
	Mat img_offs1(img.rows, img.cols, CV_8UC3, Scalar(255, 255, 255));
	Mat img_offs2(img.rows, img.cols, CV_8UC3, Scalar(255, 255, 255));

	// Split source image into three channels
	vector<Mat> channels;
	split(img, channels);

	// Find ellipses centers and save them as red,blue and green.jpg files
	Mat red, green, blue;
	vector<RotatedRect> minEllipse[3];

	findEllipses(channels, 2, red, img_resR, minEllipse[2]);
	imwrite("red.jpg", img_resR);

	findEllipses(channels, 0, blue, img_resB, minEllipse[0]);
	imwrite("blue.jpg", img_resB);

	findEllipses(channels, 1, green, img_resG, minEllipse[1]);
	imwrite("green.jpg", img_resG);


	// Take G channel as the main one and calc offsets of R and B, draw and save them
	for (int i = 0; i < minEllipse[1].size(); i++) {
		for (int j = 0; j < minEllipse[2].size(); j++) {
			if (abs(minEllipse[1][i].center.x - minEllipse[2][j].center.x) < 10 && abs(minEllipse[1][i].center.y - minEllipse[2][j].center.y) < 10) {
				swap(minEllipse[2][j], minEllipse[2][i]);

				break;
			}
		}
		Point distort;
		distort = (minEllipse[2][i].center - minEllipse[1][i].center) * 15;
		line(img_offs1, (Point)minEllipse[1][i].center, (Point)minEllipse[2][i].center + distort, Scalar(0, 0, 255), 3);
		circle(img_offs1, (Point)minEllipse[1][i].center, 1, Scalar(0, 255, 0), CV_FILLED, 15);
		//        cout << "Rect[" << i << "].center_green = " << minEllipse_green[i].center << "; Rect[" << i << "].center_red = " << minEllipse_red[i].center << endl;
	}

	imwrite("red_ffs.jpg", img_offs1);

	for (int i = 0; i < minEllipse[1].size(); i++) {
		for (int j = 0; j < minEllipse[2].size(); j++) {
			if (abs(minEllipse[1][i].center.x - minEllipse[2][j].center.x) < 10 && abs(minEllipse[1][i].center.y - minEllipse[2][j].center.y) < 10 && i != j) {
				swap(minEllipse[2][j], minEllipse[2][i]);
				break;
			}
		}
		Point distort;
		distort = (minEllipse[2][i].center - minEllipse[1][i].center) * 15;
		line(img_offs2, (Point)minEllipse[1][i].center, (Point)minEllipse[2][i].center + distort, Scalar(255, 0, 0), 3);
		circle(img_offs2, (Point)minEllipse[1][i].center, 1, Scalar(0, 255, 0), CV_FILLED, 15);
		//        cout << "Rect[" << i << "].center_green = " << minEllipse_green[i].center << "; Rect[" << i << "].center_blue = " << minEllipse_blue[i].center << endl;
		//break;
	}

	imwrite("blue_ffs.jpg", img_offs2);

	//namedWindow("Display window", WINDOW_NORMAL);
	//imshow("Display window", img_copy);

	//namedWindow("imge", WINDOW_NORMAL);
	//imshow("imge", imge);

	waitKey(0);
	red.release();
	green.release();
	blue.release();

	return 0;
}
