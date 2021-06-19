#include <string.h>
#include <math.h>
#include <time.h>
#include "stdio.h"
#include "DMA.h"
#include "parameters/DNN_parameters.h"

// set this variable to "true" to used golden as input for each layer to calculate each layer latency.
const bool use_golden_layer_input = true;

int conv1_result[6][14][14];
int conv1_quantized_result[6][14][14];
int conv2_result[16][5][5];
int conv2_quantized_result[16][5][5];
int conv2_quantized_result_unfolded[400];
int fc1_result[120];
int fc1_quantized_result[120];
int fc2_result[84];
int fc2_quantized_result[84];
int fc3_result[10];
int fc3_quantized_result[10];

void do_conv1(){
    bool pass = true;
    int M = 6;
    int start = 0;
    int end = M;
    //conv1_start_time = sc_time_stamp();
    for (int m = start; m != end; m++) {
        for (int p_mp = 0; p_mp != 14; p_mp++) {
            for(int q_mp = 0; q_mp != 14 ; q_mp ++) {
                for(int i = 0 ; i < 2 ; i++){
                    for (int j = 0 ; j < 2 ; j++){
                        int p = p_mp * 2 + i;
                        int q = q_mp * 2 + j;
                        for (int c = 0; c != 3; c++) {
                            for (int r = 0; r != 5; r++) {
                                for(int s = 0; s != 5 ; s++) {
                                    //printf("Send %d %d %d\n", c, r, s);
                                    write_data_to_ACC(CONV1_BASE_ADDR + SOURCE_ADDR, 4, input_quantized_1[c][p + r][q + s]);
                                    write_data_to_ACC(CONV1_BASE_ADDR + WEIGHT_ADDR, 4, conv1_weights[m][c][r][s]);

                                }
                            }
                        }

                    }
                }
                conv1_result[m][p_mp][q_mp] = read_data_from_ACC(CONV1_BASE_ADDR + RESULT_ADDR, 4);
               // cout << "Get data: " << result << endl;
                if(conv1_result[m][p_mp][q_mp] != conv1_max_pool_1[m][p_mp][q_mp]){
                    printf("[ERROR] %d %d: %d\n", p_mp, q_mp, conv1_result[m][p_mp][q_mp]);
                    printf("    Correct should be: %d\n", conv1_max_pool_1[m][p_mp][q_mp]);
             
                    pass = false;
                }
                conv1_quantized_result[m][p_mp][q_mp] = conv1_result[m][p_mp][q_mp] / conv1_output_scale;

            }
        }
    }

    //cout << "   Conv1 activation simulated time: " << sc_time_stamp() - conv1_start_time << endl;

    if(pass == true){
        printf("=====> Conv1 activation PASS\n");
    }
    else{
        printf("=====> Conv1 activation FAILED\n");
    }
}

void do_conv2(){
    bool pass = true;
    int M = 16;
    int start = 0;
    int end = M;
    int cnt = 0;
    //conv1_start_time = sc_time_stamp();
    for (int m = start; m != end; m++) {
        for (int p_mp = 0; p_mp != 5; p_mp++) {
            for(int q_mp = 0; q_mp != 5 ; q_mp ++) {
                for(int i = 0 ; i < 2 ; i++){
                    for (int j = 0 ; j < 2 ; j++){
                        int p = p_mp * 2 + i;
                        int q = q_mp * 2 + j;
                        for (int c = 0; c != 6; c++) {
                            for (int r = 0; r != 5; r++) {
                                for(int s = 0; s != 5 ; s++) {
                                    //printf("Send %d %d %d\n", c, r, s);
                                    if(use_golden_layer_input == true)
                                        write_data_to_ACC(CONV2_BASE_ADDR + SOURCE_ADDR, 4, conv1_quantized_result_1[c][p + r][q + s]);
                                    else
                                        write_data_to_ACC(CONV2_BASE_ADDR + SOURCE_ADDR, 4, conv1_quantized_result[c][p + r][q + s]);
                                    write_data_to_ACC(CONV2_BASE_ADDR + WEIGHT_ADDR, 4, conv2_weights[m][c][r][s]);

                                }
                            }
                        }

                    }
                }
                conv2_result[m][p_mp][q_mp] = read_data_from_ACC(CONV2_BASE_ADDR + RESULT_ADDR, 4);
               // cout << "Get data: " << result << endl;
                if(conv2_result[m][p_mp][q_mp] != conv2_max_pool_1[m][p_mp][q_mp]){
                
                    printf("[ERROR] %d %d: %d\n", p_mp, q_mp, conv1_result[m][p_mp][q_mp]);
                    printf("    Correct should be: %d\n", conv1_max_pool_1[m][p_mp][q_mp]);
               
                    pass = false;
                }
                conv2_quantized_result_unfolded[m*5*5 + p_mp*5 + q_mp] = conv2_result[m][p_mp][q_mp] / conv2_output_scale;

            }
        }
    }

    if(pass == true){
        printf("=====> Conv2 activation PASS\n");
    }
    else{
        printf("=====> Conv2 activation FAILED\n");
    }
}

