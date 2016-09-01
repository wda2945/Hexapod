//
//  JointServo.cpp
//  BugPoser
//
//  Created by Martin Lane-Smith on 5/24/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

#include "JointServo.hpp"
#include "Hexapod.hpp"

#include <GLKit/GLKMatrix4.h>

void JointServo::setServoNumber(int servoNumber)
{
    myServoId   = servoNumber;
    myLegId     = servoToLeg[servoNumber];
    myJointType = servoToJoint[servoNumber];
    
    minAngle    = (float) ((300 * mins[servoNumber] / 0x3ff) - 150) * M_PI / 180.0;
    maxAngle    = (float) ((300 * maxs[servoNumber] / 0x3ff) - 150) * M_PI / 180.0;
    
    nextJoint   = NULL;
}

void JointServo::set_ikSolution(float angle)
{
    if (isnan(angle) || angle <= minAngle || angle >= maxAngle)
    {
        servoOOR[myServoId] = 1;
    }
    else
    {
        servoOOR[myServoId] = 0;
        ikSolution = angle;
    }
}

GLKMatrix4 JointServo::getModelViewMatrix()
{
    return modelViewMatrix;
}

void JointServo::update()
{
    currentAngle = ikSolution;
    
    modelViewMatrix = GLKMatrix4Identity;
    
    GLKVector4 boneEnd;
    
    //rotate
    switch(myJointType)
    {
        case COXA_SERVO:
            boneEnd = GLKVector4Make(0, L_COXA, 0, 1.0);
            modelViewMatrix = GLKMatrix4RotateZ(modelViewMatrix, (currentAngle + naturalAngle - M_PI / 2));
            modelViewMatrix = GLKMatrix4Multiply(originTransform, modelViewMatrix);
            nextJoint->originTransform = GLKMatrix4TranslateWithVector4(modelViewMatrix, boneEnd);
            fkEndpoint = GLKMatrix4MultiplyVector4(modelViewMatrix, boneEnd);
            break;
        case FEMUR_SERVO:

            boneEnd = GLKVector4Make(0, L_FEMUR * cosf(-FEMUR_ANGLE), L_FEMUR * sinf(-FEMUR_ANGLE), 1.0);
            modelViewMatrix = GLKMatrix4RotateX(modelViewMatrix, currentAngle);
            modelViewMatrix = GLKMatrix4Multiply(originTransform, modelViewMatrix);
            nextJoint->originTransform = GLKMatrix4TranslateWithVector4(modelViewMatrix, boneEnd);
            fkEndpoint = GLKMatrix4MultiplyVector4(modelViewMatrix, boneEnd);
            break;
        default:

            boneEnd = GLKVector4Make(0, L_TIBIA * cosf(-TIBIA_ANGLE), L_TIBIA * sinf(-TIBIA_ANGLE), 1.0);
            modelViewMatrix = GLKMatrix4RotateX(modelViewMatrix, currentAngle);
            modelViewMatrix = GLKMatrix4Multiply(originTransform, modelViewMatrix);
            fkEndpoint = GLKMatrix4MultiplyVector4(modelViewMatrix, boneEnd);
            break;
    }

}