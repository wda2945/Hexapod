
#ifndef MYGAIT_H
#define MYGAIT_H

#define STAND_FRAMES1   90  //prep
#define STAND_FRAMES2   60  //up
#define SIT_FRAMES1     5   //prep
#define SIT_FRAMES2     60  //down


extern int sitHeight, sitSpread;

ik_req_t MyGaitGen(int leg){
  if( currentGait == SITDOWN )
  {
    if (stepNumber == gaitLegNo[leg]){
      // leg up, middle position
           switch(leg)
      {
      case RF:
      case RR:
      case RM:
        gaits[leg].x = 0;
        gaits[leg].y = sitSpread;
        break;
      case LF:
      case LR:
      case LM:
        gaits[leg].x = 0;
        gaits[leg].y = -sitSpread;
        break;
      } 
      gaits[leg].z = -liftHeight;
      gaits[leg].r = 0;
    }
    else if (stepNumber == gaitLegNo[leg]+1){
      // leg down position              
        gaits[leg].z = 0;
    }
    else if (stepNumber == stepsInCycle-1) {
      gaits[leg].z = -sitHeight -liftHeight; 
      tranTime = ((SIT_FRAMES2*12)-1);
    }
  } 
  else  if( currentGait == STANDUP )
  {      
    if (stepNumber == 0){
      // leg up, middle position
      switch(leg)
      {
      case RF:
      case RR:
      case RM:
        gaits[leg].x = 0;
        gaits[leg].y = sitSpread / 2;
        break;
      case LF:
      case LR:
      case LM:
        gaits[leg].x = 0;
        gaits[leg].y = -sitSpread / 2;
        break;
      } 
        gaits[leg].z = -liftHeight -sitHeight;
        gaits[leg].r = 0;
    }
    else if (stepNumber == 1){
      // leg down position              
      switch(leg)
      {
      case RF:
      case RR:
      case RM:
        gaits[leg].x = 0;
        gaits[leg].y = sitSpread;
        break;
      case LF:
      case LR:
      case LM:
        gaits[leg].x = 0;
        gaits[leg].y = -sitSpread;
        break;
      }  
        gaits[leg].z =  -sitHeight;  
        gaits[leg].r = 0; 
    }
    else if (stepNumber == 2) {
      gaits[leg].z = 0; 
      tranTime = ((STAND_FRAMES2*12)-1);
    }
  }
  return gaits[leg];
}

/* Smoother, slower gait. Legs will make a arc stroke. */
//TODO
ik_req_t MySmoothGaitGen(int leg){
  gaits[leg].z = 0;
  return gaits[leg];
}

void MyGaitSelect(int GaitType){
  currentGait = GaitType;
  if(GaitType == SITDOWN){        
    gaitGen   = &MyGaitGen;
    gaitSetup = &DefaultGaitSetup;
    gaitLegNo[RIGHT_FRONT] = 0;
    gaitLegNo[LEFT_REAR] = 2;
    gaitLegNo[LEFT_MIDDLE] = 4;
    gaitLegNo[LEFT_FRONT] = 6;
    gaitLegNo[RIGHT_REAR] = 8;
    gaitLegNo[RIGHT_MIDDLE] = 10;
    tranTime = ((SIT_FRAMES1*12)-1);
   stepsInCycle = 14;
  }
  else if (GaitType == STANDUP) 
  {
    gaitGen   = &MyGaitGen;
    gaitSetup = &DefaultGaitSetup;
    gaitLegNo[RIGHT_FRONT] = 0;
    gaitLegNo[LEFT_REAR] = 0;
    gaitLegNo[LEFT_MIDDLE] = 0;
    gaitLegNo[LEFT_FRONT] = 0;
    gaitLegNo[RIGHT_REAR] = 0;
    gaitLegNo[RIGHT_MIDDLE] = 0;
   tranTime = ((STAND_FRAMES1*12)-1);
   stepsInCycle = 4;
  }

  cycleTime = (stepsInCycle*tranTime)/1000.0;
  stepNumber = 0;
}

/* Convert servo position offset to radians. */
float servoToRad(int val){
  float rads = ((float)val / 100.0) * 51.0 / 100.0; 
  return rads;
}
//calculate endpoint from servo positions
ik_req_t legFK(char legNr, int _coxa, int _femur, int _tibia)
{
  ik_req_t ans;
  int coxa, femur, tibia;
  int offsetX, offsetY;
  switch (legNr)
  {
  case RIGHT_FRONT:
    coxa = _coxa - 368;
    femur = _femur - 524;
    tibia = _tibia - 354;
    offsetX = X_COXA;
    offsetY = Y_COXA;
    break;
  case RIGHT_REAR:
    coxa = 656 - _coxa;
    femur = _femur - 524;
    tibia = _tibia - 354;
    offsetX = -X_COXA;
    offsetY = Y_COXA;
    break;
  case LEFT_FRONT:
    coxa = 656 - _coxa;
    femur = 500 - _femur;
    tibia = 670 - _tibia;
    offsetX = X_COXA;
    offsetY = -Y_COXA;
    break;
  case LEFT_REAR:
    coxa = 368 - _coxa;
    femur = 500 - _femur;
    tibia = 670 - _tibia;
    offsetX = -X_COXA;
    offsetY = -Y_COXA;
    break;
  case RIGHT_MIDDLE:
    coxa = _coxa - 512;
    femur = _femur - 524;
    tibia = _tibia - 354;
    offsetX = 0;
    offsetY = Y_COXA;
    break;
  case LEFT_MIDDLE:
    coxa = 512 - _coxa;
    femur = 500 - _femur;
    tibia = 670 - _tibia;
    offsetX = 0;
    offsetY = -Y_COXA;
    break;
  }
  //calc all the trig functions
  float C = servoToRad(coxa);
  float sinC = sin(C);
  float cosC = cos(C);
  float F = servoToRad(femur);
  float sinF = sin(F);
  float cosF = cos(F);
  float T = servoToRad(tibia);
  float sinT = sin(T);
  float cosT = cos(T);
  //overall length of coxa to foot
  float d = (((L_FEMUR + L_TIBIA * cosT) * cosF) + L_COXA);
  if (offsetY > 0)
  {
    //right side
    ans.y = (int) d * cosC + offsetY;  
  }
  else
  {
    //left side
    ans.y = (int) (-d * cosC) + offsetY;  
  } 
  ans.x = (int) d * sinC + offsetX;
  ans.z = (int) L_FEMUR * sinF + L_TIBIA * sinT * cosF;
  return ans;
}

#endif



