//
//  main.c
//  Facetrain
//
//  Created by ljp on 17/4/4.
//  参考了Jeff Shufelt为CMU机器学习课程编写的示例代码
//

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "pgmimage.h"
#include "backprop.h"

#define TARGET_HIGH 0.9
#define TARGET_LOW 0.1

void backprop_face(IMAGELIST *, IMAGELIST *, IMAGELIST *, int, int);
void performance_on_imagelist(BPNN *, IMAGELIST *, int);
int evaluate_performance(BPNN *, double *, int);
void load_target(IMAGE *, BPNN *, int);
void load_input_with_image(IMAGE *, BPNN *);

int main(int argc, const char * argv[]) {
    IMAGELIST *trainlist, *test1list, *test2list;
    int epochs, seed, type;
    
    seed = 112495;      /*** 我的生日作随机种子 ***/
    epochs = 100;
    type = 1;           /*** 选择训练项目，1为有无墨镜，2为人脸朝向，3为双特征，否则做身份识别 ***/
    
    /*** 创建imagelists ***/
    trainlist = imgl_alloc();
    test1list = imgl_alloc();
    test2list = imgl_alloc();
    
    /*** 加载训练集和2个测试集 ***/
    imgl_load_images_from_textfile(trainlist, "/Users/yy/Desktop/trainlist.txt");
    imgl_load_images_from_textfile(test1list, "/Users/yy/Desktop/test1list.txt");
    imgl_load_images_from_textfile(test2list, "/Users/yy/Desktop/test2list.txt");
    
    /*** 训练集为空则不训练 ***/
    if (trainlist->n == 0) {
        epochs = 0;
    }
    
    /*** 初始化神经网络 ***/
    bpnn_initialize(seed);
    
    /*** 输出训练集和2个测试集中的图片数 ***/
    printf("%d images in training set\n", trainlist->n);
    printf("%d images in test1 set\n", test1list->n);
    printf("%d images in test2 set\n", test2list->n);
    
    /*** 进入训练和测试阶段 ***/
    backprop_face(trainlist, test1list, test2list, epochs, type);
    
    imgl_free(trainlist);
    imgl_free(test1list);
    imgl_free(test2list);
    
    return 0;
}

void backprop_face(IMAGELIST *trainlist, IMAGELIST *test1list, IMAGELIST *test2list,
                   int epochs, int type) {
    IMAGE *iimg;
    BPNN *net;
    int train_n, epoch, i, imgsize;
    double out_err, hid_err, sumerr;
    
    train_n = trainlist->n;
    
    if (train_n > 0) {
        printf("Creating new network\n");
        iimg = trainlist->list[0];
        imgsize = ROWS(iimg) * COLS(iimg);
        if (type == 1)      /*** 有无墨镜，2个输出 ***/
            net = bpnn_create(imgsize, 3, 2);
        else if (type == 2) /*** 人脸朝向，4个输出 ***/
            net = bpnn_create(imgsize, 4, 4);
        else if (type == 3) /*** 双特征训练，8个输出 ***/
            net = bpnn_create(imgsize, 5, 8);
        else                /*** 身份识别，20个输出 ***/
            net = bpnn_create(imgsize, 20, 20);
    } else {
        printf("No image to train on\n");
        return;
    }
    
    if (epochs > 0) {
        printf("Training underway (going to %d epochs)\n", epochs);
        fflush(stdout);
    }
    
    /*** 输出训练前初始的正确率和误差 ***/
    printf("0 0.0 ");
    performance_on_imagelist(net, trainlist, type);
    performance_on_imagelist(net, test1list, type);
    performance_on_imagelist(net, test2list, type);
    printf("\n");  fflush(stdout);
    
    /*************** 开始训练 ***************/
    for (epoch = 1; epoch <= epochs; epoch++) {
        
        printf("%d ", epoch);  fflush(stdout);
        
        sumerr = 0.0;
        for (i = 0; i < train_n; i++) {
            
            /** 将图片载入输入单元 **/
            load_input_with_image(trainlist->list[i], net);
            
            /** 为图片设置目标向量 **/
            load_target(trainlist->list[i], net, type);
            
            /** 运行反向传播算法，学习速率0.3，冲量0.3 **/
            bpnn_train(net, 0.3, 0.3, &out_err, &hid_err);
            
            sumerr += (out_err + hid_err);
        }
        printf("%g ", sumerr);
        
        /*** 评估结果并输出 ***/
        performance_on_imagelist(net, trainlist, type);
        performance_on_imagelist(net, test1list, type);
        performance_on_imagelist(net, test2list, type);
        printf("\n");  fflush(stdout);
    }
    printf("\n"); fflush(stdout);
    
    bpnn_free(net);
}

