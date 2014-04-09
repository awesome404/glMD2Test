//
//  Shader.fsh
//  glMD2Test
//
//  Created by Adam Dann on 2014-04-09.
//  Copyright (c) 2014 Nullriver. All rights reserved.
//

varying lowp vec4 colorVarying;

void main()
{
    gl_FragColor = colorVarying;
}
