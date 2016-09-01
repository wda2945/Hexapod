//
//  GameViewController.h
//  Poser
//
//  Created by Martin Lane-Smith on 6/14/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

@interface GameViewController : GLKViewController

+ (GameViewController*) getGameViewController;
- (void) updateHUD;

- (void) setFilename: (NSString*) fileName;

@end
