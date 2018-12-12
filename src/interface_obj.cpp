#include <stdio.h>
#include <sqlite3.h>
#include "net.h"
#include "dlib/image_processing/generic_image.h"
#include "dlib/image_io.h"
#include "dlib/image_processing/frontal_face_detector.h"
#include "dlib/image_processing.h"
#include "obj_recognization.h"

#ifdef JPG_DEMO
//测试使用
static int id = 0;
#endif

static  sqlite3* objectfeatures;
static  ncnn::Mat fc;

float distance(ncnn::Mat& fc1, float* fc2)
{
    float sum = 0;
    for(int i=0; i<fc1.c*fc1.h*fc1.w; i++)
    {
        sum += pow(((*(fc1.row(0)+i)) - *(fc2 + i)), 2);
    }
    return sum;
}

//工具函数
float dot(float* fc1, float* fc2)
{
    float sum = 0;
    for(int i=0; i<512; i++)
    {
        sum += (*(fc1+i)) * (*(fc2+i));
    }
    return sum;
}

int normalize(ncnn::Mat& fc1)
{
    float sq = sqrt(dot((float*)fc1.data, (float*)fc1.data));
    for(int i=0; i<fc1.w*fc1.h*fc1.c; i++)
    {
        *(fc1.row(0)+i) = (*(fc1.row(0)+i))/sq;
    }
    return 0;
}

int knn(std::vector< std::pair<int, float> >& re, int k, float threshold)
{
    std::pair<int, float> temp;
    for(int i=0; i<re.size(); i++)
    {
	    for(int j=i+1; j<re.size(); j++)
	    {
            if(re[j].second < re[i].second)
            {
            	temp = re[i];
                re[i] = re[j];
                re[j] = temp;
            }
        }
    }
   
    if(re[0].second < threshold)
    { 
        std::vector< std::pair<int, int> > vote;
        vote.push_back(std::make_pair(re[0].first, 1));
        for(int i=1; i<k; i++)
        {
            int j=0;
	        for(; j<vote.size(); j++)
            {
                if(vote[j].first == re[i].first)
                {
                    vote[j].second += 1;
                    break;
                }
            }
            if(j == vote.size())
            {
                vote.push_back(std::make_pair(re[i].first, 1));
            } 
        }
    
        float max = 0;
        // printf("(ID:%d)__ballot:%d\n", vote[0].first, vote[0].second);
        for(int j=1; j<vote.size(); j++)
        {
            if(vote[j].second > vote[0].second)
            {
                max = j;
            }
            // printf("(ID:%d)__ballot:%d\n", vote[j].first, vote[j].second);
        }   
        printf("results:\n(ID:%d)__ballot:%d\n", vote[max].first, vote[max].second);
        return vote[max].first;
    }
    else
    {
   		return -1; 
    }
}

void DKObjectRegisterInit()
{
    int rc = sqlite3_open("object_feature.db", &objectfeatures);
    
    if(rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(objectfeatures));
        sqlite3_close(objectfeatures);
        exit(1);
    }
    else
    {
        fprintf(stderr, "Opened database successfully\n");
    }
}

char * DKObjectRegisterProcess(char* rgbfilename, int iWidth, int iHeight, DKSObjectRegisterParam param)
{

#ifdef JPG_DEMO

    //使用图片路径进行测试（仅作测试时用,rgbfilename是jpg,png文件路径）
    dlib::array2d<dlib::rgb_pixel> img;
    load_image(img, rgbfilename);

#else

    FILE *stream = NULL; 
    stream = fopen(rgbfilename, "rb");   

    if(NULL == stream)
    {
        fprintf(stderr, "error:read imgdata!");
        exit(1);
    }

    unsigned char* rgbData = new unsigned char[iHeight*iWidth*3];
    fread(rgbData, 1, iHeight*iWidth*3, stream);
    fclose(stream);   

    int x_left = iWidth / 4;
    int y_top = iHeight / 4;
    int col = iWidth/2;
    int row = iHeight/2;
    int channelstep = iWidth * iHeight;
    ncnn::Mat crop_image;
    crop_image.create(col, row, 3, 1);

    #pragma omp parallel for     
    for(int i = y_top; i < y_top + row; i++)
    {
        for(int j=x_left; j < x_left + col; j++)
        {
            *((unsigned char*)(crop_image.data)+3*(i-y_top)*col+3*(j-x_left))   = *(rgbData + i * iWidth + j);
            *((unsigned char*)(crop_image.data)+3*(i-y_top)*col+3*(j-x_left)+1) = *(rgbData + channelstep + i * iWidth + j);
            *((unsigned char*)(crop_image.data)+3*(i-y_top)*col+3*(j-x_left)+2) = *(rgbData + channelstep * 2 + i * iWidth + j);
        }
    }
#endif

#ifdef JPG_DEMO
    ncnn::Mat in = ncnn::Mat::from_pixels_resize((unsigned char*)&img[0][0], ncnn::Mat::PIXEL_RGB, img.nc(), img.nr(), 227, 227);
#else
    ncnn::Mat in = ncnn::Mat::from_pixels_resize((unsigned char*)crop_image.data, ncnn::Mat::PIXEL_RGB, col, row, 227, 227);
#endif

    const float mean_vals[3] = {104.f, 117.f, 123.f};
    in.substract_mean_normalize(mean_vals, 0);

    ncnn::Net objectmatnet;
    objectmatnet.load_param("objectmatnet.param");
    objectmatnet.load_model("objectmatnet.bin");
    ncnn::Extractor ex = objectmatnet.create_extractor();
    ex.set_light_mode(true);
    ex.input("data", in);
    ex.extract("fc_embedding", fc);
    normalize(fc);
}

