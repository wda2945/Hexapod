//
//  Leg.cpp
//  BugPoser
//
//  Created by Martin Lane-Smith on 5/24/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//
#include <math.h>
#include "Leg.hpp"
#include "Hexapod.hpp"
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
            naturalAngle = -3 * M_PI / 4;
            break;
        case RIGHT_MIDDLE:
            naturalAngle = -2 * M_PI / 4;
            break;
        case RIGHT_FRONT:
            naturalAngle = -1 * M_PI / 4;
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

/* Body FK solver: compute where leg should be. */
GLKVector4 Leg::bodyFK(int X, int Y, int Z,         //endpoint nominal position
                int Xdisp, int Ydisp, float Zrot)   //body displacement
{
    GLKVector4 ans;
    
    float cosB = cos(theHexapod.bodyRotation.x);
    float sinB = sin(theHexapod.bodyRotation.x);
    float cosG = cos(theHexapod.bodyRotation.y);
    float sinG = sin(theHexapod.bodyRotation.y);
    float cosA = cos(theHexapod.bodyRotation.z+Zrot);
    float sinA = sin(theHexapod.bodyRotation.z+Zrot);
    
    int totalX = X + Xdisp + theHexapod.bodyOffset.x;
    int totalY = Y - Ydisp + theHexapod.bodyOffset.y;
    int totalZ = Z + theHexapod.bodyOffset.z;
    
    ans.x = totalX - (totalX*cosG*cosA + totalY*sinB*sinG*cosA + totalZ*cosB*sinG*cosA + totalY*cosB*sinA + totalZ*sinB*sinA) - theHexapod.bodyOffset.x;
    ans.y = totalY - (totalY*cosB*cosA + totalY*sinB*sinG*sinA + totalZ*cosB*sinG*sinA + totalX*cosG*sinA - totalZ*sinB*cosA) - theHexapod.bodyOffset.y;
    ans.z = totalZ - (-totalX*sinG - totalY*sinB*cosG + totalZ*cosB*cosG) - theHexapod.bodyOffset.z;
    
    ans.w = 1.0;
    return ans;
}
#define sq(v) ((v)*(v))

void Leg::inverseKinematics(GLKVector4 ef)
{
    GLKVector4 effector {ef.x, -ef.y, -ef.z, ef.w};
    
    // first, make this a 2DOF problem... by solving coxa
    coxa.set_ikSolution(normalizeAngle((atan2(effector.y, effector.x) - naturalAngle)));
    double trueX = sqrt(sq(effector.x)+sq(effector.y)) - L_COXA;
    double im = sqrt(sq(trueX)+sq(effector.z));    // length of imaginary leg
    
    // get femur angle above horizon...
    double q1 = atan2(effector.z,trueX);
    double d1 = sq(L_FEMUR)-sq(L_TIBIA)+sq(im);
    double d2 = 2*L_FEMUR*im;
    double q2 = acos(d1/d2);
    femur.set_ikSolution(normalizeAngle(q1 + q2 + FEMUR_ANGLE));
    
    // and tibia angle from femur...
    d1 = sq(L_FEMUR)-sq(im)+sq(L_TIBIA);
    d2 = 2*L_TIBIA*L_FEMUR;
    tibia.set_ikSolution(normalizeAngle(acos(d1/d2) - TIBIA_ANGLE - FEMUR_ANGLE));
    
}

bool Leg::newEndpoint(GLKVector4 ep)
{
    //new endpoint, do a test IK
    
    GLKVector4 bodyFKadjustment = bodyFK(ep.x, ep.y, -ep.z,
                            coxaX[myLegId], coxaY[myLegId], 0);
    
    //IK requirement
    GLKVector4 relEp;
    relEp.x = endpoint.x + bodyFKadjustment.x;
    relEp.y = endpoint.y - bodyFKadjustment.y;
    relEp.z = endpoint.z - bodyFKadjustment.z;
    
    inverseKinematics(relEp);
    endpoint = ep;
    
    return true;
}

void Leg::updateServos(GLKMatrix4 bodyTransform)
{

    //adjustment due to bodyFK
    GLKVector4 bodyFKadjustment = bodyFK(endpoint.x, endpoint.y, -endpoint.z,
                            coxaX[myLegId], coxaY[myLegId], 0);
    
    //IK requirement
    relativeEndpoint.x = endpoint.x + bodyFKadjustment.x;
    relativeEndpoint.y = endpoint.y - bodyFKadjustment.y;
    relativeEndpoint.z = endpoint.z - bodyFKadjustment.z;
    
    inverseKinematics(relativeEndpoint);
    
//    transformedCoxaOrigin = GLKMatrix4MultiplyVector4(bodyTransform, coxaOrigin);
    coxa.originTransform = GLKMatrix4TranslateWithVector4(bodyTransform, coxaOrigin);
    
    coxa.update();
    femur.update();
    tibia.update();
    
//    printf("Leg %i @ %f, %f, %f. coxa orig @ %f, %f, %f\n",
//           myLegId, endpoint.x+coxaOrigin.x, -endpoint.y+coxaOrigin.y, endpoint.z,
//           coxaOrigin.x, coxaOrigin.y, coxaOrigin.z);
//    printf("Leg %i @ %f, %f, %f. coxa end @ %f, %f, %f\n",
//           myLegId, endpoint.x+coxaOrigin.x, -endpoint.y+coxaOrigin.y, endpoint.z,
//           coxa.fkEndpoint.x, coxa.fkEndpoint.y, coxa.fkEndpoint.z);
//    printf("Leg %i @ %f, %f, %f. femur end @ %f, %f, %f\n",
//           myLegId, endpoint.x+coxaOrigin.x, -endpoint.y+coxaOrigin.y, endpoint.z,
//           femur.fkEndpoint.x, femur.fkEndpoint.y, femur.fkEndpoint.z);
//    printf("Leg %i @ %f, %f, %f. tibia end @ %f, %f, %f\n\n",
//           myLegId, endpoint.x+coxaOrigin.x, -endpoint.y+coxaOrigin.y, endpoint.z,
//           tibia.fkEndpoint.x, tibia.fkEndpoint.y, tibia.fkEndpoint.z);

}

