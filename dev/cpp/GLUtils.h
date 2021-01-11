class GLUtils {
public:
	static GLuint compileShader (GLenum type, std::string* source);
	static GLuint createProgram (GLuint vertexShader, GLuint fragmentShader);

	static void genTexture (GLuint& textureId);
	static void genBuffer (GLuint& bufferId);

	static void bindTextureWithData (GLuint texture, GLsizei texture_width, GLsizei texture_height, const GLvoid* texture_data);

	static void setArrayBuffer (GLfloat* vertex_array, GLint size, GLuint vertex_buffer, GLenum usage);
	static void setElementArrayBuffer (GLushort* index_array, GLint size, GLuint index_buffer, GLenum usage);
	static void setVertexAttrib (GLuint index, GLuint vertex_buffer);

	static void drawArrays (GLsizei count=4);
	static void drawElements (GLsizei count);
};