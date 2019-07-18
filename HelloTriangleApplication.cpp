#include "HelloTriangleApp.h"
#include <vector>

void HelloTriangleApplication::initWindow()
{
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);

}

void HelloTriangleApplication::initVulkan()
{
	createInstance();
}

void HelloTriangleApplication::createInstance()
{
	// �A�v���P�[�V�������
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";		// ����̃G���W�����w��(?)
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// �C���X�^���X�������
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// �O���[�o���g���̐ݒ�
	// GLFW�Ɗg����ڑ�����i���߂̏����j
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

#ifdef _DEBUG
	// �ۑ�
	// �C���X�^���X�����O�ɃT�|�[�g����Ă���g�����X�g���擾�i�\������j
	{
		uint32_t extensionCount = 0;
		// �g���̐����擾�i�����킩��Ȃ��Ɣz�񂪍��Ȃ��j
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		// �g������ێ�����z����쐬
		std::vector<VkExtensionProperties> extensions(extensionCount);
		// �g�������擾
		vkEnumerateInstanceExtensionProperties(nullptr, &glfwExtensionCount, extensions.data());

		// �g������\��
		std::cout << "available extensions:" << std::endl;
		for (const auto& extension : extensions)
		{
			std::cout << "\t" << extension.extensionName << std::endl;
		}
	}
#endif

	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;	// ���������ɂ��Ă���

	// �C���X�^���X�쐬
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
}

void HelloTriangleApplication::mainLoop()
{
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void HelloTriangleApplication::cleanup()
{
	vkDestroyInstance(instance, nullptr);

	glfwDestroyWindow(window);

	glfwTerminate();
}