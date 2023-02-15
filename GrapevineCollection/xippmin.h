// $Id: xippmin.h 3007 2015-11-27 22:05:03Z amwilder $
//
//  xippmin.h
//
//  This header is supplied as a minimum set of data format definitions
//  for accessing data streams sent by Xipp Instruments.
//
//  This header is free software: you can redistribute it and/or modify 
//  it under the terms of the GNU General Public License as published 
//  by the Free Software Foundation, either version 3 of the License, 
//  or (at your option) any later version.  This header is distributed 
//  in the hope that it will be useful, but WITHOUT ANY WARRANTY; and
//  without the implied warranty of MERCHANTABILITY or FITNESS FOR A 
//  PARTICULAR PURPOSE.  
//
//  See the GNU General Public License for more details.
//  If not supplied with this file, see <http://www.gnu.org/licenses/>.
//
//  Copyright © 2012 Ripple, LLC
//  contact: support@rppl.com
//

#ifndef XIPP_MIN_H
#define XIPP_MIN_H

// Use ANSI standard integer definitions and limits
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <limits.h>

// pack data members in structures to byte level 
#pragma pack(push, 1)

// XIPP Protocol Version
static const int XIPP_VERSION_MAJOR = 0; //!< XIPP library major version.
static const int XIPP_VERSION_MINOR = 8; //!< XIPP library minor version.
static const int XIPP_VERSION_PATCH = 2; //!< XIPP library minor version.

////////////////////////////////////////////////////////////////////////////////
// XIPP Network Definitions
////////////////////////////////////////////////////////////////////////////////

// The term "XIPP Instrument Network" refers to a shared medium and protocol (e.g. UDP over
// ethernet) by which data and configuration commands are transmitted between Neural Information
// Processors (NIPs) and PCs.

// A Grapevine is a kind of NIP. For UDP over ethernet networks the Grapevine assigns itself the
// IPv4 network address 192.168.42.1. All data packets and configuration acknowledgement packets
// are broadcast to 192.168.42.255 on a fixed port XIPP_NET_DACAR_PORT (see below).

// To connect a PC to the instrument network, set the network address of the PC's ethernet card to
// 192.168.42.XXX where XXX is in the range [128,254] and not shared by other network adapter on
// the instrument network. To capture XIPP data and configuration packets from the Grapevine NIP,
// open a UDP listening connection on port XIPP_NET_DACAR_PORT of the network card's address.
// (Note: For optimal performance configure the connection buffer to be large - 2MB or more)

// The data section of received UDP packets will contain one or more XIPP packets packed
// sequentially. XIPP packet structure is described in the following sections.

// An instrument's configuration consists of a set of properies. Changes to the configuration can
// be requested by sending XIPP configuration request packets to the instrument. Details of this
// process are beyond the scope of this header

static const char * XIPP_NET_INSTRUMENT_SUBNET            = "192.168.42";
static const char * XIPP_NET_INSTRUMENT_SUBNETMASK        = "255.255.255.0";
static const char * XIPP_NET_INSTRUMENT_INADDR_BROADCAST  = "192.168.42.255";
static const char * XIPP_NET_INSTRUMENT_INADDR_BASE       = "192.168.42.1";


static const uint16_t XIPP_NET_DACAR_PORT = 2046;  // UDP port used for Data And Config Acknowledgment Packets 
static const uint16_t XIPP_NET_CRR_PORT   = 2047;  // UDP port used for Config Request 
static const int XIPP_MAX_UDP_PACKET_SIZE = 1400;  // leave 100 bytes for Ethernet frame header within Ethernet MTU

/////////////////////////////////////////////////////////////////////////////////////////
// XIPP Basic Data Packet Definitions
/////////////////////////////////////////////////////////////////////////////////////////

