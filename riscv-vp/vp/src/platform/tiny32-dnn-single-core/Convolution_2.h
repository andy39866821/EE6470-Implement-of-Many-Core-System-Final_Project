#ifndef _CONVOLUTION_2_H_
#define _CONVOLUTION_2_H_
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

struct Convolution_2 : public sc_module {
    tlm_utils::simple_target_socket<Convolution_2> tsock;

    int C, R, S;
	sc_fifo< sc_dt::sc_int<32> > i_source;
	sc_fifo< sc_dt::sc_int<32> > i_weight;
	sc_fifo< sc_dt::sc_int<32> > o_result;

    SC_HAS_PROCESS(Convolution_2);

    Convolution_2(sc_module_name n):
        sc_module(n), 
        tsock("t_skt") {
        SC_THREAD( MAC );

        C = conv2_C;
        R = conv2_R;
        S = conv2_S;

        tsock.register_b_transport(this, &Convolution_2::blocking_transport);
    }

    ~Convolution_2() {

    }

    void MAC(){
        sc_int<32> source, weight, acc, max_vaule;
        
        while(true) { // m loop
            max_vaule = 0;
            for(int i = 0 ; i < 2 ; i++){
                for(int j = 0 ; j < 2 ; j++){
                    acc = 0;
                    for(int c = 0 ; c < C ; c++){
                        for(int r = 0 ; r < R ; r++){
                            for(int s = 0 ; s < S ; s++){
                            
                                source = i_source.read();
                                weight = i_weight.read();
                                //cout << "Get: " << c << "," << r << "," << s << " : " 
                                //    << source << " " << weight << endl;
                                acc +=  source * weight;
                            }
                        }
                    }
                    max_vaule = (max_vaule > acc ? max_vaule : acc);
                }
            }
            
            o_result.write(max_vaule);
            //cout << "Send data: " << acc << endl;
            
        }
        
    }
    void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay) {
        wait(delay);
        sc_dt::uint64 addr = payload.get_address();
        //addr = addr - base_offset;
        unsigned char *mask_ptr = payload.get_byte_enable_ptr();
        unsigned char *data_ptr = payload.get_data_ptr();
        sc_int<32> result, source, weight;
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
                        source.range(15,8) = data_ptr[1];
                        source.range(23,16) = data_ptr[2];
                        source.range(31,24) = data_ptr[3];
                        //cout << "conv1 socket get source: " << source << endl;
                        i_source.write(source);
                        break;
                    case WEIGHT_ADDR:
                        weight.range(7,0) = data_ptr[0];
                        weight.range(15,8) = data_ptr[1];
                        weight.range(23,16) = data_ptr[2];
                        weight.range(31,24) = data_ptr[3];
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
