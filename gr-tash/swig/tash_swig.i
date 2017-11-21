/* -*- c++ -*- */

#define TASH_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "tash_swig_doc.i"

%{
#include "tash/reader.h"
#include "tash/decoder.h"
#include "tash/gate.h"
#include "tash/tashconfig.h"
%}


%include "tash/reader.h"
GR_SWIG_BLOCK_MAGIC2(tash, reader);
%include "tash/decoder.h"
GR_SWIG_BLOCK_MAGIC2(tash, decoder);
%include "tash/gate.h"
GR_SWIG_BLOCK_MAGIC2(tash, gate);
%include "tash/tashconfig.h"
