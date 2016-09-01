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
    bool selected {false};
    
    JointServo coxa, femur, tibia;
    
    GLKVector4 endpoint;            //set endpoint
    GLKVector4 relativeEndpoint;    //endpoint relative to body (after body displacement)
    
    GLKVector4 coxaOrigin;
//    GLKVector4 transformedCoxaOrigin;
    
    double naturalAngle;     //radians
    
    void setLegNumber(int leg);
    bool newEndpoint(GLKVector4 ep);
    
    void updateServos(GLKMatrix4 bodyTransform);
    GLKVector4 bodyFK(int X, int Y, int Z,               //leg nominal position
                      int Xdisp, int Ydisp, float Zrot);
    void inverseKinematics(GLKVector4 effector);
};

#endif /* defined(__BugPoser__Leg__) */
