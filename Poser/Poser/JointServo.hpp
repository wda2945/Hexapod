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
#include "nuke.h"
#import <GLKit/GLKMathTypes.h>

#define FEMUR_ANGLE 0.26    //angle of femur bracket
#define TIBIA_ANGLE 1.57    //angle of tibia bracket. was 1.37 changed to match nuke

class JointServo {
    float ikSolution;
public:
    int myServoId;
    int myLegId;
    int myJointType;
    
    float naturalAngle {0};
    float currentAngle {0};
    
    float minAngle;
    float maxAngle;
    
    GLKVector4 fkEndpoint;
    
    GLKMatrix4 originTransform;
    GLKMatrix4 modelViewMatrix;

    JointServo  *nextJoint;
    
    void setServoNumber(int servoNumber);
    GLKMatrix4 getModelViewMatrix();
    void set_ikSolution(float angle);
    void update();
    bool isOOR() {return servoOOR[myServoId];}
};

#endif /* defined(__BugPoser__JointServo__) */
