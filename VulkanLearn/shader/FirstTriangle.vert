#version 460
#pragma shader_stage(vertex)

// 倒置的三角形,但是Vulkan或者D3D在视口变换时,y坐标会反转一下,所有就正了
vec2 positions[3] = {
	{    0, -.5f },
	{ -.5f,  .5f },
	{  .5f,  .5f }
};

void main() {
	gl_Position = vec4(positions[gl_VertexIndex], 0, 1);
}