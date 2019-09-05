#include "HelloTriangleApp.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

void HelloTriangleApplication::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void HelloTriangleApplication::initVulkan()
{
	createInstance();
	setupDebugMessenger();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createDescriptorSetLayout();
	createGraphicsPipeline();
	createCommandPool();
	createDepthResource();
	createFramebuffers();
	createTextureImage();
	createTextureImageView();
	createTextureSampler();
	loadModel();
	createVertexBuffer();
	createIndexBuffer();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
	createSyncObjects();
}

void HelloTriangleApplication::createInstance()
{
	// validationレイヤーをチェック&有効化
	if (enableValidationLayers && !checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not avilable!");
	}

	// アプリケーション情報
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";		// 特定のエンジンを指定(?)
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// インスタンス生成情報
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// グローバル拡張の設定
	// GLFWと拡張を接続する（ための準備）
	//uint32_t glfwExtensionCount = 0;
	//const char** glfwExtensions;

	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	//glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	//createInfo.enabledExtensionCount = glfwExtensionCount;
	//createInfo.ppEnabledExtensionNames = glfwExtensions;

	// validation layersを設定する
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.pNext = nullptr;
	}

	// インスタンス作成
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void HelloTriangleApplication::createSurface()
{
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
		throw std::runtime_error("failed to create window surface!");
	}


}

void HelloTriangleApplication::pickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (isDeviceSuitable(device)) {
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}

bool HelloTriangleApplication::isDeviceSuitable(VkPhysicalDevice device)
{
	//// 名前・種別・サポートしているVulkanバージョンなど
	//VkPhysicalDeviceProperties deviceProperties;
	//vkGetPhysicalDeviceProperties(device, &deviceProperties);

	//// サポートしている機能
	//VkPhysicalDeviceFeatures deviceFeatures;
	//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	//return deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU
	//	&& deviceFeatures.geometryShader;

	// Vulkanサポートしているのであれば何でも良い

	QueueFamilyIndices indices = findQueueFamilies(device);
	
	bool extensionSupported = checkDeviceExtensionSupport(device);

	bool swapChainAdequate = false;
	if (extensionSupported) {
		SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty()
			&& !swapChainSupport.presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() && extensionSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool HelloTriangleApplication::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	// デバイスがサポートしている拡張のリストを取得する
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	// setにして拡張リストの重複をなくす
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	// 有効化したい拡張と、使用可能な拡張を突き合わせる
	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

HelloTriangleApplication::QueueFamilyIndices HelloTriangleApplication::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for (const auto& queueFamily : queueFamilies) {
		if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
		if (queueFamily.queueCount > 0 && presentSupport) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
		i++;
	}

	return indices;
}

void HelloTriangleApplication::createLogicalDevice()
{
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { 
		indices.graphicsFamily.value(),
		indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	// キュー
	//VkDeviceQueueCreateInfo queueCreateInfo = {};
	//queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	//queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
	//queueCreateInfo.queueCount = 1;

	//float queuePriority = 1.0f;
	//queueCreateInfo.pQueuePriorities = &queuePriority;

	// 物理デバイス
	VkPhysicalDeviceFeatures deviceFeatures = {};

	// 異方性フィルタリング有効化
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	// 論理デバイス作成情報
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	// 論理デバイス作成
	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	// キュー取得
	vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
}

// スワップチェーン作成に必要な情報を集める
HelloTriangleApplication::SwapChainSupportDetails HelloTriangleApplication::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	// スワップチェーン作成に必要な、基本的なサーフェイス機能の問い合わせ
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

	// スワップチェーン作成に必要な、基本的なサーフェイスが対応しているスワップチェーンフォーマットについて問い合わせる
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device,
			surface,
			&formatCount,
			details.formats.data());
	}

	// サーフェイスに対応した、サポートされているpresentation modeの問い合わせ
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device,
		surface,
		&presentModeCount,
		nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device,
			surface,
			&presentModeCount,
			details.presentModes.data());
	}

	return details;
}

// カラーフォーマット・カラースペースを選択する
VkSurfaceFormatKHR HelloTriangleApplication::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM	// BGRA 8bit unsignd int
			&& availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR	// SRGB
			) {
			return availableFormat;
		}
	}

	return availableFormats[0];
}

