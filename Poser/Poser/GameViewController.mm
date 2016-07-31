//
//  GameViewController.m
//  Poser
//
//  Created by Martin Lane-Smith on 6/14/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

#import "GameViewController.h"
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
    UNIFORM_SELECTED_LEG,
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
    
    GLuint _vertexArray;
    GLuint _vertexBuffer;
    
    CGPoint panBegan;
    CGPoint panBeganViewRotation;
    CGPoint viewRotation;
    
    GLKVector3 panBeganBodyOffset;
    GLKVector3 panBeganBodyRotation;
    GLKVector4 panBeganLegEndpoint;
    GLKVector4 panNewLegEndpoint;
    
    int triangle_vertex_count;
    int firstFace[MATRIX_COUNT];
    int faceCount[MATRIX_COUNT];
    
    UIButton *activeButton;
    int selectedTibia;
    int selectedLeg;
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
    __weak IBOutlet UITextField *textField;
    __weak IBOutlet UIButton *view_button;
    __weak IBOutlet UILabel *fileError;
}

- (IBAction)textEditingDidEnd:(id)sender;
- (IBAction)resetButton:(id)sender;
- (IBAction)openButton:(id)sender;
- (IBAction)saveButton:(id)sender;

@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;

- (IBAction)lf_button:(UIButton *)sender;
- (IBAction)lm_button:(UIButton *)sender;
- (IBAction)lr_button:(UIButton *)sender;
- (IBAction)rf_button:(UIButton *)sender;
- (IBAction)rm_button:(UIButton *)sender;
- (IBAction)rr_button:(UIButton *)sender;
- (IBAction)viewButton:(UIButton *)sender;

- (IBAction)body_button:(UIButton *)sender;
- (IBAction)body_rot:(id)sender;

- (void)setupGL;
- (void)tearDownGL;

- (BOOL)loadShaders;
- (BOOL)compileShader:(GLuint *)shader type:(GLenum)type file:(NSString *)file;
- (BOOL)linkProgram:(GLuint)prog;
- (BOOL)validateProgram:(GLuint)prog;

- (void) panAction: (UIPanGestureRecognizer*) pan;
- (void) updateHUD;

@end

@implementation GameViewController

- (void)viewDidLoad
{
    activeButton = nil;
    selectedTibia = -1;
    viewRotation = CGPointZero;
    
    [super viewDidLoad];
    
    self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }
    
    glkView.context = self.context;
    glkView.drawableDepthFormat = GLKViewDrawableDepthFormat24;
    
    UIPanGestureRecognizer *pan = [[UIPanGestureRecognizer alloc] initWithTarget:self action:@selector(panAction:)];
    pan.maximumNumberOfTouches = 1;
    pan.minimumNumberOfTouches = 1;
    [self.view addGestureRecognizer:pan];
    
    [self setupGL];
    [self viewButton:view_button];
    [self resetButton:nil];
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

- (IBAction)viewButton:(UIButton *)sender {
    fileError.text = @"";
    if (activeButton) activeButton.selected = NO;
    activeButton = sender;
    sender.selected = YES;
    selectedTibia = -1;
    selectedLeg = -1;
    reset_button.titleLabel.text = @"Reset all";
}

- (IBAction)rf_button:(UIButton *)sender {
    fileError.text = @"";
    if (activeButton) activeButton.selected = NO;
    activeButton = sender;
    sender.selected = YES;
    selectedTibia = servoIds[RIGHT_FRONT][2];
    selectedLeg = RIGHT_FRONT;
    reset_button.titleLabel.text = @"Rst RF";
}

- (IBAction)rm_button:(UIButton *)sender {
    fileError.text = @"";
    if (activeButton) activeButton.selected = NO;
    activeButton = sender;
    sender.selected = YES;
    selectedTibia = servoIds[RIGHT_MIDDLE][2];
    selectedLeg = RIGHT_MIDDLE;
    reset_button.titleLabel.text = @"Rst RM";
}

- (IBAction)rr_button:(UIButton *)sender {
    fileError.text = @"";
    if (activeButton) activeButton.selected = NO;
    activeButton = sender;
    sender.selected = YES;
    selectedTibia = servoIds[RIGHT_REAR][2];
    selectedLeg = RIGHT_REAR;
    reset_button.titleLabel.text = @"Rst RR";
}

