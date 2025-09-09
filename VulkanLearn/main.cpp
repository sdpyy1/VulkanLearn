#include "GlfwGeneral.hpp"
#include "EasyVulkan.hpp"
using namespace vulkan;

pipelineLayout pipelineLayout_triangle;
pipeline pipeline_triangle;
const auto& RenderPassAndFramebuffers() {
	static const auto& rpwf = easyVulkan::CreateRpwf_Screen();
	return rpwf;
}
void CreateLayout() {
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	pipelineLayout_triangle.Create(pipelineLayoutCreateInfo);
}
void CreatePipeline() {
	// ShaderModules�ڴ������ߺ�Ϳ���������
	static shaderModule vert("shader/FirstTriangle.vert.spv");
	static shaderModule frag("shader/FirstTriangle.frag.spv");

	// ��shader�ڹ��ߵ��ĸ��׶�ʹ��(�����������shader�е�һЩ����ֵ)
	static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_triangle[2] = {
		vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
		frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	// ����ͼ�ι���
	auto Create = [] {
		graphicsPipelineCreateInfoPack pipelineCiPack;
		pipelineCiPack.createInfo.layout = pipelineLayout_triangle;
		pipelineCiPack.createInfo.renderPass = RenderPassAndFramebuffers().renderPass;
		pipelineCiPack.inputAssemblyStateCi.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		pipelineCiPack.viewports.emplace_back(0.f, 0.f, float(windowSize.width), float(windowSize.height), 0.f, 1.f);
		pipelineCiPack.scissors.emplace_back(VkOffset2D{}, windowSize);
		pipelineCiPack.multisampleStateCi.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		pipelineCiPack.colorBlendAttachmentStates.push_back({ .colorWriteMask = 0b1111 });
		pipelineCiPack.UpdateAllArrays();
		pipelineCiPack.createInfo.stageCount = 2;
		pipelineCiPack.createInfo.pStages = shaderStageCreateInfos_triangle;
		pipeline_triangle.Create(pipelineCiPack);
		};
	auto Destroy = [] {
		pipeline_triangle.~pipeline();
		};
	graphicsBase::Base().AddCallback_CreateSwapchain(Create);
	graphicsBase::Base().AddCallback_DestroySwapchain(Destroy);
	Create();
}

int main() {
	// 1. ����glfw����  2.Vulkanʵ��  3.�����豸ѡ�� 4.�����豸���� 5.����������
	if (!InitializeWindow({ 1280, 720 }))
		return -1;

	// RenderPass �� Framebuffers
	const auto& [renderPass, framebuffers] = RenderPassAndFramebuffers();

	// �������߲��ֺ�ͼ�ι���
	CreateLayout();
	CreatePipeline();

	// ͬ��GPU��CPU�Ĺ���
	fence fence;

	// ͬ��GPU�����׶εĹ���
	semaphore semaphore_imageIsAvailable; // ͼ����õ��ź�
	semaphore semaphore_renderingIsOver;  // ��Ⱦ��ɵ��ź�

	// �����������
	commandBuffer commandBuffer;
	// Command pools����
	commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	commandPool.AllocateBuffers(commandBuffer);

	VkClearValue clearColor = { .color = { 0.3f, 0.5f, 0.6f, 1.f } };



	// Main loop
	while (!glfwWindowShouldClose(glfwWindow)) {
		//  ������С����ͼ�껯���������Ż��߼�
		while (glfwGetWindowAttrib(glfwWindow, GLFW_ICONIFIED))
			glfwWaitEvents();

		graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);
		auto i = graphicsBase::Base().CurrentImageIndex();
		
		// ��ʾ��ʼ��¼�������
		commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		// ������Ⱦͨ��
		renderPass.CmdBegin(commandBuffer, framebuffers[i], { {}, windowSize }, clearColor);
		// ��ͼ�ι��� ������һ�ֹ������� VK_PIPELINE_BIND_POINT_COMPUTE
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_triangle);
		// ʵ�ʵ���Ⱦ���3�����㣬1��ʵ�����Ӷ���0��ʼ����ʵ��0��ʼ
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		// ������Ⱦͨ��
		renderPass.CmdEnd(commandBuffer);
		// ������������ļ�¼
		commandBuffer.End();

		// �ύ���������ͼ�ζ���ִ�У���ͨ��fenceͬ��CPU��GPU��ͨ��semaphore_renderingIsOver��֪���Գ�����
		graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, semaphore_imageIsAvailable, semaphore_renderingIsOver, fence);

		// ��Ⱦ����źŸ�֪���������Գ�����
		graphicsBase::Base().PresentImage(semaphore_renderingIsOver);

		glfwPollEvents();
		TitleFps();

		// cpu�ȴ�gpu�����Ⱦ����
		fence.WaitAndReset();
	}
	TerminateWindow();
	return 0;
}