// ディスプレイとの同期方法を選択する
VkPresentModeKHR HelloTriangleApplication::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentModes : availablePresentModes) {
		if (availablePresentModes == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentModes;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

// スワップチェーンの解像度を決定する
VkExtent2D HelloTriangleApplication::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height),
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}

// スワップチェーンの作成
void HelloTriangleApplication::createSwapChain()
{
	// スワップチェーン作成に必要な情報を集める
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

	// スワップチェーン作成要件から、最適な設定を選択する
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	// スワップチェーンが保持するイメージ数
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

	// スワップチェーンイメージ数が最大数を超えていないかチェック
	if (swapChainSupport.capabilities.maxImageCount > 0
		&& imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// スワップチェーンオブジェクト作成情報を作成
	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
	uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	if (indices.graphicsFamily != indices.presentFamily) {
		// グラフィックスキューファミリとプレゼンテーションキューファミリが異なる場合は
		// VK_SHARING_MODE_CONCURRENT（同時モード）を使用する
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		// グラフィックスキューファミリとプレゼンテーションキューファミリが同一の場合は
		// VK_SHARING_MODE_EXCLUSIVE（独占モード）を使用する
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;		// Optional
		createInfo.pQueueFamilyIndices = nullptr;	// Optional
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;	// ウィンドウが隠れたときなどはクリッピングする
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// スワップチェーン作成
	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void HelloTriangleApplication::recreateSwapChain()
{
	int width = 0, height = 0;
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(window, &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(device);

	cleanupSwapChain();

	createSwapChain();
	createImageViews();
	createRenderPass();
	createGraphicsPipeline();
	createDepthResource();
	createFramebuffers();
	createUniformBuffers();
	createDescriptorPool();
	createDescriptorSets();
	createCommandBuffers();
}

void HelloTriangleApplication::createImageViews()
{
	swapChainImageViews.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
	}
}

void HelloTriangleApplication::createRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = findDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void HelloTriangleApplication::createGraphicsPipeline()
{
	auto vertShaderCode = readFile("shaders/vert.spv");
	auto fragShaderCode = readFile("shaders/frag.spv");

	// シェーダーモジュール用意
	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	// vertex shader ステージ作成構造体
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	// fragment shader ステージ作成構造体
	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = {
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	// Vertex input
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Input Assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)swapChainExtent.width;
	viewport.height = (float)swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;

	// Pipeline ViewportState
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	// DepthStencilState
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.minDepthBounds = 0.0f;	// Optional
	depthStencil.maxDepthBounds = 1.0f;	// Optional

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;	//Optional
	rasterizer.depthBiasClamp = 0.0f;	// Optional
	rasterizer.depthBiasSlopeFactor = 0.0f;	// Optional

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;	// Optional
	multisampling.pSampleMask = nullptr;	// Optional
	multisampling.alphaToCoverageEnable = VK_FALSE;	//Optional
	multisampling.alphaToOneEnable = VK_FALSE;	// Optional

	// Color Blending
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = 
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;		// Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;	// Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;				// Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;		// Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;	// Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;				// Optional
	
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;	// Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;		// Optional
	colorBlending.blendConstants[1] = 0.0f;		// Optional
	colorBlending.blendConstants[2] = 0.0f;		// Optional
	colorBlending.blendConstants[3] = 0.0f;		// Optional

	// Dinamic state
	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	// PipelineLayout
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;				// Optional
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;		// Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0;		// Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr;	// Optional

	if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}

	// Create GraphicsPipeline
	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencil;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr;				// Optional
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	if (vkCreateGraphicsPipelines(
		device,
		VK_NULL_HANDLE,
		1,
		&pipelineInfo,
		nullptr,
		&graphicsPipeline) != VK_SUCCESS) {
		throw std::runtime_error("failed to create graphics pipeline!");
	}

	// グラフィックスパイプライン作成後はShaderModuleを破棄する
	vkDestroyShaderModule(device, fragShaderModule, nullptr);
	vkDestroyShaderModule(device, vertShaderModule, nullptr);
}

// シェーダーモジュール作成
VkShaderModule HelloTriangleApplication::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}
	return shaderModule;
}

