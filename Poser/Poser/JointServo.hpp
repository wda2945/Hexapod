//
//  JointServo.h
//  BugPoser
//
//  Created by Martin Lane-Smith on 5/24/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

#ifndef __BugPoser__JointServo__
#define __BugPoser__JointServo__

#include <stdio.h>
#import <GLKit/GLKMathTypes.h>

class JointServo {
public:
    int myServoId;
    int myLeg;
    int myJoint;
    
    GLKVector4 origin;
//    GLKMatrix4 originTransform;
    GLKMatrix4 modelViewMatrix;
    
    int maxAngle, minAngle;
    int boneLength;
    
    int currentAngle;
    int ikSolution;
    
    JointServo *nextJoint;
    
    void setServoNumber(int servoNumber);
    
    GLKMatrix4 getModelViewMatrix();
    
    void update();
};



#endif /* defined(__BugPoser__JointServo__) */
