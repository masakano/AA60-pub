//
//
//
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <GLFW/glfw3.h>

static GLFWwindow* s_window = nullptr;
static GLFWmonitor* s_monitor = nullptr;
static const char* s_title = "pscloud";

void init()
{
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW\n");
		exit(1);
	}

	{
		int count = 0;
		auto monitors = glfwGetMonitors(&count);
		assert(count > 0);
		s_monitor = monitors[count - 1];
		assert(s_monitor);
	}

	if (s_monitor) {
		const GLFWvidmode* mode = glfwGetVideoMode(s_monitor);
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
	}
}

void terminate() { glfwTerminate(); }

void open()
{
	int ox = 0;
	int oy = 0;
	int sx = 640;
	int sy = 480;
	bool is_fullscreen = false;
	bool is_iconic = false;

	if (is_fullscreen) {
		const GLFWvidmode* mode = glfwGetVideoMode(s_monitor);
		sx = mode->width;
		sy = mode->height;
		s_window = glfwCreateWindow(sx, sy, s_title, s_monitor, nullptr);
	}
	else {
		s_window = glfwCreateWindow(sx, sy, s_title, nullptr, nullptr);
	}
	if (!s_window) {
		fprintf(stderr, "Failed to create GLFW window sx=%d sy=%d title=[%s] monitor=%p\n", sx, sy,
		        s_title, s_monitor);
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	if (is_iconic) {
		glfwIconifyWindow(s_window);
	}

	glfwMakeContextCurrent(s_window);

	glfwSetWindowSize(s_window, sx, sy);
	glfwSetWindowPos(s_window, ox, oy);
	glfwSetWindowTitle(s_window, s_title);
}

void close()
{
	glfwSetWindowShouldClose(s_window, GLFW_TRUE);
	glfwDestroyWindow(s_window);
}

int main()
{
	printf("init GLFW\n");
	init();
	printf("hit return key...");
	getchar();

	printf("open GLFW\n");
	open();
	printf("hit return key...");
	getchar();

	printf("close GLFW\n");
	close();
	printf("hit return key...");
	getchar();

	printf("terminate GLFW\n");
	terminate();
	printf("hit return key...");
	getchar();

	printf("done\n");
	return 0;
}
