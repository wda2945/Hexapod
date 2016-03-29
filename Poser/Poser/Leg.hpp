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
    GLKVector4 endpoint, effector;
    GLKVector4 coxaOffset;
    
    void setLegNumber(int leg);
    void updateServos(GLKVector3 bodyOffset, GLKVector3 bodyRotation);
    void inverseKinematics();
};

#endif /* defined(__BugPoser__Leg__) */
