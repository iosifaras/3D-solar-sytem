// Fragment shader
#version 130

uniform samplerCube u_cubemap;

in vec3 N;
in vec3 L;
in vec3 V;

void main() {
    // Implement reflective environment mapping here. Start by
    // calculating the reflection vector and then use it to look up
    // the color value in the cube map.

	vec3 R = reflect(-V,N);
	vec3 color = texture(u_cubemap, R).rgb;


	
    gl_FragColor = vec4(color , 1.0);
}
