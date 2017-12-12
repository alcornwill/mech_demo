#version 330

//#define USE_VERTEX_COLORS
//#define UNLIT

in vec3 position;
in vec3 normal;
in vec3 color;
//in vec2 uv;

uniform mat4 MVP;
uniform mat4 NormalMatrix;
uniform vec3 DiffuseColor;

// flat out vec4 Color;
out vec4 Color;
out vec3 vtx;


vec4 emulate_precision_error(float factor, in vec4 pos)
{
    float mult = 1.0f + (1.0f - factor) * 100.0f;
    pos = floor(pos * mult) / mult;
    return pos;
}

void main()
{
    vec4 v_pos = vec4(position, 1.0f);
	//gl_Position = emulate_precision_error(0.1f, MVP * v_pos);
	gl_Position = MVP * v_pos;

	vec3 col = DiffuseColor;
	#ifdef USE_VERTEX_COLORS
	col *= color;
	#endif

    vtx = mat3(NormalMatrix) * position + normal * 0;

    // note this value is unsaturated, we would saturate with min(color, vec4(1.0)
	Color = vec4(col, 1.0f);
}
