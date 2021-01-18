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



std::string skincolor_fsh = R"(

precision mediump float;
varying vec2 TexCoordOut;
uniform sampler2D Texture;

uniform vec2 p39;
uniform vec2 p43;
uniform vec2 p73;
uniform vec2 p76;
uniform vec2 p75;
uniform vec2 p77;

uniform vec2 p65;
uniform vec2 p66;
uniform vec2 p67;
uniform vec2 p68;
uniform vec2 p38;
uniform vec2 p40;


vec4 getFaceSkinColor()
{
    mediump vec4 rgba = vec4(0.0,0.0,0.0,1.0);
    // //由于在高通636，845上只要一出现立即数，就会产生GLerror 501的错误，此处暂用此方法使rgba = vec4(0,0,0,1.0);
    // vec4 rgba = texture2D(Texture, TexCoordOut);
    // rgba = vec4(rgba.rgb - rgba.rgb,1.0);

    mediump vec4 d1 = texture2D(Texture, (p73 + p39)/2.0);
    mediump vec4 d2 = texture2D(Texture, (p75 + p43)/2.0);
    mediump vec4 d3 = texture2D(Texture, (p76 + p43)/2.0);
    mediump vec4 d4 = texture2D(Texture, (p77 + p43)/2.0);
    mediump vec4 d5 = texture2D(Texture, (p38 + p65)/2.0);
    mediump vec4 d6 = texture2D(Texture, (p40 + p66)/2.0);
    mediump vec4 d7 = texture2D(Texture, (p40 + p67)/2.0);
    mediump vec4 d8 = texture2D(Texture, (p40 + p68)/2.0);
    mediump vec4 avr = (d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8)/8.0;

    float diffmax = sqrt(0.12);//0.2*0.2 + 0.2*0.2 +0.2*0.2 = 0.12
    float num = 0.0;
    // for debug use
    if(distance(d1,avr) < diffmax) {rgba += d1;num = num + 1.0;} //else p1Color = vec4(1,0,0,1);
    if(distance(d2,avr) < diffmax) {rgba += d2;num = num + 1.0;} //else p2Color = vec4(1,0,0,1);
    if(distance(d3,avr) < diffmax) {rgba += d3;num = num + 1.0;} //else p3Color = vec4(1,0,0,1);
    if(distance(d4,avr) < diffmax) {rgba += d4;num = num + 1.0;} //else p4Color = vec4(1,0,0,1);
    if(distance(d5,avr) < diffmax) {rgba += d5;num = num + 1.0;} //else p5Color = vec4(1,0,0,1);
    if(distance(d6,avr) < diffmax) {rgba += d6;num = num + 1.0;} //else p6Color = vec4(1,0,0,1);
    if(distance(d7,avr) < diffmax) {rgba += d7;num = num + 1.0;} //else p7Color = vec4(1,0,0,1);
    if(distance(d8,avr) < diffmax) {rgba += d8;num = num + 1.0;} //else p8Color = vec4(1,0,0,1);
    if(0.0 != num)
    {
        rgba = rgba/num;
    }
    else
    {
        rgba = avr;
    }

    return rgba;
}

void main(void) {
    gl_FragColor = getFaceSkinColor();
}
)";

std::string apply_effects_shader_fsh = R"(

precision mediump float;

varying mediump vec2 TexCoordOut;
uniform sampler2D Texture;
uniform sampler2D Texture2;

uniform int need_color;
uniform vec3 eyebrow_color;
uniform  float blend;
uniform  float skin_smooth_coeff;
uniform  float skin_smooth_radius;


uniform  float brighten_face_coeff;
uniform  float face_radius;
uniform  float mouse_radius_x;
uniform  float mouse_radius_y;

uniform  float nose_radius_y;
uniform  float nose_radius_x;

uniform  float left_eye_radius_y;
uniform  float left_eye_radius_x;
uniform  float right_eye_radius_y;
uniform  float right_eye_radius_x;

uniform vec2 singleStepOffset;
uniform vec2 face_center;
uniform vec2 left_eye_center;
uniform vec2 right_eye_center;

uniform vec2 nose_tip;

uniform vec2 mouth_center;


uniform int orientation;

uniform bool isDirectSkinColor;
uniform vec3 skin_color;
uniform sampler2D faceColorTexture;

uniform bool isParted;
uniform int partIndex;
uniform float part_scale_w;
uniform float part_scale_h;

const int boxSize = 24;
const float amplify = 32.0;
const float threshold = 6.0;




const vec3 lumiNorm = vec3(0.299, 0.587, 0.114);
#define OVERLAY(a, b) ((a) < 0.5 ? 2.0 * (a) * (b) : 1.0 - 2.0 * (1.0 - (a)) * (1.0 - (b)))

vec3 hsv2rgb(float h, float s, float v)
{
    float C = v*s;
    float hh = h * 6.0;
    float X = C*(1.0-abs(mod(hh,2.0)-1.0));
    float r,g,b;
    r = g = b = 0.0;
    if( hh>=0.0 && hh<1.0 )
    {
        r = C;
        g = X;
    }
    else if( hh>=1.0 && hh<2.0 )
    {
        r = X;
        g = C;
    }
    else if( hh>=2.0 && hh<3.0 )
    {
        g = C;
        b = X;
    }
    else if( hh>=3.0 && hh<4.0 )
    {
        g = X;
        b = C;
    }
    else if( hh>=4.0 && hh<5.0 )
    {
        r = X;
        b = C;
    }
    else
    {
        r = C;
        b = X;
    }
    float m = v-C;
    r += m;
    g += m;
    b += m;
    return vec3(r,g,b);
}

