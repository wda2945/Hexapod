//
//  main_debug.h
//
//  Created by Martin Lane-Smith on 5/18/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#ifndef main_debug_h
#define main_debug_h

extern FILE *mainDebugFile;

#ifdef MAIN_DEBUG
#define DEBUGPRINT(...) tprintf( __VA_ARGS__);tfprintf(mainDebugFile, __VA_ARGS__);
#else
#define DEBUGPRINT(...) tfprintf(mainDebugFile, __VA_ARGS__);
#endif

#define ERRORPRINT(...) tprintf(__VA_ARGS__);tfprintf(mainDebugFile, __VA_ARGS__);

#endif /* main_debug_h */