// フレームバッファ作成
void HelloTriangleApplication::createFramebuffers()
{
	swapChainFramebuffers.resize(swapChainImageViews.size());

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		std::array<VkImageView, 2> attachments = {
			swapChainImageViews[i],
			depthImageView
		};

		VkFramebufferCreateInfo framebufferinfo = {};
		framebufferinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferinfo.renderPass = renderPass;
		framebufferinfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferinfo.pAttachments = attachments.data();
		framebufferinfo.width = swapChainExtent.width;
		framebufferinfo.height = swapChainExtent.height;
		framebufferinfo.layers = 1;

		if (vkCreateFramebuffer(device, &framebufferinfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer");
		}
	}
}

// コマンドプール作成
void HelloTriangleApplication::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = 0;	// Optional

	if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

// コマンドバッファ作成
void HelloTriangleApplication::createCommandBuffers()
{
	commandBuffers.resize(swapChainImages.size());

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}

	for (size_t i = 0; i < commandBuffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0;	// Optional
		beginInfo.pInheritanceInfo = nullptr; // Optional

		if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = swapChainFramebuffers[i];

		// レンダーエリア定義
		renderPassInfo.renderArea.offset = { 0,0 };
		renderPassInfo.renderArea.extent = swapChainExtent;

		// クリアカラー定義
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0] = { 0.0f, 0.0f, 0.0f, 1.0f };
		clearValues[1] = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// レンダーパス開始
		vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// グラフィックスパイプラインバインド
		vkCmdBindPipeline(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

		VkBuffer vertexBuffers[] = { vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffers[i], 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(commandBuffers[i], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		// ディスクリプタセットバインド
		vkCmdBindDescriptorSets(commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, nullptr);

		// 描画命令
		vkCmdDrawIndexed(commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		// レンダーパス記録完了
		vkCmdEndRenderPass(commandBuffers[i]);

		// コマンドバッファ記録完了
		if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

// コマンドバッファの作成とレコード開始を行う
VkCommandBuffer HelloTriangleApplication::beginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

// コマンドバッファのレコード完了・サブミット・解放
void HelloTriangleApplication::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(graphicsQueue);

	vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

// バッファ作成
void HelloTriangleApplication::createBuffer(
	VkDeviceSize size,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkBuffer& buffer,
	VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(device, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(device, buffer, bufferMemory, 0);
}

// モデルロード
void HelloTriangleApplication::loadModel() {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, MODEL_PATH.c_str())) {
		throw std::runtime_error(warn + err);
	}

	std::unordered_map<Vertex, uint32_t> uniqueVertices = {};

	for (const auto& shape : shapes) {
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};

			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = { 1.0f, 1.0f, 1.0f };
			vertices.push_back(vertex);

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
				vertices.push_back(vertex);
			}

			indices.push_back(uniqueVertices[vertex]);
		}
	}
}

void HelloTriangleApplication::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	// メモリへデータをマップする
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		vertexBuffer,
		vertexBufferMemory);

	copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

// インデックスバッファ作成
void HelloTriangleApplication::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,
		indexBufferMemory);

	copyBuffer(stagingBuffer, indexBuffer, bufferSize);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

// バッファコピー
void HelloTriangleApplication::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferCopy copyRegion = {};
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	endSingleTimeCommands(commandBuffer);
}

