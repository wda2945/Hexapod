#include <ax12.h>
#include <BioloidController.h>
#include <Arduino.h>
#include <math.h>
#include "nuke.h"

/* min and max positions for each servo */
int mins[] = {
  222, 225, 159, 164, 279, 158, 223, 229, 159, 156, 272, 155, 226, 233, 158, 157, 271, 157};
int maxs[] = {
  790, 792, 855, 862, 857, 747, 788, 794, 859, 857, 860, 747, 789, 789, 858, 860, 859, 743};

/* IK Engine */
BioloidController bioloid = BioloidController(1000000);
ik_req_t endpoints[LEG_COUNT];
float bodyRotX = 0;             // body roll (rad)
float bodyRotY = 0;             // body pitch (rad)
float bodyRotZ = 0;             // body rotation (rad)
int bodyPosX = 0;               // body offset (mm)
int bodyPosY = 0;               // body offset (mm)
int Xspeed;                     // forward speed (mm/s)
int Yspeed;                     // sideward speed (mm/s)
float Rspeed;                   // rotation speed (rad/s)

/* Gait Engine */
int gaitLegNo[LEG_COUNT];       // order to step through legs
ik_req_t gaits[LEG_COUNT];      // gait engine output
int pushSteps;                  // how much of the cycle we are on the ground
int stepsInCycle;               // how many steps in this cycle
int stepNumber;                 // current step
int tranTime;
int liftHeight;
float cycleTime;                // cycle time in seconds (adjustment from speed to step-size)

/* Setup the starting positions of the legs. */
void setupIK(){
  endpoints[RIGHT_FRONT].x = 52;
  endpoints[RIGHT_FRONT].y = 118;
  endpoints[RIGHT_FRONT].z = 97;

  endpoints[RIGHT_REAR].x = -52;
  endpoints[RIGHT_REAR].y = 118;
  endpoints[RIGHT_REAR].z = 97;

  endpoints[RIGHT_MIDDLE].x = 0;
  endpoints[RIGHT_MIDDLE].y = 118;
  endpoints[RIGHT_MIDDLE].z = 97;

  endpoints[LEFT_MIDDLE].x = 0;
  endpoints[LEFT_MIDDLE].y = -118;
  endpoints[LEFT_MIDDLE].z = 97;

  endpoints[LEFT_FRONT].x = 52;
  endpoints[LEFT_FRONT].y = -118;
  endpoints[LEFT_FRONT].z = 97;

  endpoints[LEFT_REAR].x = -52;
  endpoints[LEFT_REAR].y = -118;
  endpoints[LEFT_REAR].z = 97;

  liftHeight = 35;
  stepsInCycle = 1;
  stepNumber = 0;
}

#include "gaits.h"
#include "myGaits.h"

/* Convert radians to servo position offset. */
int radToServo(float rads){
  float val = (rads*100)/51 * 100;
  return (int) val;
}

/* Body FK solver: compute where leg should be. */
ik_req_t bodyFK(int X, int Y, int Z,               //leg nominal position
int Xdisp, int Ydisp, float Zrot)  //body displacement
{
  ik_req_t ans;

  float cosB = cos(bodyRotX);
  float sinB = sin(bodyRotX);
  float cosG = cos(bodyRotY);
  float sinG = sin(bodyRotY);
  float cosA = cos(bodyRotZ+Zrot);
  float sinA = sin(bodyRotZ+Zrot);

  int totalX = X + Xdisp + bodyPosX;
  int totalY = Y + Ydisp + bodyPosY;

  ans.x = totalX - int(totalX*cosG*cosA + totalY*sinB*sinG*cosA + Z*cosB*sinG*cosA - totalY*cosB*sinA + Z*sinB*sinA) + bodyPosX;
  ans.y = totalY - int(totalX*cosG*sinA + totalY*sinB*sinG*sinA + Z*cosB*sinG*sinA + totalY*cosB*cosA - Z*sinB*cosA) + bodyPosY;
  ans.z = Z - int(-totalX*sinG + totalY*sinB*cosG + Z*cosB*cosG);

  return ans;
}

