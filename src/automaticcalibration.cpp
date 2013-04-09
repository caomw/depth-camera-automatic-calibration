#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <stdio.h>

<<<<<<< HEAD
#define MAX_STICK_LENGTH 100
#define ITERATION_PER_FRAME 3

#define KEY_ESC 27

using namespace cv;
using namespace std;


Mat frame, circleFrame, hsv, maskredballup, maskredballdown, maskgreenballup, maskgreenballdown, mask;
Vec3f lastRed, lastGreen;
vector<pair<int, Vec3f> > pointsRed;
vector<pair<int, Vec3f> > pointsGreen;


static void onMouse( int event, int x, int y, int, void* )
{
	if (event == CV_EVENT_LBUTTONDOWN) {

		/*Vec3b color = hsv.at<Vec3b>(x, y);
		cout << (int) color[0] << endl;
		cout <<  (int)  color[1] << endl;
		cout <<  (int) color[2] << endl << endl;*/
		cout << "(" << x << ", " << y << ")" << endl;
		lastRed = Vec3f(x, y, 0);
	} else if (event == CV_EVENT_RBUTTONDOWN) {
		lastGreen = Vec3f(x, y, 0);
	}
}

double distanceBetweenPoints(Vec3f p1, Vec3f p2) {
	// sqrt((x1 - x2)^2 + (y1 - y2)^2)
	return sqrt((p1[0]-p2[0])*(p1[0]-p2[0]) + (p1[1] - p2[1])*(p1[1] - p2[1]));
}


Vec3f findClosestPoint(const vector<Vec3f>* points, Vec3f lastKnownPoint) {
	double mindistance = INFINITY;
	int index = -1;
	for (size_t i = 0; i < points->size(); i++) {
		double curDist = distanceBetweenPoints((*points)[i], lastKnownPoint);
		if (curDist < mindistance) {
			mindistance = curDist;
			index = i;
		}
	}
	return (*points)[index];
}


void drawCircleFromPoint(Vec3f p, Mat* frame) {
	Point center(cvRound(p[0]), cvRound(p[1]));
	int radius = cvRound(p[2]);
	// draw the circle center
	circle(*frame, center, 3, Scalar(0,255,0), -1, 8, 0);
	// draw the circle outline
	circle(*frame, center, radius, Scalar(0,0,255), 3, 8, 0);
}

/*
void drawCirclesAndLineBetween(Vec3f p1, Vec3f p2, Mat frame) {
	drawCircleFromPoint(p1, frame);
	drawCircleFromPoint(p2, frame);
	Point center1(cvRound(p1[0]), cvRound(p1[1]));
	Point center2(cvRound(p2[0]), cvRound(p2[1]));
	line(frame, center1, center2, Scalar(255, 0, 0), 3);
}
*/


void recordPositionOfBall(vector<pair<int, Vec3f> >* records, const vector<Vec3f>* circlesFound,
                          Vec3f* lastKnownPoint, int currentFrameNumber, Mat* frame) {
	circle(*frame, Point((*lastKnownPoint)[0], (*lastKnownPoint)[1]), 3, Scalar(255,255,0), -1, 8, 0);
	if (circlesFound->size() > 0) {
		Vec3f bestCandidate = findClosestPoint(circlesFound, *lastKnownPoint);
		if (records->size() > 0) {
			pair<int, Vec3f> lastRegistered = records->back();
			//cout << distanceBetweenPoints(lastRegistered.second, currentRed) << " --- " << (curFrame - lastRegistered.first)  << endl;
			if (*lastKnownPoint != lastRegistered.second ||
			    distanceBetweenPoints(lastRegistered.second, bestCandidate) <= (ITERATION_PER_FRAME * (currentFrameNumber - lastRegistered.first))) {
				*lastKnownPoint = bestCandidate;
			records->push_back(make_pair(currentFrameNumber, bestCandidate));
			drawCircleFromPoint(bestCandidate, frame);
		}
	}
	else {
		*lastKnownPoint = bestCandidate;
		records->push_back(make_pair(currentFrameNumber, bestCandidate));
		drawCircleFromPoint(bestCandidate, frame);
	}
}
}

void OnChangePosition(int value, void* data) {
	if (value >= 0) {
		(*(VideoCapture*) data).set(CV_CAP_PROP_POS_MSEC, 1000 * value);
	}
}



void OnChangePosition(int value, void* data) {
	if (value >= 0) {
		(*(VideoCapture*) data).set(CV_CAP_PROP_POS_MSEC, 1000 * value);
	}
}