void DKObjectRegisterEnd(int flag, int count)
{
    //查表插入
    char* zErrMsg;
    const char* sql;
    int registernum = 0;

    sql = "CREATE TABLE IF NOT EXISTS FEATURES("  \
         "NUMFEA         INT     NOT NULL," \
         "FEAOFOBJ      BLOB    NOT NULL );";
    int  rc;
    rc = sqlite3_exec(objectfeatures, sql, NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
       fprintf(stderr, "SQL error: %s\n", zErrMsg);
       sqlite3_free(zErrMsg);
    }else{
       fprintf(stdout, "Operate on Table FEATURES\n");
    }

    sqlite3_stmt* stat;
    
    if(count > 1 && count <= 10)
    {
        sqlite3_blob* blob = NULL;
        
        //获取行数
        sqlite3_stmt* stat;
        int rc = sqlite3_prepare_v2(objectfeatures, "SELECT max(rowid),NUMFEA FROM FEATURES", -1, &stat, NULL);
        if(rc!=SQLITE_OK) {
            fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(objectfeatures));
            exit(1);
        }      

        int rid, numfea;
        if( sqlite3_step(stat) == SQLITE_ROW ){
            rid = sqlite3_column_int(stat, 0);
            numfea = sqlite3_column_int(stat, 1);
        } 
        sqlite3_finalize(stat);

        //
        rc = sqlite3_blob_open(objectfeatures, "main", "FEATURES", "FEAOFOBJ", rid, 1, &blob);
        if (rc != SQLITE_OK)
        {
            printf("Failed to open BLOB: %s \n", sqlite3_errmsg(objectfeatures));             
            return ;
        }

        sqlite3_blob_write(blob, fc, 2048, numfea*2048);
        if (rc != SQLITE_OK)
        {
            fprintf(stderr, "failed to write feature_BLOB!\n");
            return ;
        }
               
        sqlite3_blob_close(blob);
        rc = sqlite3_prepare_v2(objectfeatures, " UPDATE FEATURES SET (NUMFEA) = (?) WHERE (rowid) = (?)", -1, &stat, NULL);
        sqlite3_bind_int(stat, 1, numfea+1);
        sqlite3_bind_int(stat, 2, rid);
        rc = sqlite3_step(stat);
        if( rc != SQLITE_DONE ){
        printf("%s",sqlite3_errmsg(objectfeatures));
            return ;
        }
        sqlite3_finalize(stat);
        
        registernum = numfea + 1;  
        fprintf(stderr, "Records (rowid)%d insert %dth feature successfully!\n", rid, numfea+1);
    } 
    else
    {   
        float fe[5120] = {0.f};
        for(int j=0; j<512; j++)
        { 
            fe[j] = *((float*)fc.data + j);
        }

        sqlite3_prepare_v2(objectfeatures,"INSERT INTO FEATURES (NUMFEA,FEAOFOBJ) VALUES(?,?);",-1,&stat, NULL);
        sqlite3_bind_int(stat,1,1);
        sqlite3_bind_blob(stat, 2, fe, 20480, SQLITE_STATIC);
        rc = sqlite3_step(stat);
        if( rc != SQLITE_DONE ){
            printf("%s",sqlite3_errmsg(objectfeatures));
            exit(1);
        }
        else{
            fprintf(stderr, "Records created successfully!\n");
        }
        sqlite3_finalize(stat);
    }

    if(flag == 0) 
    {
	    if(registernum >= DKMINOBJREGISTERIMGNUM)
            sqlite3_close(objectfeatures); 
        else
            fprintf(stderr, "One person needs to register at least %d facial images\n", DKMINOBJREGISTERIMGNUM);
    }

}

void DKObjectRecognizationInit()
{
    //打开数据库
    int rc = sqlite3_open("object_feature.db", &objectfeatures);    
    if(rc)
    {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(objectfeatures));
        exit(0);
    }
    else
    {
        fprintf(stdout, "Opened database successfully\n");
    }

}

