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
    femur.setServoNumber(servoIds[legNumber][FEMUR_SERVO]);
    tibia.setServoNumber(servoIds[legNumber][TIBIA_SERVO]);
    
    coxa.nextJoint = &femur;
    femur.nextJoint = &tibia;
    
    coxaOrigin.x = coxaX[legNumber];
    coxaOrigin.y = coxaY[legNumber];
    coxaOrigin.z = 0;
    coxaOrigin.w = 1.0;
    
    switch(legNumber)
    {
        case LEFT_FRONT:
            naturalAngle = 1 * M_PI / 4;
            break;
        case LEFT_MIDDLE:
            naturalAngle = 2 * M_PI / 4;
            break;
        case LEFT_REAR:
            naturalAngle = 3 * M_PI / 4;
            break;
        case RIGHT_REAR:
            naturalAngle = 5 * M_PI / 4;
            break;
        case RIGHT_MIDDLE:
            naturalAngle = 6 * M_PI / 4;
            break;
        case RIGHT_FRONT:
            naturalAngle = 7 * M_PI / 4;
            break;
    }
    coxa.naturalAngle = naturalAngle;
}

/* Convert radians to servo position offset. */
int radToServo(float rads){
    float val = (rads*100)/51 * 100;
    return (int) val;
}

double normalizeAngle(double radians)
{
    if (fabs(radians) < M_PI) return radians;
    else if (radians > 0)
    {
        while (radians >= M_PI) radians -= 2 * M_PI;
    }
    else
    {
        while (radians <= -M_PI) radians += 2 * M_PI;
    }
    return radians;
}

void Leg::inverseKinematics(GLKVector4 effector)
{
    /* Simple 3dof leg solver. effector,y,z are the lengths from the Coxa rotate to the endpoint. */
    
    // first, make this a 2DOF problem... by solving coxa
//    printf("Leg: %s\n", legNames[myLegId]);
//    printf("atan2(effector.y, effector.x) =  %.0f degrees\n", atan2(effector.y, effector.x) * 180 / M_PI);
//    printf("naturalAngle =  %.0f degrees\n", naturalAngle * 180 / M_PI);
    
    coxa.ikSolution = normalizeAngle((atan2(effector.y, effector.x) - naturalAngle));

//    printf("coxa.ikSolution =  %.0f degrees\n", coxa.ikSolution * 180 / M_PI);

    double trueX = sqrt(effector.x * effector.x + effector.y * effector.y) - L_COXA;
    double im = fabs(sqrt(trueX * trueX + effector.z * effector.z));    // length of imaginary leg
    
    // get femur angle above horizon...

    // get femur angle below horizon...
    double q1 = -atan2(fabs(effector.z) ,trueX);
    
    // get total femur angle...
    double d1 = L_FEMUR * L_FEMUR - L_TIBIA * L_TIBIA + im * im;
    double d2 = 2 * L_FEMUR * im;
    double q2 = acos(d1 / d2);
    
    femur.ikSolution = normalizeAngle(q1 + q2 + FEMUR_ANGLE);
    
    // and tibia angle from femur...
    d1 = L_FEMUR * L_FEMUR - im * im + L_TIBIA * L_TIBIA;
    d2 = 2 * L_TIBIA * L_FEMUR;
    
    tibia.ikSolution = normalizeAngle(acos((float)d1/(float)d2) - TIBIA_ANGLE - FEMUR_ANGLE);   //-1.57;
}

bool Leg::newEndpoint(GLKVector4 ep)
{
//    printf("new endpoint z = %f\n", ep.z);
    
    GLKVector4 effector = GLKVector4Subtract(ep, transformedCoxaOrigin);
    inverseKinematics(effector);
    
    if ((coxa.ikSolution > coxa.minAngle && coxa.ikSolution < coxa.maxAngle) &&
        (femur.ikSolution > femur.minAngle && femur.ikSolution < femur.maxAngle) &&
        (tibia.ikSolution > tibia.minAngle && tibia.ikSolution < tibia.maxAngle))
    {
        endpoint = ep;
        return true;
    }
    else return false;
}

void Leg::updateServos(GLKMatrix4 bodyTransform, bool forward)
{
    transformedCoxaOrigin = GLKMatrix4MultiplyVector4(bodyTransform, coxaOrigin);
    coxa.originTransform = GLKMatrix4TranslateWithVector4(bodyTransform, coxaOrigin);
    
    GLKVector4 effector = GLKVector4Subtract(endpoint, transformedCoxaOrigin);
    
    if (!forward) inverseKinematics(effector);
    
    coxa.update(forward);
    femur.update(forward);
    tibia.update(forward);
}