void performance_on_imagelist(BPNN *net, IMAGELIST *il, int type) {
    double err, val;
    int i, n, correct;
    
    err = 0.0;
    correct = 0;
    n = il->n;
    if (n > 0) {
        for (i = 0; i < n; i++) {
            
            /*** 将图片载入输入单元 **/
            load_input_with_image(il->list[i], net);
            
            /*** 用这个输入正向通过网络 **/
            bpnn_feedforward(net);
            
            /*** 为图片设置目标向量 **/
            load_target(il->list[i], net, type);
            
            /*** 对结果测试并计算误差 ***/
            if (evaluate_performance(net, &val, type))
                correct++;
            err += val;
        }
        
        err = err / (double) n;
        
        printf("%g %g ", ((double) correct / (double) n) * 100.0, err);
    } else {
        printf("0.0 0.0 ");
    }
}

int evaluate_performance(BPNN *net, double *err, int type) {
    if (type == 1) { /*** 有无墨镜 ***/
        double delta1, delta2;
        int max;
        
        delta1 = net->target[1] - net->output_units[1];
        delta2 = net->target[2] - net->output_units[2];
        
        *err = (0.5 * ((delta1 * delta1) + (delta2 * delta2)));
        
        if (net->output_units[1] > net->output_units[2])
            max = 1;
        else
            max = 2;
        
        if (net->target[max] == TARGET_HIGH)
            return 1;
        else
            return 0;
    } else if (type == 2) { /*** 人脸朝向 ***/
        double delta1, delta2, delta3, delta4;
        int max = 1;
        double maxo = net->output_units[1];
        
        delta1 = net->target[1] - net->output_units[1];
        delta2 = net->target[2] - net->output_units[2];
        delta3 = net->target[3] - net->output_units[3];
        delta4 = net->target[4] - net->output_units[4];
        
        *err = (0.5 * ((delta1 * delta1) + (delta2 * delta2) + (delta3 * delta3) + (delta4 * delta4)));
        
        for (int i = 2; i <= 4; i++)
            if (net->output_units[i] > maxo) {
                maxo = net->output_units[i];
                max = i;
            }
        
        if (net->target[max] == TARGET_HIGH)
            return 1;
        else
            return 0;
    } else if (type == 3) { /*** 双特征 ***/
        double delta1, delta2, delta3, delta4, delta5, delta6, delta7, delta8;
        int max = 1;
        double maxo = net->output_units[1];
        
        delta1 = net->target[1] - net->output_units[1];
        delta2 = net->target[2] - net->output_units[2];
        delta3 = net->target[3] - net->output_units[3];
        delta4 = net->target[4] - net->output_units[4];
        delta5 = net->target[5] - net->output_units[5];
        delta6 = net->target[6] - net->output_units[6];
        delta7 = net->target[7] - net->output_units[7];
        delta8 = net->target[8] - net->output_units[8];
        
        *err = (0.5 * ((delta1 * delta1) + (delta2 * delta2) + (delta3 * delta3) + (delta4 * delta4)
                   + (delta5 * delta5) + (delta6 * delta6) + (delta7 * delta7) + (delta8 * delta8)));
        
        for (int i = 2; i <= 8; i++)
            if (net->output_units[i] > maxo) {
                maxo = net->output_units[i];
                max = i;
            }
        
        if (net->target[max] == TARGET_HIGH)
            return 1;
        else
            return 0;
    } else {    /*** 身份识别 ***/
        double delta[20];
        int max = 1;
        double maxo = net->output_units[1];
        
        for (int i = 1; i <= 20; i++)
            delta[i - 1] = net->target[i] - net->output_units[i];
        
        double sum = 0;
        for (int j = 0; j < 20; j++)
            sum += delta[j] * delta[j];
        
        *err = (0.5 * sum);
        
        for (int k = 2; k <= 20; k++)
            if (net->output_units[k] > maxo) {
                maxo = net->output_units[k];
                max = k;
            }
        
        if (net->target[max] == TARGET_HIGH)
            return 1;
        else
            return 0;
    }
}

