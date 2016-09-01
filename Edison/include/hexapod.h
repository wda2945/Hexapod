//
//  hexapod.h
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef hexapod_h
#define hexapod_h

#include "software_profile.h"
#include "ps.h"
#include "messages.h"
#include "ps_message.h"
//---------------------Message Enums

#include "messages/MessageEnums.h"

#define EVENT_MACRO(e, n) e,

typedef enum {
#include "NotificationEventsList.h"
 EVENT_COUNT
} Event_enum;

#undef EVENT_MACRO

extern const char *eventNames[];

#define CONDITION_MACRO(e, n) e,

typedef enum {
#include "NotificationConditionsList.h"
 CONDITIONS_COUNT
} Condition_enum;

#undef CONDITION_MACRO

extern const char *conditionNames[];

//other name lookups
//extern char *batteryStateNames[BATTERY_STATUS_COUNT];

#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif /* hexapod_h */
