#include <systemc>
#include <sys/time.h> 
#include "Compile_def.h"
#include <cmath>
#include <iomanip>
#include "tlm"
#include "tlm_utils/simple_target_socket.h"

using namespace std;
using namespace sc_core;
using namespace sc_dt;

struct FC_3: public sc_module {
    int H, M;

    tlm_utils::simple_target_socket<FC_3> tsock;
    
	sc_fifo< sc_dt::sc_int<32> > i_source;
	sc_fifo< sc_dt::sc_int<32> > i_weight;
    sc_fifo< sc_dt::sc_int<32> > i_bias;
    sc_fifo< sc_dt::sc_int<32> > i_start;
    sc_fifo< sc_dt::sc_int<32> > i_end;
	sc_fifo< sc_dt::sc_int<32> > o_result;
    
    SC_HAS_PROCESS(FC_3);

    FC_3(sc_module_name n):
        sc_module(n)
    {
        SC_THREAD(Fully_Connect);

        H = FC3_H;
        M = FC3_M;

        tsock.register_b_transport(this, &FC_3::blocking_transport);

    }

    ~FC_3(){}

    void Fully_Connect(){
        sc_int<32> source, weight, bias;
        sc_int<32> start, end;
        sc_int<32> acc[10];
        sc_int<32> zero = 0;

        while(true) {
            for (int i=0; i<M; i++) acc[i] = 0;
            start = i_start.read();
            end = i_end.read();
            
            for(int i = start ; i < end ; i++){		
                for(int j = 0 ; j < H ; j++){
                    source = i_source.read();
                    weight = i_weight.read();
                    acc[i] += source * weight;
                }
            }
            for(int j = start ; j < end ; j++){
                bias = i_bias.read();
                acc[j] += bias;
                o_result.write(acc[j]);
            }
        }
    }


    void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay){
        wait(delay);
        sc_dt::uint64 addr = payload.get_address();
        //addr = addr - base_offset;
        unsigned char *mask_ptr = payload.get_byte_enable_ptr();
        unsigned char *data_ptr = payload.get_data_ptr();
        sc_int<32> result, source, weight, bias, start, end;
        switch (payload.get_command()) {
            case tlm::TLM_READ_COMMAND:
                switch (addr) {
                    case RESULT_ADDR:
                        result = o_result.read();
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
                
                break;
            case tlm::TLM_WRITE_COMMAND:
                switch (addr) {
                    case SOURCE_ADDR:
                        source.range(7,0) = data_ptr[0];
                        source.range(15,8) = data_ptr[1];
                        source.range(23,16) = data_ptr[2];
                        source.range(31,24) = data_ptr[3];
                        i_source.write(source);
                        break;
                    case WEIGHT_ADDR:
                        weight.range(7,0) = data_ptr[0];
                        weight.range(15,8) = data_ptr[1];
                        weight.range(23,16) = data_ptr[2];
                        weight.range(31,24) = data_ptr[3];
                        i_weight.write(weight);
                        break;
                    case BIAS_ADDR:
                        bias.range(7,0) = data_ptr[0];
                        bias.range(15,8) = data_ptr[1];
                        bias.range(23,16) = data_ptr[2];
                        bias.range(31,24) = data_ptr[3];
                        i_bias.write(bias);
                        break;
                    case START_ADDR:
                        start.range(7,0) = data_ptr[0];
                        start.range(15,8) = data_ptr[1];
                        start.range(23,16) = data_ptr[2];
                        start.range(31,24) = data_ptr[3];
                        i_start.write(start);
                        break;
                    case END_ADDR:
                        end.range(7,0) = data_ptr[0];
                        end.range(15,8) = data_ptr[1];
                        end.range(23,16) = data_ptr[2];
                        end.range(31,24) = data_ptr[3];
                        i_end.write(end);
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


