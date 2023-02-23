#ifndef GLOBALS_H
#define GLOBALS_H
#include <QtGlobal>

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

//Timing
extern int Test_Time;
extern int Test_Time_Frames;
extern int Test_Time_M;
extern int Test_Time_H;
extern int SetUpTime;
extern int CalibrationTime;
extern int TimeCount;
extern int RecordingTimer;
extern int TestCompleteFlag;

//Box
extern int X;
extern int Y;
extern int Width;
extern int Height;

//Circle
extern int thresh_val;
extern int max_rad;
extern int thresh_max_val;
extern int thresh_type;
extern int CED;
extern int Cent_D;

//Misc
extern int Step;
extern int OnOff;
extern int DisplaySelector;
extern int DataSavingFlag;
extern int ColorOrBW;

//Tracking
extern int CalRFrame;
extern int CalLFrame;
extern int TestFrame;
extern int RightCalibrationBefore[200];
extern int RightCalibrationAfter[200];
extern int LeftCalibrationBefore[200];
extern int LeftCalibrationAfter[200];
extern int XPositionServer[558000];
extern int XPositionClient[558000];
extern int CalFrames;
extern int X_Point;
extern int Y_Point;
extern int Radius;


#endif // GLOBALS_H
