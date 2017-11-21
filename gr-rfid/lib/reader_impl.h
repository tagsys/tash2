/* -*- c++ -*- */
/* 
 * Copyright 2015 <Nikos Kargas (nkargas@isc.tuc.gr)>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifndef INCLUDED_RFID_READER_IMPL_H
#define INCLUDED_RFID_READER_IMPL_H

#include <rfid/reader.h>
#include <vector>
#include <vector>
#include <queue>
#include <fstream>

namespace gr {
    namespace rfid {

        class reader_impl : public reader {
        private:
            int s_rate, d_rate, n_cwquery_s, n_cwack_s, n_p_down_s;
            float sample_d, n_data0_s, n_data1_s, n_cw_s, n_pw_s, n_delim_s, n_trcal_s;
            std::vector<float> data_0, data_1, cw, cw_ack, cw_query, delim, frame_sync, preamble, rtcal, trcal, query_bits, ack_bits, query_rep, nak, query_adjust_bits, p_down;
            int q_change; // 0-> increment, 1-> unchanged, 2-> decrement
            int n_cw_charging_s;
            std::vector<float> cw_charging;

            void gen_query_adjust_bits();

            void crc_append(std::vector<float> &q);

            void gen_query_bits();

            void gen_ack_bits(const float *in);

            void crc16_append(std::vector<float> &q);

            // send select and query
            // Command Waiting Time
            std::vector<float> wait_T4;
            const int SELECT_CODE[4] = {1, 0, 1, 0};
            const int SELECT_TARGET[3] = {1, 0, 0};
            struct select_command {
                int SELECT_ACTION[3];
                int SELECT_MEMBANK[2];
                int SELECT_POINTER[8];
                int SELECT_LENGTH[8];
                int SELECT_TRUNCATE[1];
                std::vector<int> SELECT_MASK;
            };
            struct eachsend {
                int slotNum;
                std::vector<select_command> select_input;
            };
            std::vector<eachsend> send;
            std::vector<float> output;

            // start time

            timeval all_start,all_end;

            void setting();

            void logfile();

            void gen_select_bits(select_command &input);

        public:
            void print_results();

            reader_impl(int sample_rate, int dac_rate);

            ~reader_impl();


            void forecast(int noutput_items, gr_vector_int &ninput_items_required);

            int general_work(int noutput_items,
                             gr_vector_int &ninput_items,
                             gr_vector_const_void_star &input_items,
                             gr_vector_void_star &output_items);


        };

    } // namespace rfid
} // namespace gr

#endif /* INCLUDED_RFID_READER_IMPL_H */

