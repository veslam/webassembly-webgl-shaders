#include "GLUtils.h"



GLuint GLUtils::compileShader (GLenum type, std::string* source) {
	// printf("[GLUtils] compileShader \n");

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
	// printf("[GLUtils] createProgram \n");

    // Build program
    GLuint programObject = glCreateProgram();

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    glLinkProgram(programObject);
    glValidateProgram(programObject);

    return programObject;
}

//---------------------------------------------------------------------------

void GLUtils::genTexture (GLuint& texture) {
	if (0 == texture) {
		glGenTextures(1, &texture);
	}
}
void GLUtils::genBuffer (GLuint& buffer) {
	if (0 == buffer) {
		glGenBuffers(1, &buffer);
	}
}

//---------------------------------------------------------------------------

void GLUtils::bindTextureWithData (GLuint texture, GLsizei texture_width, GLsizei texture_height, const GLvoid* texture_data) {
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // GL_LINEAR_MIPMAP_LINEAR

	// texture_data is the source data of your texture, in this case
	// its size is sizeof(unsigned char) * texture_width * texture_height * 4
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_width, texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture_data);
	// glGenerateMipmap(GL_TEXTURE_2D); // Unavailable in OpenGL 2.1, use gluBuild2DMipmaps() insteads.

	glBindTexture(GL_TEXTURE_2D, 0);
}

//---------------------------------------------------------------------------

void GLUtils::setArrayBuffer (GLfloat* vertex_array, GLint size, GLuint vertex_buffer, GLenum usage) {
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, size, vertex_array, usage);
}

void GLUtils::setElementArrayBuffer (GLushort* index_array, GLint size, GLuint index_buffer, GLenum usage) {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, index_array, usage);
}

void GLUtils::setVertexAttrib (GLuint index, GLuint vertex_buffer) {
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(index);
}


//---------------------------------------------------------------------------
// deprecate?
void GLUtils::drawArrays (GLsizei count) {
	printf("[GLUtils] drawArrays \n");

	GLenum mode = GL_TRIANGLE_STRIP;
 	GLint offset = 0;
 	// GLsizei count
 	glDrawArrays(mode, offset, count);
}

// deprecate?
void GLUtils::drawElements (GLsizei count) {
	printf("[GLUtils] drawElements \n");

	GLenum mode = GL_TRIANGLES;
	GLenum type = GL_UNSIGNED_SHORT;
 	const GLvoid * indices = 0;

 	glDrawElements(mode, count, type, indices);
}
