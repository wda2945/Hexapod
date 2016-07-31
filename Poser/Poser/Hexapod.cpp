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

const char *legNames[6] = {"RIGHT_FRONT","RIGHT_REAR","LEFT_FRONT","LEFT_REAR","RIGHT_MIDDLE","LEFT_MIDDLE"};

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
int coxaY[LEG_COUNT] = {-Y_COXA, -Y_COXA, Y_COXA, Y_COXA, -M_COXA, M_COXA};

Leg legs[LEG_COUNT];
GLKVector3 legReset[LEG_COUNT];
int servoOOR[SERVO_COUNT+1];

Hexapod theHexapod;

Hexapod::Hexapod()
{
    //endpoints
    legReset[RIGHT_FRONT].x = 200;
    legReset[RIGHT_FRONT].y = -200;
    legReset[RIGHT_FRONT].z = 0;
    
    legReset[RIGHT_REAR].x = -200;
    legReset[RIGHT_REAR].y = -200;
    legReset[RIGHT_REAR].z = 0;
    
    legReset[LEFT_FRONT].x = 200;
    legReset[LEFT_FRONT].y = 200;
    legReset[LEFT_FRONT].z = 0;
    
    legReset[LEFT_REAR].x = -200;
    legReset[LEFT_REAR].y = 200;
    legReset[LEFT_REAR].z = 0;
    
    legReset[RIGHT_MIDDLE].x = 0;
    legReset[RIGHT_MIDDLE].y = -280;
    legReset[RIGHT_MIDDLE].z = 0;
    
    legReset[LEFT_MIDDLE].x = 0;
    legReset[LEFT_MIDDLE].y = 280;
    legReset[LEFT_MIDDLE].z = 0;

    reset(-2);
    
    for (int i = 0; i < LEG_COUNT; i++)
    {
        legs[i].setLegNumber(i);
    }
    
    for (int i=0; i<=SERVO_COUNT; i++)
    {
        servoOOR[i] = 0;
    }
    
}
void Hexapod::reset(int leg_number)
{
    for (int i=0; i<6; i++)
    {
        if ((i == leg_number) || (leg_number == -1))
        {
            legs[i].endpoint.x = legReset[i].x;
            legs[i].endpoint.y = legReset[i].y;
            legs[i].endpoint.z = legReset[i].z;
        }
    }
    if ((leg_number == -1) || (leg_number == -2))
    {
        //body xyz
        bodyOffset  =  {0.0, 0.0, BODY_REST_HEIGHT};
    }
    if ((leg_number == -1) || (leg_number == -3))
    {
        //body rot
        bodyRotation = {0.0, 0.0, 0.0};
    }
    
}
void Hexapod::update()
{
    //update body
    
    bodyMatrix = GLKMatrix4MakeTranslation(bodyOffset.x, bodyOffset.y, bodyOffset.z);
    bodyMatrix = GLKMatrix4Rotate(bodyMatrix, bodyRotation.x, 1.0, 0.0, 0);
    bodyMatrix = GLKMatrix4Rotate(bodyMatrix, bodyRotation.y, 0, 1.0, 0);
    bodyMatrix = GLKMatrix4Rotate(bodyMatrix, bodyRotation.z, 0, 0, 1.0);
    
    for (int i = 0; i < LEG_COUNT; i++)
    {
        legs[i].updateServos(bodyMatrix, false);
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
