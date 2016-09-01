//
//  Shader.vsh
//  Poser
//
//  Created by Martin Lane-Smith on 6/14/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

#define MATRIX_COUNT 20

attribute float bone;
attribute vec4 position;
attribute vec3 normal;

varying lowp vec4 colorVarying;

uniform mat4 projectionMatrix;
uniform mat4 modelViewMatrix[MATRIX_COUNT];
uniform mat3 normalMatrix[MATRIX_COUNT];
uniform int selectedShape[MATRIX_COUNT];
uniform int OOR[MATRIX_COUNT];

void main()
{
    int boneInt = int(bone);
    
    vec3 eyeNormal = normalize(normalMatrix[boneInt] * normal);
    vec3 lightPosition = vec3(0.0, 0.0, 1.0);
    vec4 diffuseColor = vec4(0.4, 0.4, 1.0, 1.0);
    vec4 hilitColor = vec4(0.4, 1.0, 0.4, 1.0);
    vec4 oorColor = vec4(1.0, 0.4, 0.4, 1.0);
    vec4 floorColor = vec4(1.0, 1.0, 0.5, 1.0);
    
    float nDotVP = max(0.0, dot(eyeNormal, normalize(lightPosition)));
    
    if (boneInt == MATRIX_COUNT-1)
    {
        colorVarying = floorColor * nDotVP;
    }
    else if (OOR[boneInt] == 1)
    {
        colorVarying = oorColor * nDotVP;
    }
    else if (selectedShape[boneInt] == 1)
    {
        colorVarying = hilitColor * nDotVP;
    }
    else
    {
        colorVarying = diffuseColor * nDotVP;
    }
    
    gl_Position = projectionMatrix * modelViewMatrix[boneInt] * position;
}
