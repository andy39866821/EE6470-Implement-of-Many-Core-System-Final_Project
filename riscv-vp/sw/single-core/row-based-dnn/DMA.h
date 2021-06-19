#ifndef _DMA_H_
#define _DMA_H

#define WEIGHT_ADDR 0x00000000
#define SOURCE_ADDR 0x00000004
#define RESULT_ADDR 0x00000008
#define BIAS_ADDR 0x0000000C
#define START_ADDR 0x0000010
#define END_ADDR 0x0000014

static char* const CONV1_BASE_ADDR = reinterpret_cast<char* const>(0x73000000);
static char* const CONV2_BASE_ADDR = reinterpret_cast<char* const>(0x73200000);
static char* const FC1_BASE_ADDR = reinterpret_cast<char* const>(0x73400000);
static char* const FC2_BASE_ADDR = reinterpret_cast<char* const>(0x73600000);
static char* const FC3_BASE_ADDR = reinterpret_cast<char* const>(0x73800000);
// DMA 
static volatile uint32_t * const DMA_SRC_ADDR  = (uint32_t * const)0x70000000;
static volatile uint32_t * const DMA_DST_ADDR  = (uint32_t * const)0x70000004;
static volatile uint32_t * const DMA_LEN_ADDR  = (uint32_t * const)0x70000008;
static volatile uint32_t * const DMA_OP_ADDR   = (uint32_t * const)0x7000000C;
static volatile uint32_t * const DMA_STAT_ADDR = (uint32_t * const)0x70000010;
static const uint32_t DMA_OP_MEMCPY = 1;
static const uint32_t DMA_OP_NOP = 0;
//TODO fixed DMA access
bool _is_using_dma = true;


union word{
    int Integer;
    char Character[4];
};

void write_data_to_ACC(char* ADDR, int len, int value){
    word data;
    unsigned char buffer[4];

    data.Integer = value;
    buffer[0] = data.Character[0];
    buffer[1] = data.Character[1];
    buffer[2] = data.Character[2];
    buffer[3] = data.Character[3];

    if(_is_using_dma){  
        // Using DMA 
        *(DMA_SRC_ADDR) = (uint32_t)(buffer);
        *(DMA_DST_ADDR) = (uint32_t)(ADDR);
        *(DMA_LEN_ADDR) = len;
        *(DMA_OP_ADDR)  = DMA_OP_MEMCPY;

    }else{
        // Directly Send
        memcpy(ADDR, buffer, sizeof(unsigned char)*len);
    }
}
int read_data_from_ACC(char* ADDR, int len){
	
    unsigned char buffer[4];
	word data;
    if(_is_using_dma){
        // Using DMA 
        *(DMA_SRC_ADDR) = (uint32_t)(ADDR);
        *(DMA_DST_ADDR) = (uint32_t)(buffer);
        *(DMA_LEN_ADDR) = len;
        *(DMA_OP_ADDR)  = DMA_OP_MEMCPY;
    }else{
        // Directly Read
        memcpy(buffer, ADDR, sizeof(unsigned char)*len);
    }
	
    data.Character[0] = buffer[0];
    data.Character[1] = buffer[1];
    data.Character[2] = buffer[2];
    data.Character[3] = buffer[3];

	return data.Integer;
}


#endif