vec3 rgb2hsv(float r, float g, float b)
{
    float M = max(r,max(g,b));
    float m = min(r,min(g,b));
    float C = M-m;
    float h,s,v;
    if( C==0.0 ) h=0.0;
    else if( M==r ) h=mod((g-b)/C, 6.0);
    else if( M==g ) h=(b-r)/C+2.0;
    else h=(r-g)/C+4.0;
    h*=60.0;
    if( h<0.0 ) h+=360.0;
    v = M;
    if( v==0.0 )
        s = 0.0;
    else
        s = C/v;
    h = h / 360.0;
    return vec3(h,s,v);
}

/**
 * @brief 像素点RGB空间转YUV空间
 *
 * @param[in] color_rgb像素点值
 *
 * @return mediump vec3 color_rgb 转换到rgb空间的像素值（rgb）
 */
vec3 RGB2YUV(vec3 color_rgb)
{
    vec3 color_yuv;
    color_yuv.x = color_rgb.r *  0.299 + color_rgb.g * 0.587  + color_rgb.b * 0.114;
    color_yuv.y = color_rgb.r * -0.147 + color_rgb.g * -0.289 + color_rgb.b * 0.436;
    color_yuv.z = color_rgb.r *  0.615 + color_rgb.g * -0.515 + color_rgb.b *  -0.1;
    return color_yuv;
}

/**
 * @brief 像素点YUV空间转RGB空间
 *
 * @param[in] color_yuv YUV空间像素点值
 *
 * @return mediump vec3 color_rgb 转换到rgb空间的像素值（rgb）
 */
vec3 YUV2RGB(vec3 color_yuv)
{
    vec3 color_rgb;
    color_rgb.r = color_yuv.x + 1.140 * color_yuv.z;
    color_rgb.g = color_yuv.x - 0.395 * color_yuv.y - 0.581 * color_yuv.z;
    color_rgb.b = color_yuv.x + 2.032 * color_yuv.y;
    return color_rgb;
}

/**
 * @brief 从纹理中读取统计出的皮肤分布（当前读取出的是rgb值，需要每次都转换yuv空间，待优化）
 *
 * @return mediump vec4 skinColor 统计皮肤像素值（rgb）
 */
vec4 getSkinColor()
{
    mediump vec4 skinColor = vec4(1.0, 0,1.0, 1.0);
    if(isDirectSkinColor) skinColor = vec4(skin_color,1.0);
    else skinColor = texture2D(faceColorTexture, vec2(0.0, 0.0));
    return skinColor;
}

/**
 * @brief 根据统计皮肤值，返回当前输入像素点归属于皮肤的概率
 *
 * @param[in] color_yuv像素点值（注意此处应传入rgb转换后的，以尽可能避免重复计算）
 *
 * @return float skin_possibility 皮肤概率
 */
float isSkin(vec3 color_yuv)
{    
    mediump float skin_possibility = 0.0;
    vec4 skinColor = getSkinColor();

    mediump vec3 skin_yuv_tresh = RGB2YUV(vec3(skinColor.r, skinColor.g, skinColor.b));

    skin_possibility = exp(-pow((color_yuv.y - skin_yuv_tresh.y) / 0.08, 2.0) / 2.0 - pow((color_yuv.z - skin_yuv_tresh.z) / 0.06, 2.0) / 2.0);

    return skin_possibility;
}

/**
 * @brief 根据是否分块，返回分块情况下纹理坐标值的转换后的坐标
 *
 * @param[in] tco: 传入的纹理坐标值，范围(0 - 1.0)
 *
 * @return 返回的转换后的纹理坐标值，范围(0 - 2.0)
 */
vec2 getPartedTexCoordOut(vec2 tco)
{
    mediump vec2 TexCoordOut_part;
        
    if(isParted)
    {
        if(0 == partIndex) {TexCoordOut_part = tco;                        }
        else if(1 == partIndex) {TexCoordOut_part = tco + vec2(1.0 , 0.0); }
        else if(2 == partIndex) {TexCoordOut_part = tco + vec2(0.0 , 1.0); }
        else if(3 == partIndex) {TexCoordOut_part = tco + vec2(1.0 , 1.0); }
    }
    else
    {
        TexCoordOut_part = tco;
    }

    return TexCoordOut_part;
}

