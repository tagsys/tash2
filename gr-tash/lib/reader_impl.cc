/* -*- c++ -*- */
/* 
 * Copyright 2017 <+YOU OR YOUR COMPANY+>.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "reader_impl.h"
#include "tash/tashconfig.h"
#include <sys/time.h>
#include <fstream>
#include <string>
#include "rapidjson/document.h"
#include <rapidjson/document.h>
#include "rapidjson/reader.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

namespace gr {
    namespace tash {

        reader::sptr
        reader::make(int sample_rate, int dac_rate) {
            return gnuradio::get_initial_sptr
                    (new reader_impl(sample_rate, dac_rate));
        }

        /*
         * The private constructor
         */
        reader_impl::reader_impl(int sample_rate, int dac_rate)
                : gr::block("reader",
                            gr::io_signature::make(1, 1, sizeof(float)),
                            gr::io_signature::make(1, 1, sizeof(float))) {

            GR_LOG_INFO(d_logger, "Block initialized");

            sample_d = 1.0 / dac_rate * pow(10, 6);

            // Number of samples for transmitting

            n_data0_s = 2 * PW_D / sample_d;
            n_data1_s = 4 * PW_D / sample_d;
            n_pw_s = PW_D / sample_d;
            n_cw_s = CW_D / sample_d;
            n_delim_s = DELIM_D / sample_d;
            n_trcal_s = TRCAL_D / sample_d;

            GR_LOG_INFO(d_logger, "Number of samples data 0 : " << n_data0_s);
            GR_LOG_INFO(d_logger, "Number of samples data 1 : " << n_data1_s);
            GR_LOG_INFO(d_logger, "Number of samples cw : " << n_cw_s);
            GR_LOG_INFO(d_logger, "Number of samples delim : " << n_delim_s);
            GR_LOG_INFO(d_logger, "Number of slots : " << std::pow(2, FIXED_Q));

            // CW waveforms of different sizes
            n_cwquery_s = (T1_D + T2_D + RN16_D) / sample_d;     //RN16
            n_cwack_s = (3 * T1_D + T2_D + EPC_D) /
                        sample_d;    //EPC   if it is longer than nominal it wont cause tags to change inventoried flag
            n_p_down_s = (P_DOWN_D) / sample_d;
            n_cw_charging_s = 10000 / sample_d;

            std::cout << "ACK Time:" << n_cwack_s << std::endl;

            cw_charging.resize(n_cw_charging_s);
            p_down.resize(n_p_down_s);        // Power down samples
            cw_query.resize(n_cwquery_s);      // Sent after query/query rep
            cw_ack.resize(n_cwack_s);          // Sent after ack

            std::fill_n(cw_charging.begin(), cw_charging.size(), 1);
            std::fill_n(cw_query.begin(), cw_query.size(), 1);
            std::fill_n(cw_ack.begin(), cw_ack.size(), 1);

            GR_LOG_INFO(d_logger, "Carrier wave after a query transmission in samples : " << n_cwquery_s);
            GR_LOG_INFO(d_logger, "Carrier wave after ACK transmission in samples : " << n_cwack_s);

            // Construct vectors (resize() default initialization is zero)
            data_0.resize(n_data0_s);
            data_1.resize(n_data1_s);
            cw.resize(n_cw_s);
            delim.resize(n_delim_s);
            rtcal.resize(n_data0_s + n_data1_s);
            trcal.resize(n_trcal_s);

            // Fill vectors with data
            std::fill_n(data_0.begin(), data_0.size() / 2, 1);
            std::fill_n(data_1.begin(), 3 * data_1.size() / 4, 1);
            std::fill_n(cw.begin(), cw.size(), 1);
            std::fill_n(rtcal.begin(), rtcal.size() - n_pw_s, 1); // RTcal
            std::fill_n(trcal.begin(), trcal.size() - n_pw_s, 1); // TRcal

            // create preamble
            preamble.insert(preamble.end(), delim.begin(), delim.end());
            preamble.insert(preamble.end(), data_0.begin(), data_0.end());
            preamble.insert(preamble.end(), rtcal.begin(), rtcal.end());
            preamble.insert(preamble.end(), trcal.begin(), trcal.end());

            // create framesync
            frame_sync.insert(frame_sync.end(), delim.begin(), delim.end());
            frame_sync.insert(frame_sync.end(), data_0.begin(), data_0.end());
            frame_sync.insert(frame_sync.end(), rtcal.begin(), rtcal.end());

            // create query rep
            query_rep.insert(query_rep.end(), frame_sync.begin(), frame_sync.end());
            query_rep.insert(query_rep.end(), data_0.begin(), data_0.end());
            query_rep.insert(query_rep.end(), data_0.begin(), data_0.end());
            query_rep.insert(query_rep.end(), data_0.begin(), data_0.end());
            query_rep.insert(query_rep.end(), data_0.begin(), data_0.end());

            // create nak
            nak.insert(nak.end(), frame_sync.begin(), frame_sync.end());
            nak.insert(nak.end(), data_1.begin(), data_1.end());
            nak.insert(nak.end(), data_1.begin(), data_1.end());
            nak.insert(nak.end(), data_0.begin(), data_0.end());
            nak.insert(nak.end(), data_0.begin(), data_0.end());
            nak.insert(nak.end(), data_0.begin(), data_0.end());
            nak.insert(nak.end(), data_0.begin(), data_0.end());
            nak.insert(nak.end(), data_0.begin(), data_0.end());
            nak.insert(nak.end(), data_0.begin(), data_0.end());

            gen_query_bits();
            gen_query_adjust_bits();

            // T4
            wait_T4.resize(200);
            std::fill_n(wait_T4.begin(), wait_T4.size(), 1);
            setting();
        }

        void reader_impl::gen_query_bits() {
            int num_ones = 0, num_zeros = 0;
            query_bits.resize(0);
            query_bits.insert(query_bits.end(), &QUERY_CODE[0], &QUERY_CODE[4]);
            query_bits.push_back(DR);
            query_bits.insert(query_bits.end(), &M[0], &M[2]);
            query_bits.push_back(TREXT);
            query_bits.insert(query_bits.end(), &SEL[0], &SEL[2]);
            query_bits.insert(query_bits.end(), &SESSION[0], &SESSION[2]);
            query_bits.push_back(TARGET);
            query_bits.insert(query_bits.end(), &Q_VALUE[FIXED_Q][0], &Q_VALUE[FIXED_Q][4]);
            crc_append(query_bits);
        }


        void reader_impl::gen_ack_bits(const float *in) {
            ack_bits.resize(0);
            ack_bits.insert(ack_bits.end(), &ACK_CODE[0], &ACK_CODE[2]);
            ack_bits.insert(ack_bits.end(), &in[0], &in[16]);
        }

        void reader_impl::gen_query_adjust_bits() {
            query_adjust_bits.resize(0);
            query_adjust_bits.insert(query_adjust_bits.end(), &QADJ_CODE[0], &QADJ_CODE[4]);
            query_adjust_bits.insert(query_adjust_bits.end(), &SESSION[0], &SESSION[2]);
            query_adjust_bits.insert(query_adjust_bits.end(), &Q_UPDN[1][0], &Q_UPDN[1][3]);
        }


        /*
         * Our virtual destructor.
         */
        reader_impl::~reader_impl() {

        }

        void reader_impl::print_results() {
            std::cout << "\n --------------------------" << std::endl;
            std::cout << "| Number of queries/queryreps sent : " << reader_state->reader_stats.n_queries_sent - 1
                      << std::endl;
            std::cout << "| Current Inventory round : " << reader_state->reader_stats.cur_inventory_round << std::endl;
            std::cout << " --------------------------" << std::endl;

            std::cout << "| Correctly decoded EPC : " << reader_state->reader_stats.n_epc_correct << std::endl;
            std::cout << "| Number of unique tags : " << reader_state->reader_stats.tag_reads.size() << std::endl;

            std::map<int, int>::iterator it;

            for (it = reader_state->reader_stats.tag_reads.begin();
                 it != reader_state->reader_stats.tag_reads.end(); it++) {
                std::cout << std::hex << "| Tag ID : " << it->first << "  ";
                std::cout << "Num of reads : " << std::dec << it->second << std::endl;
            }
            logfile();
            std::cout << " --------------------------" << std::endl;
        }

        void
        reader_impl::forecast(int noutput_items, gr_vector_int &ninput_items_required) {
            // no input data
            ninput_items_required[0] = 0;
        }

        int
        reader_impl::general_work(int noutput_items,
                                  gr_vector_int &ninput_items,
                                  gr_vector_const_void_star &input_items,
                                  gr_vector_void_star &output_items) {

            const float *in = (const float *) input_items[0];
            float *out = (float *) output_items[0];
            std::vector<float> out_message;
            int n_output;
            int consumed = consumed = ninput_items[0];
            int written = 0;
            switch (reader_state->gen2_logic_status) {
                case START:
                    GR_LOG_INFO(d_debug_logger, "START");
                    memcpy(&out[written], &cw_charging[0], sizeof(float) * cw_charging.size());
                    written += cw_charging.size();
                    struct timeval time;
                    gettimeofday(&time, NULL);
                    //std::cout<<time.tv_sec<<" "<< reader_state->reader_stats.start.tv_sec <<std::endl;
                    if (time.tv_sec - reader_state->reader_stats.start.tv_sec < 2) {
                        // charging for starting
                        reader_state->gen2_logic_status = START;
                    } else {
                        reader_state->gen2_logic_status = SEND_SELECT;
                        std::cout << "Start Inventory" << std::endl;
                        gettimeofday(&all_start, NULL);
                    }
                    break;

                case SEND_SELECT:
                    //inventory
                    for (int i = 0;
                         i < send[reader_state->reader_stats.cur_inventory_round - 1].select_input.size(); i++) {
                        //select
                        gen_select_bits(send[reader_state->reader_stats.cur_inventory_round - 1].select_input[i]);
                        // Send FrameSync
                        memcpy(&out[written], &frame_sync[0], sizeof(float) * frame_sync.size());
                        written += frame_sync.size();
                        // Send Command
                        for (int i = 0; i < output.size(); i++) {
                            if (output[i] == 1) {
                                memcpy(&out[written], &data_1[0], sizeof(float) * data_1.size());
                                written += data_1.size();
                            } else {
                                memcpy(&out[written], &data_0[0], sizeof(float) * data_0.size());
                                written += data_0.size();
                            }
                        }
                        // wait
                        memcpy(&out[written], &wait_T4[0], sizeof(float) * wait_T4.size());
                        written += wait_T4.size();
                    }
                    std::cout << "select send" << std::endl;
                    reader_state->gen2_logic_status = SEND_QUERY;
                    break;

                case POWER_DOWN:
                    GR_LOG_INFO(d_debug_logger, "POWER DOWN");
                    memcpy(&out[written], &p_down[0], sizeof(float) * p_down.size());
                    written += p_down.size();
                    reader_state->gen2_logic_status = START;
                    break;

                case SEND_QUERY:

                    std::cout << "Inventory :" << reader_state->reader_stats.n_queries_sent << std::endl;

                    GR_LOG_INFO(d_debug_logger, "QUERY");
                    GR_LOG_INFO(d_debug_logger, "INVENTORY ROUND : " << reader_state->reader_stats.cur_inventory_round
                                                                     << " SLOT NUMBER : "
                                                                     << reader_state->reader_stats.cur_slot_number);

                    reader_state->reader_stats.n_queries_sent += 1;
                    // Controls the other two blocks
                    reader_state->decoder_status = DECODER_DECODE_RN16;
                    reader_state->gate_status = GATE_SEEK_RN16;

                    memcpy(&out[written], &preamble[0], sizeof(float) * preamble.size());
                    written += preamble.size();

                    for (int i = 0; i < query_bits.size(); i++) {
                        if (query_bits[i] == 1) {
                            memcpy(&out[written], &data_1[0], sizeof(float) * data_1.size());
                            written += data_1.size();
                        } else {
                            memcpy(&out[written], &data_0[0], sizeof(float) * data_0.size());
                            written += data_0.size();
                        }
                    }
                    // Send CW for RN16
                    memcpy(&out[written], &cw_query[0], sizeof(float) * cw_query.size());
                    written += cw_query.size();

                    // Return to IDLE
                    reader_state->gen2_logic_status = IDLE;
                    break;

                case SEND_ACK:
                    GR_LOG_INFO(d_debug_logger, "SEND ACK");
                    if (ninput_items[0] == RN16_BITS - 1) {
                        // Controls the other two blocks
                        reader_state->decoder_status = DECODER_DECODE_EPC;
                        reader_state->gate_status = GATE_SEEK_EPC;

                        gen_ack_bits(in);

                        // Send FrameSync
                        memcpy(&out[written], &frame_sync[0], sizeof(float) * frame_sync.size());
                        written += frame_sync.size();

                        for (int i = 0; i < ack_bits.size(); i++) {
                            if (ack_bits[i] == 1) {
                                memcpy(&out[written], &data_1[0], sizeof(float) * data_1.size());
                                written += data_1.size();
                            } else {
                                memcpy(&out[written], &data_0[0], sizeof(float) * data_0.size());
                                written += data_0.size();
                            }
                        }
                        consumed = ninput_items[0];
                        reader_state->gen2_logic_status = SEND_CW;
                    }
                    break;

                case SEND_CW:
                    GR_LOG_INFO(d_debug_logger, "SEND CW");
                    memcpy(&out[written], &cw_ack[0], sizeof(float) * cw_ack.size());
                    written += cw_ack.size();
                    reader_state->gen2_logic_status = IDLE;      // Return to IDLE
                    break;

                case SEND_QUERY_REP:
                    GR_LOG_INFO(d_debug_logger, "SEND QUERY_REP");
                    GR_LOG_INFO(d_debug_logger, "INVENTORY ROUND : " << reader_state->reader_stats.cur_inventory_round
                                                                     << " SLOT NUMBER : "
                                                                     << reader_state->reader_stats.cur_slot_number);
                    // Controls the other two blocks
                    reader_state->decoder_status = DECODER_DECODE_RN16;
                    reader_state->gate_status = GATE_SEEK_RN16;
                    reader_state->reader_stats.cur_slot_number += 1;
                    memcpy(&out[written], &query_rep[0], sizeof(float) * query_rep.size());
                    written += query_rep.size();

                    memcpy(&out[written], &cw_query[0], sizeof(float) * cw_query.size());
                    written += cw_query.size();

                    reader_state->gen2_logic_status = IDLE;    // Return to IDLE
                    break;

                default:
                    // IDLE
                    break;
            }
            consume_each(consumed);
            return written;
        }

        /* Function adapted from https://www.cgran.org/wiki/Gen2 */
        void reader_impl::crc_append(std::vector<float> &q) {
            int crc[] = {1, 0, 0, 1, 0};

            for (int i = 0; i < 17; i++) {
                int tmp[] = {0, 0, 0, 0, 0};
                tmp[4] = crc[3];
                if (crc[4] == 1) {
                    if (q[i] == 1) {
                        tmp[0] = 0;
                        tmp[1] = crc[0];
                        tmp[2] = crc[1];
                        tmp[3] = crc[2];
                    } else {
                        tmp[0] = 1;
                        tmp[1] = crc[0];
                        tmp[2] = crc[1];
                        if (crc[2] == 1) {
                            tmp[3] = 0;
                        } else {
                            tmp[3] = 1;
                        }
                    }
                } else {
                    if (q[i] == 1) {
                        tmp[0] = 1;
                        tmp[1] = crc[0];
                        tmp[2] = crc[1];
                        if (crc[2] == 1) {
                            tmp[3] = 0;
                        } else {
                            tmp[3] = 1;
                        }
                    } else {
                        tmp[0] = 0;
                        tmp[1] = crc[0];
                        tmp[2] = crc[1];
                        tmp[3] = crc[2];
                    }
                }
                memcpy(crc, tmp, 5 * sizeof(float));
            }
            for (int i = 4; i >= 0; i--)
                q.push_back(crc[i]);
        }

        // CRC16
        void reader_impl::crc16_append(std::vector<float> &q) {
            float crc[] = {1, 1, 1, 1,
                           1, 1, 1, 1,
                           1, 1, 1, 1,
                           1, 1, 1, 1};
            for (int i = 0; i < q.size(); i++) {
                float tmp[] = {0, 0, 0, 0,
                               0, 0, 0, 0,
                               0, 0, 0, 0,
                               0, 0, 0, 0};
                tmp[15] = crc[14];
                tmp[1] = crc[0];
                tmp[2] = crc[1];
                tmp[3] = crc[2];
                tmp[4] = crc[3];
                tmp[6] = crc[5];
                tmp[7] = crc[6];
                tmp[8] = crc[7];
                tmp[9] = crc[8];
                tmp[10] = crc[9];
                tmp[11] = crc[10];
                tmp[13] = crc[12];
                tmp[14] = crc[13];
                if (crc[15] == q[i]) {
                    //1 - XOR ==
                    tmp[0] = 0;
                    if (crc[4] == 0) {
                        //2 - XOR ==
                        tmp[5] = 0;
                    } else {
                        //2 - XOR !=
                        tmp[5] = 1;
                    }
                    if (crc[11] == 0) {
                        //3 - XOR ==
                        tmp[12] = 0;
                    } else {
                        //3 - XOR !=
                        tmp[12] = 1;
                    }
                } else {
                    //1 - XOR !=
                    tmp[0] = 1;
                    if (crc[4] == 1) {
                        //2 - XOR ==
                        tmp[5] = 0;
                    } else {
                        //2 - XOR !=
                        tmp[5] = 1;
                    }
                    if (crc[11] == 1) {
                        //3 - XOR ==
                        tmp[12] = 0;
                    } else {
                        //3 - XOR !=
                        tmp[12] = 1;
                    }
                }
                memcpy(crc, tmp, 16 * sizeof(float));
            }
            for (int i = 15; i >= 0; i--)
                q.push_back(!crc[i]);
        }

        /*setting from json*/
        void reader_impl::setting() {
            std::cout << "Setting" << std::endl;
            std::ifstream ifs("../rfid-setting.json");
            if (ifs.is_open()) {
                std::cout << "get setting file" << std::endl;
            } else {
                std::cout << "fail to get setting file" << std::endl;
            }
            //reader_state->reader_stats.max_inventory_round = 10;
            std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
            rapidjson::Document d;
            d.Parse(str.c_str());
            std::cout << str << std::endl;
            const rapidjson::Value &rfid_input = d["RFID-SETTING"];
            // inventory
            reader_state->reader_stats.max_inventory_round = rfid_input.Size();
            std::cout << "inventory round:" << rfid_input.Size() << std::endl;
            for (int i = 0; i < rfid_input.Size(); i++) {
                eachsend send_temp;
                // select set
                if (rfid_input[i].HasMember("Select")) {
                    const rapidjson::Value &select = rfid_input[i]["Select"];
                    std::cout << "select size: " << select.Size() << std::endl;
                    select_command a;
                    for (int j = 0; j < select.Size(); j++) {
                        // each select
                        for (int k = 0; k < select[j]["ACTION"].Size(); k++)
                            a.SELECT_ACTION[k] = select[j]["ACTION"][k].GetInt();
                        for (int k = 0; k < select[j]["SELECT_MEMBANK"].Size(); k++)
                            a.SELECT_MEMBANK[k] = select[j]["SELECT_MEMBANK"][k].GetInt();
                        for (int k = 0; k < select[j]["SELECT_POINTER"].Size(); k++)
                            a.SELECT_POINTER[k] = select[j]["SELECT_POINTER"][k].GetInt();
                        for (int k = 0; k < select[j]["SELECT_LENGTH"].Size(); k++)
                            a.SELECT_LENGTH[k] = select[j]["SELECT_LENGTH"][k].GetInt();
                        a.SELECT_TRUNCATE[0] = select[j]["TRUNCATE"][0].GetInt();
                        for (int k = 0; k < select[j]["SELECT_MASK"].Size(); k++)
                            a.SELECT_MASK.push_back(select[j]["SELECT_MASK"][k].GetInt());
                        send_temp.select_input.push_back(a);
                    }
                } else {
                    std::cout << "wrong json" << std::endl;
                }
                // query set
                const rapidjson::Value &query = rfid_input[i]["Query"];
                send_temp.slotNum = query["Q"].GetInt();
                send.push_back(send_temp);
            }
            std::cout << "Inventory NUM:" << send.size() << std::endl;
        }

        void reader_impl::gen_select_bits(select_command &input) {
            output.resize(0);
            output.insert(output.end(), &SELECT_CODE[0], &SELECT_CODE[4]);
            output.insert(output.end(), &SELECT_TARGET[0], &SELECT_TARGET[3]);
            output.insert(output.end(), &input.SELECT_ACTION[0], &input.SELECT_ACTION[3]);
            output.insert(output.end(), &input.SELECT_MEMBANK[0], &input.SELECT_MEMBANK[2]);
            output.insert(output.end(), &input.SELECT_POINTER[0], &input.SELECT_POINTER[8]);
            output.insert(output.end(), &input.SELECT_LENGTH[0], &input.SELECT_LENGTH[8]);
            output.insert(output.end(), &input.SELECT_MASK[0], &input.SELECT_MASK[input.SELECT_MASK.size()]);
            output.insert(output.end(), &input.SELECT_TRUNCATE[0], &input.SELECT_TRUNCATE[1]);
            crc16_append(output);
        }

        void reader_impl::logfile() {
            rapidjson::Document outfile;
            outfile.SetObject();
            rapidjson::Value in;
            std::stringstream ss;
            ss << all_start.tv_usec;
            //in.SetString(&ss.str(),ss.str().size(),outfile.GetAllocator());

        };

    } /* namespace tash */
} /* namespace gr */