- (IBAction)lf_button:(UIButton *)sender {
    fileError.text = @"";
    if (activeButton) activeButton.selected = NO;
    activeButton = sender;
    sender.selected = YES;
    selectedTibia = servoIds[LEFT_FRONT][2];
    selectedLeg = LEFT_FRONT;
    reset_button.titleLabel.text = @"Rst LF";
}

- (IBAction)lm_button:(UIButton *)sender {
    fileError.text = @"";
    if (activeButton) activeButton.selected = NO;
    activeButton = sender;
    sender.selected = YES;
    selectedTibia = servoIds[LEFT_MIDDLE][2];
    selectedLeg = LEFT_MIDDLE;
    reset_button.titleLabel.text = @"Rst LM";
}

- (IBAction)lr_button:(UIButton *)sender {
    fileError.text = @"";
    if (activeButton) activeButton.selected = NO;
    activeButton = sender;
    sender.selected = YES;
    selectedTibia = servoIds[LEFT_REAR][2];
    selectedLeg = LEFT_REAR;
    reset_button.titleLabel.text = @"Rst LR";
}

- (IBAction)body_button:(UIButton *)sender {
    fileError.text = @"";
    if (activeButton) activeButton.selected = NO;
    activeButton = sender;
    sender.selected = YES;
    selectedTibia = 0;
    selectedLeg = -2;
    reset_button.titleLabel.text = @"Rst xyz";
}

- (IBAction)body_rot:(UIButton *)sender {
    fileError.text = @"";
    if (activeButton) activeButton.selected = NO;
    activeButton = sender;
    sender.selected = YES;
    selectedTibia = 0;
    selectedLeg = -3;
    reset_button.titleLabel.text = @"Rst rot";
}

- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
    return YES;
}

- (IBAction)textEditingDidEnd:(id)sender {
    [sender resignFirstResponder];
}

- (IBAction)resetButton:(id)sender {
    fileError.text = @"";
    theHexapod.reset(selectedLeg);
    [self updateHUD];
}

- (IBAction)openButton:(id)sender {
    NSString *homeDirectory = NSHomeDirectory();
    NSString *docDirectory = [homeDirectory stringByAppendingPathComponent:@"Documents"];
    NSString *docPath = [docDirectory stringByAppendingPathComponent:textField.text];
    
    FILE *file = fopen(docPath.UTF8String, "r");
    
    if (file)
    {
        fileError.text = @"";

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
        }
        fclose(file);
        [self updateHUD];
    }
    else{
        fileError.text = [NSString stringWithFormat:@"%s", strerror(errno)];
    }
}

- (IBAction)saveButton:(id)sender {
    NSString *homeDirectory = NSHomeDirectory();
    NSString *docDirectory = [homeDirectory stringByAppendingPathComponent:@"Documents"];
    NSString *docPath = [docDirectory stringByAppendingPathComponent:textField.text];
    
    FILE *file = fopen(docPath.UTF8String, "w");
    
    if (file)
    {
        fileError.text = @"";
        
        fprintf(file, "B %f %f %f %f %f %f\n",
               theHexapod.bodyOffset.x,
               theHexapod.bodyOffset.y,
               theHexapod.bodyOffset.z,
               theHexapod.bodyRotation.x,
               theHexapod.bodyRotation.y,
               theHexapod.bodyRotation.z);
        
        for (int i=0; i< 6; i++)
        {
            fprintf(file, "L %i %f %f %f\n", i,
                   legs[i].endpoint.x,
                   legs[i].endpoint.y,
                   legs[i].endpoint.z);
        }
        fclose(file);
    }
    else{
        fileError.text = [NSString stringWithFormat:@"%s", strerror(errno)];
    }
}

