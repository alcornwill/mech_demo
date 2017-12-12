#version 330

const vec3 ambient = vec3(0.7f, 0.7f, 0.7f);
const vec3 dlight = vec3(-1.0f, -1.0f, -1.0f); // direction
const float dfact = 0.2f; // directional light intensity

//flat in vec4 Color;
in vec4 Color;

in vec3 vtx;

out vec4 outputColor;

void directional_light(vec3 surface_normal, inout vec3 scatteredLight)
{
    vec3 direction = normalize(dlight);
    float diffuse = max(0.0, dot(surface_normal, direction));
	scatteredLight += diffuse * dfact;
}

void main()
{
    #ifndef UNLIT
	vec3 scatteredLight = ambient;
    vec3 u = dFdx(vtx);
    vec3 v = dFdy(vtx);
	vec3 surface_normal = normalize(cross(u, v));
	directional_light(surface_normal, scatteredLight);
    #endif

    outputColor = vec4(scatteredLight, 1.0) * Color;
}