void main()
{
	
	 /**
     * @brief 磨皮部分代码，当前1.2.4版本为皮肤整体磨皮
     */
	vec4 all_color = texture2D(Texture, TexCoordOut);
	mediump vec3 source_color = all_color.rgb;
    mediump vec3 source_color_yuv;
	
    float skin_possibility = 0.0;
    float dis_face;
    mediump vec2 TexCoordOut_part;
	
    if (blend > 0.01 || brighten_face_coeff > 0.01 || skin_smooth_coeff > 0.01)
    {
        source_color_yuv = RGB2YUV(source_color);
        skin_possibility = isSkin(source_color_yuv);
    }
    if (skin_smooth_coeff > 0.01)
    {
        /**
         * @brief 取出当前像素点与皮肤像素点，转换到yuv空间进行皮肤判断
         *        磨皮整体皮肤判断思想与美白类似，不同的是这里我们并不直接输出皮肤概率（软判断），而是直接进行区间判断（硬判断）
         *        因为磨皮与美白不同，人眼对像素亮度变化非常敏感，美白如果采用硬判断方式会出现明显的明暗分界线，而磨皮不同，主要是改变色相，人眼对色相变化没那么敏感
         *        所以硬判断也并不会造成很明显的分界线，另外磨皮需要皮肤磨的很均匀，因此皮肤区间要取得比较广并且强度尽可能不随皮肤概率而改变
         *       （很多需要被磨的问题区域与统计皮肤值相差比较远，采用软判断方式会造成问题区域被磨效果不明显）
         *        磨皮总体流程：判断像素点是否为皮肤 -> 取像当前素点周围像素点计算个啥啥啥（貌似是高通滤波阈值，以及磨皮融合像素值） -> 高通滤波提高图像锐化程度 -> 融合周围像素信息
         */

        vec4 skin_tresh_rgb   = getSkinColor();
        vec3 skin_tresh_yuv   = RGB2YUV(vec3(skin_tresh_rgb.r, skin_tresh_rgb.g, skin_tresh_rgb.b));
        
   
        
        vec3  pass     = vec3(0.5);
        vec4 highpass = vec4(0.5);
        
        float highpassWeight =  skin_smooth_coeff * 2.0;
        float center;
        
        if (abs(source_color_yuv.y - skin_tresh_yuv.y) < 0.06 && abs(source_color_yuv.z - skin_tresh_yuv.z) < 0.045)
        {
            /**
             * @brief 取周围像素点分布范围
             */
            vec2 box[boxSize];
            
            box[ 0] = vec2( 5.0,  0.0);
            box[ 1] = vec2(-5.0,  0.0);
            box[ 2] = vec2( 0.0,  5.0);
            box[ 3] = vec2( 0.0, -5.0);
            box[ 4] = vec2( 2.5,  4.0);
            box[ 5] = vec2( 2.5, -4.0);
            box[ 6] = vec2(-2.5,  4.0);
            box[ 7] = vec2(-2.5, -4.0);
            box[ 8] = vec2( 4.0,  2.5);
            box[ 9] = vec2( 4.0, -2.5);
            box[10] = vec2(-4.0,  2.5);
            box[11] = vec2(-4.0, -2.5);
            // box[12] = vec2(-1.0, -1.0);
            // box[13] = vec2(-1.0,  1.0);
            // box[14] = vec2( 1.0,  1.0);
            // box[15] = vec2( 1.0, -1.0);
            // box[16] = vec2(-2.0,  0.0);
            // box[17] = vec2( 2.0,  0.0);
            // box[18] = vec2( 0.0,  2.0);
            // box[19] = vec2( 0.0,  2.0);
            
            /**
             * @brief 中心点权重记为2.5
             */
            float centerWeight = skin_possibility;
            center = dot(lumiNorm, source_color);
            
            float normWeight = centerWeight;
            float outColor   = center * centerWeight;
            
            
            /**
             * @brief 下面是不知道哪个神仙写的，一丁点注释都没有，大概是我太菜了实在是没太看懂
             *        反正是计算高通滤波的一个什么阈值，效果确实海星，我自己实现了一个双边滤波，效果感觉远不如现在的
             */
            vec2 coord;
            float color;
            float diff;
            float weight;
            float devide = 5.0;

            for (int i = 0; i < 12; ++i) {
                vec2  coord  = TexCoordOut + singleStepOffset * box[i] * skin_smooth_radius / 5.0;
                vec3  color_rgb = texture2D(Texture, coord).rgb;
//                vec3  color_yuv = RGB2YUV(color_rgb);
//                float possibility = isSkin(color_yuv);
                float color  = dot(lumiNorm, color_rgb);
                float diff   = abs(center - color);
                float weight = max(0.0, 1.0 - diff * threshold); // * possibility;
                normWeight  += weight;
                outColor    += weight * color;
            }

            outColor /= normWeight;
            highpass.a = center - outColor + 0.5;

            if (highpass.a <= 0.5) {
                highpass.a = pow(2.0 * highpass.a, amplify) / 2.0;
            } else {
                highpass.a = 1.0 - pow(2.0 * (1.0 - highpass.a), amplify) / 2.0;
            }
            
            /**
             * @brief 高通滤波，同时计算出周围像素融合信息
             */
            if (highpassWeight > 0.01)
            {
                vec2 box[9];
                
                box[0] = vec2(-1.0,  0.0);
                box[1] = vec2( 1.0,  0.0);
                box[2] = vec2( 0.0, -1.0);
                box[3] = vec2( 0.0,  1.0);
                box[4] = vec2(-1.0, -1.0);
                box[5] = vec2( 1.0,  1.0);
                box[6] = vec2( 1.0, -1.0);
                box[7] = vec2(-1.0,  1.0);
                
                float sigma[8];
                
                sigma[0] = 0.5;
                sigma[1] = 0.5;
                sigma[2] = 0.5;
                sigma[3] = 0.5;
                sigma[4] = 0.25;
                sigma[5] = 0.25;
                sigma[6] = 0.25;
                sigma[7] = 0.25;
                
                pass = vec3(0.0);
                
                for (int i = 0; i < 4; ++i) {
                    pass += highpassWeight * (texture2D(Texture, TexCoordOut + box[i] * singleStepOffset).rgb - source_color) * sigma[i];
                }
                
                pass = vec3(0.5) - pass * 0.3;
            }
        }
        /**
         * @brief 将计算结果存到texture里面，new_filter_blend_fast.fsh里面进一步计算
         */
        //gl_FragColor = vec4(pass, highpass);
        highpass = vec4(pass,highpass.a);
        
        
        if (abs(source_color_yuv.y - skin_tresh_yuv.y) < 0.075 && abs(source_color_yuv.z - skin_tresh_yuv.z) < 0.055)
        {
                  
            //由于flaw_protection会导致在分块情况下磨皮无效，此处暂设置为1.0.
            float flaw_protection  = 2.0 - 1.0 * skin_possibility;
            // float flaw_protection  = 1.0;
            center = dot(lumiNorm, source_color);

            float blend = 1.0 + pow(center, 0.3) * 0.07;
            vec3 smoothColor = source_color * blend - vec3(highpass.a) * (blend - 1.0);
            smoothColor = clamp(smoothColor, vec3(0.0), vec3(1.0));
            
            //smoothColor = mix(source_color, smoothColor, min(1.0, pow(center, 0.33)));

            //nose protect
            /*
            mediump vec2 TexCoordOut_part = getPartedTexCoordOut(TexCoordOut);
             
            float dis_y_nose, dis_x_nose;
            if (orientation == 0 || orientation == 180)
            {
                dis_x_nose = TexCoordOut_part.x - nose_tip.x;
                dis_y_nose = TexCoordOut_part.y - nose_tip.y;

            } else {
                dis_y_nose = TexCoordOut_part.x - nose_tip.x;
                dis_x_nose = TexCoordOut_part.y - nose_tip.y;

            }
            float nose_protection = 1.0 - exp(-pow(dis_x_nose / (0.6 * nose_radius_x), 2.0) / 2.0 - pow(dis_y_nose / (0.6 * nose_radius_y), 2.0) / 2.0);
            if(nose_protection<0.5)
            {
                nose_protection = 0.1;
            }
            else
            {
                nose_protection = 1.0;
            }
            */
            
            float nose_protection = 1.0;

            smoothColor = mix(source_color, smoothColor, min(1.0, (0.8 * skin_possibility + 0.2) * 1.8 * skin_smooth_coeff * nose_protection * pow(center, 0.39)));
            
            vec3 overlay;
            overlay.r = OVERLAY(smoothColor.r, highpass.r);
            overlay.g = OVERLAY(smoothColor.g, highpass.g);
            overlay.b = OVERLAY(smoothColor.b, highpass.b);
            
            smoothColor = mix(smoothColor, overlay, 0.5);
            
            source_color = smoothColor;         
            //source_color = vec3(highpass.a);
        }
        else{
            if(need_color == 1 && source_color_yuv.r < 0.5){
                source_color = source_color*eyebrow_color;
            }

        }

    }
        
   
       
  
    if (blend > 0.01 || brighten_face_coeff > 0.01)
    {
        //source_color_yuv = RGB2YUV(vec3(source_color.r, source_color.g, source_color.b));
        //skin_possibility = isSkin(source_color_yuv);
        TexCoordOut_part = getPartedTexCoordOut(TexCoordOut);
        dis_face = distance(TexCoordOut_part, face_center);
        vec4 skin_color = getSkinColor();
    }
    
    /**
     * @brief 美白部分代码，针对皮肤美白，且对嘴唇针对性保护并调色（偏红）
     */
    if (brighten_face_coeff > 0.01 || blend > 0.01) {

        mediump vec2 TexCoordOut_part = getPartedTexCoordOut(TexCoordOut);

        float mix_coeff = 0.5 * blend + (-0.8 * blend + 1.0) * brighten_face_coeff;

        /**
         * @brief 计算脸部空间范围，及脸部空间保护系数（注意这里并未考虑人脸朝向，包括xy平面旋转及z平面旋转，可能需要优化）
         */

        /*
         float dis_face = distance(TexCoordOut_part, face_center);
         float face_dis_protection = exp(-pow(dis_face / (1.5 * face_radius), 2.0) / 2.0);
         */

        /**
         * @brief 计算嘴部空间范围（注意当前1.2.4版本，并未传入图像真实orientation信息，拍后实时坐标系会不一致，当前通过processTexture函数及processImage函数传入假orientation信息解决）
         */


        float dis_y_mouse, dis_x_mouse;
        if (orientation == 0 || orientation == 180)
        {
            dis_x_mouse = TexCoordOut_part.x - mouth_center.x;
            dis_y_mouse = TexCoordOut_part.y - mouth_center.y;
        } else {
            dis_y_mouse = TexCoordOut_part.x - mouth_center.x;
            dis_x_mouse = TexCoordOut_part.y - mouth_center.y;
        }

        /**
         * @brief 计算嘴部调整比例，依据xy方向判断（注意当前1.2.4版本并非最佳解决方案，未考虑最不朝向问题，如果人脸不是水平的，主要是xy平面内旋转，则不能完全覆盖嘴部，不过当前效果在可接受范围内）
         */


        float mouth_dis_protection = 1.0 - exp(-pow(dis_x_mouse / (0.6 * mouse_radius_x), 2.0) / 2.0 - pow(dis_y_mouse / (0.6 * mouse_radius_y), 2.0) / 2.0);


        /**
        * @brief 计算嘴部空间范围（注意当前1.2.4版本，并未传入图像真实orientation信息，拍后实时坐标系会不一致，当前通过processTexture函数及processImage函数传入假orientation信息解决）
        */


        float dis_y_lefteye, dis_x_lefteye, dis_y_righteye, dis_x_righteye;
        if (orientation == 0 || orientation == 180)
        {
            dis_x_lefteye = TexCoordOut_part.x - left_eye_center.x;
            dis_y_lefteye = TexCoordOut_part.y - left_eye_center.y;
            dis_x_righteye = TexCoordOut_part.x - right_eye_center.x;
            dis_y_righteye = TexCoordOut_part.y - right_eye_center.y;
        } else {
            dis_y_lefteye = TexCoordOut_part.x - left_eye_center.x;
            dis_x_lefteye = TexCoordOut_part.y - left_eye_center.y;
            dis_y_righteye = TexCoordOut_part.x - right_eye_center.x;
            dis_x_righteye = TexCoordOut_part.y - right_eye_center.y;
        }

        float dis_protection_lefteye  = 1.0 - exp(-pow(dis_x_lefteye  / (0.6 * left_eye_radius_x), 2.0)  / 2.0 - pow(dis_y_lefteye  / (1.0 * left_eye_radius_y), 2.0)  / 2.0);
        float dis_protection_righteye = 1.0 - exp(-pow(dis_x_righteye / (0.6 * right_eye_radius_x), 2.0) / 2.0 - pow(dis_y_righteye / (1.0 * right_eye_radius_y), 2.0) / 2.0);

        float geometric_protection = 1.0; //dis_protection_lefteye * dis_protection_righteye * mouth_dis_protection;
        
        /**
         * @brief 单独对嘴部进行红处理
         */
        
        /*
         float red_protection = exp(-pow((0.8 - source_color.r)/ 0.04, 2.0)  / 2.0);
         
         delta_r = log(1.0 + source_color.r * (0.6 * gaussian_brighten_coeff + 0.05)) / log(1.0 + 0.6 * gaussian_brighten_coeff + 0.05) - source_color.r;
         
         source_color.r = source_color.r + delta_r * red_protection * mouth_dis_protection;
         */
        
        float fk = min(min(1.0 - source_color.r, 1.0 - source_color.g), 1.0 - source_color.b);
        vec3 outColor = (vec3(1.0 - fk) - source_color) / vec3(1.0 - fk);
        
        vec4 cmyk = vec4(outColor, fk);
        float deltaC = cmyk.x - (1.0 - pow(1.0 - cmyk.x, 0.6)); // cmyk.x * (1.0 -  0.12); 
        float deltaM = cmyk.y - (1.0 - pow(1.0 - cmyk.y, 0.66));
        float deltaY = cmyk.z - (1.0 - pow(1.0 - cmyk.z, 0.52));

        cmyk.x = cmyk.x - mix_coeff * skin_possibility * geometric_protection * deltaC;
        cmyk.y = cmyk.y - mix_coeff * skin_possibility * geometric_protection * deltaM;
        cmyk.z = cmyk.z - mix_coeff * skin_possibility * geometric_protection * deltaY;
        
        // float red_protection = exp(-pow((0.8 - source_color.r)/ 0.04, 2.0)  / 2.0);
        deltaM = cmyk.y - pow(cmyk.y, 0.93); // cmyk.y * (1.0 +  0.10 * red_protection * mix_coeff * (1.0 + mouth_dis_protection));
        deltaY = cmyk.z - pow(cmyk.z, 0.90); // cmyk.z * (1.0 +  0.05 * red_protection * mix_coeff * (1.0 + mouth_dis_protection));

        cmyk.y = clamp(cmyk.y - mix_coeff * (1.0 + mouth_dis_protection) * deltaM, 0.0, 1.0);
        cmyk.z = clamp(cmyk.z - mix_coeff * (1.0 + mouth_dis_protection) * deltaY, 0.0, 1.0);
        
        if (blend > 0.01)
        {
            deltaM = cmyk.y - cmyk.y * (1.0 +  0.10);
            deltaY = cmyk.z - cmyk.z * (1.0 +  0.05);

            cmyk.y = clamp(cmyk.y - blend * skin_possibility * geometric_protection * deltaM, 0.0, 1.0);
            cmyk.z = clamp(cmyk.z - blend * skin_possibility * geometric_protection * deltaY, 0.0, 1.0);
        }
        
        source_color = (vec3(1.0 - cmyk.a) - cmyk.rgb * vec3(1.0 - cmyk.a));
        source_color = min(vec3(1.0), max(vec3(0.0), source_color));
        
        // mediump float gaussian_brighten_coeff = 6.0 * mix_coeff * skin_possibility / (2.0 * 3.1415926);

        // float delta_r = log(1.0 + source_color.r * (1.00 * gaussian_brighten_coeff + 0.05)) / log(1.0 + 1.00 * gaussian_brighten_coeff + 0.05) - source_color.r;
        // float delta_g = log(1.0 + source_color.g * (1.00 * gaussian_brighten_coeff + 0.05)) / log(1.0 + 1.00 * gaussian_brighten_coeff + 0.05) - source_color.g;
        // float delta_b = log(1.0 + source_color.b * (1.00 * gaussian_brighten_coeff + 0.05)) / log(1.0 + 1.00 * gaussian_brighten_coeff + 0.05) - source_color.b;

        vec3 deltaRGB = pow(source_color.rgb, vec3(0.93, 0.93, 0.93)) - source_color.rgb;

        source_color.rgb = source_color.rgb + deltaRGB * mix_coeff * skin_possibility;

        // source_color.r = source_color.r + delta_r * geometric_protection * mix_coeff; // * dis_protection_lefteye * dis_protection_righteye * mouth_dis_protection;
        // source_color.g = source_color.g + delta_g * geometric_protection * mix_coeff; // * dis_protection_lefteye * dis_protection_righteye * mouth_dis_protection;
        // source_color.b = source_color.b + delta_b * geometric_protection * mix_coeff;
    }
    

    //vec3 ccc = vec3(120.0/ 255.0,  73.0/ 255.0,  41.0 / 255.0);
    gl_FragColor = vec4(source_color, all_color.a);
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

std::string blur_fsh = R"(

#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif

uniform sampler2D Texture;
uniform vec2 kSize;
varying vec2 TexCoordOut;

void main(void) {
    vec4 color = vec4(0.0);
    const int k = 2;

    for (int i = -k; i <= k; i ++) {
        for (int j = -k; j <= k; j ++) {
            vec2 pos = TexCoordOut + kSize * vec2(float(i), float(j)) / float(k);
            color += texture2D(Texture, pos);
        }
    }

    color /= float((2*k+1)*(2*k+1));

    gl_FragColor = color;
}
)";