uint32_t HelloTriangleApplication::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if (typeFilter & (1 << i)
			&& (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}

// テクスチャイメージ作成
void HelloTriangleApplication::createTextureImage()
{
	// イメージファイル読み込み
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(TEXTURE_PATH.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels) {
		throw std::runtime_error("failed to load texture image!");
	}

	// イメージ一時的にを格納するバッファを用意
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	createBuffer(imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory);

	// イメージをコピー
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(device, stagingBufferMemory);

	// イメージファイルを解放
	stbi_image_free(pixels);

	// イメージを作成
	createImage(
		texWidth,
		texHeight,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		textureImage,
		textureImageMemory);

	transitionImageLayout(
		textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	copyBufferToImage(
		stagingBuffer,
		textureImage,
		static_cast<uint32_t>(texWidth),
		static_cast<uint32_t>(texHeight)
	);

	transitionImageLayout(
		textureImage,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}

// イメージ作成
void HelloTriangleApplication::createImage(
	uint32_t width,
	uint32_t height,
	VkFormat format,
	VkImageTiling tiling,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlags properties,
	VkImage& image,
	VkDeviceMemory& imageMemory)
{
	// イメージ情報設定
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.flags = 0;	//Optimal

	if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image!");
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(device, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

	if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate image memory!");
	}

	vkBindImageMemory(device, image, imageMemory, 0);
}

// イメージビュー作成
VkImageView HelloTriangleApplication::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView;
	if (vkCreateImageView(device, &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
		throw std::runtime_error("failed to create image view!");
	}

	return imageView;
}

// イメージレイアウト遷移
void HelloTriangleApplication::transitionImageLayout(
	VkImage image,
	VkFormat format,
	VkImageLayout oldLayout,
	VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;	//TODO
	barrier.dstAccessMask = 0;	//TODO

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

		if (hasStencilComponent(format)) {
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
		}
	}
	else {
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	}

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage,
		destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);
	endSingleTimeCommands(commandBuffer);
}

// バッファをイメージにコピーする
void HelloTriangleApplication::copyBufferToImage(
	VkBuffer buffer,
	VkImage image,
	uint32_t width,
	uint32_t height)
{
	VkCommandBuffer commandBuffer = beginSingleTimeCommands();

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0,0,0 };
	region.imageExtent = {
		width,
		height,
		1
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);
	endSingleTimeCommands(commandBuffer);
}

// テクスチャのイメージビュー作成
void HelloTriangleApplication::createTextureImageView() {
	textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT);
}

// イメージサンプラを作成する
void HelloTriangleApplication::createTextureSampler() {
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	if (vkCreateSampler(device, &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
		throw std::runtime_error("failed to create texture sampler!");
	}
}

// デプスリソースを作成する
void HelloTriangleApplication::createDepthResource()
{
	VkFormat depthFormat = findDepthFormat();

	createImage(
		swapChainExtent.width,
		swapChainExtent.height,
		depthFormat,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		depthImage,
		depthImageMemory);

	depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
	
	transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

}

// フォーマットを検索する
VkFormat HelloTriangleApplication::findSupportedFormat(
	const std::vector<VkFormat>& candidates,
	VkImageTiling tiling,
	VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &props);

		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
		{
			return format;
		}
		else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	throw std::runtime_error("failed to find supported format!");
}

// デプスフォーマットを検索する
VkFormat HelloTriangleApplication::findDepthFormat() {
	return findSupportedFormat(
		{VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);
}

// ステンシルコンポーネントをもっているかどうかを確認する
bool HelloTriangleApplication::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void HelloTriangleApplication::createSyncObjects()
{
	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(
				device,
				&semaphoreInfo,
				nullptr,
				&imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(
				device,
				&semaphoreInfo,
				nullptr,
				&renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(
				device,
				&fenceInfo,
				nullptr,
				&inFlightFences[i]) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create semaphores for a frame!");
		}
	}
}

void HelloTriangleApplication::createDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;	//Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2>bindings = { uboLayoutBinding, samplerLayoutBinding };
	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}

// ユニフォームバッファ作成
void HelloTriangleApplication::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffers.resize(swapChainImages.size());
	uniformBuffersMemory.resize(swapChainImages.size());

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			uniformBuffers[i],
			uniformBuffersMemory[i]);
	}
}

// ユニフォームバッファ更新
void HelloTriangleApplication::updateUniformBuffer(uint32_t currentImage)
{
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo = {};
	ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(glm::radians(45.0f), swapChainExtent.width / (float)swapChainExtent.height, 0.1f, 10.0f);
	ubo.proj[1][1] *= -1;

	void* data;
	vkMapMemory(device, uniformBuffersMemory[currentImage], 0, sizeof(ubo), 0, &data);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(device, uniformBuffersMemory[currentImage]);
}

// ディスクリプタプール作成
void HelloTriangleApplication::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes = {};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(swapChainImages.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(swapChainImages.size());

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(swapChainImages.size());

	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}
}

// ディスクリプタセット作成
void HelloTriangleApplication::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(swapChainImages.size(), descriptorSetLayout);
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChainImages.size());
	allocInfo.pSetLayouts = layouts.data();

	descriptorSets.resize(swapChainImages.size());
	if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = textureImageView;
		imageInfo.sampler = textureSampler;

		std::array<VkWriteDescriptorSet, 2> descriptorWrites = {};

		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;	// Optional

		vkUpdateDescriptorSets(
			device,
			static_cast<uint32_t>(descriptorWrites.size()), 
			descriptorWrites.data(),
			0,
			nullptr);
	}
}


