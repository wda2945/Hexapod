//
//  Shader.fsh
//  Poser
//
//  Created by Martin Lane-Smith on 6/14/15.
//  Copyright (c) 2015 Martin Lane-Smith. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
