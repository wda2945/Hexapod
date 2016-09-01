//
//  PickerViewController.m
//  Poser
//
//  Created by Martin Lane-Smith on 8/19/16.
//  Copyright © 2016 Martin Lane-Smith. All rights reserved.
//

#import "PickerViewController.h"
#import "GameViewController.h"
#import "Hexapod.hpp"

@interface PickerViewController ()
{
    bool saving;
    NSString *fileName;
    NSArray *fileList;
}
@property (weak, nonatomic) IBOutlet UITextField *txtFileName;
- (IBAction)txtEditingDidEnd:(id)sender;
@property (weak, nonatomic) IBOutlet UITableView *tableView;
- (IBAction)OKbutton:(UIButton *)sender;

@end

@implementation PickerViewController

- (void) initForOpen
{
    saving = NO;
}
- (void) initForSave: (NSString*) filename
{
    saving = YES;
    fileName = [filename copy];
}

- (void) viewWillAppear:(BOOL)animated
{
    [super viewWillAppear:animated];
    
    if (saving)
    {
        _txtFileName.text = fileName;
    }
    else
    {
        _txtFileName.text = @"";
    }
    NSString *homeDirectory = NSHomeDirectory();
    NSString *docDirectory = [homeDirectory stringByAppendingPathComponent:@"Documents"];
    
    NSFileManager *fm = [NSFileManager defaultManager];
    fileList = [fm contentsOfDirectoryAtPath:docDirectory error:NULL];
    [_tableView reloadData];
}

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do any additional setup after loading the view from its nib.
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
    if (saving) return YES;
    else return NO;
}
- (IBAction)txtEditingDidEnd:(id)sender {
    fileName = _txtFileName.text;
}
- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    [textField resignFirstResponder];
    [self OKbutton:nil];
    return NO;
}

- (IBAction)OKbutton:(UIButton *)sender {
    if (saving)
    {
        fileName = _txtFileName.text;
        if (fileName.length == 0) return;
        NSString *homeDirectory = NSHomeDirectory();
        NSString *docDirectory = [homeDirectory stringByAppendingPathComponent:@"Documents"];
        NSString *docPath = [docDirectory stringByAppendingPathComponent: fileName];
        
        FILE *file = fopen(docPath.UTF8String, "w");
        
        if (file)
        {
            fprintf(file, "B %f %f %f %f %f %f\n",
                    theHexapod.bodyOffset.x,
                    theHexapod.bodyOffset.y,
                    theHexapod.bodyOffset.z,
                    theHexapod.bodyRotation.x,
                    theHexapod.bodyRotation.y,
                    theHexapod.bodyRotation.z);
            
            for (int i=0; i< 6; i++)
            {
                fprintf(file, "R %i %f %f %f\n", i,
                        legs[i].relativeEndpoint.x,
                        legs[i].relativeEndpoint.y,
                        legs[i].relativeEndpoint.z);
                fprintf(file, "L %i %f %f %f\n", i,
                        legs[i].endpoint.x,
                        legs[i].endpoint.y,
                        legs[i].endpoint.z);
            }
            fclose(file);
            [[GameViewController getGameViewController] setFilename:fileName];
            
        }
        else{
//            fileError.text = [NSString stringWithFormat:@"%s", strerror(errno)];
        }
    }
    else
    {
        if (fileName.length == 0) return;
        NSString *homeDirectory = NSHomeDirectory();
        NSString *docDirectory = [homeDirectory stringByAppendingPathComponent:@"Documents"];
        NSString *docPath = [docDirectory stringByAppendingPathComponent: fileName];
        
        FILE *file = fopen(docPath.UTF8String, "r");
        
        if (file)
        {
            char *lineptr = (char*) calloc(100,1);
            size_t n = 100;
            ssize_t count = 0;
            
            while (count >= 0)
            {
                count = getline(&lineptr, &n, file);
                
                if (count == 0) continue;
                
                char lineType[5] = "";
                
                sscanf(lineptr, "%1s", lineType);
                
                if (strcmp(lineType, "B") == 0)
                {
                    GLKVector3 location, rotation;
                    sscanf(lineptr, "%1s %f %f %f %f %f %f", lineType,
                           &location.x, &location.y, &location.z,
                           &rotation.x, &rotation.y, &rotation.z);
                    theHexapod.bodyOffset = location;
                    theHexapod.bodyRotation = rotation;
                }
                else if (strcmp(lineType, "L") == 0)
                {
                    int leg;
                    GLKVector4 endpoint;
                    sscanf(lineptr, "%1s %i %f %f %f ", lineType, &leg,
                           &endpoint.x, &endpoint.y, &endpoint.z);
                    endpoint.w = 1;
                    legs[leg].endpoint = endpoint;
                }
                //"R" lines not used
            }
            fclose(file);
            [[GameViewController getGameViewController] setFilename:fileName];
            [[GameViewController getGameViewController] updateHUD];
        }
        else{
//            fileError.text = [NSString stringWithFormat:@"%s", strerror(errno)];
        }
    }
    
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (IBAction)CancelButton:(UIButton *)sender {
    [self dismissViewControllerAnimated:YES completion:nil];
}

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView{
    return 1;
}

- (NSInteger)tableView:(UITableView *)tableView numberOfRowsInSection:(NSInteger)section {
    return fileList.count;
}

- (UITableViewCell *)tableView:(UITableView *)tv cellForRowAtIndexPath:(NSIndexPath *)indexPath{
    
    UITableViewCell *cell = [_tableView dequeueReusableCellWithIdentifier:@"fileCell"];
    
    if (!cell)
    {
        cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleValue1 reuseIdentifier:@"fileCell"];
    }

    cell.textLabel.text = [fileList objectAtIndex:indexPath.row];
    return cell;
}

- (NSIndexPath *)tableView:(UITableView *)tv willSelectRowAtIndexPath:(NSIndexPath *)indexPath
{
    _txtFileName.text = fileName = [fileList objectAtIndex:indexPath.row];
    return nil;
}

@end
