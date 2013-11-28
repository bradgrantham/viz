#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <GLFW/glfw3.h>

int main()
{
    GLFWwindow* window;

    if(!glfwInit())
        exit(EXIT_FAILURE);

    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        fprintf(stdout, "Couldn't open main window\n");
        exit(EXIT_FAILURE);
    }

    sleep(1);

    glfwTerminate();
}
