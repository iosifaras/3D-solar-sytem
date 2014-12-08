// Vertex shader
#version 130

in vec4 a_position;
in vec3 a_normal;

uniform mat4 mvp;
uniform mat4 u_mv; // ModelView matrix


out vec3 N;
out vec3 L;
out vec3 V;

void main() {
    // Calculate the view vector and the normal vector and pass them
    // as varying variables to the fragment shader

    

	// Transform the vertex position to view space (eye coordinates)
    vec3 position_eye = vec3(u_mv * a_position);

	// Calculate the view-space normal
    N = normalize(mat3(u_mv) * a_normal);
    
	V = -position_eye;

	gl_Position = mvp * a_position;
}