void do_fc1(){
    bool pass = true;
    int M = 120;
    int start = 0;
    int end = M;

    for (int m = start; m != end; m++) {
        for (int h = 0; h != 400; h++) {
            if(use_golden_layer_input == true)
                write_data_to_ACC(FC1_BASE_ADDR + SOURCE_ADDR, 4, fc1_input_1[h]);
            else{
                write_data_to_ACC(FC1_BASE_ADDR + SOURCE_ADDR, 4, conv2_quantized_result_unfolded[h]);
            }
            write_data_to_ACC(FC1_BASE_ADDR + WEIGHT_ADDR, 4, fc1_weights[m][h]);
        }

        fc1_result[m] = read_data_from_ACC(FC1_BASE_ADDR + RESULT_ADDR, 4);
        if(fc1_result[m] != fc1_activation_1[m]){
            printf("[ERROR] %d: %d\n", m, fc1_result[m]);
            printf("    Correct should be: %d\n", fc1_activation_1[m]);
            pass = false;
        }
        fc1_quantized_result[m] = fc1_result[m] / fc1_output_scale;
    }
    if(pass == true){
        printf("=====> Core[%d] FC1 activation PASS\n");
    }
    else{
        printf("=====> Core[%d]  FC1 activation FAILED\n");
    }
}

void do_fc2(){
    bool pass = true;
    int M = 84;
    int start = 0;
    int end = M;

    for (int m = start; m != end; m++) {
        for (int h = 0; h != 120; h++) {
            if(use_golden_layer_input == true)
                write_data_to_ACC(FC2_BASE_ADDR + SOURCE_ADDR, 4, fc1_quantized_result_1[h]);
            else
                write_data_to_ACC(FC2_BASE_ADDR + SOURCE_ADDR, 4, fc1_quantized_result[h]);
            write_data_to_ACC(FC2_BASE_ADDR + WEIGHT_ADDR, 4, fc2_weights[m][h]);
        }

        fc2_result[m] = read_data_from_ACC(FC2_BASE_ADDR + RESULT_ADDR, 4);
        if(fc2_result[m] != fc2_activation_1[m]){
            printf("[ERROR] %d: %d\n", m, fc2_result[m]);
            printf("    Correct should be: %d\n", fc2_activation_1[m]);
            pass = false;
        }
        fc2_quantized_result[m] = fc2_result[m] / fc2_output_scale;
    }
    if(pass == true){
        printf("=====> FC2 activation PASS\n");
    }
    else{
        printf("=====> FC2 activation FAILED\n");
    }
}

void do_fc3(){
    bool pass = true;
    int M = 10;
    int start = 0;
    int end = M;

    for (int m = start; m != end; m++) {
        for (int h = 0; h != 84; h++) {
            if(use_golden_layer_input == true)
                write_data_to_ACC(FC3_BASE_ADDR + SOURCE_ADDR, 4, fc2_quantized_result_1[h]);
            else
                write_data_to_ACC(FC3_BASE_ADDR + SOURCE_ADDR, 4, fc2_quantized_result[h]);
            write_data_to_ACC(FC3_BASE_ADDR + WEIGHT_ADDR, 4, fc3_weights[m][h]);
        }
        write_data_to_ACC(FC3_BASE_ADDR + BIAS_ADDR, 4, fc3_bias[m]);

        fc3_result[m] = read_data_from_ACC(FC3_BASE_ADDR + RESULT_ADDR, 4);
        if(fc3_result[m] != fc3_bias_addition_1[m]){
            printf("[ERROR] %d: %d\n", m, fc3_result[m]);
            printf("    Correct should be: %d\n", fc3_bias_addition_1[m]);
            pass = false;
        }
        fc3_quantized_result[m] = fc3_result[m] / fc3_output_scale;
    }
    if(pass == true){
        printf("=====> FC3 activation PASS\n");
    }
    else{
        printf("=====> FC3 activation FAILED\n");
    }
}

void show_predict_result(){
    int max = 0, max_index = -1;
    for (int i=0; i<10; i++){
        if (fc3_quantized_result[i] > max){
            max = fc3_quantized_result[i];
            max_index = i;
        }
        printf ("fc3_quantized_result[%d] = %d\n",i,fc3_quantized_result[i]);
    }
    printf ("Predicted answer is %d\n",max_index);
    
}

int main() {

    //CONV1
    printf("do_conv1 start\n");
    do_conv1();


    //CONV2
    printf("do_conv2 start\n");
    do_conv2();

    //FC1
    printf("do_fc1 start\n");

    do_fc1();

    //FC2
    printf("do_fc2 start\n");

    do_fc2();

    //FC3
    printf("do_fc3 start\n");

    do_fc3();

    //Print ans
    show_predict_result();
    return 0;
}