int main(int argc, char** argv) {
	if (argc < 2) {
		printf("you need to specify a file");
		return -1;
	}
	
	VideoCapture cap(argv[1]);
	if(!cap.isOpened())
		return -1;


	bool paused = false;
	bool recording = false;
	bool showpath = false;
	bool somethingToRead = true;

	namedWindow("automatic calibration", 0);
	setMouseCallback( "automatic calibration", onMouse, 0 );
	bool init = false;
	
	namedWindow("parameters", 0);
	int param1 = 20;
	int param2 = 20;
	int minradius = 2;
	int maxradius = 20;

	createTrackbar("canny threshold", "parameters", &param1, 400);
	createTrackbar("center threshold", "parameters", &param2, 400);
	createTrackbar("min radius", "parameters", &minradius, 100);
	createTrackbar("max radius", "parameters", &maxradius, 200);
	

	// PLAYBACK CONTROLS
	namedWindow("playback controls", 0);
	createTrackbar("position", "playback controls", 0, floor(cap.get(CV_CAP_PROP_FRAME_COUNT) / cap.get(CV_CAP_PROP_FPS)), OnChangePosition, (void*) &cap);

	while (1)
	{
		Mat* toshow;
		
		if (!init) {
			toshow = &frame;
			init = true;
		}

		if (!paused) {
			somethingToRead = cap.grab();
		}
		if (somethingToRead) {
			cap.retrieve(frame, 0);
		}

		setTrackbarPos("position", "playback controls", floor(cap.get(CV_CAP_PROP_POS_FRAMES) / cap.get(CV_CAP_PROP_FPS)));
		cvtColor(frame, hsv, CV_BGR2HSV);

		// RED
		inRange(hsv, Scalar(165, 50, 50), Scalar(180, 255, 255), maskredballup);
		inRange(hsv, Scalar(0, 50, 50), Scalar(10, 255, 255), maskredballdown);
		maskredballup |= maskredballdown;
		morphologyEx(maskredballup, maskredballup, MORPH_OPEN, Mat());
		morphologyEx(maskredballup, maskredballup, MORPH_CLOSE, Mat());
		medianBlur(maskredballup, maskredballup, 7);
		vector<Vec3f> redcircles;

		// void HoughCircles(Mat& image, vector<Vec3f>& circles, int method, double dp, double minDist, double param1=100, double param2=100, int minRadius=0, int maxRadius=0)
		HoughCircles(maskredballup, redcircles, CV_HOUGH_GRADIENT, 2, 20, (param1 > 0) ? param1 : 1, (param2 > 0) ? param2 : 1, (minradius > 0) ? minradius : 1, (maxradius > 0) ? maxradius : 1);

		// GREEN
		inRange(hsv, Scalar(140, 50, 50), Scalar(160, 255, 255), maskgreenballup);
		inRange(hsv, Scalar(30, 30, 50), Scalar(60, 255, 255), maskgreenballdown);
		maskgreenballup |= maskgreenballdown;
		//GaussianBlur(maskgreenballup, maskgreenballup, Size(15, 15), 2, 2);
		morphologyEx(maskgreenballup, maskgreenballup, MORPH_OPEN, Mat());
		morphologyEx(maskgreenballup, maskgreenballup, MORPH_CLOSE, Mat());
		medianBlur(maskgreenballup, maskgreenballup, 7);
		vector<Vec3f> greencircles;

		mask = maskgreenballup | maskredballup;

		// void HoughCircles(Mat& image, vector<Vec3f>& circles, int method, double dp, double minDist, double param1=100, double param2=100, int minRadius=0, int maxRadius=0)
		HoughCircles(maskgreenballup, greencircles, CV_HOUGH_GRADIENT, 2, 20, (param1 > 0) ? param1 : 1, (param2 > 0) ? param2 : 1, (minradius > 0) ? minradius : 1, (maxradius > 0) ? maxradius : 1);


		if (recording && !paused) {
			if (redcircles.size() > 0) {
				recordPositionOfBall(&pointsRed, &redcircles, &lastRed, cap.get(CV_CAP_PROP_POS_FRAMES), &frame);
			}
			if (greencircles.size() > 0) {
				recordPositionOfBall(&pointsGreen, &greencircles, &lastGreen, cap.get(CV_CAP_PROP_POS_FRAMES), &frame);
			}
		}

		if (showpath) {
			pair<int, Vec3f> *lastPoint = NULL;
			cout << "SIZE : " << pointsRed.size() << "   --   " << pointsGreen.size() << endl;
			for(size_t i = 0; i < pointsRed.size(); i++) {
				pair<int, Vec3f> point = pointsRed[i];
				Point center(point.second[0], point.second[1]);
				circle(frame, center, 3, Scalar(0, 255, 255), 1, 8, 0);
				if (lastPoint != NULL) {
					Point center2(lastPoint->second[0], lastPoint->second[1]);
					//line(frame, center, center2, Scalar(255, 255, 0), 3);
				}
				lastPoint = &point;
			}
			for(size_t i = 0; i < pointsGreen.size(); i++) {
				pair<int, Vec3f> point = pointsGreen[i];
				Point center(point.second[0], point.second[1]);
				circle(frame, center, 3, Scalar(255, 0, 255), 1, 8, 0);
				if (lastPoint != NULL) {
					Point center2(lastPoint->second[0], lastPoint->second[1]);
					//line(frame, center, center2, Scalar(255, 255, 0), 3);
				}
				lastPoint = &point;
			}
		}

		/*circle(frame, Point(lastGreen[0], lastGreen[1]), 3, Scalar(0,255,255), -1, 8, 0);
		if (greencircles.size() > 0) {
			 Vec3f currentGreen = findClosestPoint(greencircles, lastGreen);
			 lastGreen = currentGreen;
			 pointsGreen.push_back(make_pair(cap.get(CV_CAP_PROP_POS_FRAMES), currentGreen));
			 drawCircleFromPoint(currentGreen, frame);
		}*/


			 imshow("automatic calibration", *toshow);

			 int key = waitKey(30);
			 switch (key) {

			 	case '1':
			 	toshow = &frame;
			 	break;
			 	
			 	case '2':
			 	toshow = &hsv;
			 	break;
			 	
			 	case '3':
			 	toshow = &mask;
			 	break;

			 	case '4':
			 	toshow = &maskredballup;
			 	break;

			 	case '5':
			 	toshow = &maskgreenballup;
			 	break;


			 	case '6':
			 	toshow = &circleFrame;
			 	break;

			 	case 'r':
			 	recording = !recording;
			 	break;

			 	case 'w':
				//writeVectorsToFile();
			 	break;

			 	case 's':
			 	showpath = !showpath;
			 	break;

			 	case KEY_ESC: return 0;
			 	
			 	case ' ':
			 	paused = !paused;
			 	break;

			 	default:
			 	break;
			 }
			}

			return 0;
		}
