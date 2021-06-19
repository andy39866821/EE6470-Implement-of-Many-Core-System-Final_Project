#ifndef _COMPILE_DEF_H_
#define _COMPILE_DEF_H_

#define conv1_C 3
#define conv1_R 5
#define conv1_S 5
#define conv2_C 6
#define conv2_R 5
#define conv2_S 5
#define FC1_M 120
#define FC1_H 400
#define FC2_M 84
#define FC2_H 120
#define FC3_M 10
#define FC3_H 84
#define FC3_BIAS 10

#define CLOCK_PERIOD 10

#define MAX_IMAGE_BUFFER_LENTH 1024

#define input_scale 127.0
#define conv1_output_scale 2221.025418064295
#define conv2_output_scale 533.2203924303868
#define fc1_output_scale 297.85806821923927 
#define fc2_output_scale 388.07897372365807
#define fc3_output_scale 366.71929725990833


#define WEIGHT_ADDR 0x00000000
#define SOURCE_ADDR 0x00000004
#define RESULT_ADDR 0x00000008
#define BIAS_ADDR 0x0000000C

#define CONV1_EACH_ROW_TIMES 50
#define CONV2_EACH_ROW_TIMES 50
#define FC1_EACH_ROUND_TIMES 20
#define FC2_EACH_ROUND_TIMES 20
#define FC3_EACH_ROUND_TIMES 20



union word{
    int Integer;
    char Character[4];
};


#endif