//
//  PickerViewController.h
//  Poser
//
//  Created by Martin Lane-Smith on 8/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface PickerViewController : UIViewController

- (void) initForOpen;
- (void) initForSave: (NSString*) filename;

@end
