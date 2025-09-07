#include "VKBase.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")

// GLFW����
GLFWwindow* glfwWindow;


// ��ʼ��GLFW����
bool InitializeWindow(VkExtent2D size, bool isResizable = true, bool limitFrameRate = true) {
	using namespace vulkan;
// ---------------------------------------------GLFW----------------------------------------------------------------------
	if (!glfwInit()) {
		outStream << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
		return false;
	}
	// glfwWindowHint �������ô��ڵ�����   GLFW_NO_API ��ʾ������OpenGL������	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// ��ֹ�{�����ڴ�С
	glfwWindowHint(GLFW_RESIZABLE, isResizable);
	// ��������
	glfwWindow = glfwCreateWindow(size.width, size.height, "windowTitle", nullptr, nullptr);
	if (!glfwWindow) {
		outStream << std::format("[ InitializeWindow ]\nFailed to create a glfw window!\n");
		glfwTerminate();
		return false;
	}
// ---------------------------------------------GLFW----------------------------------------------------------------------


// ---------------------------------------------Vulkan----------------------------------------------------------------------
	// ͨ��glfwGetRequiredInstanceExtensions��ȡ��Ҫ����չ,��ǰ�ռ��������vector<char*>,����vulkanʵ������
	uint32_t extensionCount = 0;
	const char** extensionNames;
	extensionNames = glfwGetRequiredInstanceExtensions(&extensionCount);
	if (!extensionNames) {
		outStream << std::format("[ InitializeWindow ]\nVulkan is not available on this machine!\n");
		glfwTerminate();
		return false;
	}
	for (size_t i = 0; i < extensionCount; i++)
		graphicsBase::Base().AddInstanceExtension(extensionNames[i]);

	// ��windows���Կ�����ȷ֪����Ҫ��������չ
	//graphicsBase::Base().AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
	//graphicsBase::Base().AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);


	// ���豸������չ VK_KHR_SWAPCHAIN_EXTENSION_NAME:ʵ��ͼ����֣�Presentation������Ļ
	graphicsBase::Base().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	graphicsBase::Base().UseLatestApiVersion();

	// ����vulkanʵ��
	if (graphicsBase::Base().CreateInstance())
		return false;		

	// ����ʵ����,�������ϴ���surface,��Ϊ����Ӱ��������豸ѡ��
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	// glfw�Ѿ������˿�ƽ̨��surface��������
	if (VkResult result = glfwCreateWindowSurface(vulkan::graphicsBase::Base().Instance(), glfwWindow, nullptr, &surface)) {
		outStream << std::format("[ InitializeWindow ] ERROR\nFailed to create a window surface!\nError code: {}\n", int32_t(result));
		glfwTerminate();
		return false;
	}
	graphicsBase::Base().Surface(surface);

	// 1. ��ȡ�����豸  2. �����豸�Ƿ�֧�ָ��ֶ��м���  3. ��ѡ�õ������豸���������豸
	if (vulkan::graphicsBase::Base().GetPhysicalDevices() ||
		vulkan::graphicsBase::Base().DeterminePhysicalDevice(0, true, false) ||
		vulkan::graphicsBase::Base().CreateDevice())
		return false;

	// ����������
	if (graphicsBase::Base().CreateSwapchain(limitFrameRate))
		return false;
// ---------------------------------------------Vulkan----------------------------------------------------------------------

	return true;
}
void TerminateWindow() {
	vulkan::graphicsBase::Base().WaitIdle();
	glfwTerminate();
	glfwDestroyWindow(glfwWindow);
}




// �ڴ��ڱ�����ʾFPS
void TitleFps() {
	static double time0 = glfwGetTime();
	static double time1;
	static double dt;
	static int dframe = -1;
	static std::stringstream info;
	time1 = glfwGetTime();
	dframe++;
	if ((dt = time1 - time0) >= 1) {
		info.precision(1);
		info << "windowTitle" << "    " << std::fixed << dframe / dt << " FPS";
		glfwSetWindowTitle(glfwWindow, info.str().c_str());
		info.str("");
		time0 = time1;
		dframe = 0;
	}
}