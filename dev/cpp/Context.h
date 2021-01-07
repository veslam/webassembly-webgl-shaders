

class Context {
public:
    Context (int width, int height, char * id);

    ~Context (void);

    void run (uint8_t* buffer);
    void test (uint8_t* buffer);
    void renderMasks (uint8_t* buffer);

private:
    int width;
    int height;

    GLuint programObject;
    GLuint vertexShader;
    GLuint fragmentShader;

    EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context;

};