
/*
 * This file was automatically generated using swig_doc.py.
 *
 * Any changes to it will be lost next time it is regenerated.
 */




%feature("docstring") gr::tash::decoder "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of rfid::tag_decoder.

To avoid accidental use of raw pointers, rfid::tag_decoder's constructor is in a private implementation class. rfid::tag_decoder::make is the public interface for creating new instances.

Args:
    sample_rate : "

%feature("docstring") gr::tash::decoder::make "<+description of block+>

Constructor Specific Documentation:

Return a shared_ptr to a new instance of rfid::tag_decoder.

To avoid accidental use of raw pointers, rfid::tag_decoder's constructor is in a private implementation class. rfid::tag_decoder::make is the public interface for creating new instances.

Args:
    sample_rate : "

%feature("docstring") gr::tash::gate "The block is responsible for identifying a reader's command.

The samples related to a reader's command are blocked and consumed. Samples that belong to a Tag's message (RN16-EPC) are forwarded to the next block for further processing.

Constructor Specific Documentation:

Return a shared_ptr to a new instance of rfid::gate.

To avoid accidental use of raw pointers, rfid::gate's constructor is in a private implementation class. rfid::gate::make is the public interface for creating new instances.

Args:
    sample_rate : "

%feature("docstring") gr::tash::gate::make "The block is responsible for identifying a reader's command.

The samples related to a reader's command are blocked and consumed. Samples that belong to a Tag's message (RN16-EPC) are forwarded to the next block for further processing.

Constructor Specific Documentation:

Return a shared_ptr to a new instance of rfid::gate.

To avoid accidental use of raw pointers, rfid::gate's constructor is in a private implementation class. rfid::gate::make is the public interface for creating new instances.

Args:
    sample_rate : "

%feature("docstring") gr::tash::reader "The block is responsible for sending commands for transmission.

It moves between the following states.

Constructor Specific Documentation:

Return a shared_ptr to a new instance of rfid::reader.

To avoid accidental use of raw pointers, rfid::reader's constructor is in a private implementation class. rfid::reader::make is the public interface for creating new instances.

Args:
    sample_rate : 
    dac_rate : "



%feature("docstring") gr::tash::reader::make "The block is responsible for sending commands for transmission.

It moves between the following states.

Constructor Specific Documentation:

Return a shared_ptr to a new instance of rfid::reader.

To avoid accidental use of raw pointers, rfid::reader's constructor is in a private implementation class. rfid::reader::make is the public interface for creating new instances.

Args:
    sample_rate : 
    dac_rate : "



%feature("docstring") gr::tash::tashconfig "<+description+>"



