#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <chrono>//测量每帧的时间

using namespace std;

#define mapWidth 24
#define mapHeight 24
#define screenWidth 1280
#define screenHeight 960
#define w 1280
#define h 960

int worldMap[mapWidth][mapHeight] = 
{
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

double posX = 22, posY = 12;  //x and y start position
double dirX = -1, dirY = 0; //initial direction vector
double planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

void drawMiniMap();//小地图绘制

int main()
{
    // 初始化GLFW库
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // 设置OpenGL版本为3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // 设置OpenGL为核心模式
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Raycaster", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // 初始化GLEW
    glewExperimental = GL_TRUE; // This is needed in some systems
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        // Handle error
        std::cerr << "Error initializing GLEW: " << glewGetErrorString(err) << std::endl;
        return -1;
    }

    // 顶点着色器
    const char* vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec2 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "out vec3 ourColor;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos, 0.0, 1.0);\n"
        "   ourColor = aColor;\n"
        "}\0";

    // 片元着色器
    const char* fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "in vec3 ourColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(ourColor, 1.0);\n"
        "}\n\0";

    // 创建顶点着色器
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    //// 检查顶点着色器是否正确编译
    //GLint success;
    //glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    //if (!success)
    //{
    //    GLchar infoLog[512];
    //    glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
    //    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    //}

    // 创建片元着色器
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    //// 检查片元着色器是否正确编译
    //glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    //if (!success)
    //{
    //    GLchar infoLog[512];
    //    glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
    //    std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    //}

    // 创建着色器程序并链接着色器
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    //// 检查着色器程序是否正确链接
    //glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    //if (!success) {
    //    GLchar infoLog[512];
    //    glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
    //    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    //}

    // 删除着色器，它们已经链接到程序中
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 在初始化GLFW之后，禁用光标
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    auto oldTime = std::chrono::high_resolution_clock::now();//帧数时间，用于稳定移动速度
    double lastX = 0; // 上一帧鼠标的位置

    while (!glfwWindowShouldClose(window))
    {
        // 渲染到第一个窗口
        glfwMakeContextCurrent(window);
        glClear(GL_COLOR_BUFFER_BIT);

        // 使用着色器程序
        glUseProgram(shaderProgram);

        for (int x = 0; x < screenWidth; x++)
        {
            //calculate ray position and direction
            double cameraX = 2 * x / (double)w - 1; //x-coordinate in camera space
            double rayDirX = dirX + planeX * cameraX;
            double rayDirY = dirY + planeY * cameraX;
            //which box of the map we're in
            int mapX = int(posX);
            int mapY = int(posY);

            //length of ray from current position to next x or y-side
            double sideDistX;
            double sideDistY;

            //length of ray from one x or y-side to next x or y-side
            //these are derived as:
            //deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX))
            //deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY))
            //which can be simplified to abs(|rayDir| / rayDirX) and abs(|rayDir| / rayDirY)
            //where |rayDir| is the length of the vector (rayDirX, rayDirY). Its length,
            //unlike (dirX, dirY) is not 1, however this does not matter, only the
            //ratio between deltaDistX and deltaDistY matters, due to the way the DDA
            //stepping further below works. So the values can be computed as below.
            // Division through zero is prevented, even though technically that's not
            // needed in C++ with IEEE 754 floating point values.
            double deltaDistX = (rayDirX == 0) ? 1e30 : std::abs(1 / rayDirX);
            double deltaDistY = (rayDirY == 0) ? 1e30 : std::abs(1 / rayDirY);

            double perpWallDist;

            //what direction to step in x or y-direction (either +1 or -1)
            int stepX;
            int stepY;

            int hit = 0; //was there a wall hit?
            int side; //was a NS or a EW wall hit?
            //calculate step and initial sideDist
            if (rayDirX < 0)
            {
                stepX = -1;
                sideDistX = (posX - mapX) * deltaDistX;
            }
            else
            {
                stepX = 1;
                sideDistX = (mapX + 1.0 - posX) * deltaDistX;
            }
            if (rayDirY < 0)
            {
                stepY = -1;
                sideDistY = (posY - mapY) * deltaDistY;
            }
            else
            {
                stepY = 1;
                sideDistY = (mapY + 1.0 - posY) * deltaDistY;
            }
            //perform DDA
            while (hit == 0)
            {
                //jump to next map square, either in x-direction, or in y-direction
                if (sideDistX < sideDistY)
                {
                    sideDistX += deltaDistX;
                    mapX += stepX;
                    side = 0;
                }
                else
                {
                    sideDistY += deltaDistY;
                    mapY += stepY;
                    side = 1;
                }
                //Check if ray has hit a wall
                if (worldMap[mapX][mapY] > 0) hit = 1;
            }
            //Calculate distance projected on camera direction. This is the shortest distance from the point where the wall is
            //hit to the camera plane. Euclidean to center camera point would give fisheye effect!
            //This can be computed as (mapX - posX + (1 - stepX) / 2) / rayDirX for side == 0, or same formula with Y
            //for size == 1, but can be simplified to the code below thanks to how sideDist and deltaDist are computed:
            //because they were left scaled to |rayDir|. sideDist is the entire length of the ray above after the multiple
            //steps, but we subtract deltaDist once because one step more into the wall was taken above.
            if (side == 0) perpWallDist = (sideDistX - deltaDistX);
            else          perpWallDist = (sideDistY - deltaDistY);

            //Calculate height of line to draw on screen
            int lineHeight = (int)(h / perpWallDist) * 2;

            //calculate lowest and highest pixel to fill in current stripe
            int drawStart = -lineHeight / 2 + h / 2;
            if (drawStart < 0) drawStart = 0;
            int drawEnd = lineHeight / 2 + h / 2;
            if (drawEnd >= h) drawEnd = h - 1;

            // 选择墙壁颜色
            GLfloat red, green, blue;
            switch (worldMap[mapX][mapY])
            {
            case 1:  red = 1.0f; green = 0.0f; blue = 0.0f; break; // 红色
            case 2:  red = 0.0f; green = 1.0f; blue = 0.0f; break; // 绿色
            case 3:  red = 0.0f; green = 0.0f; blue = 1.0f; break; // 蓝色
            case 4:  red = 1.0f; green = 1.0f; blue = 1.0f; break; // 白色
            default: red = 1.0f; green = 1.0f; blue = 0.0f; break; // 黄色
            }

            // 如果是y方向的墙壁，降低亮度
            if (side == 1) { red *= 0.5f; green *= 0.5f; blue *= 0.5f; }

            // 顶点数据
            GLfloat vertices[] = {
                x / (float)screenWidth * 2 - 1, drawStart / (float)screenHeight * 2 - 1,// 转换到-1到1的范围
                x / (float)screenWidth * 2 - 1, drawEnd / (float)screenHeight * 2 - 1// 转换到-1到1的范围
            };

            // 颜色数据
            GLfloat colors[] = {
                red, green, blue,
                red, green, blue
            };

            // 创建一个新的VAO
            GLuint VAO;
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            // 创建一个新的VBO用于顶点数据
            GLuint VBO1;
            glGenBuffers(1, &VBO1);
            glBindBuffer(GL_ARRAY_BUFFER, VBO1);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            // 创建一个新的VBO用于颜色数据
            GLuint VBO2;
            glGenBuffers(1, &VBO2);
            glBindBuffer(GL_ARRAY_BUFFER, VBO2);
            glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(1);

            // 绘制线段
            glDrawArrays(GL_LINES, 0, 2);

            //// 删除VAO和VBO
            //glDeleteVertexArrays(1, &VAO);
            //glDeleteBuffers(1, &VBO1);
            //glDeleteBuffers(1, &VBO2);

            // 准星的大小
            float sight_size = 0.01f;

            // 准星的顶点数据
            GLfloat sight_vertices[] = {
                -sight_size, 0.0f,
                 sight_size, 0.0f,
                0.0f, -sight_size,
                0.0f,  sight_size
            };

            // 准星的颜色数据
            GLfloat sight_colors[] = {
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f,
                1.0f, 1.0f, 1.0f
            };

            // 创建一个新的VAO
            //GLuint VAO;
            //glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            // 创建一个新的VBO用于顶点数据
            //GLuint VBO1;
            //glGenBuffers(1, &VBO1);
            glBindBuffer(GL_ARRAY_BUFFER, VBO1);
            glBufferData(GL_ARRAY_BUFFER, sizeof(sight_vertices), sight_vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(0);

            // 创建一个新的VBO用于颜色数据
            //GLuint VBO2;
            //glGenBuffers(1, &VBO2);
            glBindBuffer(GL_ARRAY_BUFFER, VBO2);
            glBufferData(GL_ARRAY_BUFFER, sizeof(sight_colors), sight_colors, GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
            glEnableVertexAttribArray(1);

            // 绘制准星
            glDrawArrays(GL_LINES, 0, 2);
            glDrawArrays(GL_LINES, 2, 2);

            // 删除VAO和VBO
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO1);
            glDeleteBuffers(1, &VBO2);
        }

        //timing for input and FPS counter
        auto currentTime = std::chrono::high_resolution_clock::now();
        double frameTime = std::chrono::duration<double>(currentTime - oldTime).count();
        oldTime = currentTime;

        //speed modifiers
        double moveSpeed = frameTime * 5.0; //the constant value is in squares/second
        //double rotSpeed = frameTime * 3.0; //the constant value is in radians/second

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {
            if (worldMap[int(posX + dirX * moveSpeed)][int(posY)] == false) posX += dirX * moveSpeed;
            if (worldMap[int(posX)][int(posY + dirY * moveSpeed)] == false) posY += dirY * moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {
            if (worldMap[int(posX - dirX * moveSpeed)][int(posY)] == false) posX -= dirX * moveSpeed;
            if (worldMap[int(posX)][int(posY - dirY * moveSpeed)] == false) posY -= dirY * moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {
            if (worldMap[int(posX - dirY * moveSpeed)][int(posY)] == false) posX -= dirY * moveSpeed;
            if (worldMap[int(posX)][int(posY + dirX * moveSpeed)] == false) posY += dirX * moveSpeed;
        }
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {
            if (worldMap[int(posX + dirY * moveSpeed)][int(posY)] == false) posX += dirY * moveSpeed;
            if (worldMap[int(posX)][int(posY - dirX * moveSpeed)] == false) posY -= dirX * moveSpeed;
        }

        ////rotate to the right
        //if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        //{
        //    //both camera direction and camera plane must be rotated
        //    double oldDirX = dirX;
        //    dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
        //    dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
        //    double oldPlaneX = planeX;
        //    planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
        //    planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
        //}
        ////rotate to the left
        //if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        //{
        //    //both camera direction and camera plane must be rotated
        //    double oldDirX = dirX;
        //    dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
        //    dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
        //    double oldPlaneX = planeX;
        //    planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
        //    planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
        //}

        // 获取鼠标的当前位置
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // 计算鼠标自上一帧以来移动了多少像素
        double deltaX = lastX - mouseX;
        
        // 更新上一帧的鼠标位置
        lastX = mouseX;

        // 根据鼠标的移动来旋转视角
        double sensitivity = 0.001; // 调整这个值来改变鼠标的灵敏度
        double rotSpeed = deltaX * sensitivity;

        // 旋转视角
        double oldDirX = dirX;
        dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
        dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
        double oldPlaneX = planeX;
        planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
        planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);

        // 检查是否按下了ESC键
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true); // 关闭窗口
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}