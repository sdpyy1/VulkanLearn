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
	// ShaderModules在创建管线后就可以销毁了
	static shaderModule vert("shader/FirstTriangle.vert.spv");
	static shaderModule frag("shader/FirstTriangle.frag.spv");

	// 绑定shader在管线的哪个阶段使用(这里可以设置shader中的一些常量值)
	static VkPipelineShaderStageCreateInfo shaderStageCreateInfos_triangle[2] = {
		vert.StageCreateInfo(VK_SHADER_STAGE_VERTEX_BIT),
		frag.StageCreateInfo(VK_SHADER_STAGE_FRAGMENT_BIT)
	};

	// 创建图形管线
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
	// 1. 创建glfw窗口  2.Vulkan实例  3.物理设备选择 4.虚拟设备创建 5.交换链创建
	if (!InitializeWindow({ 1280, 720 }))
		return -1;

	// RenderPass 和 Framebuffers
	const auto& [renderPass, framebuffers] = RenderPassAndFramebuffers();

	// 创建管线布局和图形管线
	CreateLayout();
	CreatePipeline();

	// 同步GPU与CPU的工具
	fence fence;

	// 同步GPU各个阶段的工具
	semaphore semaphore_imageIsAvailable; // 图像可用的信号
	semaphore semaphore_renderingIsOver;  // 渲染完成的信号

	// 命令缓冲区设置
	commandBuffer commandBuffer;
	// Command pools创建
	commandPool commandPool(graphicsBase::Base().QueueFamilyIndex_Graphics(), VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	commandPool.AllocateBuffers(commandBuffer);

	VkClearValue clearColor = { .color = { 0.3f, 0.5f, 0.6f, 1.f } };



	// Main loop
	while (!glfwWindowShouldClose(glfwWindow)) {
		//  窗口最小化（图标化）场景的优化逻辑
		while (glfwGetWindowAttrib(glfwWindow, GLFW_ICONIFIED))
			glfwWaitEvents();

		graphicsBase::Base().SwapImage(semaphore_imageIsAvailable);
		auto i = graphicsBase::Base().CurrentImageIndex();
		
		// 表示开始记录命令缓冲区
		commandBuffer.Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		// 设置渲染通道
		renderPass.CmdBegin(commandBuffer, framebuffers[i], { {}, windowSize }, clearColor);
		// 绑定图形管线 还有另一种管线类型 VK_PIPELINE_BIND_POINT_COMPUTE
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_triangle);
		// 实际的渲染命令，3个顶点，1个实例，从顶点0开始，从实例0开始
		vkCmdDraw(commandBuffer, 3, 1, 0, 0);
		// 结束渲染通道
		renderPass.CmdEnd(commandBuffer);
		// 结束命令缓冲区的记录
		commandBuffer.End();

		// 提交命令缓冲区到图形队列执行，并通过fence同步CPU与GPU，通过semaphore_renderingIsOver告知可以呈现了
		graphicsBase::Base().SubmitCommandBuffer_Graphics(commandBuffer, semaphore_imageIsAvailable, semaphore_renderingIsOver, fence);

		// 渲染完成信号告知交换链可以呈现了
		graphicsBase::Base().PresentImage(semaphore_renderingIsOver);

		glfwPollEvents();
		TitleFps();

		// cpu等待gpu完成渲染工作
		fence.WaitAndReset();
	}
	TerminateWindow();
	return 0;
}