// Every XIPP packet begins with a XippHeader structure, followed by the packet payload,
// and packet contents are sent in little endian format (same used by Intel processors).
//
// All XIPP packets are a multiples of 32 bits in length.  The size of the packet payload
// in 32-bit units (or DWORDS or "quadlets") is given by the size field of the header.
// Note that a packet may be longer than its contents to provide 32-bit alignment, 
// always determine the length of a packet based on the size field.  The size field can
// also be zero, indicating that the packet consists only of the 8-byte header.
//
// The source of the packet on the instrument network is given by the processor field of
// the header.  A processor ID of 1 - 16 indicates that the packet came from NIP 1-16.
// A processor ID of 0 indicates that the packet originated from a PC on the network.
// Packet requests from PCs are used for queries and change requests of the NIP settings.
// 
// Functional software units within a NIP that interface and process groups of channels
// are called "modules", and they are enumerated from 1 to 254.  Each piece of hardware 
// (front ends) and each processing block for a group of channels within an NIP has a 
// module associated with it.  The module field of the header indicates the ID of the 
// module that published the XIPP packet.  Each processor also contains a module with a
// reserved ID of 0 that provides an interface to the processor itself.
// 
// Each module can publish up to 254 data streams that each contain data derived from one or
// more channels of information handled by the module.  If the stream field of the header
// of the packet is 0, the packet payload contains configuration and settings information about
// the module.  If the stream field is 1 to 254, the packet payload is the latest data
// for the data stream, formatted according the type of the stream.
//
// Module properties and data stream information can be queried and discovered through an 
// exchange of configuration packets with the module (packets with stream ID = 0).  
//
// Alternatively, the enumeration and typing of modules and streams follows conventions 
// for different configurations of front ends connected to the NIP.  This convention can 
// be hard-coded into software on PCs that processes data packets from the NIP.
//
// The convention for the modules and streams for the 0.9 branch of the NIP firmware and 
// Trellis are described in the subsequent data sections.
//
// The time field of the header in the packet has the current value of the free-running
// timer for events recorded by the NIP at the time the packet was sent.  For Grapevine 
// systems the timer increments with the system sampling clock that runs at 30 ksps.
// The timer free-runs from 0 to (2^32 - 1) and rolls over (every ~40 hrs for Grapevine).
// For packets with a series of samples in time (e.g., a spike waveform), the time field
// contains the timer value for the first sample of the series.  For configuration
// packets, the time field has the timer value where the configuration change began.

typedef struct              // General header common to all Xipp Packets
{                           //
    uint8_t   size;         // size of the packet payload section in quadlets (32-bit units)
    uint8_t   processor;    // ID of processor sending the packet (1-16) or 0 if sent from PC 
    uint8_t   module;       // ID of module sending the packet (1-254) or 0 for processor module
    uint8_t   stream;       // ID of data stream on module (1-254) or 0 for configuraton data.
    uint32_t  time;         // value of free running system timer associated with the packet.

} XippHeader;


typedef struct              // generic archetype description for all Xipp Packets
{                           //
    XippHeader header;      // header at the beginning of every packet
    uint8_t    payload[];   // generic declaration of the payload section following the header

} XippPacket;

// The XippTarget and XippConfigPacket structures are used to exchange configuration information.
// They are provided below for general reference, but not discussed in detail in this header.

typedef struct              // sub-header used to specify target of configuration packets
{                           //
    uint8_t  processor;     // ID of processor (1-254) affected/described by configuration packet
    uint8_t  module;        // ID of module (1-254) affected/described by configuration packet
    uint16_t property;      // ID of property (1-65534) affected/described by configuration packet

} XippTarget;

typedef struct             // format of configuration packets (header.stream of packet == 0)
{                           // 
    XippHeader header;      // header describing the source of the packet on the network
    XippTarget target;      // sub-header referencing property being operated on
    uint8_t    config[];    // generic declaration of the configuration data field

} XippConfigPacket;

// 
// To extract data packets from NIP/Trellis running the 0.9 branch, extract all packets 
// with (processor == 1) and (stream > 0) in the header.  Other packets are configuration 
// packets that can be disregarded.
// 
// Data packets from a single stream are always the same type, and packets do not normally
// need to be tested for type.  However, a data type identifier is also included within each 
// packet as a means of error checking and debugging.  The basic XIPP data packet has a
// header and type identifier, followed by data section
// 

