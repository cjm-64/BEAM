#include "globals.h"
#include <QString>


//Test Timing
int Test_Time = 0;
int Test_Time_Frames = 0;
int Test_Time_M = 1;
int Test_Time_H = 0;
int SetUpTime = 300000;
int CalibrationTime = 3000;
int TimeCount = 0;
int RecordingTimer = 0;
int TestCompleteFlag = 0;

//Box
int X = 320;
int Y = 240;
int Width = 160;
int Height = 120;

//Circle
int thresh_val = 50;
int max_rad = 50;
int thresh_max_val = 100;
int thresh_type = 1;
int CED = 1;
int Cent_D = 1;

//Misc
int Step = 0;
int OnOff = 0;
int DisplaySelector = 0; //0 = w/Panel 1 = w/o
int DataSavingFlag = 0; //0 = None, 1 = CalR, 2 = CalL 3 = Full Test
int ColorOrBW = 0; //Color Video or BW Video; Color = 0, BW = 1
int MiscCounter = 0;
QString IP = "169.254.68.216";

//Tracking
int CalRFrame = 0;
int CalLFrame = 0;
int TestFrame = 0;
int RightCalibrationBefore[200] = {0};
int RightCalibrationAfter[200] = {0};
int LeftCalibrationBefore[200] = {0};
int LeftCalibrationAfter[200] = {0};
int XPositionServer[558000] = {0};
int XPositionClient[558000] = {0};
int CalFrames = 0;
int X_Point = 0;
int Y_Point = 0;
int Radius = 0;
