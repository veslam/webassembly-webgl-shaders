#include "GLUtils.h"



GLuint GLUtils::compileShader (GLenum type, std::string* source) {
	printf("[GLUtils] compileShader \n");

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

GLuint GLUtils::createProgram (GLuint vertexShader, GLuint fragmentShader) {
	printf("[GLUtils] createProgram \n");

    // Build program
    GLuint programObject = glCreateProgram();

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    glLinkProgram(programObject);
    glValidateProgram(programObject);

    return programObject;
}

//---------------------------------------------------------------------------

void GLUtils::genTexture(GLuint* texture) {
	if (0 == *texture) {
		glGenTextures(1, texture);
	}
}
void GLUtils::genBuffer(GLuint* buffer) {
	if (0 == *buffer) {
		glGenBuffers(1, buffer);
	}
}

// GLuint GLUtils::setArrayBuffer (array, GLuint buffer, usage=GL_STATIC_DRAW) {
// 	glBindBuffer(GL_ARRAY_BUFFER, buffer);
// 	glBufferData(GL_ARRAY_BUFFER, array, usage);
// 	return buffer;
// }

//---------------------------------------------------------------------------
// use?
void GLUtils::drawArrays (int count) {
	printf("[GLUtils] drawArrays \n");

	GLenum mode = GL_TRIANGLE_STRIP;
 	GLint offset = 0;
 	// GLsizei count
 	glDrawArrays(mode, offset, count);
}

// use?
void GLUtils::drawElements (int count) {
	printf("[GLUtils] drawElements \n");

	GLenum mode = GL_TRIANGLES;
	GLenum type = GL_UNSIGNED_SHORT;
 	const GLvoid * indices = 0;

 	glDrawElements(mode, count, type, indices);
}
