#include<stdio.h>
#include "obj_recognization.h"

int main(int argc, char* argv[])
{
    int  id, flag, count;
    
    DKSObjectRegisterParam rgp = {0};
    DKSObjectRecognizationParam rcp;
    rcp.threshold = 1;
    rcp.k = 3;
    
    //注册
    if(*(argv[1]) == '0')
    {
        DKObjectRegisterInit();
        int count = *(argv[2]) - 48;
        for(int i = 0; i < count; i++)
        {
            char*   rgbfilename = argv[2+i+1];
            DKObjectRegisterProcess(rgbfilename, 100, 100, rgp);//示例中没有用到iWidth,iHeight两个参数。
            DKObjectRegisterEnd(count - (i+1) ? 1 : 0, i+1);
        }
    }

    //识别
    if(*(argv[1]) == '1')
    {
        char*   rgbfilename = argv[2];
        DKObjectRecognizationInit();
        id = DKObjectRecognizationProcess(rgbfilename, 100, 100, rcp);//示例中没有用到100,100两个参数。
        DKObjectRecognizationEnd();
        printf("ID:%d\n", id);
    }
    return 0;
}
