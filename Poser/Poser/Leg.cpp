//
//  Leg.cpp
//  BugPoser
//
//  Created by Martin Lane-Smith on 5/24/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//
#include <math.h>
#include "Leg.hpp"
#include "nuke.h"
#include <GLKit/GLKMatrix4.h>

void Leg::setLegNumber(int legNumber)
{
    myLegId = legNumber;
    
    coxa.setServoNumber(servoIds[legNumber][COXA_SERVO]);
    coxa.boneLength = L_COXA;
    femur.setServoNumber(servoIds[legNumber][FEMUR_SERVO]);
    femur.boneLength = L_FEMUR;
    tibia.setServoNumber(servoIds[legNumber][TIBIA_SERVO]);
    tibia.boneLength = L_TIBIA;
    coxa.nextJoint = &femur;
    femur.nextJoint = &tibia;
    
    coxaOffset.x = coxaX[legNumber];
    coxaOffset.y = coxaY[legNumber];
    coxaOffset.z = 0;
}

/* Convert radians to servo position offset. */
int radToServo(float rads){
    float val = (rads*100)/51 * 100;
    return (int) val;
}

void Leg::inverseKinematics()
{
    /* Simple 3dof leg solver. effector,y,z are the lengths from the Coxa rotate to the endpoint. */
    
    // first, make this a 2DOF problem... by solving coxa
    coxa.ikSolution = 368 + radToServo(atan2(effector.x, effector.y));
    double trueX = sqrt(effector.x * effector.x + effector.y * effector.y) - L_COXA;
    double im = sqrt(effector.x * effector.x + effector.z * effector.z);    // length of imaginary leg
    
    // get femur angle above horizon...
    double q1 = -atan2(effector.z ,trueX);
    double d1 = L_FEMUR * L_FEMUR - L_TIBIA * L_TIBIA + im * im;
    double d2 = 2 * L_FEMUR * im;
    double q2 = acos(d1 / d2);
    femur.ikSolution = 524 + radToServo(q1+q2);
    
    // and tibia angle from femur...
    d1 = L_FEMUR * L_FEMUR - im * im + L_TIBIA * L_TIBIA;
    d2 = 2 * L_TIBIA * L_FEMUR;
    tibia.ikSolution = 354 + radToServo(acos((float)d1/(float)d2)-1.57);
    
}

void Leg::updateServos(GLKVector3 bodyOffset, GLKVector3 bodyRotation)
{
    GLKMatrix4 coxaTransform = GLKMatrix4MakeTranslation(bodyOffset.x, bodyOffset.y, bodyOffset.z);
    coxaTransform = GLKMatrix4Rotate(coxaTransform, bodyRotation.x, 1.0, 0.0, 0);
    coxaTransform = GLKMatrix4Rotate(coxaTransform, bodyRotation.y, 0, 1.0, 0);
    coxaTransform = GLKMatrix4Rotate(coxaTransform, bodyRotation.z, 0, 0, 1.0);
    
    GLKVector4 coxaPosition = GLKMatrix4MultiplyVector4(coxaTransform, coxaOffset);
    effector = GLKVector4Subtract(endpoint,coxaPosition);
    
    coxa.origin             = coxaPosition;
//    coxa.originTransform    = coxaTransform;
    
    inverseKinematics();
    
    coxa.update();
    femur.update();
    tibia.update();
}