std::string lipcolor_fsh = R"(

precision mediump float;
varying vec2 TexCoordOut;
uniform sampler2D Texture;

uniform vec2 p[128];
const vec3 lumiNorm = vec3(0.299, 0.587, 0.114);

vec4 getAverageColor(vec2 a, vec2 b, int m){
    mediump vec4 color = vec4(0.0);

    for (int i = 0; i < 1; i++) {
        mediump vec4 tmp = texture2D(Texture, a + float(i + 1) / float(m + 1) * (b-a));
        color += tmp;
    }
    return color / float(m);
}
vec4 getMaxColor(vec2 a, vec2 b, int m){
    mediump vec4 color = vec4(0.0);

    for (int i = 0; i < 1; i++) {
        mediump vec4 tmp = texture2D(Texture, a + float(i + 1) / float(m + 1) * (b-a));
        if(dot(color.rgb, lumiNorm) < dot(tmp.rgb, lumiNorm)){
            color = tmp;
        }
    }
    return color;
}

vec4 getFaceSkinColor()
{
    mediump vec4 rgba = vec4(0.0);
    int count = 0;
    if (TexCoordOut.x < 0.5) {
        for (int i = 67; i < 94; i++) {
            vec4 tmp = getMaxColor(p[i], p[192-i], 1);
            if(dot(rgba.rgb, lumiNorm) < dot(tmp.rgb, lumiNorm)){
                rgba = tmp;
            }
        }
    }
    else {
        for (int i = 0; i < 128; i++) {
            // int j;
            vec4 tmp;
            if (i >= 3 && i <= 29) {
                // j = 64 - i;
                tmp = getAverageColor(p[i], p[64 - i], 1);
            } else if (i >= 67 && i <= 93) {
                // j = 192 - i;
                tmp = getAverageColor(p[i], p[192 - i], 1);
            } else {
                continue;
            }
            // vec4 tmp = getAverageColor(p[i], p[j], 1);
            if(tmp.r > 0.0 || tmp.g > 0.0 || tmp.b > 0.0){
                rgba += tmp;
                count += 1;
            }
        }
    }

    if (count != 0) {
        rgba = rgba / float(count);
    }

    return rgba;
}