static const uint32_t XIPP_STREAM_UNDEFINED      = 0x00; // reserved value for indeterminate types
static const uint32_t XIPP_STREAM_CONTINUOUS     = 0x01; // samples from a group of channels at a single point in time
static const uint32_t XIPP_STREAM_SEGMENT        = 0x02; // sequential set of samples from single channel (eg, spike waveform)
static const uint32_t XIPP_STREAM_DIGEST         = 0x04; // compressed min/max data used for generating raster displays
static const uint32_t XIPP_STREAM_LEGACY_DIGITAL = 0x06; // used to send legacy digital data packets for NEV files

typedef struct
{
    XippHeader header;          // header describing the source of the packet on the network
    uint16_t   streamType;      // one of the XIPP_STREAM_* types
    uint16_t   count;           // running counter (spike streams only)
    uint8_t    data[];          // generic declaration of the stream data

} XippDataPacket;

//
// Received packets are formatted as follows according to type.  Again, the determination
// of the proper structure to read a received packet can be made purely using the module
// and stream ID numbers, the streamType field is not required.
// 

typedef struct
{
    XippHeader header;          // header describing the source of the packet on the network
    uint16_t   streamType;      // set to XIPP_STREAM_CONTINUOUS
    uint16_t   PADDING;         // unused
    int16_t    i16[];           // array of samples in the sample group

} XippContinousDataPacket;

typedef struct
{
    XippHeader header;          // header describing the source of the packet on the network
    uint16_t   streamType;      // set to XIPP_STREAM_SEGMENT
    uint16_t   count;           // running counter (used to detect packet drops)
    uint16_t   classID;         // spike unit class (0 for unsorted, 1-16 for unit 1-16, 255 for noise)
    uint16_t   sampleCnt;       // number of samples in data area of packet
    int16_t    i16[];           // the waveform data 

} XippSegmentDataPacket;

typedef struct
{
    XippHeader header;          // header describing the source of the packet on the network
    uint16_t   streamType;      // set to XIPP_STREAM_LEGACY_DIGITAL
    uint16_t   count;           // running counter (used to detect packet drops)
    uint16_t   changeFlag;      // NEV digital packet change flag field
    uint16_t   parallel;        // value from the parallel port
    uint16_t   event[4];        // value from each of the SMA ports

} XippLegacyDigitalDataPacket;

//
// Two Module IDs are assigned to each electrode front end connected to the NIP. The
// front ends are ordered by 1,2,3,4 in each front end bus and then by bus A,B,C,D. The first
// front end has module IDs 1 and 2, the second has IDs 3 and 4, and so on.  The first module
// represents the front end itself, and the second module represents some form of processing
// applied to the input signals. For example, the second module for a micro front end outputs
// spike and LFP streams. The modules for a front end process the signals from that front end
// only.
//
// Streams on the NIP can be enabled and disabled. Disabled streams are not broadcast on the
// Instrument Network.
// 
// [Raw Signals]
// If enabled, the raw unfiltered 30ksps data from a front end are published on the
// first module ID for the front end (header.module == ((2 * BaseZeroFrontEndNumber) + 1))
// using stream ID 1 (header.stream == 1) and formatted as a XippContinuousDataPacket with
// 32 raw sample values at time (header.time) in the i16[] member array.
// 
// [Processed Signals]
// If enabled, the LFP data for each front end are subsampled to 1 ksps (packet sent every 
// even divisor of 30 samples as numbered in the time field of the header).  These are sent
// on the second front end module ID (header.module == ((2 * BaseZeroFrontEndNumber) + 2)).
// For micro, nano, and surfD fron ends, LFP data are published on stream ID 1 (header.stream == 1)
// and formatted as a XippContinuousDataPacket with 32 LFP sample values at time (header.time)
// in the i16[] member array.
//
// For micro, nano and surfD front ends a float representation of the low pass filtered data
// is sent using stream ID 2 on the second module ID assigned for each front end.  These should
// be disregarded.
//
// For micro and nano front ends compressed stream information for rendering raster plots is
// sent using stream ID 3 on the second module ID assigned for each front end.  These should
// be disregarded.
// 
// If enabled, spikes from one of the 32 channels on a micro or nano front end are published on
// the second module ID for the front end (header.module == ((2 * BaseZeroFrontEndNumber) + 2))
// using stream IDs 4 to 35 (header.stream == 4 to 35) and formatted as XippSegmentDataPacket
// with the header.time field corresponding to the time of the first sample in the packet.
// The number samples in the waveform are in sampleCnt, and the samples are in i16[]. 
//
// If an analog I/O front end is present, it is assigned module ID 33.  Its input samples
// are published as packets with (header.module == 33). A downsampled (1 ksamp/sec) version
// of the inputs is broadcast on stream ID 1 (header.stream == 1). The full 30 ksamp/sec
// version of the inputs is broadcast on stream ID 2 (header.stream == 2). Both of these streams
// are formatted as XippContinousDataPacket.  The i16[] field contains the samples from the
// 4 SMA ports, the 24 microDB connector ports, and the audio input ports. 
// 
// if a digital I/O front end is present, it is assigned module ID 34.  Its input data
// are published on packets with (header.module == 34) and (header.stream == 1) and
// formatted as XippLegacyDigitalDataPacket.  
// 


