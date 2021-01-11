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




std::string base_vsh = R"(

precision mediump float;

attribute vec3 aPosition;

attribute vec2 TexCoordIn;
varying vec2 TexCoordOut;

void main(void) {
    gl_Position = vec4(aPosition, 1.);

    TexCoordOut = TexCoordIn;
}
)";

std::string base_fsh = R"(

#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

uniform sampler2D Texture;
uniform sampler2D filterTexture;

uniform int do_filter; // 滤镜美颜开关
uniform int do_tilt;
uniform vec3 tilt_factor;

uniform float mouse_protection_value;
uniform vec2 mouth_center;
uniform float mouse_radius_x;
uniform float mouse_radius_y;
uniform int orientation;

varying vec2 TexCoordOut;

vec3 filterRGB(vec3 color) {
    mediump float blueColor = color.b * 63.0;

    mediump vec2 quad1;
    quad1.y = floor(floor(blueColor) / 8.0);
    quad1.x = floor(blueColor) - (quad1.y * 8.0);

    mediump vec2 quad2;
    quad2.y = floor(ceil(blueColor) / 8.0);
    quad2.x = ceil(blueColor) - (quad2.y * 8.0);

    mediump vec2 texPos1;
    texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.r);
    texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.g);

    mediump vec2 texPos2;
    texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.r);
    texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * color.g);

    lowp vec4 newColor1 = texture2D(filterTexture, texPos1);
    lowp vec4 newColor2 = texture2D(filterTexture, texPos2);

    lowp vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
    return newColor.rgb;
}

void main(void) {
    vec4 color = texture2D(Texture, TexCoordOut);

    if (1 == do_filter) {
        vec3 filtered_rgb = filterRGB(color.rgb);

        if (mouse_protection_value > 0.0) {
            float dis_y_mouse, dis_x_mouse;
            if (orientation == 0 || orientation == 180)
            {
                dis_x_mouse = TexCoordOut.x - mouth_center.x;
                dis_y_mouse = TexCoordOut.y - mouth_center.y;
            } else {
                dis_y_mouse = TexCoordOut.x - mouth_center.x;
                dis_x_mouse = TexCoordOut.y - mouth_center.y;
            }

            // mouth_dis_protection = 0.0最保护
            float mouth_dis_protection = 1.0 - mouse_protection_value * exp((-0.5) * (pow(dis_x_mouse / (0.6 * mouse_radius_x), 2.0) + pow(dis_y_mouse / (0.6 * mouse_radius_y), 2.0)));
            float geometric_protection = mouth_dis_protection;
            color.rgb = mix(color.rgb, filtered_rgb, geometric_protection);
        }
        else {
            color.rgb = filtered_rgb;
        }
    }

    if (1 == do_tilt) {
        float alpha = 0.5; // 目前获取不了肤色所以不能与美容shader统一，因此全屏tilt，设0.5为了缓和一些。
        vec3 _tilt_factor = tilt_factor;
        _tilt_factor.y = 0.95;
        color.rgb *=  alpha * _tilt_factor + (1.0 - alpha) * vec3(1.0);
    }

    gl_FragColor = color;
}
)";






// const char* mask_vsh = R"(
std::string mask_vsh = R"(

attribute vec3 aPosition;
attribute vec2 TexCoordIn;

varying float dis;

void main() {
    gl_Position = vec4(aPosition, 1.0);
    dis = TexCoordIn.x;
}

)";

// const char* mask_fsh = R"(
std::string mask_fsh = R"(

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