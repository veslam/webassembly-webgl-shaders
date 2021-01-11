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
        vertexShader = GLUtils::compileShader(GL_VERTEX_SHADER, &vertex_source);
        fragmentShader = GLUtils::compileShader(GL_FRAGMENT_SHADER, &texture_load_fragment_source);

    } else if (std::string(id) == "edgeDetect") {
        // vertexShader = GLUtils::compileShader(GL_VERTEX_SHADER, &vertex_source);
        // fragmentShader = GLUtils::compileShader(GL_FRAGMENT_SHADER, &edge_detect_fragment_source);
        std::string vsh_str = mask_vsh;
        std::string fsh_str = mask_fsh;
        vertexShader = GLUtils::compileShader(GL_VERTEX_SHADER, &vsh_str);
        fragmentShader = GLUtils::compileShader(GL_FRAGMENT_SHADER, &fsh_str);
    }

    // Build program
    programObject = GLUtils::createProgram(vertexShader, fragmentShader);
    // glBindAttribLocation(programObject, 0, "position");






    glGenTextures(1, &baseTexture);

    loadShader("base", base_vsh, base_fsh);
    loadShader("standardlips", mask_vsh, mask_fsh);





    printf("[Context] construct done. \n");
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
    GLuint vVertexBuffer = 0, tVertexBuffer = 0;
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

    GLUtils::genBuffer(vVertexBuffer);
    GLUtils::setArrayBuffer(vVertices, sizeof(vVertices), vVertexBuffer, GL_STATIC_DRAW);
    GLUtils::setVertexAttrib(positionLoc, vVertexBuffer);

    GLUtils::genBuffer(tVertexBuffer);
    GLUtils::setArrayBuffer(tVertices, sizeof(tVertices), tVertexBuffer, GL_STATIC_DRAW);
    GLUtils::setVertexAttrib(texCoordLoc, tVertexBuffer);

    GLUtils::genBuffer(indexObject);
    GLUtils::setElementArrayBuffer(indices, sizeof(indices), indexObject, GL_STATIC_DRAW);

    // Draw
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}





void Context::loadShader (std::string type, std::string vsh_str, std::string fsh_str) {
    // load base shader
    GLuint vertexShader = GLUtils::compileShader(GL_VERTEX_SHADER, &vsh_str);
    GLuint fragmentShader = GLUtils::compileShader(GL_FRAGMENT_SHADER, &fsh_str);
    GLuint program = GLUtils::createProgram(vertexShader, fragmentShader);
    programs[type] = program;

    glUseProgram(program);
    // TODO 设置不变参数
}


GLint Context::getUnifLocation (std::string type, std::string key) {
    if (programs.find(type) == programs.end()) {
        return -1; // not found
    }

    GLuint program = programs[type];
    glUseProgram(program);
    return glGetUniformLocation(program, key.c_str());
}

bool Context::setUnif1i (std::string type, std::string key, GLint value) {
    GLuint location = getUnifLocation(type, key);
    if (location <= 0) {
        return false;
    }
    glUniform1i(location, value);
    return true;
}
bool Context::setUnif1f (std::string type, std::string key, GLfloat value) {
    GLuint location = getUnifLocation(type, key);
    if (location <= 0) {
        return false;
    }
    glUniform1f(location, value);
    return true;
}
bool Context::setUnif2f (std::string type, std::string key, GLfloat value1, GLfloat value2) {
    GLuint location = getUnifLocation(type, key);
    if (location <= 0) {
        return false;
    }
    glUniform2f(location, value1, value2);
    return true;
}
bool Context::setUnif3f (std::string type, std::string key, GLfloat value1, GLfloat value2, GLfloat value3) {
    GLuint location = getUnifLocation(type, key);
    if (location <= 0) {
        return false;
    }
    glUniform3f(location, value1, value2, value3);
    return true;
}
bool Context::setUnif2fv (std::string type, std::string key, GLsizei count, GLfloat* value) {
    GLuint location = getUnifLocation(type, key);
    if (location <= 0) {
        return false;
    }
    glUniform2fv(location, count, value);
    return true;
}






void Context::draw (uint8_t* buffer) {
    printf("[Context] draw \n");

    // Make the context current and use the program
    emscripten_webgl_make_context_current(context);

    // for (int i = 0; i < 500; i ++) {
        _draw(buffer);
    // }
}

