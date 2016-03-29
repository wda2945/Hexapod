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
    
    Leg legs[LEG_COUNT];
    GLKVector3 bodyOffset;
    GLKVector3 bodyRotation;

    GLKMatrix4 bodyMatrix;
    
    Hexapod();
    
    void update();
    
    GLKMatrix4 getMatrix(int i);
};

extern Hexapod *theHexapod;

#endif /* defined(__BugPoser__Hexapod__) */
