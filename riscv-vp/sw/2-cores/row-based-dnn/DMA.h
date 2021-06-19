#ifndef _DMA_H_
#define _DMA_H

#define WEIGHT_ADDR 0x00000000
#define SOURCE_ADDR 0x00000004
#define RESULT_ADDR 0x00000008
#define BIAS_ADDR 0x0000000C
#define START_ADDR 0x0000010
#define END_ADDR 0x0000014

static char* const CONV1_BASE_ADDR[2] = {reinterpret_cast<char* const>(0x73000000),
											reinterpret_cast<char* const>(0x73100000)};
static char* const CONV2_BASE_ADDR[2] = {reinterpret_cast<char* const>(0x73200000),
											reinterpret_cast<char* const>(0x73300000)};
static char* const FC1_BASE_ADDR[2] = {reinterpret_cast<char* const>(0x73400000),
											reinterpret_cast<char* const>(0x73500000)};
static char* const FC2_BASE_ADDR[2] = {reinterpret_cast<char* const>(0x73600000),
											reinterpret_cast<char* const>(0x73700000)};
static char* const FC3_BASE_ADDR[2] = {reinterpret_cast<char* const>(0x73800000),
											reinterpret_cast<char* const>(0x73900000)};
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

// dma lock for dma access
uint32_t dma_lock; 

// printf lock to promise consist string printed by each core
uint32_t print_lock; 

union word{
    int Integer;
    char Character[4];
};

int sem_init (uint32_t *__sem, uint32_t count) __THROW{
	*__sem=count;
	return 0;
}
int sem_wait (uint32_t *__sem) __THROW{
	uint32_t value, success; //RV32A
	__asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     beqz %[value],L%=                   # if zero, try again\n\t\
     addi %[value],%[value],-1           # value --\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  	return 0;
}

int sem_post (uint32_t *__sem) __THROW{
	uint32_t value, success; //RV32A
	__asm__ __volatile__("\
L%=:\n\t\
     lr.w %[value],(%[__sem])            # load reserved\n\t\
     addi %[value],%[value], 1           # value ++\n\t\
     sc.w %[success],%[value],(%[__sem]) # store conditionally\n\t\
     bnez %[success], L%=                # if the store failed, try again\n\t\
"
    : [value] "=r"(value), [success]"=r"(success)
    : [__sem] "r"(__sem)
    : "memory");
  	return 0;
}

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
	    sem_wait(&dma_lock);
        *(DMA_SRC_ADDR) = (uint32_t)(buffer);
        *(DMA_DST_ADDR) = (uint32_t)(ADDR);
        *(DMA_LEN_ADDR) = len;
        *(DMA_OP_ADDR)  = DMA_OP_MEMCPY;
	    sem_post(&dma_lock);

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
	    sem_wait(&dma_lock);
        *(DMA_SRC_ADDR) = (uint32_t)(ADDR);
        *(DMA_DST_ADDR) = (uint32_t)(buffer);
        *(DMA_LEN_ADDR) = len;
        *(DMA_OP_ADDR)  = DMA_OP_MEMCPY;
	    sem_post(&dma_lock);
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