void main() {
    gl_FragColor = getFaceSkinColor();
}

)";

std::string lipstick_vsh = R"(

attribute vec3 aPosition;
attribute vec2 TexCoordBrow; //眉毛纹理坐标

// 传入纹理坐标
varying vec2 TexCoordOut;
varying vec2 MaskTextureOut;

void main() {
    gl_Position = vec4(aPosition, 1.0);
    TexCoordOut = TexCoordBrow;
    MaskTextureOut = TexCoordBrow;
}

)";

std::string lipstick_fsh = R"(

precision highp float;
uniform float transparency;
uniform sampler2D  Texture;
uniform sampler2D  Texture2; // MaskTexture
uniform vec3 lightPos;
uniform vec3 viewPos;
uniform float shininess;
uniform vec3 lightColor;
uniform vec3 color0;
uniform vec3 new_lipavgcolor;
uniform vec3 new_lipmaxcolor;
uniform vec2 point[128];
uniform float defuseStrength;
uniform float specularStrength;
uniform float smoothRadius;
uniform float new_smoothRadius;
uniform float specularThreshold;
uniform float mouthWidth;
uniform int model;
uniform int version;

//uniform bool isParted;
//uniform int partIndex;

uniform float videoHeight;
uniform float videoWidth;
uniform float maskHeight;
uniform float maskWidth;
uniform vec2 singleStepOffset;