void load_target(img, net, type)
IMAGE *img;
BPNN *net;
int type;
{
    int scale;
    char userid[40], head[40], expression[40], eyes[40], photo[40];
    
    userid[0] = head[0] = expression[0] = eyes[0] = photo[0] = '\0';
    
    /*** 由文件名获得图片的人脸特征 ***/
    sscanf(NAME(img), "%[^_]_%[^_]_%[^_]_%[^_]_%d.%[^_]",
           userid, head, expression, eyes, &scale, photo);
   
    if (type == 1) { /*** 有无墨镜 ***/
        if (!strcmp(eyes, "open")) {
            net->target[1] = TARGET_HIGH;
            net->target[2] = TARGET_LOW;
        } else {
            net->target[1] = TARGET_LOW;
            net->target[2] = TARGET_HIGH;
    }
    } else if (type == 2) { /*** 人脸朝向 ***/
        net->target[1] = TARGET_LOW;
        net->target[2] = TARGET_LOW;
        net->target[3] = TARGET_LOW;
        net->target[4] = TARGET_LOW;
        if (!strcmp(head, "left")) {
            net->target[1] = TARGET_HIGH;
        } else if (!strcmp(head, "straight")) {
            net->target[2] = TARGET_HIGH;
        } else if (!strcmp(head, "right")) {
            net->target[3] = TARGET_HIGH;
        } else {
            net->target[4] = TARGET_HIGH;
        }
    } else if (type == 3) { /*** 双特征 ***/
        net->target[1] = TARGET_LOW;
        net->target[2] = TARGET_LOW;
        net->target[3] = TARGET_LOW;
        net->target[4] = TARGET_LOW;
        net->target[5] = TARGET_LOW;
        net->target[6] = TARGET_LOW;
        net->target[7] = TARGET_LOW;
        net->target[8] = TARGET_LOW;
        
        if (!strcmp(eyes, "open")) {
            if (!strcmp(head, "left")) {
                net->target[1] = TARGET_HIGH;
            } else if (!strcmp(head, "straight")) {
                net->target[2] = TARGET_HIGH;
            } else if (!strcmp(head, "right")) {
                net->target[3] = TARGET_HIGH;
            } else {
                net->target[4] = TARGET_HIGH;
            }
        } else {
            if (!strcmp(head, "left")) {
                net->target[5] = TARGET_HIGH;
            } else if (!strcmp(head, "straight")) {
                net->target[6] = TARGET_HIGH;
            } else if (!strcmp(head, "right")) {
                net->target[7] = TARGET_HIGH;
            } else {
                net->target[8] = TARGET_HIGH;
            }
        }
    } else {    /*** 身份识别 ***/
        for (int i = 1; i <= 20; i++)
            net->target[i] = TARGET_LOW;
        char* id[20] = {"an2i", "at33", "boland", "bpm", "ch4f", "cheyer", "choon", "danieln", "glickman", "karyadi",
            "kawamura", "kk49", "megak", "mitchell", "night", "phoebe", "saavik", "steffi", "sz24", "tammo"};
        for (int j = 1; j <= 20; j++)
            if (!strcmp(userid, id[j - 1])) {
                net->target[j] = TARGET_HIGH;
                break;
            }
    }
}

void load_input_with_image(img, net)
IMAGE *img;
BPNN *net;
{
    double *units;
    int nr, nc, imgsize, i, j, k;
    
    nr = ROWS(img);
    nc = COLS(img);
    imgsize = nr * nc;;
    if (imgsize != net->input_n) {
        printf("LOAD_INPUT_WITH_IMAGE: This image has %d pixels,\n", imgsize);
        printf("   but your net has %d input units.  I give up.\n", net->input_n);
        exit (-1);
    }
    
    units = net->input_units;
    k = 1;
    for (i = 0; i < nr; i++) {
        for (j = 0; j < nc; j++) {
            units[k] = ((double) img_getpixel(img, i, j)) / 255.0;
            k++;
        }
    }
}

