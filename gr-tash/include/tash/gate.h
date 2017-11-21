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


#ifndef INCLUDED_TASH_GATE_H
#define INCLUDED_TASH_GATE_H

#include <tash/api.h>
#include <gnuradio/block.h>

namespace gr {
    namespace tash {

        /*!
         * \brief The block is responsible for identifying a reader's command
         *
         * The samples related to a reader's command are blocked and consumed.
         * Samples that belong to a Tag's message (RN16-EPC) are forwarded to the next block for further processing.
         * \ingroup rfid
         *
         */
        class TASH_API gate :

        virtual public gr::block {
        public:
        typedef boost::shared_ptr<gate> sptr;

        /*!
         * \brief Return a shared_ptr to a new instance of rfid::gate.
         *
         * To avoid accidental use of raw pointers, rfid::gate's
         * constructor is in a private implementation
         * class. rfid::gate::make is the public interface for
         * creating new instances.
         */
        static sptr make(int sample_rate);

    };

} // namespace tash
} // namespace gr

#endif /* INCLUDED_TASH_GATE_H */

