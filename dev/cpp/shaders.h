// Shaders
std::string vertex_source =
    "attribute vec4 position;   \n"
    "attribute vec2 texCoord;   \n"
    "varying vec2 v_texCoord;     \n"
    "void main()                  \n"
    "{                            \n"
    "   gl_Position = position; \n"
    "   v_texCoord = texCoord;  \n"
    "}                            \n";

std::string texture_load_fragment_source =
    "precision mediump float;                            \n"
    "varying vec2 v_texCoord;                            \n"
    "uniform sampler2D texture;                        \n"
    "void main()                                         \n"
    "{                                                   \n"
    "  gl_FragColor = texture2D( texture, v_texCoord );   \n"
    "}                                                   \n";


std::string edge_detect_fragment_source =
    "precision mediump float;                            \n"
    "varying vec2 v_texCoord;                            \n"
    "uniform sampler2D texture;                        \n"
    "uniform float width;  \n"
    "uniform float height;  \n"
    "void main()                                         \n"
    "{                                                   \n"
    "  vec4 pixel = texture2D(texture, v_texCoord);              \n"
    "  vec4 n[9];\n"

    "  float w = 1.0 / width;\n"
    "  float h = 1.0 / height;\n"

    "  n[0] = texture2D(texture, v_texCoord + vec2(0.0, 0.0) );\n"
    "  n[1] = texture2D(texture, v_texCoord + vec2(w, 0.0) );\n"
    "  n[2] = texture2D(texture, v_texCoord + vec2(2.0*w, 0.0) );\n"
    "  n[3] = texture2D(texture, v_texCoord + vec2(0.0*w, h) );\n"
    "  n[4] = texture2D(texture, v_texCoord + vec2(w, h) );\n"
    "  n[5] = texture2D(texture, v_texCoord + vec2(2.0*w, h) );\n"
    "  n[6] = texture2D(texture, v_texCoord + vec2(0.0, 2.0*h) );\n"
    "  n[7] = texture2D(texture, v_texCoord + vec2(w, 2.0*h) );\n"
    "  n[8] = texture2D(texture, v_texCoord + vec2(2.0*w, 2.0*h) );\n"

    "  vec4 sobel_x = n[2] + (2.0*n[5]) + n[8] - (n[0] + (2.0*n[3]) + n[6]);\n"
    "  vec4 sobel_y = n[0] + (2.0*n[1]) + n[2] - (n[6] + (2.0*n[7]) + n[8]);\n"

    "  float avg_x = (sobel_x.r + sobel_x.g + sobel_x.b) / 3.0;\n"
    "  float avg_y = (sobel_y.r + sobel_y.g + sobel_y.b) / 3.0;\n"

    "  sobel_x.r = avg_x;\n"
    "  sobel_x.g = avg_x;\n"
    "  sobel_x.b = avg_x;\n"
    "  sobel_y.r = avg_y;\n"
    "  sobel_y.g = avg_y;\n"
    "  sobel_y.b = avg_y;\n"

    "  vec3 sobel = vec3(sqrt((sobel_x.rgb * sobel_x.rgb) + (sobel_y.rgb * sobel_y.rgb)));\n"
    "  gl_FragColor = vec4( sobel, 1.0 );   \n"
    "}                                                   \n";


const char* mask_vsh = R"(

attribute vec3 aPosition;
attribute vec2 TexCoordIn;

varying float dis;

void main() {
    gl_Position = vec4(aPosition, 1.0);
    dis = TexCoordIn.x;
}

)";

const char* mask_fsh = R"(

precision highp float;
varying float dis;
uniform int mode;

void main() {
    float weight = 1.0;

    if (0 == mode) // lips
    {
        weight = 1.0;
    }
    else if (1 == mode) // eyes
    {
        weight = 1.0;
        if (dis < 0.05) {
            weight = 0.5 + 0.5 * (dis/0.05);
        }
        else if (dis > 0.95) {
            weight = 1.0 - 0.5 * ((dis-0.95)/0.05);
        }
    }

    gl_FragColor = vec4(vec3(weight), 1.0);
}

)";