- (void) panAction: (UIPanGestureRecognizer*) pan
{
    switch(pan.state)
    {
        case UIGestureRecognizerStateBegan:
            panBegan = [pan translationInView:self.view];
            switch (selectedLeg)
        {
            case -1:    //view
                panBeganViewRotation = viewRotation;
                break;
            case -2:    //body xyz
                panBeganBodyOffset   = theHexapod.bodyOffset;
                break;
            case -3:    //body rot
                panBeganBodyRotation = theHexapod.bodyRotation;
                break;
            default:    //leg
                panNewLegEndpoint = panBeganLegEndpoint  = legs[selectedLeg].endpoint;
                break;
        }
            break;
        case UIGestureRecognizerStateChanged:
        {
            CGPoint panChanged = [pan translationInView:self.view];
            float xChange = panBegan.x - panChanged.x;
            float yChange = panBegan.y - panChanged.y;
            switch (selectedLeg)
            {
                case -1:    //view
                    viewRotation.x = panBeganViewRotation.x - xChange / 100;
                    break;
                case -2:    //body xyz
                    theHexapod.bodyOffset.z = panBeganBodyOffset.z + yChange / 2;
                    theHexapod.bodyOffset.x = panBeganBodyOffset.x - cosf(viewRotation.x) * xChange / 2;
                    theHexapod.bodyOffset.y = panBeganBodyOffset.y + sinf(viewRotation.x) * xChange / 2;
                    break;
                case -3:    //body rot
                    theHexapod.bodyRotation.z = panBeganBodyRotation.z - xChange / 500;
                    theHexapod.bodyRotation.x = panBeganBodyRotation.x + cosf(viewRotation.x) * yChange / 500;
                    theHexapod.bodyRotation.y = panBeganBodyRotation.y + sinf(viewRotation.x) * yChange / 500;
                    break;
                default:    //leg
                    panNewLegEndpoint.z = panBeganLegEndpoint.z + yChange / 2;
                    panNewLegEndpoint.x = panBeganLegEndpoint.x - cosf(viewRotation.x) * xChange / 2;
                    panNewLegEndpoint.y = panBeganLegEndpoint.y + sinf(viewRotation.x) * xChange / 2;
                    legs[selectedLeg].newEndpoint(panNewLegEndpoint);
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
    for (int i=0; i<6; i++)
    {
        
//        NSLog(@"Leg %i coxa=%f, femur=%f, tibia=%f", i,
//              legs[i].coxa.ikSolution, legs[i].femur.ikSolution,legs[i].tibia.ikSolution);
        
//        NSLog(@"legs[%i].endpoint.x = %f;", i, legs[i].tibia.fkEndpoint.x);
//        NSLog(@"legs[%i].endpoint.y = %f;", i, legs[i].tibia.fkEndpoint.y);
//        NSLog(@"legs[%i].endpoint.z = %f;", i, legs[i].tibia.fkEndpoint.z);
    }
    
    float aspect = fabs(glkView.bounds.size.width / glkView.bounds.size.height);
    
    _projectionMatrix = GLKMatrix4MakePerspective(GLKMathDegreesToRadians(65.0f), aspect, 0.1f, 1500.0f);
    
    GLKMatrix4 baseModelViewMatrix = GLKMatrix4MakeTranslation(0.0f, 0.0f, -600.0f);
    baseModelViewMatrix = GLKMatrix4Rotate(baseModelViewMatrix, -45.0, 1.0f, 0.0f, 0.0f);
    baseModelViewMatrix = GLKMatrix4Rotate(baseModelViewMatrix, viewRotation.x, 0.0f, 0.0f, 1.0f);
    
    for (int i=0; i<MATRIX_COUNT-1; i++)
    {
        GLKMatrix4 modelViewMatrix = theHexapod.getMatrix(i);
        _modelViewMatrix[i] = GLKMatrix4Multiply(baseModelViewMatrix, modelViewMatrix);
        _normalMatrix[i] = GLKMatrix3InvertAndTranspose(GLKMatrix4GetMatrix3(_modelViewMatrix[i]), NULL);
    }
    _modelViewMatrix[MATRIX_COUNT-1] = baseModelViewMatrix;
    _normalMatrix[MATRIX_COUNT-1] = GLKMatrix3InvertAndTranspose(GLKMatrix4GetMatrix3(baseModelViewMatrix), NULL);
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
    glUniform1i(uniforms[UNIFORM_SELECTED_LEG], selectedTibia);
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
    uniforms[UNIFORM_SELECTED_LEG] = glGetUniformLocation(_program, "selectedTibia");
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
