## 总体目标
按照V4L2框架，模拟出一个虚拟的摄像头，生成条纹测试图，在应用层能按标准的摄像头操作流程使用这个虚拟摄像头获取到条纹测试图。

### /dev/videox设备节点的创建
v4l2的基本框架
在驱动层注册video_register_device，在应用层形成可操作的设备节点/dev/videox

### 如何编译
1. Makefile中指定你的内核源码路径
2. make

### 驱动和应用程序

驱动仓库：
git@github.com:mcli244/v4l2_dev.git
https://github.com/mcli244/v4l2_dev.git


应用程序仓库：
git@github.com:mcli244/v4l2_dev_app.git
https://github.com/mcli244/v4l2_dev_app.git