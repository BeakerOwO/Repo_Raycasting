#include <GL/glew.h>
#include <GL/glut.h>
#include <GLFW/glfw3.h>
#include <cmath>
#include <vector>
#include <iostream>
#include <cstdint>//用于纹理
#include <chrono>//测量每帧的时间,用于计算移动(视角)速度
#include <opencv2/opencv.hpp>

using namespace std;

#define mapWidth 24
#define mapHeight 24
#define screenWidth 1280
#define screenHeight 960
#define w 1280
#define h 960

#define texWidth 128
#define texHeight 128   //纹理大小

const double PI = 3.14159265357;
int worldMap[mapWidth][mapHeight] = 
{
  {2,3,4,5,6,2,3,4,5,6,2,3,4,5,6,2,3,4,5,6,2,3,4,5},
  {2,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,5},
  {6,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2},
  {5,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,3},
  {4,0,0,0,0,6,5,4,3,2,0,0,0,0,6,5,4,3,2,0,0,0,0,4},
  {3,0,0,0,0,2,4,4,4,4,6,0,0,2,4,4,4,4,6,0,0,0,0,5},
  {2,0,0,0,0,3,4,4,4,4,5,0,0,3,4,4,4,4,5,0,0,0,0,6},
  {6,0,0,0,0,4,4,4,4,4,4,0,0,4,4,4,4,4,4,0,0,0,0,2},
  {5,0,0,0,0,5,4,4,4,4,3,0,0,5,4,4,4,4,3,0,0,0,0,3},
  {4,0,0,0,0,6,4,4,4,4,2,0,0,6,4,4,4,4,2,0,0,0,0,4},
  {3,3,0,0,0,0,2,3,4,5,0,0,0,0,2,3,4,5,0,0,0,0,5,5},
  {2,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,4,4},
  {6,4,3,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,3,4,3},
  {5,5,0,0,0,0,5,4,3,2,0,0,0,0,5,4,3,2,0,0,0,0,3,2},
  {4,0,0,0,0,2,4,4,4,4,6,0,0,2,4,4,4,4,6,0,0,0,0,4},
  {3,0,0,0,0,3,4,4,4,4,5,0,0,3,4,4,4,4,5,0,0,0,0,5},
  {2,0,0,0,0,4,4,4,4,4,4,0,0,4,4,4,4,4,4,0,0,0,0,6},
  {6,0,0,0,0,5,4,4,4,4,3,0,0,5,4,4,4,4,3,0,0,0,0,2},
  {5,0,0,0,0,6,4,4,4,4,2,0,0,6,4,4,4,4,2,0,0,0,0,3},
  {4,0,0,0,0,2,3,4,5,6,0,0,0,0,2,3,4,5,6,0,0,0,0,4},
  {3,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,5},
  {2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,6},
  {6,0,0,0,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,0,0,0,2},
  {5,5,4,3,2,6,5,4,3,2,6,5,4,3,2,6,5,4,3,2,6,5,4,3}
};

double posX = 9, posY = 2;  //x和y的起始位置
double dirX = -1, dirY = 0; //初始方向向量
double planeX = 0, planeY = 0.66; //2D光线投射的相机平面

void drawMiniMap();//小地图绘制
// 有8个纹理
const int numTextures = 8;
// 纹理文件名，每个元素是一个string
vector<string> textureFileNames(numTextures);
// 图像数据存储在一个数组中
vector<cv::Mat> textureData(numTextures);
// OpenGL纹理ID，每个元素是一个GLuint
vector<GLuint> textureIDs(numTextures);
//纹理初始化
void texture_init();

