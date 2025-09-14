#version 450

// Vertex positions for a triangle (in normalized device coordinates)
vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),   // Bottom vertex
    vec2(0.5, 0.5),    // Top right vertex
    vec2(-0.5, 0.5)    // Top left vertex
);

// Colors for each vertex
vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0), // Red
    vec3(0.0, 1.0, 0.0), // Green
    vec3(0.0, 0.0, 1.0)  // Blue
);

// Output variables to fragment shader
layout(location = 0) out vec3 fragColor;

void main() {
    // Set vertex position using built-in vertex index
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
    
    // Pass color to fragment shader
    fragColor = colors[gl_VertexIndex];
}