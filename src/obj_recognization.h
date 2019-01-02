#ifndef OBJ_RECOGNIZATION_h
#define OBJ_RECOGNIZATION_h
#include "net.h"


#ifndef DKMINOBJREGISTERIMGNUM
#define DKMINOBJREGISTERIMGNUM  5 // 人脸注册时最少需采集人脸数目
#endif

typedef struct
{
    float threshold; // 相似度阈值，相似度超过该值认为不能认出该物体
    int k;//最近邻k值
}DKSObjectRecognizationParam;

typedef struct
{
    int undefined; //边界框索引
}DKSObjectRegisterParam;
/*

工具函数

*/

//计算向量内积
float distance(ncnn::Mat& fc1, ncnn::Mat& fc2);

//规范化向量
int normalize(ncnn::Mat& fc1);

//knn最近邻实现
int knn(std::vector< std::pair<int, float> >& re, int k);


/*

物体注册识别相关函数

*/

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

// 将计算好的特征存入sqlite中（flag为1），或取消学习（flag为0）
void DKObjectRegisterEnd(int flag, int count);

#endif 
