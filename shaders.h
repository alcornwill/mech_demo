
const GLchar * vsSource = "#version 330\n\n//#define USE_VERTEX_COLORS\n//#define UNLIT\n\nconst vec3 ambient = vec3(0.3f, 0.3f, 0.3f);\nconst vec3 dlight = vec3(0.03f, -0.98f, 0.08f); // direction\nconst float dfact = 0.7f; // directional light intensity\n\nin vec3 position;\nin vec3 normal;\nin vec3 color;\n//in vec2 uv;\n\nuniform mat4 MVP;\nuniform vec3 DiffuseColor;\n\n//flat out vec4 Color;\nout vec4 Color;\n\nvoid directional_light(vec3 surface_normal, inout vec3 scatteredLight)\n{\n    vec3 direction = normalize(dlight);\n    float diffuse = max(0.0, dot(surface_normal, direction));\n	scatteredLight += diffuse * dfact;\n}\n\nvec4 emulate_precision_error(float factor, in vec4 pos)\n{\n    float mult = 1.0f + (1.0f - factor) * 100.0f;\n    pos = floor(pos * mult) / mult;\n    return pos;\n}\n\nvoid main()\n{\n    vec4 v_pos = vec4(position, 1.0f);\n	//gl_Position = emulate_precision_error(0.1f, MVP * v_pos);\n	gl_Position = MVP * v_pos;\n\n	vec3 col = DiffuseColor;\n	#ifdef USE_VERTEX_COLORS\n	col *= color;\n	#endif\n\n    #ifndef UNLIT\n	vec3 scatteredLight = ambient;\n	vec3 surface_normal = normalize(MVP * vec4(normal, 1.0f)).xyz;\n	directional_light(surface_normal, scatteredLight);\n    col *= scatteredLight;\n    #endif\n\n    // note this value is unsaturated, we would saturate with min(color, vec4(1.0)\n	Color = vec4(col, 1.0f);\n}\n";

const GLchar * fsSource = "#version 330\n\n//flat in vec4 Color;\nin vec4 Color;\n\nout vec4 outputColor;\n\nvoid main()\n{\n    vec4 outColor = Color;\n    outputColor = outColor;\n}\n\n";
