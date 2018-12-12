# Object Matching

编译静态库：
```
cd src
make 
生成libobj.a
```

demo测试：
```
cd src
make MODE="-DJPG_DEMO"
生成demo_obj。

src目录下使用以下命令运行：
1.特征存储 ：./demo_obj 0 ${num_features} ${image1_path} ${image2_path} ...
2.识别：./demo_obj 1 ${image_path}
```

库函数说明：
```
// 说明：从已注册的物体中识别对应的物体
// 初始化，连接sqlite，获取物体库中各个物体的特征
void DKObjectRecognizationInit();

// 计算图像中心区域（1/4原图像大小）特征，并与sqlite中特征进行对比，如果相似度大于某一阈值（在识别参数中定义），则输出null，否则输出识别出的物体的index。
int DKObjectRecognizationProcess(char * rgbfilename, int iWidth, int iHeight, DKSObjectRecognizationParam param);

// 释放物体识别资源
void DKObjectRecognizationEnd();


// 说明：录入物体，在物体学习阶段，获取至少DKMINOBJREGISTERIMGNUM张同一物体图片
// 初始化，连接sqlite，准备写入物体特征和语音，初始化学习图片次数为0
void DKObjectRegisterInit();

// 根据图像中间位置（1/4原图像大小）子图像计算特征
char * DKObjectRegisterProcess(char * yuvfilename, int iWidth, int iHeight, DKSObjectRegisterParam param);

// 将计算好的特征存入sqlite中（flag为1），或取消物体特征学习（flag为0），count表示要存储特征的序数（是否是新物体第一个特征）。
void DKObjectRegisterEnd(int flag, int count);
```
变量及结构体：

```
#ifndef DKMINOBJREGISTERIMGNUM
#define DKMINOBJREGISTERIMGNUM  5 // 人脸注册时最少需采集人脸数目

typedef struct
{

	    float threshold; // 相似度阈值，相似度超过该值认为不能认出该物体
}DKSObjectRecognizationParam;

typedef struct
{

	    int undefined; //边界框索引
}DKSObjectRegisterParam;

```

