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

    void rb ();
    void renderBase (GLuint program, bool revert=false, bool do_tilt=false, bool do_filter=false);
    void renderToScreen ();

    void renderSkinColor(int width, int height, float faceLandmarks[81][2]);
    void renderBeauty(int width, int height, float faceLandmarks[81][2], FrameBufferTexture* skinColor);
    void renderMasks (int width, int height, float lipsLandmarks[128][2], float eyesLandmarks[2][128][2]);
    void renderMaskBlurred (int width, int height, float faceLandmarks[81][2]);
    void renderLipsColor (float lipsLmks_1d[256]);
    void renderLips (int width, int height, float lipsLandmarks[128][2], float lipsLmks_1d[256], float avgLipsColor[3], float maxLipsColor[3], FrameBufferTexture* mask);



    // --------------------
	FrameBufferTexture* createFrameBufferTexture (std::string name, int width, int height);

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

    GLuint vb_lips_vec = 0;
    // GLuint vb_lips_tex = 0; // unused.
    GLuint vb_lips_dis = 0;
    GLuint ib_lips = 0;

    GLuint vb_lips_box_vec = 0;
    GLuint vb_lips_box_tex = 0;
    GLuint ib_lips_box = 0;

    GLubyte* lipsColorData = nullptr;


    std::map<std::string, GLuint> programs;

    GLuint baseTexture;
    std::map<std::string, FrameBufferTexture*> fbts;
    SwapFrameBufferTexture* swapFbt = nullptr;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

};