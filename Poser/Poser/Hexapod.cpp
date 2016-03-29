//
//  Hexapod.cpp
//  BugPoser
//
//  Created by Martin Lane-Smith on 5/24/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

#include "Hexapod.hpp"
#include "nuke.h"
#include <GLKit/GLKMatrix4.h>

/* min and max positions for each servo */
int mins[] = {0,
    222, 225, 159, 164, 279, 158, 223, 229, 159, 156, 272, 155, 226, 233, 158, 157, 271, 157};

int maxs[] = {0,
    790, 792, 855, 862, 857, 747, 788, 794, 859, 857, 860, 747, 789, 789, 858, 860, 859, 743};

//SERVO IDS
int servoIds[LEG_COUNT][3] = {
    {RF_COXA, RF_FEMUR, RF_TIBIA},
    {RR_COXA, RR_FEMUR, RR_TIBIA},
    {LF_COXA, LF_FEMUR, LF_TIBIA},
    {LR_COXA, LR_FEMUR, LR_TIBIA},
    {RM_COXA, RM_FEMUR, RM_TIBIA},
    {LM_COXA, LM_FEMUR, LM_TIBIA}
};

int servoToLeg[SERVO_COUNT+1] = {-1,
    LEFT_FRONT, RIGHT_FRONT, LEFT_FRONT, RIGHT_FRONT, LEFT_FRONT, RIGHT_FRONT,
    LEFT_REAR, RIGHT_REAR, LEFT_REAR, RIGHT_REAR, LEFT_REAR, RIGHT_REAR,
    LEFT_MIDDLE, RIGHT_MIDDLE, LEFT_MIDDLE, RIGHT_MIDDLE, LEFT_MIDDLE, RIGHT_MIDDLE};

int servoToJoint[SERVO_COUNT+1] = {-1,
    COXA_SERVO, COXA_SERVO, FEMUR_SERVO, FEMUR_SERVO, TIBIA_SERVO, TIBIA_SERVO,
    COXA_SERVO, COXA_SERVO, FEMUR_SERVO, FEMUR_SERVO, TIBIA_SERVO, TIBIA_SERVO,
    COXA_SERVO, COXA_SERVO, FEMUR_SERVO, FEMUR_SERVO, TIBIA_SERVO, TIBIA_SERVO};

//coxa locations relative to body center
int coxaX[LEG_COUNT] = {X_COXA, -X_COXA, X_COXA, -X_COXA, 0, 0};
int coxaY[LEG_COUNT] = {Y_COXA, Y_COXA, -Y_COXA, -Y_COXA, M_COXA, -M_COXA};

Hexapod *theHexapod;

Hexapod::Hexapod()
{
    theHexapod = this;
    
    bodyOffset = bodyRotation = {0,0,0};
    
    bodyMatrix = GLKMatrix4MakeScale(X_COXA * 2.0, Y_COXA * 2.0, 50.0);
    
    legs[RIGHT_FRONT].endpoint.x = 52;
    legs[RIGHT_FRONT].endpoint.y = 118;
    legs[RIGHT_FRONT].endpoint.z = 97;
    
    legs[RIGHT_REAR].endpoint.x = -52;
    legs[RIGHT_REAR].endpoint.y = 118;
    legs[RIGHT_REAR].endpoint.z = 97;
    
    legs[RIGHT_MIDDLE].endpoint.x = 0;
    legs[RIGHT_MIDDLE].endpoint.y = 118;
    legs[RIGHT_MIDDLE].endpoint.z = 97;
    
    legs[LEFT_MIDDLE].endpoint.x = 0;
    legs[LEFT_MIDDLE].endpoint.y = -118;
    legs[LEFT_MIDDLE].endpoint.z = 97;
    
    legs[LEFT_FRONT].endpoint.x = 52;
    legs[LEFT_FRONT].endpoint.y = -118;
    legs[LEFT_FRONT].endpoint.z = 97;
    
    legs[LEFT_REAR].endpoint.x = -52;
    legs[LEFT_REAR].endpoint.y = -118;
    legs[LEFT_REAR].endpoint.z = 97;
    
    for (int i = 0; i < LEG_COUNT; i++)
    {
        legs[i].setLegNumber(i);
        legs[i].inverseKinematics();
    }
}

void Hexapod::update()
{
    //update body
    for (int i = 0; i < LEG_COUNT; i++)
    {
        legs[i].updateServos(bodyOffset, bodyRotation);
    }
}
GLKMatrix4 Hexapod::getMatrix(int servo)
{
    switch(servoToJoint[servo])
    {
        case -1:
            //body
            return bodyMatrix;
            break;
        case COXA_SERVO:
            return legs[servoToLeg[servo]].coxa.getModelViewMatrix();
            break;
        case FEMUR_SERVO:
            return legs[servoToLeg[servo]].femur.getModelViewMatrix();
            break;
        case TIBIA_SERVO:
            return legs[servoToLeg[servo]].tibia.getModelViewMatrix();
            break;
        default:
            return GLKMatrix4Identity;
    }
    
}