/////////////////////////////////////////////////////////////////////////////////////////
// Additional XIPP processor, module, and stream range limit constants
/////////////////////////////////////////////////////////////////////////////////////////

static const uint32_t XIPP_MAX_NUM_PROCCESSORS              = 255;
static const uint32_t XIPP_MAX_NUM_MODULES_PER_PROCESSOR    = 255;
static const uint32_t XIPP_MAX_NUM_PROPERTIES_PER_MODULE    = 65535;
static const uint32_t XIPP_MAX_NUM_OUTPUTSTREAMS_PER_MODULE = 255;
static const uint32_t XIPP_MAX_NUM_MODULES_TOTAL            = 255 * 255;//XIPP_MAX_NUM_PROCCESSORS * XIPP_MAX_NUM_MODULES_PER_PROCESSOR;

static const uint8_t  XIPP_NET_PROCESSOR_RANGE_MIN          = 1;
static const uint8_t  XIPP_NET_PROCESSOR_RANGE_MAX          = 254;
static const uint8_t  XIPP_NET_OPERATOR_RANGE_MIN           = 1;
static const uint8_t  XIPP_NET_OPERATOR_RANGE_MAX           = 254;
#define XIPP_NET_PROCESSOR_RANGE                            (XIPP_NET_PROCESSOR_RANGE_MIN), (XIPP_NET_PROCESSOR_RANGE_MAX)
#define XIPP_NET_OPERATOR_RANGE                             (XIPP_NET_OPERATOR_RANGE_MIN), (XIPP_NET_OPERATOR_RANGE_MAX)

// Special header field values
static const uint8_t  XIPP_PROCESSOR_ID_ROOT                = 0;
static const uint8_t  XIPP_PROCESSOR_ID_MASTER              = 1;
static const uint8_t  XIPP_PROCESSOR_ID_ALL                 = 255;
static const uint8_t  XIPP_MODULE_ID_ALL                    = 255;
static const uint8_t  XIPP_MODULE_ID_PROCESSOR              = 0;
static const uint16_t XIPP_PROPERTY_ID_MODULE_DESC          = 0;
static const uint16_t XIPP_PROPERTY_ID_PROPERTY_BLOCK       = 1;
static const uint16_t XIPP_PROPERTY_ID_ALL                  = 65535;
static const uint16_t XIPP_PROPERTY_ID_APP                  = 65535;
static const uint16_t XIPP_OUTSTREAM_ID_CONFIG              = 0;
static const uint16_t XIPP_OUTSTREAM_ID_XML                 = 255;
static const uint16_t XIPP_OPERATOR_ID_ALL                  = 255;

