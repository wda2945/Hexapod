//
//  GameViewController.m
//  Poser
//
//  Created by Martin Lane-Smith on 6/14/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

#import "GameViewController.h"
#import "PickerViewController.h"
#import <OpenGLES/ES2/glext.h>
#import "Hexapod.hpp"
#include <vector>
#include <string>
#include <errno.h>
#include <string.h>

//GL debug macro
#define OGLESdebug {int OGLESE = glGetError();\
if (OGLESE != GL_NO_ERROR) NSLog(@"Error: %0x\n", OGLESE);assert( OGLESE == GL_NO_ERROR);}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

// Uniform index.
enum
{
    UNIFORM_PROJECTION_MATRIX,
    UNIFORM_MODELVIEW_MATRIX,
    UNIFORM_NORMAL_MATRIX,
    UNIFORM_SELECTED_SHAPE,
    UNIFORM_OOR,
    NUM_UNIFORMS
};
GLint uniforms[NUM_UNIFORMS];

// Attribute index.
enum
{
    ATTRIB_VERTEX,
    ATTRIB_NORMAL,
    ATTRIB_BONE,
    NUM_ATTRIBUTES
};

#define MATRIX_COUNT   20

@interface GameViewController () {
    GLuint _program;
    
    __weak IBOutlet GLKView *glkView;
    
    GLKMatrix4 _projectionMatrix;
    GLKMatrix4 _modelViewMatrix[MATRIX_COUNT];
    GLKMatrix3 _normalMatrix[MATRIX_COUNT];
    int        selectedShape[MATRIX_COUNT];
    
    GLKMatrix4 baseModelViewMatrix;
    float aspectRatio;
    
    GLuint _vertexArray;
    GLuint _vertexBuffer;
    
    CGPoint panBegan;
    int     panTouches;
    CGPoint panBeganViewRotation;
    CGPoint viewRotation;
    
    GLKVector3 panBeganBodyOffset;
    GLKVector3 panBeganBodyRotation;
    GLKVector4 panBeganLegEndpoint[6];
    
    int triangle_vertex_count;
    int firstFace[MATRIX_COUNT];
    int faceCount[MATRIX_COUNT];
    
    UIButton *activeButton;
    bool bodyRotateActive;
    
    UIButton *legButtons[6];
    
    __weak IBOutlet UIButton *bodyTranslate;
    __weak IBOutlet UIButton *bodyRotate;
    __weak IBOutlet UIButton *RFbutton;
    __weak IBOutlet UIButton *RMbutton;
    __weak IBOutlet UIButton *RRbutton;
    __weak IBOutlet UIButton *LFbutton;
    __weak IBOutlet UIButton *LMbutton;
    __weak IBOutlet UIButton *LRbutton;
    __weak IBOutlet UILabel *RFC;
    __weak IBOutlet UILabel *RFF;
    __weak IBOutlet UILabel *RFT;
    __weak IBOutlet UILabel *RMC;
    __weak IBOutlet UILabel *RMF;
    __weak IBOutlet UILabel *RMT;
    __weak IBOutlet UILabel *RRC;
    __weak IBOutlet UILabel *RRF;
    __weak IBOutlet UILabel *RRT;
    __weak IBOutlet UILabel *LFC;
    __weak IBOutlet UILabel *LFF;
    __weak IBOutlet UILabel *LFT;
    __weak IBOutlet UILabel *LMC;
    __weak IBOutlet UILabel *LMF;
    __weak IBOutlet UILabel *LMT;
    __weak IBOutlet UILabel *LRC;
    __weak IBOutlet UILabel *LRF;
    __weak IBOutlet UILabel *LRT;
    __weak IBOutlet UIButton *reset_button;
    __weak IBOutlet UILabel *lblFilename;
    
    PickerViewController *pickerView;
}

- (IBAction)resetButton:(UIButton*)sender;
- (IBAction)openButton:(UIButton*)sender;
- (IBAction)saveButton:(UIButton*)sender;
- (IBAction)translateButton:(UIButton*)sender;
- (IBAction)rotateButton:(UIButton*)sender;

@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;