int DKObjectRecognizationProcess(char* rgbfilename, int iWidth, int iHeight, DKSObjectRecognizationParam param)
{
    clock_t start = clock();
#ifdef JPG_DEMO
    //使用图片路径进行测试（仅作测试时用,rgbfilename是jpg,png文件路径）
    dlib::array2d<dlib::rgb_pixel> img;
    load_image(img, rgbfilename);
#else    
    FILE *stream = NULL; 
    stream = fopen(rgbfilename, "rb");   

    if(NULL == stream)
    {
        fprintf(stderr, "error:read imgdata!");
        exit(1);
    }

    unsigned char* rgbData = new unsigned char[iHeight*iWidth*3];
    fread(rgbData, 1, iHeight*iWidth*3, stream);
    fclose(stream); 

    int x_left = iWidth / 4;
    int y_top = iHeight / 4;
    int col = iWidth/2;
    int row = iHeight/2;
    int channelstep = iWidth * iHeight;
    ncnn::Mat crop_image;
    crop_image.create(col, row, 3, 1);

    #pragma omp parallel for     
    for(int i = y_top; i < y_top + row; i++)
    {
        for(int j=x_left; j < x_left + col; j++)
        {
            *((unsigned char*)(crop_image.data)+3*(i-y_top)*col+3*(j-x_left))   = *(rgbData + i * iWidth + j);
            *((unsigned char*)(crop_image.data)+3*(i-y_top)*col+3*(j-x_left)+1) = *(rgbData + channelstep + i * iWidth + j);
            *((unsigned char*)(crop_image.data)+3*(i-y_top)*col+3*(j-x_left)+2) = *(rgbData + channelstep * 2 + i * iWidth + j);
        }
    }
#endif

#ifdef JPG_DEMO
    ncnn::Mat in = ncnn::Mat::from_pixels_resize((unsigned char*)&img[0][0], ncnn::Mat::PIXEL_RGB, img.nc(), img.nr(), 227, 227);
#else  
    ncnn::Mat in = ncnn::Mat::from_pixels_resize((unsigned char*)(crop_image.data), ncnn::Mat::PIXEL_RGB, col, row, 227, 227);
#endif

    const float mean_vals[3] = {104.f, 117.f, 123.f};
    in.substract_mean_normalize(mean_vals, 0);
    
    ncnn::Net objectmatnet;
    objectmatnet.load_param("objectmatnet.param");
    objectmatnet.load_model("objectmatnet.bin");
    ncnn::Extractor ex = objectmatnet.create_extractor();
    ex.set_light_mode(false);
    ex.input("data", in);
    ex.extract("fc_embedding", fc);
    normalize(fc);
    
    clock_t finsh = clock();
    fprintf(stderr, "get_feature cost %d ms\n", (finsh-start)/1000);
   
    //获取行数
    sqlite3_stmt* stat;
    int rc = sqlite3_prepare_v2(objectfeatures, "SELECT max(rowid),NUMFEA FROM FEATURES", -1, &stat, NULL);
    if(rc!=SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(objectfeatures));
        exit(1);
    }      

    int rows, numfea;
    if( sqlite3_step(stat) == SQLITE_ROW ){
        rows = sqlite3_column_int(stat, 0);
        numfea = sqlite3_column_int(stat, 1);
    } 
    sqlite3_finalize(stat);

    //读取数据库中脸部特征数据
    start = clock();
    int i=0;
    float dis;
    std::vector< std::pair<int, float> > results;
    for(; i< rows; i++)
    {
        sqlite3_blob* blob = NULL;
        rc = sqlite3_blob_open(objectfeatures,  "main", "FEATURES", "FEAOFOBJ", i+1, 0, &blob);
        if (rc != SQLITE_OK)
        {
            printf("Failed to open BLOB: %s \n", sqlite3_errmsg(objectfeatures));
            return -1;
        }
//        int blob_length = sqlite3_blob_bytes(blob);
//        printf("blob_length_%d\n", blob_length);
    
        float buf[512] = {0.f};
        int offset = 0;
        while (offset < numfea * 2048)
        {
            int size = 20480 - offset;
            if (size > 2048) size = 2048;
            rc = sqlite3_blob_read(blob, buf, size, offset);
            if (rc != SQLITE_OK)
            {
                printf("failed to read BLOB!\n");
                break;
            }
        
            offset += size;
            dis = distance(fc, buf);
            fprintf(stderr, "%d_distance:%f\n",i, dis);
            results.push_back(std::make_pair(i, dis));
        }
        sqlite3_blob_close(blob);
    }
    
    int ID;
    ID = knn(results, 3, param.threshold);
    
    finsh = clock();
    fprintf(stderr, "knn cost %d ms\n", (finsh - start)/1000);
    
    return ID;

}

void DKObjectRecognizationEnd()
{
    sqlite3_close(objectfeatures);
}

