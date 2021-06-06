#ifndef GUI_SHADERS_H
#define GUI_SHADERS_H

#define GGUI_GLSL_VERSION "#version 330"


static const char* const GGUI_VERTEX_SHADER =
GGUI_GLSL_VERSION "\n"
"in vec2 pos;"

"void main() {"
	"gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);"
"}"
;



#define GGUI_SHADER_HEADER \
GGUI_GLSL_VERSION "\n"     \
"out vec4 out_color;"      \
"uniform vec2 pos;"        \
"uniform vec2 size;"       \
"uniform vec2 win_size;"   \
"uniform vec2 mouse;"      \
"uniform float value;"     \
"\n#define PI 3.14159\n"   \
"vec2 getuv() {"           \
	"return (gl_FragCoord.xy-vec2(pos.x, win_size.y-pos.y))/size;" \
"}" \
"\n"
;


static const char* const GGUI_FONT_SHADER = 
GGUI_SHADER_HEADER

"\n"
"#define NUM_CHARS 95"
"\n"

"uniform sampler2D texture0;"

"\n"
"layout(std140) uniform ubo__font_data {\n"
	"vec4 font_data[NUM_CHARS];"
"};"

"void main() {"

	"vec2 uv = getuv()+0.5;"
	"uv.y = 1.0-uv.y;"

	"int str[6];"
	"str[0] = 'h';"
	"str[1] = 'e';"
	"str[2] = 'l';"
	"str[3] = 'l';"
	"str[4] = 'o';"
	
	"float cur = 0;"
	"float result = 0.0;"
	
	"for(int i = 0; i < 5; i++) {"
		"vec4 fd = font_data[str[i]-32];"
		"vec2 uv2 = uv;"
		"uv2.x += fd.x-cur;"
		"uv2.y += fd.y+(fd.w-fd.y)-0.5;"
		
		"float t = texture(texture0, uv2).r;"

		"t *= step(fd.x, uv2.x)*step(uv2.x, fd.z);"
		"t *= step(fd.y, uv2.y)*step(uv2.y, fd.w);"
			
		"cur += fd.z-fd.x;"
		"result += t;"
	"}"



	"out_color = vec4(vec3(result), 1.0);"

"}"
;




static const char* const GGUI_CHECKBOX_SHADER = 
GGUI_SHADER_HEADER

"#define E0  0.3 \n"
"#define E1  0.2 \n"

"void main() {"
	"vec2 uv = getuv();"
	"float l = value*0.1;"
	"float d = smoothstep(E0, E1-l, length(uv)-0.2-l*0.3);"

	"out_color = vec4(vec3(0.3, value+0.3, 0.3), d);"
"}"
;



static const char* const GGUI_KNOB_SHADER = 
GGUI_SHADER_HEADER

"#define E0  0.3 \n"
"#define E1  0.2 \n"
"#define S smoothstep\n"

"uniform vec2 limits;"

"float create_line(vec2 p0, vec2 p1, vec2 uv) {"
	"vec2 dst = uv-p0;"
	"vec2 pdst = p1-p0;"
	"return S(0.035, 0.0, length(dst-pdst*clamp(dot(dst, pdst)/dot(pdst, pdst), 0.0, 1.0)));"
"}"


"void main() {"
	"vec2 uv = getuv();"
	"vec2 m = (vec2(mouse.x, pos.y)-vec2(pos.x, mouse.y))/size;" 

	"float n = (value-limits.y)/(limits.x-limits.y);"
	"float p = PI/4.0;"
	"float a = (PI-p)*n*2.0+p;"
	"float x =  sin(a)*0.4;"
	"float y = -cos(a)*0.4;"

	"vec2 d = vec2(x, y);"

	"float line = create_line(d*0.35, d, uv);"
	
	"float l = length(uv*0.8);"
	"float circle  = 1.0-abs(S(0.3, 0.4, l)-S(0.41, 0.3, l));"

	"vec3 line_col = line*vec3(0.8);"
	"vec3 circle_col = circle*vec3(0.1, 0.8, 0.1);"

	"out_color = vec4(line_col+circle_col, line+circle);"
"}"
;




#endif
