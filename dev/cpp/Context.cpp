#include "Context.h"
#include "shaders.h"
#include "GLUtils.cpp"

Context::Context (int w, int h, char * id) {

    width = w;
    height = h;

    // Context configurations
    EmscriptenWebGLContextAttributes attrs;
    attrs.explicitSwapControl = 0;
    attrs.depth = 1;
    attrs.stencil = 1;
    attrs.antialias = 1;
    attrs.majorVersion = 2;
    attrs.minorVersion = 0;

    std::string id_str = id;
    std::string sharp_id_str = "#" + id_str;

    context = emscripten_webgl_create_context(sharp_id_str.c_str(), &attrs);
    emscripten_webgl_make_context_current(context);

    // Compile shaders
    if (std::string(id) == "textureLoad") {
        fragmentShader = GLUtils::compileShader(GL_FRAGMENT_SHADER, &texture_load_fragment_source);
        vertexShader = GLUtils::compileShader(GL_VERTEX_SHADER, &vertex_source);

    } else if (std::string(id) == "edgeDetect") {
        fragmentShader = GLUtils::compileShader(GL_FRAGMENT_SHADER, &edge_detect_fragment_source);
        vertexShader = GLUtils::compileShader(GL_VERTEX_SHADER, &vertex_source);
    }

    // Build program
    programObject = GLUtils::createProgram(vertexShader, fragmentShader);
    // glBindAttribLocation(programObject, 0, "position");
}

Context::~Context (void) {
    emscripten_webgl_destroy_context(context);
}


void Context::run (uint8_t* buffer) {

    // Make the context current and use the program
    emscripten_webgl_make_context_current(context);
    glUseProgram( programObject );

    GLuint texId;
    GLuint vVertexObject, tVertexObject;
    GLuint indexObject;

    // Get the attribute/sampler locations
    GLint positionLoc = glGetAttribLocation(programObject, "position");
    GLint texCoordLoc = glGetAttribLocation(programObject, "texCoord");
    GLint textureLoc = glGetUniformLocation(programObject, "texture");

    // For "ERROR :GL_INVALID_OPERATION : glUniform1i: wrong uniform function for type"
    // https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glUniform.xhtml
    float widthUniform = glGetUniformLocation(programObject, "width");
    float heightUniform = glGetUniformLocation(programObject, "height");
    glUniform1f(widthUniform, (float) width);
    glUniform1f(heightUniform, (float) height);


    // Generate a texture object
    glGenTextures(1, &texId);
    glUniform1i(textureLoc, 0);

    // Bind it
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId);

    // Load the texture from the image buffer
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Set the viewport
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertex data of texture bounds
    GLfloat vVertices[] = {
        -1.0, 1.0,
        -1.0, -1.0,
        1.0, -1.0,
        1.0, 1.0
    };
    GLfloat tVertices[] = {
        0.0, 0.0,
        0.0, 1.0,
        1.0, 1.0,
        1.0, 0.0
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    glGenBuffers(1, &vVertexObject);
    glBindBuffer(GL_ARRAY_BUFFER, vVertexObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);
    // Load and enable the vertex position and texture coordinates
    glVertexAttribPointer(positionLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

    glGenBuffers(1, &tVertexObject);
    glBindBuffer(GL_ARRAY_BUFFER, tVertexObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(tVertices), tVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);

    glEnableVertexAttribArray(positionLoc);
    glEnableVertexAttribArray(texCoordLoc);

    glGenBuffers(1, &indexObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Draw
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}







void Context::test (uint8_t* buffer) {

    // Make the context current and use the program
    emscripten_webgl_make_context_current(context);
    glUseProgram( programObject );

    GLuint texId = 0;
    GLuint vVertexObject = 0, tVertexObject = 0;
    GLuint indexObject = 0;

    // Get the attribute/sampler locations
    GLint positionLoc = glGetAttribLocation(programObject, "position");
    GLint texCoordLoc = glGetAttribLocation(programObject, "texCoord");
    GLint textureLoc = glGetUniformLocation(programObject, "texture");

    // For "ERROR :GL_INVALID_OPERATION : glUniform1i: wrong uniform function for type"
    // https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glUniform.xhtml
    float widthUniform = glGetUniformLocation(programObject, "width");
    float heightUniform = glGetUniformLocation(programObject, "height");
    glUniform1f(widthUniform, (float) width);
    glUniform1f(heightUniform, (float) height);


    // Generate a texture object
    GLUtils::genTexture(texId);
    glUniform1i(textureLoc, 0);

    // Bind it
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texId);

    // Load the texture from the image buffer
    glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Set the viewport
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertex data of texture bounds
    GLfloat vVertices[] = {
        -1.0, 1.0,
        -1.0, -1.0,
        1.0, -1.0,
        1.0, 1.0
    };
    GLfloat tVertices[] = {
        0.0, 0.0,
        0.0, 1.0,
        1.0, 1.0,
        1.0, 0.0
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    GLUtils::genBuffer(vVertexObject);
    GLUtils::setArrayBuffer(vVertices, sizeof(vVertices), vVertexObject, GL_STATIC_DRAW);
    GLUtils::setVertexAttrib(positionLoc);

    GLUtils::genBuffer(tVertexObject);
    GLUtils::setArrayBuffer(tVertices, sizeof(tVertices), tVertexObject, GL_STATIC_DRAW);
    GLUtils::setVertexAttrib(texCoordLoc);

    GLUtils::genBuffer(indexObject);
    GLUtils::setElementArrayBuffer(indices, sizeof(indices), indexObject, GL_STATIC_DRAW);

    // Draw
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}