int main(int argc, char** argv)
{
    // 初始化GLFW库
    if (!glfwInit())
    {
        cerr << "Failed to initialize GLFW" << endl;
        return -1;
    }
    //GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "Raycaster", glfwGetPrimaryMonitor(), NULL);
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
        cerr << "Error initializing GLEW: " << glewGetErrorString(err) << endl;
        return -1;
    }

    // 在初始化GLFW之后，禁用光标
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    auto oldTime = chrono::high_resolution_clock::now();//帧数时间，用于稳定移动速度
    double lastX = 0; // 上一帧鼠标的位置

    // 初始化GLUT库
    glutInit(&argc, argv);

    //设置屏幕居中
    int cx = glutGet(GLUT_SCREEN_WIDTH);
    int cy = glutGet(GLUT_SCREEN_HEIGHT);
    glutInitWindowPosition((cx - 1920) / 2, (cy - 1080) / 2);

    //启用纹理映射
    glEnable(GL_TEXTURE_2D);
    glDisable(GL_DEPTH_TEST);
    //初始化纹理
    texture_init();

    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);

        // FLOOR CASTING
        
        // 创建一个空的BGR图像
        cv::Mat img(screenHeight, screenWidth, CV_8UC3);
        
        for (int y = 0; y < h; y++)
        {
            // 最左边的光线（x = 0）和最右边的光线（x = w）的rayDir
            float rayDirX0 = dirX - planeX;
            float rayDirY0 = dirY - planeY;
            float rayDirX1 = dirX + planeX;
            float rayDirY1 = dirY + planeY;

            // 当前y(行)位置相对于屏幕中心
            int p = y - screenHeight / 2;

            // 相机的垂直位置
            float posZ = 0.5 * screenHeight;

            // 当前行的相机到地板的水平距离。
            // 0.5是地板和天花板之间正好在中间的z位置。
            float rowDistance = posZ / p;

            // 计算我们必须为每个x添加的真实世界步进向量 (平行于相机平面)
            // 逐步增加，避免了在内部循环中与权重相乘
            float floorStepX = rowDistance * (rayDirX1 - rayDirX0) / screenWidth;
            float floorStepY = rowDistance * (rayDirY1 - rayDirY0) / screenWidth;

            // 最左列的真实世界坐标。当我们从左往右绘制下一条线时，这将被更新。
            float floorX = posX + rowDistance * rayDirX0;
            float floorY = posY + rowDistance * rayDirY0;

            for (int x = 0; x < screenWidth; ++x)
            {
                // 单元格坐标只是从floorX和floorY的整数部分得到的
                int cellX = (int)(floorX);
                int cellY = (int)(floorY);

                // 从小数部分获取纹理坐标
                int tx = (int)(texWidth * (floorX - cellX)) & (texWidth - 1);
                int ty = (int)(texHeight * (floorY - cellY)) & (texHeight - 1);

                floorX += floorStepX;
                floorY += floorStepY;

                // 选择纹理并绘制像素
                int floorTexture = 6;
                int ceilingTexture = 7;
                // 地板
                cv::Vec3b color;
                color = textureData[floorTexture].at<cv::Vec3b>(ty, tx);
                img.at<cv::Vec3b>(screenHeight - y - 1, x) = color;

                // 天花板 (symmetrical, at screenHeight - y - 1 instead of y)
                //color = textureData[ceilingTexture].at<cv::Vec3b>(ty, tx);
                //img.at<cv::Vec3b>(y, x) = color;
            }
        }
        // *************绘制地板和天花板****************
        // 启用纹理映射
        glEnable(GL_TEXTURE_2D);
        // 创建一个纹理对象
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        // 将img上传为纹理
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img.cols, img.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, img.data);

        // 绘制一个覆盖整个屏幕的四边形，并将纹理映射到上面
        glColor3f(0.8, 0.8, 0.8);
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(-1, -1);
        glTexCoord2f(1, 0); glVertex2f(1, -1);
        glTexCoord2f(1, 1); glVertex2f(1, 1);
        glTexCoord2f(0, 1); glVertex2f(-1, 1);
        glEnd();

        // 删除纹理对象
        glDeleteTextures(1, &texture);

        //WALL CASTING
        for (int x = 0; x < screenWidth; x++)
        {
            //计算光线位置和方向
            double cameraX = 2 * x / (double)w - 1; //相机空间中的x坐标
            double rayDirX = dirX + planeX * cameraX;
            double rayDirY = dirY + planeY * cameraX;

            //我们的位置在地图的哪个格子里
            int mapX = int(posX);
            int mapY = int(posY);

            //从当前位置到下一个x(垂直)或y(水平)边的光线长度
            double sideDistX;
            double sideDistY;

            //从一个x或y边到下一个x或y边的光线长度
            //这些是由以下公式推导出来的：
            //deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX))
            //deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY))
            //可以简化为 abs(|rayDir| / rayDirX) and abs(|rayDir| / rayDirY)
            //其中 |rayDir| 是向量(rayDirX, rayDirY)的长度。它的长度，
            //不像(dirX, dirY)，不是1，但这并不重要，因为按照下面DDA算法的工作原理，
            //只有deltaDistX和deltaDistY之间的比例才重要，所以可以按照下面的方式计算值。
            //注意除数不能为0，尽管在C++中使用IEEE 754浮点值，但技术上这并不是必需的。
            double deltaDistX = (rayDirX == 0) ? 1e30 : abs(1 / rayDirX);
            double deltaDistY = (rayDirY == 0) ? 1e30 : abs(1 / rayDirY);

            double perpWallDist;

            //在x或y方向上步进的方向（+1或-1）
            int stepX;
            int stepY;

            int hit = 0; //是否击中了墙壁？
            int side; //是NS墙还是EW墙被击中？
            //计算步进方向step，和当前位置到下一个x或y边的光线长度sideDist
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

            //应用DDA算法
            int flag = 0;//记录是否是“半高”墙
            //初始化画线点
            int drawStart = 0;
            int drawEnd = 0;
            while (hit == 0)
            {
                //跳到下一个地图方块，可以是x方向，也可以是y方向
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
                //检查光线是否击中了墙壁
                if (worldMap[mapX][mapY] > 0) 
                {
                    if (worldMap[mapX][mapY] > 1)
                    {
                        hit = 1;
                        if (side == 0) perpWallDist = (sideDistX - deltaDistX);
                        else          perpWallDist = (sideDistY - deltaDistY);

                        //计算屏幕上要绘制的线的高度
                        int lineHeight = (int)(h / perpWallDist);

                        //计算当前条纹中要填充的最低和最高像素
                        drawStart = -lineHeight / 2 + h / 2;
                        if (drawStart < 0) drawStart = 0;
                        drawEnd = lineHeight / 2 + h / 2;
                        if (drawEnd >= h) drawEnd = h - 1;

                        //如果flag=1，即已经画出了下半部分半高墙，该画出剩下正确距离的全高墙的上半部分
                        if (flag == 1)
                        {
                            drawStart =  h / 2;
                            drawEnd = lineHeight / 2 + h / 2;
                            if (drawEnd >= h) drawEnd = h - 1;
                        }

                        //计算使用对应的纹理
                        int texNum = worldMap[mapX][mapY] - 1; //下标0 ~ n-1

                        //计算wallX的值
                        double wallX; //墙壁被击中的确切位置
                        if (side == 0) wallX = posY + perpWallDist * rayDirY;
                        else           wallX = posX + perpWallDist * rayDirX;
                        wallX -= floor((wallX));

                        //纹理上的x坐标
                        int texX = int(wallX * double(texWidth));
                        if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
                        if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

                        // 每个屏幕像素增加的纹理坐标量
                        double step = 1.0 * texHeight / lineHeight;
                        // 开始的纹理坐标
                        double texPos = (drawStart - screenHeight / 2 + lineHeight / 2) * step;

                        // 将纹理坐标转换为整数，在溢出时用& 运算将其循环回 0 到 texHeight - 1 的范围
                        int texY = (int)texPos & (texHeight - 1);
                        texPos += step;

                        // 计算纹理的y坐标
                        double texY1 = (drawStart - h / 2 + lineHeight / 2) * step;
                        double texY2 = (drawEnd - h / 2 + lineHeight / 2) * step;

                        //启用纹理映射
                        glEnable(GL_TEXTURE_2D);
                        // 绑定纹理
                        glBindTexture(GL_TEXTURE_2D, textureIDs[texNum]);
                        // 使用glTexCoord2f和glVertex2f绘制一个纹理映射的线段
                        glBegin(GL_LINES);
                        // 如果是x方向的墙壁，降低亮度
                        if (side == 0) { glColor3f(0.5, 0.5, 0.5); }// 设置颜色
                        else { glColor3f(1, 1, 1); }// 设置颜色
                        glTexCoord2f((texX + 0.5) / (float)texWidth, texY1/ (float)texHeight); glVertex2f(x / (float)screenWidth * 2 - 1, drawStart / (float)screenHeight * 2 - 1);
                        glTexCoord2f((texX + 0.5) / (float)texWidth, texY2/ (float)texHeight); glVertex2f(x / (float)screenWidth * 2 - 1, drawEnd / (float)screenHeight * 2 - 1);
                        glEnd();
                        

                        // *************绘制天空***************
                        //禁用纹理映射
                        glDisable(GL_TEXTURE_2D);
                        glBegin(GL_LINES);
                        glColor3f(0.611, 0.862, 0.921); // 设置颜色
                        glVertex2f(x / (float)screenWidth * 2 - 1, drawEnd / (float)screenHeight * 2 - 1); // 转换到-1到1的范围
                        glVertex2f(x / (float)screenWidth * 2 - 1, 1); // 转换到-1到1的范围
                        glEnd();

                        //// 绘制地板
                        //if (flag == 0)//在画半高墙时已经画出了半高墙下面的地板
                        //{
                        //    glBegin(GL_LINES);
                        //    glColor3f(0.3, 0.3, 0.3); // 设置颜色
                        //    glVertex2f(x / (float)screenWidth * 2 - 1, -1); // 转换到-1到1的范围
                        //    glVertex2f(x / (float)screenWidth * 2 - 1, drawStart / (float)screenHeight * 2 - 1); // 转换到-1到1的范围
                        //    glEnd();
                        //}
                        
                    }
                    //如果不是白色，那么判断是“半高”的墙
                    if (worldMap[mapX][mapY] == 1)
                    {
                        //如果已经画过了半高的墙，那么不应该重复画出半高墙后面的半高墙，因此直接continue
                        if (flag == 1)
                            continue;
                        //判断是初次画半高的墙，画下半部分即可
                        //令hit=0，使其可以再次进入DDA循环，并记录flag=1，目的是为了正确画出上半部分正确的“全高”墙
                        hit = 0;
                        flag = 1;
                        if (side == 0) perpWallDist = (sideDistX - deltaDistX);
                        else          perpWallDist = (sideDistY - deltaDistY);

                        //计算屏幕上要绘制的线的高度
                        int lineHeight = (int)(h / perpWallDist);

                        //计算当前条纹中要填充的最低和最高像素
                        drawStart = -lineHeight / 2 + h / 2;
                        if (drawStart < 0) drawStart = 0;
                        //只需画出下半部分墙实现“半高”
                        drawEnd = h / 2;

                        //计算使用对应的纹理
                        int texNum = worldMap[mapX][mapY] - 1; //下标0 ~ n-1

                        //计算wallX的值
                        double wallX; //墙壁被击中的确切位置
                        if (side == 0) wallX = posY + perpWallDist * rayDirY;
                        else           wallX = posX + perpWallDist * rayDirX;
                        wallX -= floor((wallX));

                        //纹理上的x坐标
                        int texX = int(wallX * double(texWidth));
                        if (side == 0 && rayDirX > 0) texX = texWidth - texX - 1;
                        if (side == 1 && rayDirY < 0) texX = texWidth - texX - 1;

                        // 每个屏幕像素增加的纹理坐标量
                        double step = 1.0 * texHeight / lineHeight;
                        
                        // 计算纹理的y坐标
                        double texY1 = (drawStart - h / 2 + lineHeight / 2) * step;
                        double texY2 = (drawEnd - h / 2 + lineHeight / 2) * step;

                        //启用纹理映射
                        glEnable(GL_TEXTURE_2D);
                        // 绑定纹理
                        glBindTexture(GL_TEXTURE_2D, textureIDs[texNum]);
                        // 使用glTexCoord2f和glVertex2f绘制一个纹理映射的线段
                        glBegin(GL_LINES);
                        // 如果是x方向的墙壁，降低亮度
                        if (side == 0) { glColor3f(0.5, 0.5, 0.5); }// 设置颜色
                        else { glColor3f(1, 1, 1); }// 设置颜色
                        glTexCoord2f(texX / (float)texWidth, texY1 / (float)texHeight); glVertex2f(x / (float)screenWidth * 2 - 1, drawStart / (float)screenHeight * 2 - 1);
                        glTexCoord2f(texX / (float)texWidth, texY2 / (float)texHeight); glVertex2f(x / (float)screenWidth * 2 - 1, drawEnd / (float)screenHeight * 2 - 1);
                        glEnd();
                        

                        //// ***************绘制地板************
                        ////禁用纹理映射
                        //glDisable(GL_TEXTURE_2D);
                        //glBegin(GL_LINES);
                        //glColor3f(0.3, 0.3, 0.3); // 设置颜色
                        //glVertex2f(x / (float)screenWidth * 2 - 1, -1); // 转换到-1到1的范围
                        //glVertex2f(x / (float)screenWidth * 2 - 1, drawStart / (float)screenHeight * 2 - 1); // 转换到-1到1的范围
                        //glEnd();
                    }
                }
            }
            
        }
        // 在调用drawMiniMap之前保存视口、投影矩阵
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glPushAttrib(GL_CURRENT_BIT);
        // 在调用drawMiniMap之前禁用纹理映射
        glDisable(GL_TEXTURE_2D);

        // 绘制小地图
        drawMiniMap();

        // 恢复原来的视口、投影矩阵
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glPopAttrib();
        glMatrixMode(GL_MODELVIEW);
        // 在调用drawMiniMap之后启用纹理映射
        glEnable(GL_TEXTURE_2D);

        //计算输入帧数时间差，用来计算速度
        auto currentTime = chrono::high_resolution_clock::now();
        double frameTime = chrono::duration<double>(currentTime - oldTime).count();
        oldTime = currentTime;

        //视角移动、旋转速度
        double moveSpeed = frameTime * 3.0; 
        double rotSpeed = frameTime * 3.0; 
        //前后左右移动
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
        //向右旋转
        if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        {
            //相机方向和相机平面都必须旋转
            double oldDirX = dirX;
            dirX = dirX * cos(-rotSpeed) - dirY * sin(-rotSpeed);
            dirY = oldDirX * sin(-rotSpeed) + dirY * cos(-rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * cos(-rotSpeed) - planeY * sin(-rotSpeed);
            planeY = oldPlaneX * sin(-rotSpeed) + planeY * cos(-rotSpeed);
        }
        //向左旋转
        if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        {
            //相机方向和相机平面都必须旋转
            double oldDirX = dirX;
            dirX = dirX * cos(rotSpeed) - dirY * sin(rotSpeed);
            dirY = oldDirX * sin(rotSpeed) + dirY * cos(rotSpeed);
            double oldPlaneX = planeX;
            planeX = planeX * cos(rotSpeed) - planeY * sin(rotSpeed);
            planeY = oldPlaneX * sin(rotSpeed) + planeY * cos(rotSpeed);
        }

        // 获取鼠标的当前位置
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        // 计算鼠标自上一帧以来移动了多少像素
        double deltaX = lastX - mouseX;

        // 更新上一帧的鼠标位置
        lastX = mouseX;

        // 根据鼠标的移动来旋转视角
        double sensitivity = 0.001; // 调整这个值来改变鼠标的灵敏度
        rotSpeed = deltaX * sensitivity;

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

void drawMiniMap()
{
    int miniMapScale = 8; // 缩放因子

    // 设置正交投影以便我们可以在2D中绘制
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screenWidth, screenHeight, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // 绘制小地图的空地
    // 遍历地图的每个单元格
    for (int x = 0; x < mapWidth; x++)
    {
        for (int y = 0; y < mapHeight; y++)
        {
            // 如果单元格中没有墙壁，就在对应的位置绘制一个小矩形
            if (worldMap[x][y] == 0)
            {
                glColor3f(0.0f, 0.0f, 0.0f); // 设置颜色为黑色
                glBegin(GL_QUADS);
                glVertex2i(x * miniMapScale, (mapHeight - y) * miniMapScale);
                glVertex2i((x + 1) * miniMapScale, (mapHeight - y) * miniMapScale);
                glVertex2i((x + 1) * miniMapScale, (mapHeight - (y + 1)) * miniMapScale);
                glVertex2i(x * miniMapScale, (mapHeight - (y + 1)) * miniMapScale);
                glEnd();
            }
        }
    }
    // 在玩家的位置绘制一个点
    glPointSize(3.0f);
    glColor3f(1.0f, 0.0f, 0.0f); // 设置颜色为红色
    glBegin(GL_POINTS);
    glVertex2i(posX * miniMapScale, (mapHeight - posY) * miniMapScale);
    glEnd();

    //绘制视野(与主函数DDA算法一致)
    for (int x = 0; x < screenWidth; x++)
    {
        double cameraX = 2 * x / (double)w - 1; //相机空间中的x坐标
        double rayDirX = dirX + planeX * cameraX; //计算光线在x方向上的方向
        double rayDirY = dirY + planeY * cameraX; //计算光线在y方向上的方向

        int mapX = int(posX); //我们在地图的哪个格子里（x坐标）
        int mapY = int(posY); //我们在地图的哪个格子里（y坐标）

        double sideDistX; //从当前位置到下一个x边的光线长度
        double sideDistY; //从当前位置到下一个y边的光线长度

        double deltaDistX = (rayDirX == 0) ? 1e30 : abs(1 / rayDirX); //从一个x边到下一个x边的光线长度
        double deltaDistY = (rayDirY == 0) ? 1e30 : abs(1 / rayDirY); //从一个y边到下一个y边的光线长度

        double perpWallDist; //垂直墙壁距离

        int stepX; //在x方向上步进的方向（+1或-1）
        int stepY; //在y方向上步进的方向（+1或-1）

        int hit = 0; //是否击中了墙壁？
        int side; //是NS墙还是EW墙被击中？
        //计算步长和初始的sideDist
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
        //应用DDA算法
        while (hit == 0)
        {
            //跳到下一个地图方块，可以是x方向，也可以是y方向
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
            //检查光线是否击中了墙壁
            if (worldMap[mapX][mapY] > 1) hit = 1;
        }

        if (side == 0) perpWallDist = (sideDistX - deltaDistX);
        else          perpWallDist = (sideDistY - deltaDistY);

        double distx = posX + perpWallDist * rayDirX;
        double disty = posY + perpWallDist * rayDirY;

        //在小地图上画出视野的线
        glColor3f(0.5f, 0.5f, 0.5f);
        glBegin(GL_LINES);
        glVertex2i(posX * miniMapScale, (mapHeight - posY) * miniMapScale);
        glVertex2i(distx * miniMapScale, (mapHeight - disty) * miniMapScale);
        glEnd();
    }
    // 绘制小地图的墙
    // 遍历地图的每个单元格
    for (int x = 0; x < mapWidth; x++)
    {
        for (int y = 0; y < mapHeight; y++)
        {
            // 如果单元格中有墙壁，就在对应的位置绘制一个小矩形
            if (worldMap[x][y] > 0)
            {
                if (worldMap[x][y] > 1)
                {
                    glColor3f(1.0f, 1.0f, 1.0f); // 设置颜色为白色
                }
                else
                {
                    glColor3f(0.804f, 0.522f, 0.247f); // 设置颜色为棕色
                }
                glBegin(GL_QUADS);
                glVertex2i(x * miniMapScale, (mapHeight - y) * miniMapScale);
                glVertex2i((x + 1) * miniMapScale, (mapHeight - y) * miniMapScale);
                glVertex2i((x + 1) * miniMapScale, (mapHeight - (y + 1)) * miniMapScale);
                glVertex2i(x * miniMapScale, (mapHeight - (y + 1)) * miniMapScale);
                glEnd();
            }
        }
    }
}

void texture_init()
{
    // 纹理文件的路径，每个元素是一个string
    textureFileNames = {
        "pics/texture1.png",
        "pics/texture2.png",
        "pics/texture3.png",
        "pics/texture4.png",
        "pics/texture5.png",
        "pics/texture6.png",
        "pics/texture7.png",
        "pics/texture8.png"
    };

    glGenTextures(numTextures, textureIDs.data());

    for (int i = 0; i < numTextures; i++) {
        glBindTexture(GL_TEXTURE_2D, textureIDs[i]);

        // 使用OpenCV读取纹理图像
        cv::Mat image = cv::imread(textureFileNames[i], cv::IMREAD_COLOR);
        if (image.empty()) {
            cerr << "Failed to load texture: " << textureFileNames[i] << endl;
            continue;
        }

        // 将图像数据存储在一个数组中
        textureData[i] = image.clone();

        // 将图像数据上传到OpenGL
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image.cols, image.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, image.data);

        // 设置纹理参数
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}