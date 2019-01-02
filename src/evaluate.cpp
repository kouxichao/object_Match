#include<stdio.h>
#include<unistd.h>
#include<iostream>
#include "obj_recognization.h"
#include<cstring>
int main(int argc, char* argv[])
{
    const char *root_dir = argv[2];
    
    char bbox_path[50];
    strcpy(bbox_path, root_dir);
    strcat(bbox_path, "image.txt"); 
    FILE *fp = fopen(bbox_path, "r+");
    if(NULL == fp)
    {
	    fprintf(stderr, "fopen bbox.xy error\n");
    }

    DKSObjectRegisterParam rgp = {0};
    DKSObjectRecognizationParam rcp;
    rcp.threshold = 1;
    rcp.k = 3;
    char pre_name[50] = {};

    //注册
    if(*(argv[1]) == '0')
    {
        FILE* fp_idx_name = fopen("idx_name", "w+");
        if(NULL == fp_idx_name)
        {
	        fprintf(stderr, "fopen idx_name error\n");
        }
//	    std::vector<char*> idx_name;
        char name[50];
        char idx[5];
        int right,left,bottom,top;
        DKObjectRegisterInit();
        while(1)
        {

            if((fscanf(fp, "%s %s", name, idx)) == EOF)
	        {
	            fprintf(stderr, "fscanf end(error)\n");
                break;
            }

            fprintf(stderr, "name : %s\n", name);
	        if(strstr(name, "test") == NULL)
            {
                std::string rgbfilename = std::string(root_dir) + std::string(name) + \
                 '/' + "support/" + name + "_" + idx;
                printf("PATH: %s\n", rgbfilename.data()); 
                if(access((rgbfilename + std::string(".jpg")).data(), 0) == 0)
                    rgbfilename = rgbfilename + std::string(".jpg");
                else
                    rgbfilename = rgbfilename + std::string(".png");

                DKObjectRegisterProcess((char *)rgbfilename.data(), 100, 100, rgp);//示例中没有用到iWidth,iHeight两个参数。
                if(strcmp(pre_name, name) == 0)
                    DKObjectRegisterEnd(1, 2); //第二个参数大于1小于10，表示直接插入最后的记录中。
                else
                {
//                    idx_name.push_back(name);
                    fprintf(fp_idx_name, "%s\n", name);
                    fprintf(stderr, "%s\n", name);
                    DKObjectRegisterEnd(1, 1); //第二个参数等于1,表示增加新的记录。
                }
                strcpy(pre_name,  name);
            }
        }
        fclose(fp_idx_name);

    }

    //识别
    if(*(argv[1]) == '1')
    {
        FILE* fp_idx_name = fopen("idx_name", "r+");
        if(NULL == fp_idx_name)
        {
	        fprintf(stderr, "fopen idx_name error\n");
        }
        char idx_name[20][30];
        int index = 0;
        while(fscanf(fp_idx_name, "%s", idx_name[index]) != EOF)
        {
//            fprintf(stderr, "temp_name : %s\n", idx_name[index]);
            index++;
        }
//        fprintf(stderr, "temp_name : %s\n", idx_name[0]);

        DKObjectRecognizationInit();
        char name[30];
        char idx[5];
        float num = 0, acc = 0;
        while(1)
        {

            if((fscanf(fp, "%s %s", name, idx)) == EOF)
    	    {
//		        fprintf(stderr, "fscanf end(error)\n");
                break;
            }

//            fprintf(stderr, "name : %s\n", name);
	        if(strstr(name, "test") != NULL)// && strcmp(name, "xiena_test") == 0)
            {
//                fprintf(stderr, "ori_pos:%d_%d_%d_%d\n", right, left, top, bottom);
//                 printf("name: %s\n", name); 
                std::size_t found = std::string(name).find_last_of("_");
                const std::string rea_name = std::string(name).substr(0, found);  
                std::string rgbfilename = std::string(root_dir) + rea_name + std::string("/") + std::string("test/") + std::string(name) + std::string("_") + std::string(idx);

                if(access((rgbfilename + std::string(".jpg")).data(), 0) == 0)
                    rgbfilename = rgbfilename + std::string(".jpg");
                else
                    rgbfilename = rgbfilename + std::string(".png");

//                printf("PATH: %s\n", rgbfilename.data()); 
//                 printf("rename: %s\n", rea_name.data()); 
                int id;
        	    id = DKObjectRecognizationProcess((char*)rgbfilename.data(), 100, 100, rcp);//示例中没有用到100,100两个参数。
        	    printf("image:%s \t gt_name:%s \t pre_name(ID):%s_(%d)\n", \
                (std::string(name) + "_" + idx).data(), rea_name.data(), idx_name[id], id);
                if((strcmp(rea_name.data(), idx_name[id])) == 0)
                    acc++;
                num++;
            }
	}

       	DKObjectRecognizationEnd();
        fclose(fp_idx_name);
        printf("num:%f \t acc:%f \n", num, acc);

        float accuracy = acc / num;
        fprintf(stderr, "accuracy: %.2f%%\n", accuracy * 100);
    }

    fclose(fp);
    return 0;
}
