# 初始化Vulkan
1. 创建Vulkan实例
	- 验证层
	- 拓展层
2. 创建Window surface
	- 这本来是一个跨平台的操作,但是GLFW帮我们做了
3. 物理设备
	- 寻找所有可用的物理设备
	- 检查设备是否支持各种需要的队列家族(Queue Family)
	- 创建逻辑设备
		- 选择需要创建的队列家族信息
		- 设备也有自己的验证层和拓展层  (Vulkan早期版本曾明确区分实例级与设备级验证层，但这一区分在当前实现中已不再适用。)
		- 创建
4. 创建交换链
	- 查询物理设备对当前 Surface 的支持能力
	- 根据支持能力配置各种缓冲区信息,缓冲区数量/画布大小/透明度/图像格式和色彩空间/图像用途/图像变换/呈现模式等
	- 创建交换链
	- 创建Image,这里就是指交换链的缓冲区
	- 创建对应Image的ImageViews

5. 创建管线
	- 加载ShaderModules,并设置每个shader是哪个阶段使用
	- 设置Vertex input,说明顶点数据格式
	- 设置Input assembly,说明图元(三角形/线段/点)
	- 设置Viewports(屏幕大小和深度范围),设置裁剪矩形在viewport的基础上进一步裁剪,超出部分会被光栅化舍弃(这两个属性可以设置为动态状态Dynamic State)
	- 设置光栅化阶段的参数 设置只渲染线框\只渲染点等也在这里设置,甚至可以设置阴影偏移
	- 多采样设置
	- 深度测试\模板测试设置
	- 颜色混合设置
	- Pipeline layout,用于向shader传递全局变量或者资源绑定
    - 设置RenderPass (渲染过程中如何使用帧缓冲区附件（如颜色缓冲、深度缓冲）),比如新的一帧开始时清除颜色缓冲
6. 创建帧缓冲区
	- 每个交换链的缓冲区都需要一个对应的帧缓冲区
7. 创建命令缓冲区
	- 创建命令池
	- 分配命令缓冲区


# 渲染流程
1. vkBeginCommandBuffer 表明开始记录命令，设置命令缓冲区的使用标志（一次性提交、重复使用等）。
2. vkCmdBeginRenderPass 开始一个渲染通道，指定帧缓冲区、渲染区域和清除值。
3. vkCmdBindPipeline 绑定图形管线，设置渲染状态和着色器。
4. vkCmdSetViewport 和 vkCmdSetScissor 设置视口和裁剪矩形（如果是动态状态）。
5. vkCmdDraw 
6. vkCmdEndRenderPass 结束渲染通道。
7. vkEndCommandBuffer 完成命令记录，命令缓冲区准备好提交执行。
8. vkQueueSubmit 提交命令缓冲区到图形队列，等待 GPU 执行。
9. vkQueuePresentKHR 将渲染结果呈现到屏幕，通过交换链交换图像。
10. 同步操作，确保渲染完成后再进行下一帧的渲染。

# 同步机制
Semaphores：PV操作 用于队列间同步
Fences：CPU-GPU同步 CPU等待GPU完成某个操作




# 一些概念
	- 验证层:Vulkan API的设计理念是追求最小化驱动程序开销，这一目标的体现之一就是默认情况下API中的错误检查功能极其有限。即便是像将枚举值设置为错误数值或向必填参数传递空指针这样简单的错误，通常也不会被明确处理.验证层是可选组件，通过对接入Vulkan函数调用来应用额外操作。
	- 拓展层:Vulkan的功能可以通过加载拓展来增强。拓展可以为现有的Vulkan对象添加新功能，或者引入全新的对象类型。与验证层类似，拓展也是通过拦截Vulkan函数调用来实现的。

	- Queue Family: GPU 不只是一个“单一工作单元”它内部有很多 队列（Queue） 来执行不同类型的任务：图形渲染（Graphics）计算（Compute）数据传输（Transfer / DMA）同类队列通常被归到一个 Queue Family（队列族）每个队列族中的队列，功能、能力是相同的
	
	- Window surface: 让Vulkan渲染结果输出到屏幕上.由于Vulkan是平台无关的API，它无法直接与窗口系统对接。要实现Vulkan与窗口系统的连接并将结果呈现在屏幕上，我们需要使用WSI（窗口系统集成）扩展 .VKKHRurface扩展是一个实例级扩展，我们实际上已经启用了它，因为它包含在glfwGetRequiredInstanceExtensions.返回的列表中
	
	- Swap chain: GPU 渲染的结果不是直接显示在屏幕上，而是先写入 交换链中的图像（Images）Swapchain 维护一组连续的图像缓冲区（通常双缓冲或三缓冲）.这样可以避免屏幕撕裂，同时保证连续帧渲染
	- vkImage:  VkImage 是 Vulkan 中表示 GPU 上一块存储像素或纹理数据的对象句柄。
	- Image views:  Vulkan 中用来“解释和访问”VkImage 的对象句柄，指定图像的格式、维度、Mipmap 和数组层。

	- Shader modules: 就是用来封装Shader的类.加载到设备中的 SPIR-V 字节码容器，是连接编译后的着色器与管线的关键桥梁。它本质上是对 SPIR-V 二进制数据的封装，供管线创建时引用。
	- 动态状态（Dynamic State): 虽然Vulkan的管线是固定的,但少数状态允许在绘制时动态修改，无需重新创建管线 —— 这些状态就称为 “动态状态”,例如视口大小,裁剪矩形,混合常量等,但是启用动态状态后,必须在绘制前通过专门的命令动态设置
	- Pipeline layout:用于向shader传递全局变量或者资源绑定
	- Command buffers: Vulkan 中用于记录和存储一系列渲染命令的对象。它们充当了 CPU 与 GPU 之间的桥梁，允许应用程序预先定义渲染操作，然后一次性提交给 GPU 执行。命令缓冲区的设计旨在最大化性能和灵活性，使得复杂的渲染任务能够高效地进行。


# 管线设计
## OpenGL / D3D 里的管线
    可变管线：你可以在渲染过程中随时修改各种状态：比如调用 glBlendFunc 改混合模式OMSetBlendState 改混合/输出模式换着不同的 shader 或 framebuffer 都很灵活
随时切换各种状态就是体现OpenGL是状态机

好处：灵活，写代码方便
坏处：驱动每次修改状态都要处理、检查和优化，可能影响性能

## Vulkan 的管线设计
几乎不可变（Immutable）：
	一旦创建了 VkPipeline，里面包含的 shader、混合模式、深度模板设置等都固定了
	如果想改变 shader、framebuffer 或混合函数，你必须 重新创建一个新的 pipeline
好处：
	驱动在创建 pipeline 时就知道所有操作，可以做大量优化
	渲染时几乎没有状态检查开销 → 性能更高
坏处：
	你需要提前创建 多个 pipeline 来对应不同的渲染状态组合
	管理起来比较麻烦，需要规划好各种组合


# 不同点
vulkan的NDC坐标Y轴是朝下的,而OpenGL是朝上的
vulkan的NDC坐标Z轴是0到1,而OpenGL是-1到1