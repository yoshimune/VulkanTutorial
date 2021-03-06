﻿#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>
#include <vector>
#include <array>
#include <cstring>
#include <optional>
#include <set>
#include <algorithm>
#include <fstream>
#include <unordered_map>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>



class HelloTriangleApplication {
public:
	const int WIDTH = 800;
	const int HEIGHT = 600;

	const std::string MODEL_PATH = "models/chalet.obj";
	const std::string TEXTURE_PATH = "textures/chalet.jpg";

	const std::vector<const char*> validationLayers = {
		// SDK内にある一般的なvalidation layer
		"VK_LAYER_KHRONOS_validation"
	};

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

	struct Vertex {
		glm::vec3 pos;
		glm::vec3 color;
		glm::vec2 texCoord;

		static VkVertexInputBindingDescription getBindingDescription() {
			VkVertexInputBindingDescription bindingDescription = {};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
			return bindingDescription;
		}

		static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
			std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[0].offset = offsetof(Vertex, pos);
			
			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescriptions[1].offset = offsetof(Vertex, color);

			attributeDescriptions[2].binding = 0;
			attributeDescriptions[2].location = 2;
			attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

			return attributeDescriptions;
		}

		bool operator==(const Vertex& other) const {
			return pos == other.pos && color == other.color && texCoord == other.texCoord;
		}
	};


private:



	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	VkBuffer vertexBuffer;
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer;
	VkDeviceMemory indexBufferMemory;

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	uint32_t mipLevels;
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;

	VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;

	VkImageView textureImageView;
	VkSampler textureSampler;

	VkImage depthImage;
	VkDeviceMemory depthImageMemory;
	VkImageView depthImageView;

	VkImage colorImage;
	VkDeviceMemory colorImageMemory;
	VkImageView colorImageView;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;
	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	const int MAX_FRAMES_IN_FLIGHT = 2;
	size_t currentFrame = 0;

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete() {
			return graphicsFamily.has_value()
				&& presentFamily.has_value();
		}
	};

	// スワップチェーン作成に必要な情報
	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;		// 基本的なサーフェイス機能
		std::vector<VkSurfaceFormatKHR> formats;	// カラーフォーマット、カラースペース
		std::vector<VkPresentModeKHR> presentModes;	// 表示（Vsyncなど）モード
	};

	void initWindow();

	void initVulkan();
	void createInstance();
	void createSurface();
	void mainLoop();
	void drawFrame();

	void cleanup();
	void cleanupSwapChain();

	GLFWwindow* window;

	VkSurfaceKHR surface;

	VkInstance instance;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;
	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkCommandPool commandPool;
	std::vector<VkCommandBuffer> commandBuffers;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;
	bool framebufferResized = false;

	void pickPhysicalDevice();
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);

	void createLogicalDevice();
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

	// カラーフォーマット・カラースペースを選択する
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	
	// ディスプレイとの同期方法を選択する
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

	// スワップチェーンの解像度を決定する
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	// スワップチェーン作成
	void createSwapChain();

	// スワップチェーン再生成
	void recreateSwapChain();

	// イメージビュー作成
	void createImageViews();

	// Render pass 作成
	void createRenderPass();

	// グラフィックスパイプライン作成
	void createGraphicsPipeline();

	// シェーダーモジュール作成
	VkShaderModule createShaderModule(const std::vector<char>& code);

	// フレームバッファ作成
	void createFramebuffers();

	// コマンドプール作成
	void createCommandPool();

	// コマンドバッファ作成
	void createCommandBuffers();

	// コマンドバッファの作成とレコード開始を行う
	VkCommandBuffer beginSingleTimeCommands();

	// コマンドバッファのレコード完了・サブミット・解放
	void endSingleTimeCommands(VkCommandBuffer commandBuffer);

	// バッファ作成
	void createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);

	// モデルロード
	void loadModel();

	// 頂点バッファ作成
	void createVertexBuffer();

	// インデックスバッファ作成
	void createIndexBuffer();

	// バッファコピー
	void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

	// メモリタイプを見つける
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	// テクスチャイメージ作成
	void createTextureImage();

	// イメージ作成
	void createImage(
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		VkSampleCountFlagBits numSamples,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory);

	// イメージビュー作成
	VkImageView createImageView(
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags, 
		uint32_t mipLevels);

	// イメージレイアウト遷移
	void transitionImageLayout(
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		uint32_t mipLevels);

	// バッファをイメージにコピーする
	void copyBufferToImage(
		VkBuffer buffer,
		VkImage image,
		uint32_t width,
		uint32_t height);

	// テクスチャのイメージビュー作成
	void createTextureImageView();

	// イメージサンプラを作成する
	void createTextureSampler();

	// ミップマップを生成する
	void generateMipmaps(
		VkImage image,
		VkFormat imageFormat,
		uint32_t texWidth,
		uint32_t texHeight,
		uint32_t mipLevels
	);

	// MSAAサンプリング数を取得する
	VkSampleCountFlagBits getMaxUsableSampleCount();

	// デプスリソースを作成する
	void createDepthResource();

	// カラーリソースを作成する
	void createColorResources();

	// フォーマットを検索する
	VkFormat findSupportedFormat(
		const std::vector<VkFormat>& candidates,
		VkImageTiling tiling,
		VkFormatFeatureFlags features);

	// デプスフォーマットを検索する
	VkFormat findDepthFormat();

	// ステンシルコンポーネントを持っているか確認する
	bool hasStencilComponent(VkFormat format);

	// 同期オブジェクト作成
	void createSyncObjects();

	// ディスクリプタセットレイアウト作成
	void createDescriptorSetLayout();

	// ユニフォームバッファ作成
	void createUniformBuffers();

	// ユニフォームバッファ更新
	void updateUniformBuffer(uint32_t currentImage);

	// ディスクリプタプール作成
	void createDescriptorPool();

	// ディスクリプタセット作成
	void createDescriptorSets();

	VkDebugUtilsMessengerEXT debugMessenger;

	bool checkValidationLayerSupport();
	std::vector<const char*> getRequiredExtensions();

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	void setupDebugMessenger();

	VkResult CreateDebugUtilsMessengerEXT(
		VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pDebugMessenger
	);

	static void DestroyDebugUtilsMessengerEXT(
		VkInstance instance, 
		VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator
	);

	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	static std::vector<char> readFile(const std::string& filename);
	VkPipeline graphicsPipeline;

	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
};

namespace std {
	template<> struct hash<HelloTriangleApplication::Vertex>
	{
		size_t operator()(HelloTriangleApplication::Vertex const& vertex) const {
			return ((hash<glm::vec3>()(vertex.pos) ^
				(hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
				(hash<glm::vec2>()(vertex.texCoord) << 1);
		}
	};
}