- (IBAction) lf_button:(UIButton *)sender;
- (IBAction) lm_button:(UIButton *)sender;
- (IBAction) lr_button:(UIButton *)sender;
- (IBAction) rf_button:(UIButton *)sender;
- (IBAction) rm_button:(UIButton *)sender;
- (IBAction) rr_button:(UIButton *)sender;

- (void) setupGL;
- (void) tearDownGL;

- (BOOL) loadShaders;
- (BOOL) compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file;
- (BOOL) linkProgram:(GLuint)prog;
- (BOOL) validateProgram:(GLuint)prog;

- (void) panAction: (UIPanGestureRecognizer*) pan;
- (void) updateLeg: (int) leg selected: (bool) selected;
- (void) unselectAll;

@end

@implementation GameViewController

GameViewController* gameViewController;

+ (GameViewController*) getGameViewController
{
    return gameViewController;
}

- (void) setFilename: (NSString*) fileName
{
    lblFilename.text = fileName;
}

- (void)viewDidLoad
{
    gameViewController = self;
    activeButton = nil;
    viewRotation = CGPointZero;
    
    [super viewDidLoad];
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    glkView.context = self.context;
    glkView.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    UITapGestureRecognizer *tap = [[UITapGestureRecognizer alloc] initWithTarget:self action:@selector(tapAction:)];
     [self.view addGestureRecognizer:tap];
    
    UIPanGestureRecognizer *pan = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(panAction:)];
    pan.maximumNumberOfTouches = 4;
    pan.minimumNumberOfTouches = 1;
    [self.view addGestureRecognizer:pan];
    
    legButtons[RIGHT_FRONT] = RFbutton;
    legButtons[RIGHT_MIDDLE] = RMbutton;
    legButtons[RIGHT_REAR] = RRbutton;
    legButtons[LEFT_FRONT] = LFbutton;
    legButtons[LEFT_MIDDLE] = LMbutton;
    legButtons[LEFT_REAR] = LRbutton;
    
    [self setupGL];
    [self resetButton:nil];
    lblFilename.text = @"unnamed";
    [self translateButton: bodyTranslate];
    
    pickerView = [[PickerViewController alloc] initWithNibName:@"PickerViewController" bundle:NULL];
}

- (void)dealloc
{    
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];

    if ([self isViewLoaded] && ([[self view] window] == nil)) {
        self.view = nil;
        
        [self tearDownGL];
        
        if ([EAGLContext currentContext] == self.context) {
            [EAGLContext setCurrentContext:nil];
        }
        self.context = nil;
    }

    // Dispose of any resources that can be recreated.
}

- (BOOL)prefersStatusBarHidden {
    return YES;
}

- (void) updateLeg: (int) leg selected: (bool) selected
{
    if (selected)
    {
        legs[leg].selected = false;
        legButtons[leg].selected = NO;
    }
    else
    {
        legs[leg].selected = true;
        legButtons[leg].selected = YES;
    }
}

- (IBAction)rf_button:(UIButton *)sender {
    [self updateLeg: RIGHT_FRONT selected: sender.selected];
}

- (IBAction)rm_button:(UIButton *)sender {
    [self updateLeg: RIGHT_MIDDLE selected: sender.selected];
}

- (IBAction)rr_button:(UIButton *)sender {
    [self updateLeg: RIGHT_REAR selected: sender.selected];
}

- (IBAction)lf_button:(UIButton *)sender {
    [self updateLeg: LEFT_FRONT selected: sender.selected];
}

- (IBAction)lm_button:(UIButton *)sender {
    [self updateLeg: LEFT_MIDDLE selected: sender.selected];
}

- (IBAction)lr_button:(UIButton *)sender {
    [self updateLeg: LEFT_REAR selected: sender.selected];
}

- (void) unselectAll
{
    for (int i=0; i<6; i++)
    {
        legs[i].selected = false;
        legButtons[i].selected = NO;
    }
}

- (IBAction)resetButton:(UIButton*)sender {
    theHexapod.reset(-1);
    [self updateHUD];
}

