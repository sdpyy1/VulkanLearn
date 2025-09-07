#include "VKBase.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#pragma comment(lib, "glfw3.lib")

// GLFW窗口
GLFWwindow* glfwWindow;


// 初始化GLFW窗口
bool InitializeWindow(VkExtent2D size, bool isResizable = true, bool limitFrameRate = true) {
	using namespace vulkan;
// ---------------------------------------------GLFW----------------------------------------------------------------------
	if (!glfwInit()) {
		outStream << std::format("[ InitializeWindow ] ERROR\nFailed to initialize GLFW!\n");
		return false;
	}
	// glfwWindowHint 用于配置窗口的属性   GLFW_NO_API 表示不创建OpenGL上下文	
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	// 禁止調整窗口大小
	glfwWindowHint(GLFW_RESIZABLE, isResizable);
	// 创建窗口
	glfwWindow = glfwCreateWindow(size.width, size.height, "windowTitle", nullptr, nullptr);
	if (!glfwWindow) {
		outStream << std::format("[ InitializeWindow ]\nFailed to create a glfw window!\n");
		glfwTerminate();
		return false;
	}
// ---------------------------------------------GLFW----------------------------------------------------------------------


// ---------------------------------------------Vulkan----------------------------------------------------------------------
	// 通过glfwGetRequiredInstanceExtensions获取需要的拓展,提前收集起来组成vector<char*>,用于vulkan实例创建
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

	// 在windows电脑可以明确知道需要这两个拓展
	//graphicsBase::Base().AddInstanceExtension(VK_KHR_SURFACE_EXTENSION_NAME);
	//graphicsBase::Base().AddInstanceExtension(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);


	// 给设备设置拓展 VK_KHR_SWAPCHAIN_EXTENSION_NAME:实现图像呈现（Presentation）到屏幕
	graphicsBase::Base().AddDeviceExtension(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	graphicsBase::Base().UseLatestApiVersion();

	// 创建vulkan实例
	if (graphicsBase::Base().CreateInstance())
		return false;		

	// 创建实例后,必须马上创建surface,因为他会影响后续的设备选择
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	// glfw已经集成了跨平台的surface创建函数
	if (VkResult result = glfwCreateWindowSurface(vulkan::graphicsBase::Base().Instance(), glfwWindow, nullptr, &surface)) {
		outStream << std::format("[ InitializeWindow ] ERROR\nFailed to create a window surface!\nError code: {}\n", int32_t(result));
		glfwTerminate();
		return false;
	}
	graphicsBase::Base().Surface(surface);

	// 1. 获取可用设备  2. 检查该设备是否支持各种队列家族  3. 用选好的物理设备创建虚拟设备
	if (vulkan::graphicsBase::Base().GetPhysicalDevices() ||
		vulkan::graphicsBase::Base().DeterminePhysicalDevice(0, true, false) ||
		vulkan::graphicsBase::Base().CreateDevice())
		return false;

	// 创建交换链
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




// 在窗口标题显示FPS
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