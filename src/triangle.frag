#version 450

layout(binding = 1) uniform FragShaderUniform {
	vec4 transparency_color;
	bool is_no_texture;	
 } fsu;


layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 2) uniform sampler2D texSampler;

void main() {
	if(fsu.is_no_texture){
        	outColor = fsu.transparency_color;
	}else{
		outColor = texture(texSampler, fragTexCoord);
	}
}