- (IBAction)openButton:(UIButton*)sender {
    [pickerView initForOpen];
    
    pickerView.modalPresentationStyle = UIModalPresentationFormSheet;
    
    // Get the popover presentation controller and configure it.
    UIPopoverPresentationController *presentationController = [pickerView popoverPresentationController];
    presentationController.sourceView = self.view;
    
    [self presentViewController:pickerView animated: YES completion: nil];
}

- (IBAction)saveButton:(UIButton*)sender {
    [pickerView initForSave: lblFilename.text];
    
    pickerView.modalPresentationStyle = UIModalPresentationFormSheet;
    
    // Get the popover presentation controller and configure it.
    UIPopoverPresentationController *presentationController = [pickerView popoverPresentationController];
    presentationController.sourceView = self.view;
    
    [self presentViewController:pickerView animated: YES completion: nil];
}

- (IBAction)translateButton:(UIButton*)sender {
    bodyRotateActive = NO;
    bodyTranslate.selected = YES;
    bodyRotate.selected = NO;
}

- (IBAction)rotateButton:(UIButton*)sender {
    bodyRotateActive = YES;
    bodyTranslate.selected = NO;
    bodyRotate.selected = YES;
}

- (void) tapAction: (UITapGestureRecognizer*) tap
{
    if (tap.state == UIGestureRecognizerStateEnded)
    {
        [self unselectAll];
        
        CGPoint tapLocation = [tap locationInView:self.view];

        NSLog(@"Tap Location: %f, %f", tapLocation.x, tapLocation.y);
        
        CGPoint viewspace = CGPointMake(tapLocation.x - glkView.bounds.size.width/2, -tapLocation.y + glkView.bounds.size.height/2);
        
//        CGPoint viewspace = CGPointMake((2 * tapLocation.x / glkView.bounds.size.width) - 1,
//                                        -((2 * tapLocation.y / glkView.bounds.size.height) - 1));
        NSLog(@"Viewspace: %f, %f", viewspace.x, viewspace.y);
        
        double dsq = 1000000;
        int leg = -1;
        for (int i=0; i<6; i++)
        {
            //adjust for axes differences
            GLKVector4 adjEndpoint;
            adjEndpoint.x =  legs[i].tibia.fkEndpoint.x;
            adjEndpoint.y =  legs[i].tibia.fkEndpoint.y;
            adjEndpoint.z = -legs[i].tibia.fkEndpoint.z;
            
            //convert endpoint to screen coordinates
            GLKVector4 ep = GLKMatrix4MultiplyVector4(GLKMatrix4Multiply(_projectionMatrix, baseModelViewMatrix), adjEndpoint);
            
            double range = (ep.x - viewspace.x)*(ep.x - viewspace.x)
            + (ep.y - viewspace.y)*(ep.y - viewspace.y);
            
            NSLog(@"Leg %i @ %f, %f, %f. screen: %f, %f", i, adjEndpoint.x, adjEndpoint.y, adjEndpoint.z, ep.x, ep.y);
            
            if (range < dsq)
            {
                dsq = range;
                leg = i;
            }
        }
        if (dsq < 300 && leg >= 0) legs[leg].selected = true;
    }
}

