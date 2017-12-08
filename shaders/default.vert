#version 330

//#define USE_VERTEX_COLORS
//#define UNLIT

const vec3 ambient = vec3(0.3f, 0.3f, 0.3f);
const vec3 dlight = vec3(0.03f, -0.98f, 0.08f); // direction
const float dfact = 0.7f; // directional light intensity

in vec3 position;
in vec3 normal;
in vec3 color;
//in vec2 uv;

uniform mat4 MVP;
uniform vec3 DiffuseColor;

//flat out vec4 Color;
out vec4 Color;

void directional_light(vec3 surface_normal, inout vec3 scatteredLight)
{
    vec3 direction = normalize(dlight);
    float diffuse = max(0.0, dot(surface_normal, direction));
	scatteredLight += diffuse * dfact;
}

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

    #ifndef UNLIT
	vec3 scatteredLight = ambient;
	vec3 surface_normal = normalize(MVP * vec4(normal, 1.0f)).xyz;
	directional_light(surface_normal, scatteredLight);
    col *= scatteredLight;
    #endif

    // note this value is unsaturated, we would saturate with min(color, vec4(1.0)
	Color = vec4(col, 1.0f);
}
