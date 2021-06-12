#ifndef SOBEL_FILTER_RS_H_
#define SOBEL_FILTER_RS_H_
#include <systemc>
#include <cmath>
#include <iomanip>
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "filter_def.h"


struct SobelFilter_RS : public sc_module {
    tlm_utils::simple_target_socket<SobelFilter_RS> tsock;

    unsigned int i_buffer[3 * MAX_IMAGE_BUFFER_LENTH] = {0};
    unsigned int o_result_buffer[MAX_IMAGE_BUFFER_LENTH] = {0};

    unsigned char gray_buffer[3][MAX_IMAGE_BUFFER_LENTH] = {0};

    sc_fifo< unsigned int > i_index;
    sc_fifo< unsigned int > i_height;
    sc_fifo< unsigned int > i_width;
    sc_fifo< unsigned int > o_index;
    sc_fifo< unsigned int > o_data;
    unsigned int dataCountPerRow = 0;
    unsigned int countPerRow_W = 0;
    unsigned int countPerRow_R = 0;
    unsigned int max_Row_num_R = 0;
    unsigned int index = 0;
    unsigned int read_count = 0;

    SC_HAS_PROCESS(SobelFilter_RS);

    SobelFilter_RS(sc_module_name n): 
    sc_module(n), 
    tsock("t_skt"), 
    base_offset(0) 
    { 
        tsock.register_b_transport(this, &SobelFilter_RS::blocking_transport);
        SC_THREAD(do_filter);
    }

    ~SobelFilter_RS() {
    }

    int val[MASK_N];
    unsigned int base_offset;

    void do_filter(){
        int row_index = 0;
        int width = 0;
        // int height = 0;
        int cal_value = 0;

        { wait(CLOCK_PERIOD, SC_NS); }

        for (int i = 0; i < width; i++) {
            gray_buffer[0][i] = 0;
            gray_buffer[1][i] = 0;
            gray_buffer[2][i] = 0;
            wait(CLOCK_PERIOD, SC_NS);
        }

        width = i_width.read();
        // height = i_height.read();

        while (true) {

            for(int i = 0; i < MASK_N; i++) {
                val[i] = 0;
                wait(CLOCK_PERIOD, SC_NS);
            }

            row_index = i_index.read();
            // cout << "[SB] START - " << row_index << endl;

            for (int i = 0; i < width; i++) {
                gray_buffer[row_index % 3][i] = (i_buffer[3 * i + 2] + i_buffer[3 * i + 1] + i_buffer[3 * i + 0]) / 3;
                wait(CLOCK_PERIOD, SC_NS);
            }

            double total = 0;
            int result = 0;

            // cout << "<"; 
            // for(int i = 0; i < 3; i++){
            //     cout << endl;                    
                                   
            //     for(int j = 0; j < width; j++){
            //         // gray_buffer[i][j] = 10;
            //         cout << (int) gray_buffer[i][j]  << " ";
            //     }
            //     cout << endl;                                       
            // }
            // cout << ">" <<  endl;
 
            for(int x = 0; x < width; x++) {
                // convolution
                for(int i = 0; i != MASK_N; i++) {
                    cal_value = 0;
                    for(int v = 0; v < MASK_Y; v++) {
                        for(int u = 0; u < MASK_X; u++) {
                            if(((x + u - 1) >= 0) && ((x + u - 1) < width)){
                                cal_value += mask[i][v][u] *  (unsigned int) gray_buffer[(row_index - 2 + v) % 3][x + u -1];
                                wait(CLOCK_PERIOD, SC_NS);
                            }
                        }
                    }
                    val[i] = cal_value;
                }
                // caculate result
                total = 0;
                for (int i = 0; i != MASK_N; i++) {
                    total += val[i] * val[i];
                    wait(CLOCK_PERIOD, SC_NS);
                }
                result = static_cast<int>(std::sqrt(total));
                o_result_buffer[x] = (unsigned int) (result > THRESHOLD) ? 255 : 0;
                // o_result_buffer[x] = (unsigned int) gray_buffer[row_index % 3][x];
                wait(CLOCK_PERIOD, SC_NS);
            }

            // output 
            for(int j = 0; j < width; j++)
                o_data.write(o_result_buffer[j]);

            o_index.write(row_index);
        }

    }

