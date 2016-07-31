//
//  Leg.h
//  BugPoser
//
//  Created by Martin Lane-Smith on 5/24/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

#ifndef __BugPoser__Leg__
#define __BugPoser__Leg__

#include <stdio.h>
#include "JointServo.hpp"
#include "nuke.h"

class Leg {
public:
    int myLegId;
    
    JointServo coxa, femur, tibia;
    
    GLKVector4 endpoint;
    GLKVector4 coxaOrigin;
    GLKVector4 transformedCoxaOrigin;
    
    double naturalAngle;     //radians
    
    void setLegNumber(int leg);
    bool newEndpoint(GLKVector4 ep);
    
    void updateServos(GLKMatrix4 bodyTransform, bool forward);
    
    void inverseKinematics(GLKVector4 effector);
};

#endif /* defined(__BugPoser__Leg__) */
