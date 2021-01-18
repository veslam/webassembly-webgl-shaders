#include "Context.h"
#include "shaders.h"
#include "GLUtils.cpp"
#include "Point.cpp"
// #include <time.h>

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


    // programObject = GLUtils::createProgram(vertexShader, fragmentShader);

    } else if (std::string(id) == "runTest") {
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






    // glGenTextures(1, &baseTexture);
    baseFbt = createFrameBufferTexture("base", width, height);

    loadShader("base", base_vsh, base_fsh);
    loadShader("skincolor", base_vsh, skincolor_fsh);
    loadShader("beauty", base_vsh, apply_effects_shader_fsh);
    loadShader("mask", mask_vsh, mask_fsh);
    loadShader("blur", base_vsh, blur_fsh);
    loadShader("lipcolor", base_vsh, lipcolor_fsh);
    loadShader("lipstick", lipstick_vsh, lipstick_fsh);
    // skincolor, beauty, apply_effects_shader, blur



    //printf("[Context] construct done. \n");
}

Context::~Context (void) {
    emscripten_webgl_destroy_context(context);
}

Point lmks81new (float faceLandmarks[81][2], int idx_MG) {
    return faceLandmarks[idx_MG];
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
    //printf("[Context] loadShader - %s \n", type.c_str());

    // load base shader
    GLuint vertexShader = GLUtils::compileShader(GL_VERTEX_SHADER, &vsh_str);
    GLuint fragmentShader = GLUtils::compileShader(GL_FRAGMENT_SHADER, &fsh_str);
    GLuint program = GLUtils::createProgram(vertexShader, fragmentShader);
    programs[type] = program;

    glUseProgram(program);

    // 设置不变参数
    if (0 == type.compare("lipstick")) {
        glUniform3f(glGetUniformLocation(program, "lightPos"), -0.5, -0.0, 2.0);
        glUniform3f(glGetUniformLocation(program, "viewPos"), 0.2, 0.0, 1.0);
        glUniform3f(glGetUniformLocation(program, "lightColor"), 1, 1, 1);
        glUniform1f(glGetUniformLocation(program, "transparency"), 1.0);
    }
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






// void Context::draw (uint8_t* buffer) {
void Context::draw (GLuint texture_offset) {
    //printf("[Context] draw \n");

    // Make the context current and use the program
    emscripten_webgl_make_context_current(context);

    // clock_t start, end;
    // start = clock();

    // for (int i = 0; i < 500; i ++) {
        _draw(texture_offset);
    // }

    // end = clock();
    // printf("duration = %f \n", float(end-start)/CLOCKS_PER_SEC);
}

// void Context::_draw (uint8_t* buffer) {
void Context::_draw (GLuint texture_offset) {
    // printf("[Context] _draw  texture_offset = %d\n", texture_offset);


    float lmks81[81][2] = {{357.0332251545839, 590.5873488917221}, {215.14550342016963, 354.35155725984316}, {213.04614047250232, 387.9332321022413}, {214.28205932648785, 421.73824826523037}, {218.49465782187605, 455.1360912400546}, {226.13735631966767, 487.98334302617593}, {240.21737437721393, 518.8410604348464}, {261.0069418604583, 545.5176331937074}, {287.77947890670305, 567.6712365389187}, {319.57531097347515, 583.8518523742041}, {503.2887269600001, 370.96338252946214}, {501.7399268066724, 402.50144970738813}, {497.4047897035719, 434.1596804741952}, {490.0274804270349, 465.43116348879124}, {480.05247024092444, 496.325112740638}, {465.5608485432367, 525.3644679163765}, {446.16331922454725, 550.5079466173871}, {421.5594255459295, 570.9347493235493}, {392.30541926165625, 585.8267378026521}, {305.64214364482336, 361.62410789809104}, {277.3051081665667, 356.44335805002277}, {291.2770252041454, 360.3090269118869}, {319.7426810935558, 360.75474356272827}, {305.49147663225364, 358.7569061539398}, {333.02574303570447, 358.5954576376949}, {305.35316229669604, 356.9375472870163}, {291.19316372686654, 356.70682904515434}, {319.74524677645104, 357.0435150404162}, {257.6817174930984, 319.3390818312147}, {279.47800471786786, 318.7902535279158}, {301.912646053452, 318.0245750536802}, {325.3188811459787, 318.30075022490456}, {348.5954913531743, 316.6956678333672}, {277.0857882545713, 303.07903761197}, {302.26610945625964, 297.96296455704}, {328.3009021010437, 302.01172809691485}, {312.021730686963, 499.9690095042882}, {364.274326490471, 522.303374506202}, {339.87683849055895, 501.78485034523345}, {326.5617764987429, 511.0190279068046}, {343.79195748003656, 518.6904983653795}, {389.6233374389966, 504.4028020296018}, {400.0554110936999, 515.0212291890139}, {383.7302006091612, 520.9424186241677}, {366.02619337289883, 503.40071831042496}, {413.27274514888336, 505.50809083559653}, {366.33825070246326, 501.55651137506914}, {354.30496531928077, 484.17756215117663}, {333.0257462174827, 490.6118585183475}, {340.33869225665035, 499.66456447232684}, {381.08908472058386, 485.84286864298457}, {398.34356847894605, 494.28438887557036}, {389.7183962405296, 502.3748297948406}, {367.587175102182, 488.014752213433}, {351.832539512131, 360.23484131504495}, {337.8058545795142, 413.2140858459112}, {348.005441886836, 446.8767212371872}, {371.40358056546836, 454.02542675136135}, {397.9740354845244, 363.0987330344175}, {405.6525922931681, 417.46087741352244}, {393.0099630395986, 449.0623751378715}, {326.83643862961196, 436.4908122208589}, {412.44373045363704, 441.40577677234546}, {374.7989263154989, 425.58895634714884}, {441.004124528158, 369.89011350204663}, {414.2110206896884, 363.23451576982205}, {427.2944842402255, 367.4562973490716}, {454.0955762992904, 369.93209788159726}, {440.34175609594206, 367.4370353217792}, {466.76889584726257, 368.01113713597175}, {441.536940106374, 365.5557056395867}, {427.57855965289104, 363.8931455632987}, {454.6267900001318, 366.8185625801343}, {409.11119564471534, 320.8150883472871}, {430.4963283300678, 324.7898969276757}, {451.90439866883605, 327.3170183575162}, {471.17684771086766, 330.841273903791}, {489.19044014590537, 333.77968371415466}, {429.7362874222158, 308.85816138807456}, {454.18050041144176, 307.8948484268239}, {476.05615034018274, 315.69593002325865}};

    float lipsPosition[128][2] = {{299.86060800221543, 500.1942056319774}, {303.54712625814, 499.1759157035221}, {308.1796577454476, 498.35563878549857}, {312.8848733746257, 497.77438838045487}, {317.23155416747534, 497.08737966379806}, {321.6726603962738, 496.4784677409902}, {326.5493102971345, 495.8818473921501}, {330.8557772882582, 495.2544235003842}, {335.12508530858213, 494.59391441030505}, {339.95707302138237, 493.6663234890571}, {344.54588860966936, 492.778445931198}, {348.20442764741557, 492.0413350378427}, {352.5160575529061, 491.4909597725839}, {357.3516127894546, 491.58154714733536}, {362.17196753616827, 492.22052164464344}, {366.34273448036544, 493.23941350702114}, {370.4068999101023, 494.25091707186334}, {373.43421844067655, 493.836415455185}, {376.6361139840593, 492.8867508663899}, {379.7699751691415, 491.9634395825741}, {383.0157520501404, 491.13239063448873}, {386.523480334914, 490.8536363709366}, {390.2775095264459, 491.0263105526034}, {394.0019562722593, 491.6177060879968}, {397.4183554148648, 492.354627965282}, {400.38549728542455, 493.31958109662395}, {402.88089933436487, 494.5598208117632}, {405.45755376077545, 495.996332374433}, {408.0014110025233, 497.2450252214689}, {410.2543255061651, 498.8954606790338}, {411.41348512566856, 500.507090886071}, {412.90754975911364, 502.0027719909114}, {413.8909792961408, 503.5453299714245}, {412.45295432278147, 503.32040940333616}, {410.1820226121441, 503.4132257619857}, {408.008919006441, 503.4174975135596}, {405.23158080734026, 503.3498053359672}, {402.44474045950307, 503.41755443120417}, {399.73575368170646, 503.36876883569204}, {397.0665368558051, 503.3092139418361}, {394.08481469598655, 503.0982578491109}, {390.49208978535535, 503.0522314910547}, {386.90716712432874, 503.2358323080193}, {383.43453655561467, 503.5198315194025}, {380.43997654619267, 503.8634708350154}, {377.84388227943896, 504.173138189359}, {375.09240466713806, 504.7113787614054}, {371.8909382003125, 504.8853111552063}, {368.71886109416687, 505.0265735387029}, {364.9525884567473, 505.01447529476184}, {360.37688175485164, 504.7035805657141}, {355.9574579105996, 503.8688400995989}, {351.81800984529696, 503.3353209544905}, {348.2807056674742, 502.8575285196591}, {344.5333292302256, 502.4706188680753}, {340.08822841480935, 502.109347448681}, {335.5093656140315, 501.9678185301724}, {331.43612030741826, 501.9079610170048}, {327.26002882871137, 501.87568269230434}, {322.7670278333451, 501.8733160893776}, {318.3882375381015, 501.79334492230214}, {314.27037686747326, 501.60888661104775}, {310.08409465653483, 501.1557932332357}, {305.50152124125486, 500.68998740314345}, {301.3890172139182, 500.3087420950839}, {302.0191001691127, 500.41379073969375}, {305.97743190700214, 500.6127656674995}, {309.89374136325, 500.827938227146}, {314.3352291796679, 500.95322675070724}, {318.0499676644867, 501.36445701907286}, {322.2230169146257, 501.6389433792649}, {326.51168710920274, 501.9950640644306}, {330.73151618266445, 502.10883173916756}, {334.5931367280775, 502.4324239521821}, {338.69180967890077, 502.6136707531284}, {343.2628173979675, 502.964146412545}, {347.118893351666, 503.04695458898044}, {350.4222018072287, 503.3299381140073}, {354.01345206402175, 503.3836360808642}, {358.42042780748005, 503.5239160525719}, {363.03771738317386, 503.74269198055714}, {366.8627281246137, 503.68956039643234}, {369.97535059507453, 503.4103900992376}, {373.0787195391431, 503.16063746906127}, {375.9513233572532, 502.86780480134684}, {378.74592999697984, 502.4723640541171}, {381.59661147619124, 502.29194431331155}, {385.01173702910506, 502.036724946429}, {388.70859049782587, 502.4509242703887}, {392.48813973966094, 502.9237319648481}, {395.6859322606212, 503.30743860261225}, {398.58535276300137, 503.88347246631645}, {401.6290181360801, 504.38221747138215}, {404.5595093515012, 504.6545855646906}, {407.6753473904264, 504.63640595818924}, {410.06040706988546, 504.4205909641104}, {412.46307177521237, 504.0159011311181}, {411.72569145189254, 505.3100177796621}, {409.427535568247, 506.7105681453296}, {407.04603473642703, 508.4132302832055}, {403.83390389066784, 510.4936492480192}, {400.7829772745168, 512.6897414949299}, {397.84345287795395, 514.9206198170808}, {394.472306051631, 516.7313994216918}, {390.5978835010876, 518.3198580973303}, {386.47815940665106, 519.9327376761819}, {382.6473484851704, 521.3712863847512}, {379.509440386614, 522.330478648614}, {375.8856642533118, 523.5469116608168}, {372.0915882671711, 524.2279798621115}, {368.5003089530883, 524.8788429605688}, {364.5564877449492, 525.602279751328}, {360.41968384089705, 526.2505034745625}, {355.8798922371911, 526.0416657378844}, {352.1864310021032, 525.7746599858925}, {348.4871824886859, 525.3456768304693}, {344.0431620366595, 524.8603328843578}, {339.59727867171284, 523.7789081994756}, {335.4551928158064, 522.593369940596}, {331.2264802588888, 521.1727551858866}, {326.88723909019586, 519.5419748843359}, {322.74548861563875, 517.7486639701563}, {318.98373274568064, 515.5957735274794}, {315.35406669658727, 513.6229509754788}, {311.4474809444819, 510.9439927021648}, {307.90553234219055, 508.0965756018122}, {304.81540523119656, 505.29048745619855}, {302.1715036659939, 502.91465326375777}};

    // ------------------------------------------------------------

    // buffer -> baseTexture
    // glViewport(0, 0, width, height);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, 0);
    // GLUtils::bindTextureWithData(baseTexture, width, height, buffer);


    // bool lmksReady = !(nullptr == lmks81 || nullptr == lipsPosition);
    bool lmksReady = true;

    // render to baseFbt
    if (lmksReady) {
        baseFbt->useFrameBuffer();
        setUnif1i("base", "Texture", texture_offset);
        renderBase(programs["base"]);
    }
    // 直接画到屏幕 tested & closed switch.
    else {
        setUnif1i("base", "Texture", texture_offset);
        renderBase(programs["base"], true);
        return;
    }
    // 尽快恢复
    setUnif1i("base", "Texture", 0);


    // 画到 Swap
    // create Swap
    if (nullptr == swapFbt) {
        swapFbt = new SwapFrameBufferTexture(width, height);
        swapFbt->setFuncRenderBase(std::bind(&Context::rb, this));
    }


    // [baseTexture -> swap]
    swapFbt->getToWrite()->useFrameBuffer();
    glActiveTexture(GL_TEXTURE0);
    baseFbt->useTexture();
    renderBase(programs["base"]);
    swapFbt->swap();



    // ----------------------------- start applying effects -----------------------------

    FrameBufferTexture* skinColor = createFrameBufferTexture("skincolor", 1, 1);
    skinColor->useFrameBuffer();
    glActiveTexture(GL_TEXTURE0);
    baseFbt->useTexture();
    renderSkinColor(width, height, lmks81);

    // [swap -> beauty]
    FrameBufferTexture* beauty = createFrameBufferTexture("beauty", width, height);
    beauty->useFrameBuffer();
    glActiveTexture(GL_TEXTURE0);
    swapFbt->getToRead()->useTexture();
    renderBeauty(width, height, lmks81, skinColor);

    // [beauty -> swap]
    swapFbt->getToWrite()->useFrameBuffer();
    glActiveTexture(GL_TEXTURE0);
    beauty->useTexture();
    renderBase(programs["base"]);
    swapFbt->swap();

    // mask
    bool need_render_lips_mask = true;
    bool need_render_eyes_mask = false;
    bool need_render_mask = need_render_lips_mask || need_render_eyes_mask;

    if (need_render_mask && programs.find("mask") != programs.end()) {

        // 画嘴唇Alpha [beauty -> base_2]
        FrameBufferTexture* base_2 = createFrameBufferTexture("base_2", width/2, height/2);
        base_2->useFrameBuffer();
        glActiveTexture(GL_TEXTURE0);
        beauty->useTexture();
        renderBase(programs["base"]);
        // validated.

        // [base_2 -> mask_shape]
        FrameBufferTexture* mask_shape = createFrameBufferTexture("mask_shape", width, height);
        mask_shape->useFrameBuffer();
        glActiveTexture(GL_TEXTURE0);
        base_2->useTexture();

        renderMasks(width, height, lipsPosition, nullptr);

        // [mask_shape -> mask_blurred]
        FrameBufferTexture* mask_blurred = createFrameBufferTexture("mask_blurred", width, height);
        mask_blurred->useFrameBuffer();
        glActiveTexture(GL_TEXTURE0);
        mask_shape->useTexture();
        renderMaskBlurred(width, height, lmks81);


    // // debug
    // swapFbt->getToWrite()->useFrameBuffer();
    // glActiveTexture(GL_TEXTURE0);
    // mask_blurred->useTexture();
    // renderBase(programs["base"]);
    // swapFbt->swap();

    // renderToScreen();
    // return;

        // lipcolor
        if (need_render_lips_mask) {
            float lipsLmks_1d[128 * 2];
            for (int i = 0; i < 128; i ++) {
                lipsLmks_1d[i * 2] = lipsPosition[i][0] / width;
                lipsLmks_1d[i * 2 + 1] = lipsPosition[i][1] / height;
            }

            FrameBufferTexture* lipsColor = createFrameBufferTexture("lipscolor", 2, 2);
            lipsColor->useFrameBuffer();
            glActiveTexture(GL_TEXTURE0);
            baseFbt->useTexture();
            renderLipsColor(lipsLmks_1d);

            if (nullptr == lipsColorData) {
                lipsColorData = new GLubyte[2*2*4];
            }
            glReadPixels(0, 0, 2, 2, GL_RGBA, GL_UNSIGNED_BYTE, lipsColorData);

            float maxLipsColor[3] = {float(int(lipsColorData[0])), float(int(lipsColorData[1])), float(int(lipsColorData[2]))};
            float avgLipsColor[3] = {float(int(lipsColorData[12])), float(int(lipsColorData[13])), float(int(lipsColorData[14]))};
            // validated.




            // TODO smooth

            {
            setUnif1f("lipstick", "videoWidth", width);
            setUnif1f("lipstick", "videoHeight", height);
            }

            swapFbt->getToWrite()->useFrameBuffer();
            glActiveTexture(GL_TEXTURE0);
            swapFbt->getToRead()->useTexture();
            // 画口红 [swap -> gl.TEXTURE0, mask_blurred -> gl.TEXTURE1]
            renderLips(width, height, lipsPosition, lipsLmks_1d, avgLipsColor, maxLipsColor, mask_blurred);
            // renderLips(width, height, lipsPosition, lipsLmks_1d);
            swapFbt->swap();
        }
        if (need_render_eyes_mask) {}
    }

    renderToScreen();
}


//---------------------------------------------------------------------------

void Context::renderToScreen () {
    //printf("[Context] renderToScreen \n");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);
    glActiveTexture(GL_TEXTURE0);
    swapFbt->getToRead()->useTexture();
    renderBase(programs["base"], true);
}

void Context::rb () {
    renderBase(programs["base"]);
}
void Context::renderBase (GLuint program, bool revert, bool do_tilt, bool do_filter) {
    //printf("[Context] renderBase program = %d \n", program);
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
    GLint positionLoc = glGetAttribLocation(program, "aPosition");
    GLint texCoordLoc = glGetAttribLocation(program, "TexCoordIn");


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


void Context::renderSkinColor(int width, int height, float faceLandmarks[81][2]) {
    //printf("[Context] renderSkinColor \n");

    setUnif2f("skincolor", "p39", faceLandmarks[39][0] / width, faceLandmarks[39][1] / height);
    setUnif2f("skincolor", "p43", faceLandmarks[43][0] / width, faceLandmarks[43][1] / height);
    setUnif2f("skincolor", "p73", faceLandmarks[73][0] / width, faceLandmarks[73][1] / height);
    setUnif2f("skincolor", "p76", faceLandmarks[76][0] / width, faceLandmarks[76][1] / height);
    setUnif2f("skincolor", "p75", faceLandmarks[75][0] / width, faceLandmarks[75][1] / height);
    setUnif2f("skincolor", "p77", faceLandmarks[77][0] / width, faceLandmarks[77][1] / height);

    setUnif2f("skincolor", "p65", faceLandmarks[65][0] / width, faceLandmarks[65][1] / height);
    setUnif2f("skincolor", "p66", faceLandmarks[66][0] / width, faceLandmarks[66][1] / height);
    setUnif2f("skincolor", "p67", faceLandmarks[67][0] / width, faceLandmarks[67][1] / height);
    setUnif2f("skincolor", "p68", faceLandmarks[68][0] / width, faceLandmarks[68][1] / height);
    setUnif2f("skincolor", "p38", faceLandmarks[38][0] / width, faceLandmarks[38][1] / height);
    setUnif2f("skincolor", "p40", faceLandmarks[40][0] / width, faceLandmarks[40][1] / height);

    renderBase(programs["skincolor"]);
}

void Context::renderBeauty(int width, int height, float faceLandmarks[81][2], FrameBufferTexture* skinColor) {
    //printf("[Context] renderBeauty \n");

    GLuint program = programs["beauty"];
    glUseProgram(program);

    glActiveTexture(GL_TEXTURE1);
    skinColor->useTexture();
    glUniform1i(glGetUniformLocation(program, "faceColorTexture"), 1);

    float skin_smooth_radius = 10.0 * sqrt(width * height / float(1920 * 1080));
    if (skin_smooth_radius < 10.0) {
        skin_smooth_radius = 10.0;
    }

    float chin[2] = {faceLandmarks[64][0] / width, faceLandmarks[64][1] / height};

    float nose_tip[2] = {faceLandmarks[34][0] / width, faceLandmarks[34][1] / height};
    float face_center[2] = {faceLandmarks[34][0] / width, faceLandmarks[34][1] / height};
    float left_eye_center[2] = {faceLandmarks[0][0] / width, faceLandmarks[0][1] / height};
    float right_eye_center[2] = {faceLandmarks[9][0] / width, faceLandmarks[9][1] / height};
    float mouth_top[2] = {faceLandmarks[47][0] / width, faceLandmarks[47][1] / height};
    float mouth_bottom[2] = {faceLandmarks[54][0] / width, faceLandmarks[54][1] / height};
    float mouth_center[2] = {(mouth_top[0] + mouth_bottom[0]) / 2, (mouth_top[1] + mouth_bottom[1]) / 2};
    float singleStepOffset[2] = {float(1) / width, float(1) / height};

    float face_radius = sqrt((face_center[0] - chin[0]) * (face_center[0] - chin[0]) + (face_center[1] - chin[1]) * (face_center[1] - chin[1]));
    float mouth_rightmost[2] = {faceLandmarks[45][0] / width, faceLandmarks[45][1] / height};
    float mouth_leftmost[2] = {faceLandmarks[44][0] / width, faceLandmarks[44][1] / height};
    float mouth_topmost[2] = {faceLandmarks[46][0] / width, faceLandmarks[46][1] / height};
    float mouth_lowmost[2] = {faceLandmarks[55][0] / width, faceLandmarks[55][1] / height};
    float mouse_radius_x = 0.5 * sqrt((mouth_rightmost[0] - mouth_leftmost[0]) * (mouth_rightmost[0] - mouth_leftmost[0]) + (mouth_rightmost[1] - mouth_leftmost[1]) * (mouth_rightmost[1] - mouth_leftmost[1]));
    float mouse_radius_y = 0.5 * sqrt((mouth_topmost[0] - mouth_lowmost[0]) * (mouth_topmost[0] - mouth_lowmost[0]) + (mouth_topmost[1] - mouth_lowmost[1]) * (mouth_topmost[1] - mouth_lowmost[1]));

    float left_eye_rightmost[2] = {faceLandmarks[1][0] / width, faceLandmarks[1][1] / height};
    float left_eye_leftmost[2] = {faceLandmarks[2][0] / width, faceLandmarks[2][1] / height};
    float left_eye_topmost[2] = {faceLandmarks[3][0] / width, faceLandmarks[3][1] / height};
    float left_eye_lowmost[2] = {faceLandmarks[4][0] / width, faceLandmarks[4][1] / height};
    float left_eye_radius_x = 0.5 * sqrt((left_eye_rightmost[0] - left_eye_leftmost[0]) * (left_eye_rightmost[0] - left_eye_leftmost[0]) + (left_eye_rightmost[1] - left_eye_leftmost[1]) * (left_eye_rightmost[1] - left_eye_leftmost[1]));
    float left_eye_radius_y = 0.5 * sqrt((left_eye_topmost[0] - left_eye_lowmost[0]) * (left_eye_topmost[0] - left_eye_lowmost[0]) + (left_eye_topmost[1] - left_eye_lowmost[1]) * (left_eye_topmost[1] - left_eye_lowmost[1]));

    float right_eye_rightmost[2] = {faceLandmarks[10][0] / width, faceLandmarks[10][1] / height};
    float right_eye_leftmost[2] = {faceLandmarks[11][0] / width, faceLandmarks[11][1] / height};
    float right_eye_topmost[2] = {faceLandmarks[12][0] / width, faceLandmarks[12][1] / height};
    float right_eye_lowmost[2] = {faceLandmarks[13][0] / width, faceLandmarks[13][1] / height};
    float right_eye_radius_x = 0.5 * sqrt((right_eye_rightmost[0] - right_eye_leftmost[0]) * (right_eye_rightmost[0] - right_eye_leftmost[0]) + (right_eye_rightmost[1] - right_eye_leftmost[1]) * (right_eye_rightmost[1] - right_eye_leftmost[1]));
    float right_eye_radius_y = 0.5 * sqrt((right_eye_topmost[0] - right_eye_lowmost[0]) * (right_eye_topmost[0] - right_eye_lowmost[0]) + (right_eye_topmost[1] - right_eye_lowmost[1]) * (right_eye_topmost[1] - right_eye_lowmost[1]));


    setUnif1f("beauty", "blend", 0);
    setUnif1f("beauty", "skin_smooth_coeff", 0.4); // 0.2
    setUnif1f("beauty", "skin_smooth_radius", skin_smooth_radius);
    setUnif1f("beauty", "brighten_face_coeff", 0.6); // 0.4
    setUnif1f("beauty", "face_radius", face_radius);
    setUnif1f("beauty", "mouse_radius_x", mouse_radius_x);
    setUnif1f("beauty", "mouse_radius_y", mouse_radius_y);
    setUnif1f("beauty", "left_eye_radius_y", left_eye_radius_y);
    setUnif1f("beauty", "left_eye_radius_x", left_eye_radius_x);
    setUnif1f("beauty", "right_eye_radius_y", right_eye_radius_y);
    setUnif1f("beauty", "right_eye_radius_x", right_eye_radius_x);

    setUnif2f("beauty", "nose_tip", nose_tip[0], nose_tip[1]);
    setUnif2f("beauty", "face_center", face_center[0], face_center[1]);
    setUnif2f("beauty", "left_eye_center", left_eye_center[0], left_eye_center[1]);
    setUnif2f("beauty", "right_eye_center", right_eye_center[0], right_eye_center[1]);
    setUnif2f("beauty", "mouth_center", mouth_center[0], mouth_center[1]);
    setUnif2f("beauty", "singleStepOffset", singleStepOffset[0], singleStepOffset[1]);

    setUnif1i("beauty", "need_color", 0);
    setUnif1i("beauty", "isParted", 0);
    setUnif1i("beauty", "orientation", 0);
    setUnif1i("beauty", "isDirectSkinColor", 0);

    renderBase(programs["beauty"]);
}


void Context::renderMasks (int width, int height, float lipsLandmarks[128][2], float eyesLandmarks[2][128][2]) {
    //printf("[Context] renderMasks \n");

    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint program = programs["mask"];
    glUseProgram(program);


    // Get the attribute/sampler locations
    GLint positionLoc = glGetAttribLocation(program, "aPosition");
    GLint texCoordLoc = glGetAttribLocation(program, "TexCoordIn");
    GLint modeLoc = glGetUniformLocation(program, "mode");

    // ------------------------------------------------------------

    if (nullptr != lipsLandmarks) {
        GLfloat aPositionArray[128 * 2];
        GLfloat texCoordArray[128 * 2];

        for (int i = 0; i < 128; i ++) {
            texCoordArray[i * 2] = lipsLandmarks[i][0] / width;
            texCoordArray[i * 2 + 1] = lipsLandmarks[i][1] / height;

            aPositionArray[i * 2] = texCoordArray[i * 2] * 2 - 1;
            aPositionArray[i * 2 + 1] = texCoordArray[i * 2 + 1] * 2 - 1;
        }


        GLUtils::genBuffer(vb_lips_vec);
        GLUtils::setArrayBuffer(aPositionArray, sizeof(aPositionArray), vb_lips_vec, GL_STATIC_DRAW);
        GLUtils::setVertexAttrib(positionLoc, vb_lips_vec);

        if (0 == vb_lips_dis) {
            GLfloat disArray[256];
            for (int i = 0; i < 128; i ++) {
                disArray[i * 2] = disArray[i * 2 + 1] = 1;
            }
            GLUtils::genBuffer(vb_lips_dis);
            GLUtils::setArrayBuffer(disArray, sizeof(disArray), vb_lips_dis, GL_STATIC_DRAW);
        }
        GLUtils::setVertexAttrib(texCoordLoc, vb_lips_dis);

        glUniform1i(modeLoc, 0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        if (0 == ib_lips) {
            GLushort indices[] = {0, 63, 1, 1, 63, 2, 63, 62, 2, 2, 62, 3, 62, 61, 3, 3, 61, 4, 61, 60, 4, 4, 60, 5, 60, 59, 5, 5, 59, 6, 59, 58, 6, 6, 58, 7, 58, 57, 7, 7, 57, 8, 57, 56, 8, 8, 56, 9, 56, 55, 9, 9, 55, 10, 55, 54, 10, 10, 54, 11, 54, 53, 11, 11, 53, 12, 53, 52, 12, 12, 52, 13, 52, 51, 13, 13, 51, 14, 51, 50, 14, 14, 50, 15, 50, 49, 15, 15, 49, 16, 49, 48, 16, 16, 48, 17, 48, 47, 17, 17, 47, 18, 47, 46, 18, 18, 46, 19, 46, 45, 19, 19, 45, 20, 45, 44, 20, 20, 44, 21, 44, 43, 21, 21, 43, 22, 43, 42, 22, 22, 42, 23, 42, 41, 23, 23, 41, 24, 41, 40, 24, 24, 40, 25, 40, 39, 25, 25, 39, 26, 39, 38, 26, 26, 38, 27, 38, 37, 27, 27, 37, 28, 37, 36, 28, 28, 36, 29, 36, 35, 29, 29, 35, 30, 35, 34, 30, 30, 34, 31, 34, 33, 31, 31, 33, 32, 64, 127, 65, 65, 127, 66, 127, 126, 66, 66, 126, 67, 126, 125, 67, 67, 125, 68, 125, 124, 68, 68, 124, 69, 124, 123, 69, 69, 123, 70, 123, 122, 70, 70, 122, 71, 122, 121, 71, 71, 121, 72, 121, 120, 72, 72, 120, 73, 120, 119, 73, 73, 119, 74, 119, 118, 74, 74, 118, 75, 118, 117, 75, 75, 117, 76, 117, 116, 76, 76, 116, 77, 116, 115, 77, 77, 115, 78, 115, 114, 78, 78, 114, 79, 114, 113, 79, 79, 113, 80, 113, 112, 80, 80, 112, 81, 112, 111, 81, 81, 111, 82, 111, 110, 82, 82, 110, 83, 110, 109, 83, 83, 109, 84, 109, 108, 84, 84, 108, 85, 108, 107, 85, 85, 107, 86, 107, 106, 86, 86, 106, 87, 106, 105, 87, 87, 105, 88, 105, 104, 88, 88, 104, 89, 104, 103, 89, 89, 103, 90, 103, 102, 90, 90, 102, 91, 102, 101, 91, 91, 101, 92, 101, 100, 92, 92, 100, 93, 100, 99, 93, 93, 99, 94, 99, 98, 94, 94, 98, 95, 98, 97, 95, 95, 97, 96,\
                    0, 127, 64, 32, 96, 97};
            GLUtils::genBuffer(ib_lips);
            GLUtils::setElementArrayBuffer(indices, sizeof(indices), ib_lips, GL_STATIC_DRAW);
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_lips);

        // Draw
        glDrawElements(GL_TRIANGLES, 378, GL_UNSIGNED_SHORT, 0);
    }
    //printf("over.\n");
}

void Context::renderMaskBlurred (int width, int height, float faceLandmarks[81][2]) {
    //printf("[Context] renderMaskBlurred \n");

    glClearColor(0, 0, 0, 1); // black
    glClear(GL_COLOR_BUFFER_BIT);

    float mouthWidth = pDistance(lmks81new(faceLandmarks, 44), lmks81new(faceLandmarks, 45));
    float mouthHeight = (pDistance(lmks81new(faceLandmarks, 48), lmks81new(faceLandmarks, 55)) + pDistance(lmks81new(faceLandmarks, 49), lmks81new(faceLandmarks, 55))) / 2;
    float mouth_open = 0.5 * pDistance(lmks81new(faceLandmarks, 47), lmks81new(faceLandmarks, 54));
    float ratio_x = 0.025;
    float ratio_y = 0.05;
    setUnif2f("blur", "kSize", ratio_x * mouthWidth / width, ratio_y * (mouthHeight - mouth_open) / height);

    renderBase(programs["blur"]);
}

void Context::renderLipsColor (float lipsLmks_1d[256]) {
    //printf("[Context] renderLipsColor \n");
    
    setUnif2fv("lipcolor", "p", 256, lipsLmks_1d);
    renderBase(programs["lipcolor"]);
}

void Context::renderLips (int width, int height, float lipsLandmarks[128][2], float lipsLmks_1d[256], float avgLipsColor[3], float maxLipsColor[3], FrameBufferTexture* mask) {
    //printf("[Context] renderLips \n");

    GLuint program = programs["lipstick"];
    glUseProgram(program);

    setUnif3f("lipstick", "new_lipavgcolor", avgLipsColor[0]/255, avgLipsColor[1]/255, avgLipsColor[2]/255);
    setUnif3f("lipstick", "new_lipmaxcolor", maxLipsColor[0]/255, maxLipsColor[1]/255, maxLipsColor[2]/255);

    setUnif2fv("lipstick", "point", 256, lipsLmks_1d);

    float radius = 10.0 * sqrt(width * height / (1920.0 * 1080.0));
    setUnif1f("lipstick", "new_smoothRadius", 2.0 * 0.075 * radius); // TODO get lipconfig->smoothRadius
    setUnif2f("lipstick", "singleStepOffset", 1.0/width, 1.0/height);

    float mouthWidth = 0.6 * (pDistance(lipsLandmarks[16], lipsLandmarks[48]) + pDistance(lipsLandmarks[80], lipsLandmarks[112]));
    setUnif1f("lipstick", "mouthWidth", mouthWidth);

    // debug ASM
    setUnif3f("lipstick", "color0", 1.0, 0.0, 0.0);


    GLint positionLoc = glGetAttribLocation(program, "aPosition");
    GLint texCoordBrowLoc = glGetAttribLocation(program, "TexCoordBrow");



    GLfloat aPositionArray[4 * 2];
    GLfloat texCoordArray[4 * 2];

    float box_x_a = std::min({lipsLandmarks[0][0], lipsLandmarks[32][0], lipsLandmarks[16][0], lipsLandmarks[112][0]});
    float box_x_b = std::max({lipsLandmarks[0][0], lipsLandmarks[32][0], lipsLandmarks[16][0], lipsLandmarks[112][0]});
    float box_y_a = std::min({lipsLandmarks[0][1], lipsLandmarks[32][1], lipsLandmarks[16][1], lipsLandmarks[112][1]});
    float box_y_b = std::max({lipsLandmarks[0][1], lipsLandmarks[32][1], lipsLandmarks[16][1], lipsLandmarks[112][1]});
    box_x_a -= mouthWidth;
    box_x_b += mouthWidth;
    box_y_a -= mouthWidth;
    box_y_b += mouthWidth;

    float pts4[4][2];
    pts4[0][0] = box_x_a; pts4[0][1] = box_y_a; // ld
    pts4[1][0] = box_x_b; pts4[1][1] = box_y_a; // rd
    pts4[2][0] = box_x_b; pts4[2][1] = box_y_b; // ru
    pts4[3][0] = box_x_a; pts4[3][1] = box_y_b; // lu

    for (int i = 0; i < 4; i++) {
        texCoordArray[i * 2] = pts4[i][0] / width;
        texCoordArray[i * 2 + 1] = pts4[i][1] / height;

        aPositionArray[i * 2] = texCoordArray[i * 2] * 2 - 1;
        aPositionArray[i * 2 + 1] = texCoordArray[i * 2 + 1] * 2 - 1;
    }

    GLUtils::genBuffer(vb_lips_box_vec);
    GLUtils::setArrayBuffer(aPositionArray, sizeof(aPositionArray), vb_lips_box_vec, GL_STATIC_DRAW);
    GLUtils::setVertexAttrib(positionLoc, vb_lips_box_vec);

    GLUtils::genBuffer(vb_lips_box_tex);
    GLUtils::setArrayBuffer(texCoordArray, sizeof(texCoordArray), vb_lips_box_tex, GL_STATIC_DRAW);
    GLUtils::setVertexAttrib(texCoordBrowLoc, vb_lips_box_tex);

    glUniform1i(glGetUniformLocation(program, "Texture"), 0);

    glActiveTexture(GL_TEXTURE1);
    mask->useTexture();
    glUniform1i(glGetUniformLocation(program, "Texture2"), 1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    if (0 == ib_lips_box) {
        GLushort indices[] = {0, 1, 3, 3, 1, 2};
        GLUtils::genBuffer(ib_lips_box);
        GLUtils::setElementArrayBuffer(indices, sizeof(indices), ib_lips_box, GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ib_lips_box);

    // Draw
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
}

//---------------------------------------------------------------------------
FrameBufferTexture* Context::createFrameBufferTexture (std::string name, int width, int height) {
    if (fbts.find(name) == fbts.end()) {
        fbts[name] = new FrameBufferTexture(width, height);
    }

    // float hh[1][2] = {{1.0, 2.0}};
    // Point a = Point(hh[0]);
    // Point b = Point(3.0, 4.0);
    // Point c = pAdd(a, b);
    // //printf("c = %f, %f \n", c.x, c.y);

    return fbts[name];
}
