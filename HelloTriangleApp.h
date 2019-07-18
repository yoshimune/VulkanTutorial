#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <functional>


class HelloTriangleApplication {
public:
	const int WIDTH = 800;
	const int HEIGHT = 600;

	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

private:
	void initWindow();

	void initVulkan();
	void createInstance();

	void mainLoop();

	void cleanup();

	GLFWwindow* window;

	VkInstance instance;
};

