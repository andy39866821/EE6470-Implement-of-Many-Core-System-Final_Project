#ifndef FILTER_H_
#define FILTER_H_
#include <systemc>
#include <cmath>
#include <iomanip>
#include <iostream>
using namespace std;
using namespace sc_core;

#include <tlm>
#include <tlm_utils/simple_target_socket.h>

#include "filter_def.h"

struct Filter : public sc_module {
tlm_utils::simple_target_socket<Filter> tsock;

sc_fifo<unsigned char> i_r;
sc_fifo<unsigned char> i_g;
sc_fifo<unsigned char> i_b;
sc_fifo<unsigned char> o_r;
sc_fifo<unsigned char> o_g;
sc_fifo<unsigned char> o_b;

SC_HAS_PROCESS(Filter);

Filter(sc_module_name n): 
    sc_module(n), 
    tsock("t_skt"), 
    base_offset(0) 
    {
        tsock.register_b_transport(this, &Filter::blocking_transport);
        SC_THREAD(do_filter);
    }

    ~Filter() {
    }

    unsigned int base_offset;

    void do_filter(){
        { 
            wait(CLOCK_PERIOD, SC_NS); 
        }
        int cnt = 0;
        while (true) {
            int R = 0;
            int G = 0;
            int B = 0;
            for (unsigned int i = 0; i < MASK_Y; ++i) {
                for (unsigned int j = 0; j < MASK_X; ++j) {
                    unsigned char r = i_r.read();
                    unsigned char g = i_g.read();
                    unsigned char b = i_b.read();
                    //cout << name() << " " << cnt << " " << i << " " << j << " R: " << (int)r << 
                    //    " G: " << (int)g << " B: " << (int)b << endl;
                    wait(CLOCK_PERIOD, SC_NS);
                    R += filter[i][j] * (int)r;
                    G += filter[i][j] * (int)g;
                    B += filter[i][j] * (int)b;


                }
            }   
            cnt++;

            o_r.write((unsigned char)(R/factor));
            o_g.write((unsigned char)(G/factor));
            o_b.write((unsigned char)(B/factor));
        }
    }

    void blocking_transport(tlm::tlm_generic_payload &payload, sc_core::sc_time &delay){
        wait(delay);
        // unsigned char *mask_ptr = payload.get_byte_enable_ptr();
        // auto len = payload.get_data_length();
        tlm::tlm_command cmd = payload.get_command();
        sc_dt::uint64 addr = payload.get_address();
        unsigned char *data_ptr = payload.get_data_ptr();

        addr -= base_offset;


        // cout << (int)data_ptr[0] << endl;
        // cout << (int)data_ptr[1] << endl;
        // cout << (int)data_ptr[2] << endl;
        word buffer;

        switch (cmd) {
        case tlm::TLM_READ_COMMAND:
            // cout << "READ" << endl;
            switch (addr) {
            case FILTER_RESULT_ADDR:
                buffer.uc[0] = o_r.read();
                buffer.uc[1] = o_g.read();
                buffer.uc[2] = o_b.read();
                buffer.uc[3] = 0;
                break;
            default:
                std::cerr << "READ Error! Filter::blocking_transport: address 0x"
                        << std::setfill('0') << std::setw(8) << std::hex << addr
                        << std::dec << " is not valid" << std::endl;
            }
            data_ptr[0] = buffer.uc[0];
            data_ptr[1] = buffer.uc[1];
            data_ptr[2] = buffer.uc[2];
            data_ptr[3] = buffer.uc[3];
            break;
        case tlm::TLM_WRITE_COMMAND:
            // cout << "WRITE" << endl;
            switch (addr) {
            case FILTER_R_ADDR:
                i_r.write(data_ptr[0]);
                i_g.write(data_ptr[1]);
                i_b.write(data_ptr[2]);
                break;
            default:
                std::cerr << "WRITE Error! Filter::blocking_transport: address 0x"
                        << std::setfill('0') << std::setw(8) << std::hex << addr
                        << std::dec << " is not valid" << std::endl;
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