- (void) panAction: (UIPanGestureRecognizer*) pan
{
    switch(pan.state)
    {
        case UIGestureRecognizerStateBegan:
            panTouches = (int) pan.numberOfTouches;
            panBegan = [pan translationInView:self.view];
            switch (panTouches)
        {
            case 2:
                for (int i=0; i< 6; i++)
                {
                    panBeganLegEndpoint[i]  = legs[i].endpoint;
                }
                break;
            case 1:     //view
                panBeganViewRotation = viewRotation;
                break;
            case 3:
                if (bodyRotateActive)
                {
                    //body rot
                    panBeganBodyRotation = theHexapod.bodyRotation;
                }
                else
                {
                    //body xyz
                    panBeganBodyOffset   = theHexapod.bodyOffset;
                }
                break;
        }
            break;
        case UIGestureRecognizerStateChanged:
        {
            CGPoint panChanged = [pan translationInView:self.view];
            float xChange = panChanged.x - panBegan.x;
            float yChange = panChanged.y - panBegan.y;
            switch (panTouches)
            {
                case 2:
                    for (int i=0; i< 6; i++)
                    {
                        if (legs[i].selected)
                        {
                            GLKVector4 panNewLegEndpoint;
                            panNewLegEndpoint.x = panBeganLegEndpoint[i].x + cosf(viewRotation.x) * xChange / 2;
                            panNewLegEndpoint.y = panBeganLegEndpoint[i].y + sinf(viewRotation.x) * xChange / 2;
                            panNewLegEndpoint.z = panBeganLegEndpoint[i].z + yChange / 2;
                            legs[i].newEndpoint(panNewLegEndpoint);
                        }
                    }
                    break;
                case 1:     //view
                    viewRotation.x = panBeganViewRotation.x + xChange / 100;
                    break;
                case 3:     //body rot
                    if (bodyRotateActive)
                    {
                        theHexapod.bodyRotation.z = panBeganBodyRotation.z + xChange / 500;
                        theHexapod.bodyRotation.x = panBeganBodyRotation.x + cosf(viewRotation.x) * yChange / 500;
                        theHexapod.bodyRotation.y = panBeganBodyRotation.y - sinf(viewRotation.x) * yChange / 500;
                    }
                    else
                    {
                        theHexapod.bodyOffset.z = panBeganBodyOffset.z - yChange / 2;
                        theHexapod.bodyOffset.x = panBeganBodyOffset.x + cosf(viewRotation.x) * xChange / 2;
                        theHexapod.bodyOffset.y = panBeganBodyOffset.y - sinf(viewRotation.x) * xChange / 2;
                    }
                    break;
            }
            [self updateHUD];
        }
            break;
        case UIGestureRecognizerStateEnded:
        case UIGestureRecognizerStateCancelled:
        default:
            break;
    }
}

- (void) updateHUD
{
    theHexapod.update();
    
    RFC.text = [NSString stringWithFormat:@"%i", (int) (legs[RIGHT_FRONT].coxa.currentAngle * 180 / M_PI)];
    RFF.text = [NSString stringWithFormat:@"%i", (int) (legs[RIGHT_FRONT].femur.currentAngle * 180 / M_PI)];
    RFT.text = [NSString stringWithFormat:@"%i", (int) (legs[RIGHT_FRONT].tibia.currentAngle * 180 / M_PI)];
    
    RMC.text = [NSString stringWithFormat:@"%i", (int) (legs[RIGHT_MIDDLE].coxa.currentAngle * 180 / M_PI)];
    RMF.text = [NSString stringWithFormat:@"%i", (int) (legs[RIGHT_MIDDLE].femur.currentAngle * 180 / M_PI)];
    RMT.text = [NSString stringWithFormat:@"%i", (int) (legs[RIGHT_MIDDLE].tibia.currentAngle * 180 / M_PI)];
    
    RRC.text = [NSString stringWithFormat:@"%i", (int) (legs[RIGHT_REAR].coxa.currentAngle * 180 / M_PI)];
    RRF.text = [NSString stringWithFormat:@"%i", (int) (legs[RIGHT_REAR].femur.currentAngle * 180 / M_PI)];
    RRT.text = [NSString stringWithFormat:@"%i", (int) (legs[RIGHT_REAR].tibia.currentAngle * 180 / M_PI)];
    
    LFC.text = [NSString stringWithFormat:@"%i", (int) (legs[LEFT_FRONT].coxa.currentAngle * 180 / M_PI)];
    LFF.text = [NSString stringWithFormat:@"%i", (int) (legs[LEFT_FRONT].femur.currentAngle * 180 / M_PI)];
    LFT.text = [NSString stringWithFormat:@"%i", (int) (legs[LEFT_FRONT].tibia.currentAngle * 180 / M_PI)];
    
    LMC.text = [NSString stringWithFormat:@"%i", (int) (legs[LEFT_MIDDLE].coxa.currentAngle * 180 / M_PI)];
    LMF.text = [NSString stringWithFormat:@"%i", (int) (legs[LEFT_MIDDLE].femur.currentAngle * 180 / M_PI)];
    LMT.text = [NSString stringWithFormat:@"%i", (int) (legs[LEFT_MIDDLE].tibia.currentAngle * 180 / M_PI)];
    
    LRC.text = [NSString stringWithFormat:@"%i", (int) (legs[LEFT_REAR].coxa.currentAngle * 180 / M_PI)];
    LRF.text = [NSString stringWithFormat:@"%i", (int) (legs[LEFT_REAR].femur.currentAngle * 180 / M_PI)];
    LRT.text = [NSString stringWithFormat:@"%i", (int) (legs[LEFT_REAR].tibia.currentAngle * 180 / M_PI)];
}

