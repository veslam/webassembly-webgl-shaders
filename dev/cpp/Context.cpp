#include "Context.h"
#include "shaders.h"


GLuint CompileShader (GLenum type, std::string *source) {

    // Create shader object
    const GLchar* sourceString[1];
    GLint sourceStringLengths[1];

    sourceString[0] = source->c_str();
    sourceStringLengths[0] = source->length();
    GLuint shader = glCreateShader(type);

    if (shader == 0) {
        return 0;
    }

    // Assign and compile the source to the shader object
    glShaderSource(shader, 1, sourceString, sourceStringLengths);
    glCompileShader(shader);

    // Check if there were errors
    int infoLen = 0;
    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLen );

    if (infoLen > 1) {

        char infoLog[infoLen];

        // And print them out
        glGetShaderInfoLog( shader, infoLen, NULL, infoLog );
        printf("%s\n", infoLog);

        free ( infoLog );
    }

    return shader;
}


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
        fragmentShader = CompileShader(GL_FRAGMENT_SHADER, &texture_load_fragment_source);
        vertexShader = CompileShader(GL_VERTEX_SHADER, &vertex_source);

    } else if (std::string(id) == "edgeDetect") {
        fragmentShader = CompileShader(GL_FRAGMENT_SHADER, &edge_detect_fragment_source);
        vertexShader = CompileShader(GL_VERTEX_SHADER, &vertex_source);
    }

    // Build program
    programObject = glCreateProgram();

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    glBindAttribLocation(programObject, 0, "position");

    glLinkProgram(programObject);
    glValidateProgram(programObject);
}

Context::~Context (void) {
    emscripten_webgl_destroy_context(context);
}


void Context::run (uint8_t* buffer) {

    // Make the context current and use the program
    emscripten_webgl_make_context_current(context);
    glUseProgram( programObject );

    GLuint texId;
    GLuint vertexObject;
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

    // Vertex data of texture bounds
    GLfloat vVertices[] = { -1.0,  1.0, 0.0, 0.0, 0.0, -1.0, -1.0, 0.0, 0.0, 1.0,
                             1.0, -1.0, 0.0, 1.0,  1.0, 1.0, 1.0, 0.0, 1.0,  0.0};
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    glGenBuffers(1, &vertexObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vVertices), vVertices, GL_STATIC_DRAW);

    glGenBuffers(1, &indexObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set the viewport
    glViewport(0, 0, width, height);
    glClear(GL_COLOR_BUFFER_BIT);

    // Load and enable the vertex position and texture coordinates
    glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), 0);
    glVertexAttribPointer(texCoordLoc, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*) (3 * sizeof(GLfloat)));

    glEnableVertexAttribArray(positionLoc);
    glEnableVertexAttribArray(texCoordLoc);

    // Draw
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}