    void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay){
        wait(delay);
        // unsigned char *mask_ptr = payload.get_byte_enable_ptr();
        // auto len = payload.get_data_length();
        tlm::tlm_command cmd = payload.get_command();
        sc_dt::uint64 addr = payload.get_address();
        unsigned char *data_ptr = payload.get_data_ptr();
        int width = 0;
        int height = 0;
        int out = 0;
        
        addr -= base_offset;
        switch (cmd) {
            case tlm::TLM_READ_COMMAND:
                // cout << "[SB] READ : " << std::hex << addr << std::dec <<endl;
                switch (addr) {
                    case SOBEL_FILTER_RS_RESULT_ADDR:
                        // cout << "\n[SB] READ : count = " << countPerRow_R << endl; 
                        
                        for(int i = 0; i < DNA_TRANS; i++ ){
                            data_ptr[i] = (unsigned char) o_data.read();
                            // data_ptr[i] = (unsigned char) o_result_buffer[(read_count % (dataCountPerRow / 3)) * DNA_TRANS + i];
                            // cout << /*"(" << (read_count % (dataCountPerRow / 3)) << ")"<<*/ (unsigned int) data_ptr[i] << " - ";
                        } 

                        
                        read_count++;
                        if(read_count >= 4){
                            read_count = 0;
                            out = o_index.read();

                            // cout << " >> "<< out << " << " << endl;
                            countPerRow_R++;
                        }
                        // cout << (int)o_result_buffer[DNA_TRANS * countPerRow_R] << endl;
                        break;
                    default:
                        std::cerr   << "READ Error! SobelFilter_RS::blocking_transport: address 0x"
                                    << std::setfill('0') << std::setw(8) << std::hex << addr
                                    << std::dec << " is not valid" << std::endl;
                        break;
                }
                break;
            case tlm::TLM_WRITE_COMMAND:
                // cout << "[SB] WRITE : " << std::hex << addr << std::dec <<endl;
                switch (addr) {
                    case SOBEL_FILTER_RS_W_WIDTH:
                        for(int i = 0; i < 4; i++ )
                            width += (int) data_ptr[i] * (256 << (i - 1)*4);
                        // cout <<  width << endl;      
                        index = 0;
                        i_width.write(width);
                        dataCountPerRow = (width * 3) / DNA_TRANS + (((width * 3) % DNA_TRANS == 0) ? 0 : 1);
                        break;
                    case SOBEL_FILTER_RS_W_HEIGHT:
                        for(int i = 0; i < 4; i++ )
                            height += (int) data_ptr[i] * (256 << (i - 1)*4);
                        // cout << height << endl;
                        index = 0;
                        i_height.write(height);
                        max_Row_num_R = height;
                        break;
                    case SOBEL_FILTER_RS_W_DATA:
                        for(int i = 0; i < DNA_TRANS; i++ ){
                            i_buffer[countPerRow_W * DNA_TRANS + i] = (unsigned int) data_ptr[i];
                            // cout << i_buffer[countPerRow_W * DNA_TRANS + i] << " ";
                        }
                        // cout << endl;
                        countPerRow_W ++;
                        if(countPerRow_W == dataCountPerRow){
                            countPerRow_W = 0;
                            i_index.write(index);
                            index ++;
                        }
                        break;
                    default:
                        std::cerr   << "WRITE Error! SobelFilter::blocking_transport: address 0x"
                                    << std::setfill('0') << std::setw(8) << std::hex << addr
                                    << std::dec << " is not valid" << std::endl;
                        break;
                }
                break;
            case tlm::TLM_IGNORE_COMMAND:
                payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
                return;
            default:
                payload.set_response_status(tlm::TLM_GENERIC_ERROR_RESPONSE);
                return;
        }
        payload.set_response_status(tlm::TLM_OK_RESPONSE); // Always OK
    }
};
#endif