// default sample count for spike event segements
static const uint32_t XIPP_SPIKE_WAVEFORM_SAMPLE_COUNT = 52;

// char array lengths for various fields (all lengths include null termination character).
#if defined(__cplusplus)
  static const uint32_t XIPP_STRLEN_LABEL = 128;
#else
  #define XIPP_STRLEN_LABEL 128
#endif

// To set the output on the digital I/O -
//
// Send a UDP packet to address XIPP_NET_INSTRUMENT_INADDR_BASE, port XIPP_NET_CRR_PORT
// with a single with a XippConfigPacket structure in it as follows
//
// XippConfigPacket cfgPkt;
// cfgPkt.header.processor      = 0;
// cfgPkt.header.module         = 0;
// cfgPkt.header.stream         = 0;
// cfgPkt.header.time           = 0;
// cfgPkt.target.processorID    = GVPRCID;
// cfgPkt.target.moduleID       = GVMID_DIGITAL;
// cfgPkt.target.propertyID     = GVPID_DIGITAL_OUT;
// ((XippInt *)(cfgPkt.config))->value = <NewValue>;
//
// Examples,
//
// To hold Out 1 low,  <NewValue> = GVDIO_HOLD_OUT1;
// To hold Out 1 high, <NewValue> = GVDIO_HOLD_OUT1 + GVDIO_SET_OUT1;
// To hold Out 1 and 2 low, <NewValue> = GVDIO_HOLD_OUT1 + GVDIO_HOLD_OUT2;
// To hold the parallel port at 0x1234, <NewValue> = GVDIO_HOLD_PARALLEL + 0x1234;
// To release all value holds, <NewValue> = 0;
//
// To hold Out 1 low, Out 2 high, Parallel port at 0xCAFE,
// <NewValue> = GVDIO_HOLD_OUT1 + GVDIO_HOLD_OUT2 + GVDIO_SET_OUT2 + GVDIO_HOLD_PARALLEL + 0xCAFE;
//

#define GVPRCID             1
#define GVMID_DIGITAL       34
#define GVPID_DIGITAL_OUT   53

#define GVDIO_HOLD_OUT1     0x10000000
#define GVDIO_HOLD_OUT2     0x20000000
#define GVDIO_HOLD_OUT3     0x40000000
#define GVDIO_HOLD_OUT4     0x80000000
#define GVDIO_HOLD_PARALLEL 0x0F000000

#define GVDIO_SET_OUT1      0x00010000
#define GVDIO_SET_OUT2      0x00020000
#define GVDIO_SET_OUT3      0x00040000
#define GVDIO_SET_OUT4      0x00080000
#define GVDIO_SET_PARALLEL  0x0000FFFF

//! \brief header information for xipp properties allows for typing and for general purpose flags
typedef struct 
{
    uint8_t  vendor;
    uint8_t  type;
    uint16_t flags;        //!< example: editable
} XippPropertyHeader;


static const uint8_t XIPP_TYPE_BOOL        = 3;
static const uint8_t XIPP_TYPE_INT         = 4;
static const uint8_t XIPP_TYPE_STRING      = 6;
static const uint8_t XIPP_TYPE_UNSPECIFIED = UINT8_MAX;

/*! \brief Contains bool information
 */
typedef struct
{
  XippPropertyHeader  header;
  uint8_t value;
  uint8_t def;
} XippBool;

//! \brief describes aspects of int value
typedef struct 
{
  XippPropertyHeader header;

  int32_t value;
  int32_t min;
  int32_t max;
  int32_t def;
} XippInt; 

/*! \brief Contains a string and length information
 */
typedef struct
{
  XippPropertyHeader  header;
  uint16_t maxLength;  //! max string length supported by this property
  uint16_t length;     //! current length of string
  char     value[]; //! NULL terminated ASCII string
} XippString;

#pragma pack(pop)

#endif // XIPP_MIN_H
