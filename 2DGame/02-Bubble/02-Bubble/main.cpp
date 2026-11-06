#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Game.h"


#define TARGET_FRAMERATE 60.0f


static int windowedX = 100, windowedY = 100;
static int windowedWidth = SCREEN_WIDTH, windowedHeight = SCREEN_HEIGHT;
static bool windowedMaximized = false;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// Keep the same logical camera and HUD at every window size.
	int viewWidth = width;
	int viewHeight = width * SCREEN_HEIGHT / SCREEN_WIDTH;
	if(viewHeight > height)
	{
		viewHeight = height;
		viewWidth = height * SCREEN_WIDTH / SCREEN_HEIGHT;
	}
	glViewport((width - viewWidth) / 2, (height - viewHeight) / 2, viewWidth, viewHeight);
}

void toggleFullscreen(GLFWwindow* window)
{
	if(glfwGetWindowMonitor(window) != NULL)
	{
		glfwSetWindowMonitor(window, NULL, windowedX, windowedY, windowedWidth, windowedHeight, 0);
		if(windowedMaximized)
			glfwMaximizeWindow(window);
		return;
	}

	// Use the monitor containing most of the window.
	int x, y, width, height, monitorCount;
	glfwGetWindowPos(window, &x, &y);
	glfwGetWindowSize(window, &width, &height);
	GLFWmonitor **monitors = glfwGetMonitors(&monitorCount);
	GLFWmonitor *monitor = glfwGetPrimaryMonitor();
	int bestOverlap = 0;
	for(int i = 0; i < monitorCount; i++)
	{
		int mx, my;
		glfwGetMonitorPos(monitors[i], &mx, &my);
		const GLFWvidmode *mode = glfwGetVideoMode(monitors[i]);
		int overlap = glm::max(0, glm::min(x + width, mx + mode->width) - glm::max(x, mx))
		            * glm::max(0, glm::min(y + height, my + mode->height) - glm::max(y, my));
		if(overlap > bestOverlap)
		{
			bestOverlap = overlap;
			monitor = monitors[i];
		}
	}
	windowedMaximized = glfwGetWindowAttrib(window, GLFW_MAXIMIZED) != 0;
	if(windowedMaximized)
		glfwRestoreWindow(window);
	glfwGetWindowPos(window, &windowedX, &windowedY);
	glfwGetWindowSize(window, &windowedWidth, &windowedHeight);
	const GLFWvidmode *mode = glfwGetVideoMode(monitor);
	glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_F11)
	{
		if(action == GLFW_PRESS)
			toggleFullscreen(window);
		return;
	}
	if (action == GLFW_PRESS)
		Game::instance().keyPressed(key);
	else if (action == GLFW_RELEASE)
		Game::instance().keyReleased(key);
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	Game::instance().mouseMove(int(xpos), int(ypos));
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS)
		Game::instance().mousePress(button);
	else if (action == GLFW_RELEASE)
		Game::instance().mouseRelease(button);
}


int main(void)
{
	GLFWwindow* window;
	double timePerFrame = 1.f / TARGET_FRAMERATE, timePreviousFrame, currentTime;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Pocket Bomberman", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	/* Set window initial position */
	glfwSetWindowPos(window, 100, 100);
	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	/* Set callbacks */
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	/* Init glew to have access to GL extensions */
	glewExperimental = GL_TRUE;
	glewInit();
	int framebufferWidth, framebufferHeight;
	glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
	framebuffer_size_callback(window, framebufferWidth, framebufferHeight);

	/* Init step of the game loop */
	Game::instance().init();
	timePreviousFrame = glfwGetTime();

	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
		if(framebufferWidth == 0 || framebufferHeight == 0 || glfwGetWindowAttrib(window, GLFW_ICONIFIED))
		{
			glfwWaitEvents();
			timePreviousFrame = glfwGetTime();
			continue;
		}
		currentTime = glfwGetTime();
		if (currentTime - timePreviousFrame >= timePerFrame)
		{
			/* Update & render steps of the game loop */
			if(!Game::instance().update(int(1000.0f * (currentTime - timePreviousFrame))))
				glfwSetWindowShouldClose(window, GLFW_TRUE);
			Game::instance().render();
			timePreviousFrame = currentTime;

			/* Swap front and back buffers */
			glfwSwapBuffers(window);
		}

		/* Poll for and process events */
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