- (void)setupGL
{
    typedef struct {
        float x,y,z;
    } vertex_t;
    
    std::vector<vertex_t> vertices;
    std::vector<vertex_t> normals;
    
    typedef struct {
        int v1, v2, v3;
        int n1, n2, n3;
    } triangle_t;
    
    std::vector<triangle_t> faces;

    enum {BODY, COXA, FEMUR, TIBIA} currentElement = BODY;
    
    int bodyStart, coxaStart, femurStart, tibiaStart;
    int bodySize {0};
    int coxaSize {0};
    int femurSize {0};
    int tibiaSize {0};
    
#define max(a,b) (a > b ? a : b)
    
    FILE *objfile;
    NSString *path = [[NSBundle mainBundle] pathForResource:@"hex_pieces" ofType:@"obj"];
    
    objfile = fopen(path.UTF8String, "r");
    
    if (!objfile)
    {
        NSLog(@"Failed to open %@", path);
    }
    else
    {
        char *lineptr = (char*) calloc(100,1);
        size_t n = 100;
        ssize_t count = 0;
        
        while (count >= 0)
        {
            count = getline(&lineptr, &n, objfile);
            
            if (count == 0) continue;
            
            char lineType[5] = "";
            
            sscanf(lineptr, "%3s", lineType);
            
            if (strcmp(lineType, "v") == 0)     //vertex
            {
                float x {0};
                float y {0};
                float z {0};
                
                sscanf(lineptr, "%3s %f %f %f", lineType, &x, &y, &z);
                
                vertex_t v {x, y, z};
                vertices.push_back(v);
            }
            else if (strcmp(lineType, "vn") == 0)     //vertex
            {
                float x {0};
                float y {0};
                float z {0};
                
                sscanf(lineptr, "%3s %f %f %f", lineType, &x, &y, &z);
                
                vertex_t v {x, y, z};
                normals.push_back(v);
            }
            else if (strcmp(lineType, "f") == 0)     //face
            {
                int v1 {0};
                int t1 {0};
                int n1 {0};
                int v2 {0};
                int t2 {0};
                int n2 {0};
                int v3 {0};
                int t3 {0};
                int n3 {0};
                
                sscanf(lineptr, "%3s %i/%i/%i %i/%i/%i %i/%i/%i",
                       lineType, &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
                
                triangle_t f {v1-1, v2-1, v3-1, n1-1, n2-1, n3-1};
                faces.push_back(f);
                
                if ((max(v1, max(v2,v3)) > vertices.size()) || (max(n1, max(n2,n3)) > normals.size()))
                {
                    NSLog(@"OOR: %s", lineptr);
                }

                switch(currentElement)
                {
                    case BODY:
                        bodySize++;
                        break;
                    case COXA:
                        coxaSize++;
                        break;
                    case FEMUR:
                        femurSize++;
                        break;
                    case TIBIA:
                        tibiaSize++;
                        break;
                }
            }
            else if (strcmp(lineType, "g") == 0)     //group
            {
                if (strstr(lineptr, "body"))
                {
                    currentElement = BODY;
                    bodyStart = (int) faces.size();
                }
                else if (strstr(lineptr, "coxa"))
                {
                    currentElement = COXA;
                    coxaStart = (int) faces.size();
                }
                else if (strstr(lineptr, "femur"))
                {
                    currentElement = FEMUR;
                    femurStart = (int) faces.size();
                }
                else if (strstr(lineptr, "tibia"))
                {
                    currentElement = TIBIA;
                    tibiaStart = (int) faces.size();
                }
            }
        }
        
        fclose(objfile);
    }
    
    NSLog(@"%i vertices", (int) vertices.size());
    NSLog(@"%i normals", (int) normals.size());
    NSLog(@"%i faces", (int) faces.size());

    NSLog(@"body: %i faces from %i", bodySize, bodyStart);
    NSLog(@"coxa: %i faces from %i", coxaSize, coxaStart);
    NSLog(@"femur: %i faces from %i", femurSize, femurStart);
    NSLog(@"tibia: %i faces from %i", tibiaSize, tibiaStart);
    
    triangle_vertex_count = 0;
#define TRIANGLES        (bodySize + 6 * (coxaSize + femurSize + tibiaSize) + 2)
#define VERTEX_COUNT     (TRIANGLES * 3)
#define VERTEX_DATA_SIZE (VERTEX_COUNT * 7)
    
    GLfloat *vertexData = (GLfloat*) calloc(sizeof(GLfloat), VERTEX_DATA_SIZE);
    NSAssert(vertexData, @"calloc fail");
    
    GLfloat *nextVertexData = vertexData;
    
    for (int m=0; m < MATRIX_COUNT-1; m++)
    {
        int start, count;
        
        switch(servoToJoint[m])
        {
            case COXA_SERVO:
                start = coxaStart;
                count = coxaSize;
                break;
            case FEMUR_SERVO:
                start = femurStart;
                count = femurSize;
                break;
            case TIBIA_SERVO:
                start = tibiaStart;
                count = tibiaSize;
                break;
            default:
            case -1:
                start = bodyStart;
                count = bodySize;
                break;
        }

        for (int f = start; f < (start+count); f++)
        {
            triangle_t t = faces[f];
            vertex_t v[3] = {vertices[t.v1], vertices[t.v2], vertices[t.v3]};
            vertex_t n[3] = {normals[t.n1], normals[t.n2], normals[t.n3]};
            
            
            for (int i=0; i<3; i++)
            {
                triangle_vertex_count++;
                *nextVertexData++ = v[i].x;
                *nextVertexData++ = v[i].y;
                *nextVertexData++ = v[i].z;
                *nextVertexData++ = n[i].x;
                *nextVertexData++ = n[i].y;
                *nextVertexData++ = n[i].z;
                *nextVertexData++ = m;
            }
        }
    }
    
#define FLOOR_SIZE  500
    triangle_vertex_count++;
    *nextVertexData++ = FLOOR_SIZE;
    *nextVertexData++ = FLOOR_SIZE;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 1;
    *nextVertexData++ = MATRIX_COUNT-1;
    
    triangle_vertex_count++;
    *nextVertexData++ = FLOOR_SIZE;
    *nextVertexData++ = -FLOOR_SIZE;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 1;
    *nextVertexData++ = MATRIX_COUNT-1;
    
    triangle_vertex_count++;
    *nextVertexData++ = -FLOOR_SIZE;
    *nextVertexData++ = -FLOOR_SIZE;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 1;
    *nextVertexData++ = MATRIX_COUNT-1;

    triangle_vertex_count++;
    *nextVertexData++ = -FLOOR_SIZE;
    *nextVertexData++ = FLOOR_SIZE;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 1;
    *nextVertexData++ = MATRIX_COUNT-1;

    triangle_vertex_count++;
    *nextVertexData++ = FLOOR_SIZE;
    *nextVertexData++ = FLOOR_SIZE;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 1;
    *nextVertexData++ = MATRIX_COUNT-1;

    triangle_vertex_count++;
    *nextVertexData++ = -FLOOR_SIZE;
    *nextVertexData++ = -FLOOR_SIZE;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 0;
    *nextVertexData++ = 1;
    *nextVertexData++ = MATRIX_COUNT-1;

    
    [glkView bindDrawable];
    [EAGLContext setCurrentContext:self.context];
    
    [self loadShaders];
    NSAssert([self validateProgram:_program], @"Shader problem");
    
    glEnable(GL_DEPTH_TEST);
    
    glGenVertexArraysOES(1, &_vertexArray);
    glBindVertexArrayOES(_vertexArray);
    
    glGenBuffers(1, &_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, _vertexBuffer);
    
#define STRIDE (sizeof(GLfloat) * 7)
    glBufferData(GL_ARRAY_BUFFER, VERTEX_DATA_SIZE * sizeof(GLfloat), vertexData, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    glVertexAttribPointer(ATTRIB_VERTEX, 3, GL_FLOAT, GL_FALSE, STRIDE, BUFFER_OFFSET(0));
    
    glEnableVertexAttribArray(ATTRIB_NORMAL);
    glVertexAttribPointer(ATTRIB_NORMAL, 3, GL_FLOAT, GL_FALSE, STRIDE, BUFFER_OFFSET(12));
    
    glEnableVertexAttribArray(ATTRIB_BONE);
    glVertexAttribPointer(ATTRIB_BONE, 1, GL_FLOAT, GL_FALSE, STRIDE, BUFFER_OFFSET(24));
    
    glBindVertexArrayOES(0);
    
    OGLESdebug
    
    free(vertexData);
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];
    
    glDeleteBuffers(1, &_vertexBuffer);
    glDeleteVertexArraysOES(1, &_vertexArray);
    
    if (_program) {
        glDeleteProgram(_program);
        _program = 0;
    }
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
    aspectRatio = fabs(glkView.bounds.size.width / glkView.bounds.size.height);
    
    _projectionMatrix = GLKMatrix4MakePerspective(GLKMathDegreesToRadians(65.0f), aspectRatio, 0.1f, 1500.0f);
    
    baseModelViewMatrix = GLKMatrix4MakeTranslation(0.0f, 0.0f, -600.0f);
    baseModelViewMatrix = GLKMatrix4Rotate(baseModelViewMatrix, -45.0, 1.0f, 0.0f, 0.0f);
    baseModelViewMatrix = GLKMatrix4Rotate(baseModelViewMatrix, viewRotation.x, 0.0f, 0.0f, 1.0f);
    
    for (int i=0; i<MATRIX_COUNT-1; i++)
    {
        GLKMatrix4 modelViewMatrix = theHexapod.getMatrix(i);
        _modelViewMatrix[i] = GLKMatrix4Multiply(baseModelViewMatrix, modelViewMatrix);
        _normalMatrix[i] = GLKMatrix3InvertAndTranspose(GLKMatrix4GetMatrix3(_modelViewMatrix[i]), NULL);
        selectedShape[i] = theHexapod.getSelected(i);
    }
    
    _modelViewMatrix[MATRIX_COUNT-1] = GLKMatrix4Translate(baseModelViewMatrix, 0.0, 0.0, -theHexapod.getFloor());
    _normalMatrix[MATRIX_COUNT-1] = GLKMatrix3InvertAndTranspose(GLKMatrix4GetMatrix3(_modelViewMatrix[MATRIX_COUNT-1]), NULL);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glBindVertexArrayOES(_vertexArray);
    
    glUseProgram(_program);
    
    glUniformMatrix4fv(uniforms[UNIFORM_PROJECTION_MATRIX], 1, 0, _projectionMatrix.m);
    
    glUniformMatrix4fv(uniforms[UNIFORM_MODELVIEW_MATRIX], MATRIX_COUNT, 0, _modelViewMatrix[0].m);
    glUniformMatrix3fv(uniforms[UNIFORM_NORMAL_MATRIX], MATRIX_COUNT, 0, _normalMatrix[0].m);
    
    glUniform1iv(uniforms[UNIFORM_SELECTED_SHAPE], MATRIX_COUNT, selectedShape);
    glUniform1iv(uniforms[UNIFORM_OOR], MATRIX_COUNT, servoOOR);
 
    glDrawArrays(GL_TRIANGLES, 0, triangle_vertex_count);
    
    OGLESdebug
}


#pragma mark -  OpenGL ES 2 shader compilation

- (BOOL)loadShaders
{
    GLuint vertShader, fragShader;
    NSString *vertShaderPathname, *fragShaderPathname;
    
    // Create shader program.
    _program = glCreateProgram();
    
    // Create and compile vertex shader.
    vertShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"vsh"];
    if (![self compileShader:&vertShader type:GL_VERTEX_SHADER file:vertShaderPathname]) {
        NSLog(@"Failed to compile vertex shader");
        return NO;
    }
    
    // Create and compile fragment shader.
    fragShaderPathname = [[NSBundle mainBundle] pathForResource:@"Shader" ofType:@"fsh"];
    if (![self compileShader:&fragShader type:GL_FRAGMENT_SHADER file:fragShaderPathname]) {
        NSLog(@"Failed to compile fragment shader");
        return NO;
    }
    
    // Attach vertex shader to program.
    glAttachShader(_program, vertShader);
    
    // Attach fragment shader to program.
    glAttachShader(_program, fragShader);
    
    // Bind attribute locations.
    // This needs to be done prior to linking.
    glBindAttribLocation(_program, ATTRIB_VERTEX, "position");
    glBindAttribLocation(_program, ATTRIB_NORMAL, "normal");
    glBindAttribLocation(_program, ATTRIB_BONE, "bone");
    
    // Link program.
    if (![self linkProgram:_program]) {
        NSLog(@"Failed to link program: %d", _program);
        
        if (vertShader) {
            glDeleteShader(vertShader);
            vertShader = 0;
        }
        if (fragShader) {
            glDeleteShader(fragShader);
            fragShader = 0;
        }
        if (_program) {
            glDeleteProgram(_program);
            _program = 0;
        }
        
        return NO;
    }
    
    // Get uniform locations.
    uniforms[UNIFORM_PROJECTION_MATRIX] = glGetUniformLocation(_program, "projectionMatrix");
    uniforms[UNIFORM_MODELVIEW_MATRIX] = glGetUniformLocation(_program, "modelViewMatrix");
    uniforms[UNIFORM_NORMAL_MATRIX] = glGetUniformLocation(_program, "normalMatrix");
    uniforms[UNIFORM_SELECTED_SHAPE] = glGetUniformLocation(_program, "selectedShape");
    uniforms[UNIFORM_OOR] = glGetUniformLocation(_program, "OOR");
    
    // Release vertex and fragment shaders.
    if (vertShader) {
        glDetachShader(_program, vertShader);
        glDeleteShader(vertShader);
    }
    if (fragShader) {
        glDetachShader(_program, fragShader);
        glDeleteShader(fragShader);
    }
    
    return YES;
}

- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file
{
    GLint status;
    const GLchar *source;
    
    source = (GLchar *)[[NSString stringWithContentsOfFile:file encoding:NSUTF8StringEncoding error:nil] UTF8String];
    if (!source) {
        NSLog(@"Failed to load vertex shader");
        return NO;
    }
    
    *shader = glCreateShader(type);
    glShaderSource(*shader, 1, &source, NULL);
    glCompileShader(*shader);
    
#if defined(DEBUG)
    GLint logLength;
    glGetShaderiv(*shader, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetShaderInfoLog(*shader, logLength, &logLength, log);
        NSLog(@"Shader compile log:\n%s", log);
        free(log);
    }
#endif
    
    glGetShaderiv(*shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        glDeleteShader(*shader);
        return NO;
    }
    
    return YES;
}

- (BOOL)linkProgram:(GLuint)prog
{
    GLint status;
    glLinkProgram(prog);
    
#if defined(DEBUG)
    GLint logLength;
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program link log:\n%s", log);
        free(log);
    }
#endif
    
    glGetProgramiv(prog, GL_LINK_STATUS, &status);
    if (status == 0) {
        return NO;
    }
    
    return YES;
}

- (BOOL)validateProgram:(GLuint)prog
{
    GLint logLength, status;
    
    glValidateProgram(prog);
    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLength);
    if (logLength > 0) {
        GLchar *log = (GLchar *)malloc(logLength);
        glGetProgramInfoLog(prog, logLength, &logLength, log);
        NSLog(@"Program validate log:\n%s", log);
        free(log);
    }
    
    glGetProgramiv(prog, GL_VALIDATE_STATUS, &status);
    if (status == 0) {
        return NO;
    }
    
    return YES;
}


@end
