#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

void error_callback(int error, const char *description)
{
    fprintf(stderr, "Error (%d): %s\n", error, description);
}

int main(void)
{

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    printf("glfw init\n");

    GLFWwindow *window = glfwCreateWindow(640, 480, "tjtech1", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    while (!glfwWindowShouldClose(window)) { glfwPollEvents(); }

    glfwDestroyWindow(window);
    glfwTerminate();

    exit(EXIT_SUCCESS);
}