varying vec2 TexCoordOut;
varying vec2 MaskTextureOut;
//varying vec3 Normal;
//varying vec3 FragPos;

const vec3 lumiNorm = vec3(0.299, 0.587, 0.114);
const vec3 avgLipColor = vec3(192.0/255.0, 124.0/255.0, 120.0/255.0);

#define OVERLAY(a, b) ((a) < 0.5 ? 2.0 * (a) * (b) : 1.0 - 2.0 * (1.0 - (a)) * (1.0 - (b)))
float OVERLAY_T(float a, float b, float a_thres)
{
    if (a == 0.0 || a == 1.0 || a_thres == 0.0 || a_thres == 1.0) {
        return a;
    } else {
        if (a < a_thres) {
            return a * b / a_thres;
        } else {
            return 1.0 - (1.0 - a) * (1.0 - b) / (1.0 - a_thres);
        }
    }
}
float distance_offset(vec2 p1, vec2 p2){
    float normal_x = (p1.x - p2.x) / singleStepOffset.x / 3.0;
    float normal_y = (p1.y - p2.y) / singleStepOffset.y;
    return sqrt(normal_x * normal_x + normal_y * normal_y);
}

float light_region(vec2 pt){

    float max_distance = 0.01;
    float strength = 0.0;
    for (int i = 67; i<=93; i++) {
        float max_strength = 1.0;
        if (i < 70) {
            max_strength = float(i+4 - 70)/4.0;
        }
        else if (i > 80) {
            max_strength = float(94 - i) / 4.0;
        }
        vec2 center_p = (point[i] + point[192 - i]) / 2.0;
        float distance_p = distance_offset(point[i] + (point[192 - i] - point[i])/3.0, center_p);
        float distance_pt = distance_offset(pt, center_p);
        if (distance_pt < distance_p){
            strength = max(strength, clamp((distance_p - distance_pt) / distance_p * 2.0, 0.0, max_strength));
        }
    }
    return strength;
}

