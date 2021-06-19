#ifndef _CONVOLUTION_1_H_
#define _CONVOLUTION_1_H_
#include <systemc>
#include <sys/time.h>
#include <cmath>
#include <iomanip>
#include "Compile_def.h"
#include "tlm"
#include "tlm_utils/simple_target_socket.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

struct Convolution_1 : public sc_module {
    tlm_utils::simple_target_socket<Convolution_1> tsock;

    int C, R, S;
	sc_fifo< sc_dt::sc_int<8> > i_source;
	sc_fifo< sc_dt::sc_int<8> > i_weight;
	sc_fifo< sc_dt::sc_int<24> > o_result;

    SC_HAS_PROCESS(Convolution_1);

    Convolution_1(sc_module_name n):
        sc_module(n), 
        tsock("t_skt") {
        SC_THREAD( MAC );

        C = conv1_C;
        R = conv1_R;
        S = conv1_S;

        tsock.register_b_transport(this, &Convolution_1::blocking_transport);
    }

    ~Convolution_1() {

    }

    void MAC(){
        sc_int<8> source, weight;
        sc_int<24> acc, max_value;
            
        sc_int<8> input_buffer[3][5][32];
        sc_int<8> weight_buffer[3][5][5];
        sc_int<24> activation_buffer[2][28];

        bool even_row;

        while(true) { // m loop
           
            for (int c = 0; c != C; c++) {// read 3D weight
                for(int r = 0 ; r < R ; r++){
                    for(int s = 0 ; s < S ; s++){
                        weight_buffer[c][r][s] = i_weight.read();
                    }
                }
            }
           
            for(int i = 0 ; i < R-1 ; i++) {// read inital 4-row input
                for(int c = 0;  c != C ; c++) {
                    for(int q = 0;  q != 32 ; q ++) {
                        input_buffer[c][i][q] = i_source.read(); 
                    }
                }
            }


            for(int p = 4 ; p<32 ; p++) {

                for(int c = 0;  c != C ; c++) {
                    for(int q = 0;  q != 32 ; q ++) {
                        input_buffer[c][4][q] = i_source.read(); 
                    }
                }
                for(int index = 0 ; index < 28 ; index++) {
                    acc = 0;
                    for(int c = 0 ; c != C ; c++){
                        for(int r = 0 ; r != R ; r++){
                            for(int s = 0 ; s != S ; s++){
                                acc += weight_buffer[c][r][s] *  input_buffer[c][r][index + s];
                            }
                        }
                    }
                    activation_buffer[even_row][index] = acc;
                }
                for(int c = 0 ; c != C ; c++){
                    for(int i = 0 ; i < R-1 ; i++) {// shift 4-upper-row input
                        for(int index = 0 ; index < 32 ; index++){
                            input_buffer[c][i][index] = input_buffer[c][i+1][index];
                        }
                    }
                }

                if(even_row == true){
                    for(int index = 0 ; index < 14 ; index++){
                        max_value = 0;
                        for(int i = 0; i < 2 ; i++){
                            for(int j = 0 ; j < 2 ; j++){
                                max_value = (max_value > activation_buffer[i][index*2+j] ? max_value : activation_buffer[i][index*2+j]);
                            }
                        }
                        o_result.write(max_value);
                        //cout << "conv write result: " << max_value << endl;
                        
                    }
                }
                even_row = !even_row;
            }
        }
    }
    void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay) {
        wait(delay);
        sc_dt::uint64 addr = payload.get_address();
        //addr = addr - base_offset;
        unsigned char *mask_ptr = payload.get_byte_enable_ptr();
        unsigned char *data_ptr = payload.get_data_ptr();
        sc_int<24> result;
        sc_int<8> source, weight;
        switch (payload.get_command()) {
            case tlm::TLM_READ_COMMAND:
                //cout << "TLM READ: " << addr << endl;
                switch (addr) {
                    case RESULT_ADDR:
                        //cout << "conv1 socket try read result: " << endl;
                        result = o_result.read();
                        // cout << "conv1 socket read result: " << result << endl;
                        break;
                    default:
                        std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
                                << std::setfill('0') << std::setw(8) << std::hex << addr
                                << std::dec << " is not valid" << std::endl;
                        break;
                }
                word buffer;
                buffer.Integer = result;
                data_ptr[0] = buffer.Character[0];
                data_ptr[1] = buffer.Character[1];
                data_ptr[2] = buffer.Character[2];
                data_ptr[3] = buffer.Character[3];
                
                //cout << "conv socket set result: " << result << endl;
                break;
            case tlm::TLM_WRITE_COMMAND:
                //cout << "TLM WRITE: " << addr << endl;
                switch (addr) {
                    case SOURCE_ADDR:
                        source.range(7,0) = data_ptr[0];
                        //source.range(15,8) = data_ptr[1];
                        //source.range(23,16) = data_ptr[2];
                        //source.range(31,24) = data_ptr[3];
                        //cout << "conv1 socket get source: " << source << endl;
                        i_source.write(source);
                        break;
                    case WEIGHT_ADDR:
                        weight.range(7,0) = data_ptr[0];
                        //weight.range(15,8) = data_ptr[1];
                        //weight.range(23,16) = data_ptr[2];
                        //weight.range(31,24) = data_ptr[3];
                        //cout << "conv1 socket get weight: " << weight << endl;
                        i_weight.write(weight);
                        break;
                    default:
                        std::cerr << "Error! SobelFilter::blocking_transport: address 0x"
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
