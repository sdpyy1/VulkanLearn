#version 460
#pragma shader_stage(vertex)

// ���õ�������,����Vulkan����D3D���ӿڱ任ʱ,y����ᷴתһ��,���о�����
vec2 positions[3] = {
	{    0, -.5f },
	{ -.5f,  .5f },
	{  .5f,  .5f }
};

void main() {
	gl_Position = vec4(positions[gl_VertexIndex], 0, 1);
}