vec3 RGB2YUV(vec3 color_rgb)
{
    vec3 color_yuv;
    color_yuv.x = color_rgb.r *  0.299 + color_rgb.g * 0.587  + color_rgb.b * 0.114;
    color_yuv.y = color_rgb.r * -0.147 + color_rgb.g * -0.289 + color_rgb.b * 0.436;
    color_yuv.z = color_rgb.r *  0.615 + color_rgb.g * -0.515 + color_rgb.b *  -0.1;
    return color_yuv;
}
vec3 YUV2RGB(vec3 color_yuv)
{
    vec3 color_rgb;
    color_rgb.r = color_yuv.x + 1.140 * color_yuv.z;
    color_rgb.g = color_yuv.x - 0.395 * color_yuv.y - 0.581 * color_yuv.z;
    color_rgb.b = color_yuv.x + 2.032 * color_yuv.y;
    return color_rgb;
}


vec3 adjustColor(vec3 color){
    vec3 cnt_color = color.rgb;

    float l = - 0.0;
    if(l > 0.0){
        cnt_color.r = cnt_color.r * (1.0 - l) + 255.0 * l;
        cnt_color.g = cnt_color.g * (1.0 - l) + 255.0 * l;
        cnt_color.b = cnt_color.b * (1.0 - l) + 255.0 * l;
    }
    else{
        cnt_color.r = cnt_color.r * (1.0 + l);
        cnt_color.g = cnt_color.g * (1.0 + l);
        cnt_color.b = cnt_color.b * (1.0 + l);
    }
    float s = -0.55;
    float rgbMax = max(max(cnt_color.r * 1.0, cnt_color.g * 1.0), cnt_color.b*1.0);
    float rgbMin = min(min(cnt_color.r * 1.0, cnt_color.g * 1.0), cnt_color.b*1.0);
    float delta = (rgbMax - rgbMin) / 255.0;

    float SS = 1.0 * (rgbMax - rgbMin) / (rgbMax + 1e-5);


    float value = (rgbMax + rgbMin) / 255.0;
    float L = value / 2.0;
    float S, alpha;
    if (L < 0.5){
        S = delta / value;
    }
    else{
        S = delta / (2.0 - value);
    }
    if (s >= 0.0){
        if(S + s>=1.0){
            alpha = S;
        }
        else{
            alpha = 1.0 - S;
        }
        alpha = 1.0 / alpha - 1.0;
        cnt_color.r = cnt_color.r + (cnt_color.r - L * 255.0)*alpha;
        cnt_color.g = cnt_color.g + (cnt_color.g - L * 255.0)*alpha;
        cnt_color.b = cnt_color.b + (cnt_color.b - L * 255.0)*alpha;
    }
    else{
        alpha = s;
        cnt_color.r = L * 255.0 + (cnt_color.r - L * 255.0)*(1.0 + alpha);
        cnt_color.g = L * 255.0 + (cnt_color.g - L * 255.0)*(1.0 + alpha);
        cnt_color.b = L * 255.0 + (cnt_color.b - L * 255.0)*(1.0 + alpha);
    }
    if(SS < 0.35){
        return color.rgb;
    }
    else if(SS < 0.7 && (cnt_color.r+cnt_color.g+cnt_color.b)/3.0> 0.5){
        cnt_color.rgb = (1.0 - 0.3) * cnt_color.rgb;
        if(cnt_color.r > 165.0 / 255.0){
            cnt_color.r = 0.8 * cnt_color.r;
        }

    }
    return cnt_color;
}