void HelloTriangleApplication::mainLoop()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		drawFrame();
	}

	vkDeviceWaitIdle(device);
}

void HelloTriangleApplication::drawFrame()
{
	vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		device,
		swapChain,
		UINT64_MAX,
		imageAvailableSemaphores[currentFrame],
		VK_NULL_HANDLE,
		&imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		throw std::runtime_error("failed to acuire swap chain image!");
	}

	updateUniformBuffer(imageIndex);
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;

	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;
	vkResetFences(device, 1, &inFlightFences[currentFrame]);

	if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	presentInfo.pResults = nullptr;	// Optional

	result = vkQueuePresentKHR(presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
		framebufferResized = false;
		recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

	vkQueueWaitIdle(presentQueue);
}

void HelloTriangleApplication::cleanupSwapChain()
{
	vkDestroyImageView(device, depthImageView, nullptr);
	vkDestroyImage(device, depthImage, nullptr);
	vkFreeMemory(device, depthImageMemory, nullptr);

	for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
		vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
	}

	vkFreeCommandBuffers(
		device,
		commandPool,
		static_cast<uint32_t>(commandBuffers.size()),
		commandBuffers.data());

	vkDestroyPipeline(device, graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
	vkDestroyRenderPass(device, renderPass, nullptr);

	for (size_t i = 0; i < swapChainImageViews.size(); i++) {
		vkDestroyImageView(device, swapChainImageViews[i], nullptr);
	}

	vkDestroySwapchainKHR(device, swapChain, nullptr);

	for (size_t i = 0; i < swapChainImages.size(); i++) {
		vkDestroyBuffer(device, uniformBuffers[i], nullptr);
		vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
	}

	vkDestroyDescriptorPool(device, descriptorPool, nullptr);
}

void HelloTriangleApplication::cleanup()
{
	cleanupSwapChain();

	vkDestroySampler(device, textureSampler, nullptr);
	vkDestroyImageView(device, textureImageView, nullptr);

	vkDestroyImage(device, textureImage, nullptr);
	vkFreeMemory(device, textureImageMemory, nullptr);

	vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
	vkDestroyBuffer(device, vertexBuffer, nullptr);
	vkFreeMemory(device, vertexBufferMemory, nullptr);
	vkDestroyBuffer(device, indexBuffer, nullptr);
	vkFreeMemory(device, indexBufferMemory, nullptr);

	for (rsize_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(device, inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(device, commandPool, nullptr);

	vkDestroyDevice(device, nullptr);

	if (enableValidationLayers) {
		DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
	}

	vkDestroySurfaceKHR(instance, surface, nullptr);
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}


// 必要なvalidation layerがサポートされているかをチェックする
bool HelloTriangleApplication::checkValidationLayerSupport()
{
	// 有効なレイヤーリストを取得
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	// 必要なレイヤーリスト(validationLayers)と
	// 有効なレイヤーリスト(availableLayers)を比較し、
	// 必要なレイヤーリストが全て存在するかをチェック
	for (const char* layerName : validationLayers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			std::cerr << layerProperties.layerName << std::endl;

			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

std::vector<const char*> HelloTriangleApplication::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	// GLFWが必要とするVulkanインスタンス拡張名前の配列を取得します
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

void HelloTriangleApplication::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
		| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;	// Optional
}

void HelloTriangleApplication::setupDebugMessenger()
{
	if (!enableValidationLayers) { return; }

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger)
		!= VK_SUCCESS)
	{
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

VkResult HelloTriangleApplication::CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger
)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void HelloTriangleApplication::DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator
)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL HelloTriangleApplication::debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
	return VK_FALSE;
}

std::vector<char> HelloTriangleApplication::readFile(const std::string& filename)
{
	// std::ios::ate -> ファイルの最後から読み込む
	// std::ios::binary -> バイナリ形式で読み込む
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	// ファイルサイズから用意すべき配列サイズを決定する
	// file.tellg() -> 現在の読み込み位置
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	
	// 終わりから開始にむかって読み込む
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

void HelloTriangleApplication::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
	app->framebufferResized = true;
}