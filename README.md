# GAME101
The assignments of GAME101 (Introduction to Computer Graphics) whose instructor is Lingqi Yan, if you are interested, please refer to [lecture website](https://sites.cs.ucsb.edu/~lingqi/teaching/games101.html) for more information. this lecture mainly includes four parts: rasterization, geometry, ray tracing and animation. (shown as the following four pictures)

![games101 banner](picture/games101.png)

## Assignment 0: Configure Assignments Environment

### Install Eigen
In the assignemnts of this lecture, we use Eigen for mathematics related calculation, such as vectors, matrices, and so on. so Eigen shoule be installed beforehand. If you want to know more about Eigen, please go to [Eigen website](https://eigen.tuxfamily.org/index.php?title=Main_Page) for more details. installing Eigen is straightforward. following these steps will be fine:
- Download Eigen. Go to [Eigen offical website](https://eigen.tuxfamily.org/index.php?title=Main_Page), select a latest stable version, and download it.
- Install Eigen. Upzip the package, follow the instructions in the INSTALL file which is under the root directory to install Eigen to the system path. then you can refer to Eigen header files in your code just as this:
```C++
#include<eigen3/Eigen/Core>
#include<eigen3/Eigen/Dense>
```

### Install OpenCV
To show the rendering result, We need OpenCV to do some pixel-level processing, you can go to [OpenCV Documents](https://docs.opencv.org/4.x/d0/db2/tutorial_macos_install.html), and follow these steps to install OpenCV, it may take several minutes to compile, be patient! after installing OpenCV successfully, you can use OpenCV by including its header files:
```C++
#include <opencv2/opencv.hpp>
```

### Configure VSCode
On the other hand, using VSCode as our development environment is highly recommended. the configuration corresponding to VSCode is under the .vscode directory in the root directory of every assignment. I only add the configuration for Mac OS System.(sorry about that)

After installing Eigen and OpenCV, make sure adding the include directory in the CMakeList.txt, so that your compiler can find the header file.

```
include_directories(/usr/local/include)
include_directories(/usr/local/include/opencv4)
```

Of course, the packages should be added too.

```
find_package(OpenCV REQUIRED)
find_package(Eigen3 REQUIRED)
```