void main() {
    vec4 source = texture2D(Texture, TexCoordOut);
    vec4 maskOut = texture2D(Texture2, MaskTextureOut);
    vec4 render = vec4(vec3(color0), maskOut.r); // don't use maskOut.a because Huawei RGBA5551
    // vec4 render = vec4(vec3(color0), 1.0); // don't use maskOut.a because Huawei RGBA5551
    //    vec4 render = vec4(vec3(250.0/255.0, 35.0/255.0, 110.0/255.0), maskOut.r);

    vec4 color = vec4(0.0);
    vec4 gauss_color = vec4(0.0);
    const int coreSize = 5;
//    float texelOffset = smoothRadius * mouthWidth;
    float texelOffset = 0.2 * mouthWidth;
    if(texelOffset  < 1.0){
        texelOffset = 1.0;
    }
    if(version == 1){
        render.rgb = adjustColor(render.rgb);
    }
    float kernel[9];
    kernel[6] = 1.0; kernel[7] = 2.0; kernel[8] = 1.0;
    kernel[3] = 2.0; kernel[4] = 4.0; kernel[5] = 2.0;
    kernel[0] = 1.0; kernel[1] = 2.0; kernel[2] = 1.0;

    float count_ = 0.0;
    float gauss_count_ = 0.0;

    float max_source_y = 0.0;
    float cnt_source_y = 0.0;

    for(int y = 0; y < coreSize; y++)
    {
        for(int x = 0;x<coreSize;x++)
        {
            vec4 currentColor = texture2D(Texture, TexCoordOut + vec2(float(-2+x)*texelOffset/videoWidth,float(-2+y)*texelOffset/videoHeight), 2.0);
            vec4 renderColor = texture2D(Texture2, MaskTextureOut + vec2(float(-2+x)*texelOffset/videoWidth,-float(-2+y)*texelOffset/videoHeight));
            cnt_source_y = dot(currentColor.rgb, lumiNorm);
            max_source_y = max(max_source_y, cnt_source_y);
            if (!(x == 0 || x == 4 || y ==0 || y == 4)) {
                gauss_color += currentColor * kernel[(x-1) + (y-1) * 3] * renderColor.a;
                gauss_count_ += kernel[(x-1) + (y-1) * 3] * renderColor.a;
            }
        }
    }

    //color /= (count_ + 1.0e-5);
    gauss_color /= (gauss_count_ + 1.0e-5);

    color = gauss_color;

    vec3 source_yuv = RGB2YUV(source.rgb);
    vec3 color_yuv = RGB2YUV(color.rgb);

    float diff = 1.0;
    float spec = 0.0;

    if (model == 1)
    {
        // 雾面效果逻辑 y>y_thres时spec为负，减弱亮度
        float y_thres = 0.4;
        if (source_yuv.x > y_thres) {
            // specularStrength == 0.25
            spec = -1.0 * ((source_yuv.x - y_thres)/(1.0 - y_thres));
        } else {
            spec = 0.0;
        }
        // 强化邻域内亮暗对比
        spec += source_yuv.x - color_yuv.x;
    }
    else if (model == 2)
    {
        if (source_yuv.x - color_yuv.x > 0.2 * specularThreshold) {
            spec = pow(clamp((source_yuv.x - color_yuv.x)/(max_source_y - color_yuv.x), 0.0, 1.0), shininess);
        }
        if (source_yuv.x < color_yuv.x) {
            spec = 0.0;
            diff = clamp(diff * pow((source.r / (color.r + 1.0e-5) + 1.0e-5),
            5.0 * viewPos.x), 0.0, 1.0);
        }
    }
    else if(model == 3)
    {
        kernel[0] = 0.2270270270;
        kernel[1] = 0.1945945946;
        kernel[2] = 0.1216216216;
        kernel[3] = 0.0540540541;
        kernel[4] = 0.0162162162;
        const int shift_size = 9;

        float sum = 0.0;
        vec3 sum_color = vec3(0.0);
        for(int i =0; i< shift_size; i++){
            for(int j =0; j< shift_size;j++){
                vec2  coord  = vec2(TexCoordOut.x + singleStepOffset.x * float(i - shift_size/2) * new_smoothRadius * 0.3, TexCoordOut.y + singleStepOffset.y * float(j - shift_size/2) * new_smoothRadius * 0.3);
                //sum+=kernel[i * 3 + j];
                float weight = 1.0 * kernel[int(abs(float(i - shift_size/2)))] / kernel[0] * kernel[int(abs(float(j - shift_size/2)))] / kernel[0];
                sum+= weight;
                //sum_color += kernel[i * 3 + j] * texture2D(Texture, coord).rgb;
                sum_color += weight * texture2D(Texture, coord).rgb;
            }
        }
        sum_color /= (sum + 1.0e-5);
        float sum_color_y = dot(sum_color, lumiNorm);

        float max_y, avg_y;
        avg_y = dot(new_lipavgcolor, lumiNorm);
        max_y = dot(new_lipmaxcolor, lumiNorm);
//        spec = pow(clamp((sum_color_y) / (max_y), 0.0, 1.0), shininess);
        spec = pow(clamp((sum_color_y) / (max_y), 0.0, 1.0), 256.0);

        spec *= light_region(TexCoordOut);

    }
    vec3 specular = render.a * specularStrength * spec * lightColor;

    vec3 diffuse = diff * defuseStrength * lightColor;

    float alpha = render.a * transparency;
    float gamma = 2.0 * alpha * pow(1.0-alpha + 1.0e-5,
    (max(max(render.r,render.g),render.b)
    -min(min(render.r,render.g),render.b))
    / max(max(render.r,render.g),render.b));

    vec3 delta = render.rgb;
//    if (version == 2) {
        source.r = source.r - transparency * (source.r - OVERLAY_T(source.r, delta.r, avgLipColor.r));
        source.g = source.g - transparency * (source.g - OVERLAY_T(source.g, delta.g, avgLipColor.g));
        source.b = source.b - transparency * (source.b - OVERLAY_T(source.b, delta.b, avgLipColor.b));
//    } else {
//        source.r = source.r - transparency * (source.r - OVERLAY(source.r, delta.r));
//        source.g = source.g - transparency * (source.g - OVERLAY(source.g, delta.g));
//        source.b = source.b - transparency * (source.b - OVERLAY(source.b, delta.b));
//    }
    gl_FragColor = vec4(render.a * (source.rgb + specular * clamp(alpha * 5.0, 0.0, 1.0)), render.a);
    if(model >= 3){
        gl_FragColor = vec4(render.a * (source.rgb + (vec3(1.0) - source.rgb) * specular * clamp(alpha * 5.0, 0.0, 1.0)), render.a);
    }
}

)";


