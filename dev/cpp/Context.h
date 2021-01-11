#include <map>
#include "FrameBufferTexture.cpp"

class Context {
public:
    Context (int width, int height, char * id);

    ~Context (void);

    void run (uint8_t* buffer);
    void test (uint8_t* buffer);
    void renderMasks (uint8_t* buffer);

    void loadShader (std::string type, std::string vsh_str, std::string fsh_str);

    GLint getUnifLocation(std::string type, std::string key);
    // ----- setAttr -----
    bool setUnif1i (std::string type, std::string key, GLint value);
    bool setUnif1f (std::string type, std::string key, GLfloat value);
    bool setUnif2f (std::string type, std::string key, GLfloat value1, GLfloat value2);
    bool setUnif3f (std::string type, std::string key, GLfloat value1, GLfloat value2, GLfloat value3);
	bool setUnif2fv (std::string type, std::string key, GLsizei count, GLfloat* value);

    // --------------------

    void draw (uint8_t* buffer);
    void _draw (uint8_t* buffer);
    // void oho ();
    void renderBase (GLuint program, bool revert=false, bool do_tilt=false, bool do_filter=false);

private:
    int width;
    int height;

    GLuint programObject;
    GLuint vertexShader;
    GLuint fragmentShader;

    // vertex buffers
    // TODO extract into some struct?
    GLuint vb_base_vec = 0;
    GLuint vb_base_tex = 0;
    GLuint vb_base_tex_revert = 0;


    std::map<std::string, GLuint> programs;

    GLuint baseTexture;
    std::map<std::string, FrameBufferTexture> fbts;
    SwapFrameBufferTexture* swapFbt = nullptr;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

};