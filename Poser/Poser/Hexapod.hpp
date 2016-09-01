//
//  Hexapod.h
//  BugPoser
//
//  Created by Martin Lane-Smith on 5/24/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

#ifndef __BugPoser__Hexapod__
#define __BugPoser__Hexapod__

#include <stdio.h>
#include "Leg.hpp"
#include "nuke.h"

class Hexapod {
public:
    
    GLKVector3 bodyOffset {0.0, 0.0, BODY_REST_HEIGHT};
    GLKVector3 bodyRotation {0.0, 0.0, 0.0};

    GLKMatrix4 bodyMatrix;
    
    Hexapod();
    
    void update();
    void reset(int leg_number);
    
    GLKMatrix4 getMatrix(int i);
    float getFloor();
    bool getSelected(int servo);
};

extern Leg legs[LEG_COUNT];
extern GLKVector3 legReset[LEG_COUNT];
extern Hexapod theHexapod;

#endif /* defined(__BugPoser__Hexapod__) */
