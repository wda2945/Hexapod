//
//  JointServo.cpp
//  BugPoser
//
//  Created by Martin Lane-Smith on 5/24/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

#include "JointServo.hpp"
#include "nuke.h"
#include <GLKit/GLKMatrix4.h>

void JointServo::setServoNumber(int servoNumber)
{
    myServoId   = servoNumber;
    myLeg       = servoToLeg[servoNumber];
    myJoint     = servoToJoint[servoNumber];
    
    minAngle    = mins[servoNumber];
    maxAngle    = maxs[servoNumber];
    
    nextJoint   = NULL;
}

GLKMatrix4 JointServo::getModelViewMatrix()
{
    return modelViewMatrix;
}

void JointServo::update()
{
    currentAngle = ikSolution;
    
    //scale bone to size
    GLKMatrix4 mvMatrix = GLKMatrix4MakeScale(10, boneLength, 10);
    //transform to one end
    switch(myLeg)
    {
        case LEFT_FRONT:
        case LEFT_MIDDLE:
        case LEFT_REAR:
            mvMatrix = GLKMatrix4Translate(mvMatrix, 0, -boneLength / 2, 0);
            break;
        default:
            //right
            mvMatrix = GLKMatrix4Translate(mvMatrix, 0, boneLength / 2, 0);
            break;
    }
    //rotate
    switch(myJoint)
    {
        case COXA_SERVO:
            mvMatrix = GLKMatrix4RotateZ(mvMatrix, currentAngle * M_PI / 180.0);
            break;
        default:
            mvMatrix = GLKMatrix4RotateX(mvMatrix, currentAngle * M_PI / 180.0);
            break;
    }
    //transform to position
    modelViewMatrix = GLKMatrix4Translate(mvMatrix, origin.x, origin.y, origin.z);
    
    //position of nextJoint
    if (nextJoint)
    {
        GLKVector4 boneEnd;
        switch(myLeg)
        {
            case LEFT_FRONT:
            case LEFT_MIDDLE:
            case LEFT_REAR:
                boneEnd = GLKVector4Make(0, boneLength / 2, 0, 1.0);
                break;
            default:
                //right
                boneEnd = GLKVector4Make(0, -boneLength / 2, 0, 1.0);
                break;
        }
        
        nextJoint->origin = GLKMatrix4MultiplyVector4(modelViewMatrix, boneEnd);
    }
    
}