/* Simple 3dof leg solver. X,Y,Z are the lengths from the Coxa rotate to the endpoint. */
ik_sol_t legIK(int X, int Y, int Z){
  ik_sol_t ans;

  // first, make this a 2DOF problem... by solving coxa
  ans.coxa = radToServo(atan2(X,Y));
  long trueX = sqrt(sq((long)X)+sq((long)Y)) - L_COXA;
  long im = sqrt(sq((long)trueX)+sq((long)Z));    // length of imaginary leg

  // get femur angle above horizon...
  float q1 = -atan2(Z,trueX);
  long d1 = sq(L_FEMUR)-sq(L_TIBIA)+sq(im);
  long d2 = 2*L_FEMUR*im;
  float q2 = acos((float)d1/(float)d2);
  ans.femur = radToServo(q1+q2);

  // and tibia angle from femur...
  d1 = sq(L_FEMUR)-sq(im)+sq(L_TIBIA);
  d2 = 2*L_TIBIA*L_FEMUR;
  ans.tibia = radToServo(acos((float)d1/(float)d2)-1.57);

  return ans;
}

void doIK(){
  int servo;
  ik_req_t req, gait;
  ik_sol_t sol;

  gaitSetup();

  // right front leg
  gait = gaitGen(RIGHT_FRONT);
  req = bodyFK(endpoints[RIGHT_FRONT].x+gait.x,  //leg
  endpoints[RIGHT_FRONT].y+gait.y, 
  endpoints[RIGHT_FRONT].z+gait.z, 
  X_COXA, Y_COXA, gait.r);          //body
  sol = legIK(endpoints[RIGHT_FRONT].x+req.x+gait.x,
  endpoints[RIGHT_FRONT].y+req.y+gait.y,
  endpoints[RIGHT_FRONT].z+req.z+gait.z);
  servo = 368 + sol.coxa;
  if(servo < maxs[RF_COXA-1] && servo > mins[RF_COXA-1])
    bioloid.setNextPose(RF_COXA, servo);
  else{
    ServoFail(RFC, servo);
  }
  servo = 524 + sol.femur;
  if(servo < maxs[RF_FEMUR-1] && servo > mins[RF_FEMUR-1])
    bioloid.setNextPose(RF_FEMUR, servo);
  else{
    ServoFail(RFF, servo);
  }
  servo = 354 + sol.tibia;
  if(servo < maxs[RF_TIBIA-1] && servo > mins[RF_TIBIA-1])
    bioloid.setNextPose(RF_TIBIA, servo);
  else{
    ServoFail(RFT, servo);
  }

  // right rear leg
  gait = gaitGen(RIGHT_REAR);
  req = bodyFK(endpoints[RIGHT_REAR].x+gait.x,
  endpoints[RIGHT_REAR].y+gait.y, 
  endpoints[RIGHT_REAR].z+gait.z, 
  -X_COXA, Y_COXA, gait.r);
  sol = legIK(-endpoints[RIGHT_REAR].x-req.x-gait.x,
  endpoints[RIGHT_REAR].y+req.y+gait.y,
  endpoints[RIGHT_REAR].z+req.z+gait.z);
  servo = 656 - sol.coxa;
  if(servo < maxs[RR_COXA-1] && servo > mins[RR_COXA-1])
    bioloid.setNextPose(RR_COXA, servo);
  else{
    ServoFail(RRC, servo);
  }
  servo = 524 + sol.femur;
  if(servo < maxs[RR_FEMUR-1] && servo > mins[RR_FEMUR-1])
    bioloid.setNextPose(RR_FEMUR, servo);
  else{
    ServoFail(RRF, servo);
  }
  servo = 354 + sol.tibia;
  if(servo < maxs[RR_TIBIA-1] && servo > mins[RR_TIBIA-1])
    bioloid.setNextPose(RR_TIBIA, servo);
  else{
    ServoFail(RRT, servo);
  }

  // left front leg
  gait = gaitGen(LEFT_FRONT);
  req = bodyFK(endpoints[LEFT_FRONT].x+gait.x,
  endpoints[LEFT_FRONT].y+gait.y, 
  endpoints[LEFT_FRONT].z+gait.z, 
  X_COXA, -Y_COXA, gait.r);
  sol = legIK(endpoints[LEFT_FRONT].x+req.x+gait.x,
  -endpoints[LEFT_FRONT].y-req.y-gait.y,
  endpoints[LEFT_FRONT].z+req.z+gait.z);
  servo = 656 - sol.coxa;
  if(servo < maxs[LF_COXA-1] && servo > mins[LF_COXA-1])
    bioloid.setNextPose(LF_COXA, servo);
  else{
    ServoFail(LFC, servo);
  }
  servo = 500 - sol.femur;
  if(servo < maxs[LF_FEMUR-1] && servo > mins[LF_FEMUR-1])
    bioloid.setNextPose(LF_FEMUR, servo);
  else{
    ServoFail(LFF, servo);
  }
  servo = 670 - sol.tibia;
  if(servo < maxs[LF_TIBIA-1] && servo > mins[LF_TIBIA-1])
    bioloid.setNextPose(LF_TIBIA, servo);
  else{
    ServoFail(LFT, servo);
  }

  // left rear leg
  gait = gaitGen(LEFT_REAR);
  req = bodyFK(endpoints[LEFT_REAR].x+gait.x,
  endpoints[LEFT_REAR].y+gait.y, 
  endpoints[LEFT_REAR].z+gait.z, 
  -X_COXA, -Y_COXA, gait.r);
  sol = legIK(-endpoints[LEFT_REAR].x-req.x-gait.x,
  -endpoints[LEFT_REAR].y-req.y-gait.y,
  endpoints[LEFT_REAR].z+req.z+gait.z);
  servo = 368 + sol.coxa;
  if(servo < maxs[LR_COXA-1] && servo > mins[LR_COXA-1])
    bioloid.setNextPose(LR_COXA, servo);
  else{
    ServoFail(LRC, servo);
  }
  servo = 500 - sol.femur;
  if(servo < maxs[LR_FEMUR-1] && servo > mins[LR_FEMUR-1])
    bioloid.setNextPose(LR_FEMUR, servo);
  else{
    ServoFail(LRF, servo);
  }
  servo = 670 - sol.tibia;
  if(servo < maxs[LR_TIBIA-1] && servo > mins[LR_TIBIA-1])
    bioloid.setNextPose(LR_TIBIA, servo);
  else{
    ServoFail(LRT, servo);
  }

  // right middle leg
  gait = gaitGen(RIGHT_MIDDLE);
  req = bodyFK(endpoints[RIGHT_MIDDLE].x+gait.x,
  endpoints[RIGHT_MIDDLE].y+gait.y, 
  endpoints[RIGHT_MIDDLE].z+gait.z, 
  0, Y_COXA, gait.r);
  sol = legIK(endpoints[RIGHT_MIDDLE].x+req.x+gait.x,
  endpoints[RIGHT_MIDDLE].y+req.y+gait.y,
  endpoints[RIGHT_MIDDLE].z+req.z+gait.z);
  servo = 512 + sol.coxa;
  if(servo < maxs[RM_COXA-1] && servo > mins[RM_COXA-1])
    bioloid.setNextPose(RM_COXA, servo);
  else{
    ServoFail(RMC, servo);
  }
  servo = 524 + sol.femur;
  if(servo < maxs[RM_FEMUR-1] && servo > mins[RM_FEMUR-1])
    bioloid.setNextPose(RM_FEMUR, servo);
  else{
    ServoFail(RMF, servo);
  }
  servo = 354 + sol.tibia;
  if(servo < maxs[RM_TIBIA-1] && servo > mins[RM_TIBIA-1])
    bioloid.setNextPose(RM_TIBIA, servo);
  else{
    ServoFail(RMT, servo);
  }

  // left middle leg
  gait = gaitGen(LEFT_MIDDLE);
  req = bodyFK(endpoints[LEFT_MIDDLE].x+gait.x,
  endpoints[LEFT_MIDDLE].y+gait.y, 
  endpoints[LEFT_MIDDLE].z+gait.z, 
  0, -Y_COXA, gait.r);
  sol = legIK(endpoints[LEFT_MIDDLE].x+req.x+gait.x,
  -endpoints[LEFT_MIDDLE].y-req.y-gait.y,
  endpoints[LEFT_MIDDLE].z+req.z+gait.z);
  servo = 512 - sol.coxa;
  if(servo < maxs[LM_COXA-1] && servo > mins[LM_COXA-1])
    bioloid.setNextPose(LM_COXA, servo);
  else{
    ServoFail(LMC, servo);
  }
  servo = 500 - sol.femur;
  if(servo < maxs[LM_FEMUR-1] && servo > mins[LM_FEMUR-1])
    bioloid.setNextPose(LM_FEMUR, servo);
  else{
    ServoFail(LMF, servo);
  }
  servo = 670 - sol.tibia;
  if(servo < maxs[LM_TIBIA-1] && servo > mins[LM_TIBIA-1])
    bioloid.setNextPose(LM_TIBIA, servo);
  else{
    ServoFail(LMT, servo);
  }
  stepNumber = (stepNumber+1);

  if (stepNumber >= stepsInCycle)
  {
    endOfCycle = 1;
    stepNumber = 0;
  }
}