void Context::_draw (uint8_t* buffer) {
    // TODO set display glViewport();

    // TODO bind incoming frame data GLUtils::bindTextureWithData();
    // frame.data -> baseTexture
    glViewport(0, 0, width, height);
    glActiveTexture(GL_TEXTURE0);
    GLUtils::bindTextureWithData(baseTexture, width, height, buffer);
    // TODO check: baseTexture should be filled.

    if (true) {
        glBindTexture(GL_TEXTURE_2D, baseTexture);
        renderBase(programs["base"], true);
        return;
    }

    if (nullptr == swapFbt) {
        swapFbt = new SwapFrameBufferTexture(width, height);
    }

    swapFbt->getToWrite()->useFrameBuffer();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, baseTexture);
    // renderBase();
    swapFbt->swap();

}


void Context::renderBase (GLuint program, bool revert, bool do_tilt, bool do_filter) {
    glUseProgram(program);

    if (programs["base"] == program) {
        if (true == do_tilt) {
            glUniform1i(glGetUniformLocation(program, "do_tilt"), 1);
            glUniform3f(glGetUniformLocation(program, "tilt_factor"), 1.0, 0.95, 1.0);
        } else {
            glUniform1i(glGetUniformLocation(program, "do_tilt"), 0);
        }
        // TODO filter logic
    }


    GLuint vVertexBuffer = 0, tVertexBuffer = 0;
    GLint positionLoc = glGetAttribLocation(programObject, "aPosition");
    GLint texCoordLoc = glGetAttribLocation(programObject, "TexCoordIn");


    if (0 == vb_base_vec) {
        GLfloat vVertices[] = {
            -1.0, 1.0,  // bottom left
            1.0, 1.0,   // bottom right
            -1.0, -1.0, // top left
            1.0, -1.0
        };
        GLUtils::genBuffer(vb_base_vec);
        GLUtils::setArrayBuffer(vVertices, sizeof(vVertices), vb_base_vec, GL_STATIC_DRAW);
    }
    GLUtils::setVertexAttrib(positionLoc, vb_base_vec);

    if (revert) {
        if (0 == vb_base_tex_revert) {
            GLfloat tVertices[] = {
                0.0, 0.0,
                1.0, 0.0,
                0.0, 1.0,
                1.0, 1.0
            };
            GLUtils::genBuffer(vb_base_tex_revert);
            GLUtils::setArrayBuffer(tVertices, sizeof(tVertices), vb_base_tex_revert, GL_STATIC_DRAW);
        }
        GLUtils::setVertexAttrib(texCoordLoc, vb_base_tex_revert);
    } else {
        if (0 == vb_base_tex) {
            GLfloat tVertices[] = {
                0.0, 1.0,
                1.0, 1.0,
                0.0, 0.0,
                1.0, 0.0
            };
            GLUtils::genBuffer(vb_base_tex);
            GLUtils::setArrayBuffer(tVertices, sizeof(tVertices), vb_base_tex, GL_STATIC_DRAW);
        }
        GLUtils::setVertexAttrib(texCoordLoc, vb_base_tex);
    }

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


void Context::renderMasks (uint8_t* buffer) {
    printf("[Context] renderMasks \n");

    // Make the context current and use the program
    emscripten_webgl_make_context_current(context);
    glUseProgram(programObject);

    // GLuint texId = 0;
    GLuint vVertexBuffer = 0, tVertexBuffer = 0;
    GLuint disObject = 0;
    GLuint indexObject = 0;

    // Get the attribute/sampler locations
    GLint positionLoc = glGetAttribLocation(programObject, "aPosition");
    GLint texCoordLoc = glGetAttribLocation(programObject, "TexCoordIn");
    GLint modeLoc = glGetUniformLocation(programObject, "mode");

    // // For "ERROR :GL_INVALID_OPERATION : glUniform1i: wrong uniform function for type"
    // // https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glUniform.xhtml
    // float widthUniform = glGetUniformLocation(programObject, "width");
    // float heightUniform = glGetUniformLocation(programObject, "height");
    // glUniform1f(widthUniform, (float) width);
    // glUniform1f(heightUniform, (float) height);

    glUniform1i(modeLoc, 0);

    // Generate a texture object
    // GLUtils::genTexture(texId);
    // glUniform1i(texLoc, 0);

    printf("[Context] ======= width = %d, height = %d\n", width, height);

    // // Bind it
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, texId);

    // // Load the texture from the image buffer
    // glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
    // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Set the viewport
    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);


    float lmks [128][2] = {{896.5467848967558,436.0461051171361},{897.2667625474678,433.1388248029119},{898.7181435249881,430.2700774871845},{900.6262043375114,427.57461499990825},{902.8423253770047,425.04036196392866},{905.2617900394565,422.66057976274294},{907.8206235067494,420.4279700777064},{910.4835353462613,418.3316970656905},{913.2319563749226,416.36417890918955},{916.0711302521827,414.54066287582384},{919.0059320591143,412.9198064530994},{922.0404171489839,411.6148745194387},{925.1738967371135,410.77385640636874},{928.380083524808,410.4834557601478},{931.5991307699103,410.7204063813623},{934.8037122889554,411.2297909114847},{938.0059895151851,411.6211610281653},{941.2071863756832,411.63618178283144},{944.4025924427734,411.344008328249},{947.618847560625,411.0513844685639},{950.8298374375524,411.00166505474385},{953.9820931872381,411.402365049667},{957.0566006601391,412.30512594928354},{960.0263025006694,413.6638886215615},{962.8889817897365,415.37001002573567},{965.6342928445137,417.3350188365314},{968.2629528855783,419.50001593986485},{970.7679105366533,421.81992736787714},{973.1447176271654,424.25587523207247},{975.3688142962833,426.782418258673},{977.396345973103,429.4167123355075},{979.0362933036577,432.1960249924379},{979.9086752081985,435.0162899341386},{978.224565347444,434.66859455557267},{975.6968167368321,434.19231271450775},{973.1534315906874,433.6784597665379},{970.5999071577369,433.1173119141176},{968.0402781918988,432.50965889494165},{965.4668141096115,431.88787708267006},{962.8889366129558,431.29349015887135},{960.2862155548145,430.78808606639046},{957.6456706889938,430.40447266874116},{954.9882466824776,430.1589173120891},{952.3140448897159,430.0430191746173},{949.6264280590512,430.0221351182459},{946.9235394334795,430.0309937283986},{944.2257790250388,430.0485343247998},{941.530810416693,430.0420846298541},{938.841963527427,430.0075768005031},{936.1541872327713,429.9362187956211},{933.4687580482307,429.84975275819534},{930.7912511276359,429.7741593838101},{928.1240491674916,429.7522224755702},{925.4655413746709,429.82004695847775},{922.8213344296349,429.98699724176015},{920.1957734794273,430.2875210725334},{917.591719848909,430.7319207595058},{915.0206244088487,431.30395006590413},{912.47792026853,431.9711662984388},{909.9535933385102,432.69804081377646},{907.4441355855187,433.4320079339465},{904.9344650852047,434.13440102696103},{902.4256480874351,434.79234312935466},{899.9116805454873,435.39482614500105},{897.3848921875948,435.9109234778324},{898.4077694269242,435.8840791449349},{900.9957051239489,435.3666425434113},{903.5805922300318,434.7495767195978},{906.1591449650709,434.0790691878468},{908.7264135735794,433.35706277723466},{911.3050367637182,432.61282289357615},{913.8786123037235,431.8917507190273},{916.4579810188138,431.23808836780967},{919.0533137080025,430.7022453286254},{921.6641996404218,430.30422569763994},{924.2887507592906,430.04033111337094},{926.9231005735143,429.9071572950804},{929.565598135407,429.87633572135996},{932.2175191221246,429.9227485749343},{934.8700349749485,430.01062846444944},{937.5201764627822,430.11396218170574},{940.1782916544279,430.18679024210724},{942.8294404369451,430.20722299586276},{945.4832500458399,430.1878228634876},{948.1325610019509,430.1392471991969},{950.773914253918,430.08423471966023},{953.406685661111,430.07817310798805},{956.0247274660886,430.15051132777},{958.6256551264294,430.3514034282167},{961.2076764441313,430.6808198511286},{963.7759032693799,431.15720060398496},{966.3421034662967,431.739200908216},{968.9004479674131,432.37810339383026},{971.4553434671307,433.02833939663964},{973.9972830368172,433.6486336323065},{976.5114339250564,434.20761605516265},{979.0097965783687,434.70213226186166},{977.9360476369858,436.4775188686071},{975.6815109698015,437.7251047396857},{973.2557398493411,438.7788839031692},{970.7282259112923,439.6718822567274},{968.1420969481558,440.4082636274371},{965.5122116153827,441.0124983659584},{962.8529450662347,441.50815981457265},{960.1900648007875,441.9258211438807},{957.522267355633,442.29731991186964},{954.844355374657,442.6147555632886},{952.1472554077044,442.88764751677815},{949.4224245095143,443.1084257532419},{946.6836562182573,443.26676100308805},{943.9303877103229,443.36922298060466},{941.1735687536321,443.4259288614652},{938.4093301511645,443.4444367050196},{935.6480669761127,443.41328012907496},{932.8882286474326,443.3403904811747},{930.1401271176809,443.2324605256081},{927.4051873050196,443.05248401837684},{924.6981145356682,442.82781589083083},{922.015522182287,442.5575475167597},{919.3497005492402,442.25076042988906},{916.6990936882075,441.9045550598759},{914.0562942652776,441.5278633369795},{911.4257309420236,441.09786507871365},{908.8012619057042,440.59045704624106},{906.1956187995451,439.9885529904704},{903.6178201169374,439.2739060280011},{901.0885169635516,438.4234576028024},{898.680130147896,437.3822420381548}};

    GLfloat tVertices[256];
    for (int i = 0; i < 128; i ++) {
        tVertices[i * 2] = (lmks[i][0] - 800) / width;
        tVertices[i * 2 + 1] = (lmks[i][1] - 400) / height + 0.5;
    }

    GLfloat vVertices[256];
    for (int i = 0; i < 128; i ++) {
        vVertices[i * 2] = tVertices[i * 2] * 2 - 1;
        vVertices[i * 2 + 1] = tVertices[i * 2 + 1] * 2 - 1;
    }

    GLfloat disArray[256];
    for (int i = 0; i < 128; i ++) {
        disArray[i * 2] = disArray[i * 2 + 1] =1 ;
    }


    GLushort indices[] = {0, 63, 1, 1, 63, 2, 63, 62, 2, 2, 62, 3, 62, 61, 3, 3, 61, 4, 61, 60, 4, 4, 60, 5, 60, 59, 5, 5, 59, 6, 59, 58, 6, 6, 58, 7, 58, 57, 7, 7, 57, 8, 57, 56, 8, 8, 56, 9, 56, 55, 9, 9, 55, 10, 55, 54, 10, 10, 54, 11, 54, 53, 11, 11, 53, 12, 53, 52, 12, 12, 52, 13, 52, 51, 13, 13, 51, 14, 51, 50, 14, 14, 50, 15, 50, 49, 15, 15, 49, 16, 49, 48, 16, 16, 48, 17, 48, 47, 17, 17, 47, 18, 47, 46, 18, 18, 46, 19, 46, 45, 19, 19, 45, 20, 45, 44, 20, 20, 44, 21, 44, 43, 21, 21, 43, 22, 43, 42, 22, 22, 42, 23, 42, 41, 23, 23, 41, 24, 41, 40, 24, 24, 40, 25, 40, 39, 25, 25, 39, 26, 39, 38, 26, 26, 38, 27, 38, 37, 27, 27, 37, 28, 37, 36, 28, 28, 36, 29, 36, 35, 29, 29, 35, 30, 35, 34, 30, 30, 34, 31, 34, 33, 31, 31, 33, 32, 64, 127, 65, 65, 127, 66, 127, 126, 66, 66, 126, 67, 126, 125, 67, 67, 125, 68, 125, 124, 68, 68, 124, 69, 124, 123, 69, 69, 123, 70, 123, 122, 70, 70, 122, 71, 122, 121, 71, 71, 121, 72, 121, 120, 72, 72, 120, 73, 120, 119, 73, 73, 119, 74, 119, 118, 74, 74, 118, 75, 118, 117, 75, 75, 117, 76, 117, 116, 76, 76, 116, 77, 116, 115, 77, 77, 115, 78, 115, 114, 78, 78, 114, 79, 114, 113, 79, 79, 113, 80, 113, 112, 80, 80, 112, 81, 112, 111, 81, 81, 111, 82, 111, 110, 82, 82, 110, 83, 110, 109, 83, 83, 109, 84, 109, 108, 84, 84, 108, 85, 108, 107, 85, 85, 107, 86, 107, 106, 86, 86, 106, 87, 106, 105, 87, 87, 105, 88, 105, 104, 88, 88, 104, 89, 104, 103, 89, 89, 103, 90, 103, 102, 90, 90, 102, 91, 102, 101, 91, 91, 101, 92, 101, 100, 92, 92, 100, 93, 100, 99, 93, 93, 99, 94, 99, 98, 94, 94, 98, 95, 98, 97, 95, 95, 97, 96,\
                    0, 127, 64, 32, 96, 97};

    GLUtils::genBuffer(vVertexBuffer);
    GLUtils::setArrayBuffer(vVertices, sizeof(vVertices), vVertexBuffer, GL_STATIC_DRAW);
    GLUtils::setVertexAttrib(positionLoc, vVertexBuffer);

    GLUtils::genBuffer(tVertexBuffer);
    GLUtils::setArrayBuffer(disArray, sizeof(disArray), tVertexBuffer, GL_STATIC_DRAW);
    GLUtils::setVertexAttrib(texCoordLoc, tVertexBuffer);

    GLUtils::genBuffer(indexObject);
    GLUtils::setElementArrayBuffer(indices, sizeof(indices), indexObject, GL_STATIC_DRAW);

    // Draw
    glDrawElements(GL_TRIANGLES, 378, GL_UNSIGNED_SHORT, 0);

    // swapFbt = new SwapFrameBufferTexture(width, height);
    // swapFbt->setFuncRenderBase(std::bind(&Context::oho, this));


    printf("over.\n");
}

// void Context::oho () {
//     printf("oho.\n");

// }


