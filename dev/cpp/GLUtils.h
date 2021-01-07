class GLUtils {
public:
	static GLuint compileShader (GLenum type, std::string* source);
	static GLuint createProgram (GLuint vertexShader, GLuint fragmentShader);

	static void genTexture(GLuint* textureId);
	static void genBuffer(GLuint* bufferId);

	static void drawArrays (int count=4);
	static void drawElements (int count);
};