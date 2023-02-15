// $Id: count_nip_packets.c 2870 2015-10-23 22:43:11Z amwilder $
//
//  XippMinExample.c
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

#include "xippmin.h"
#include "xippmin_functions.h"

// -- Main Program -- //
int main(int argc, char *argv[])
{
#if defined(_WIN32)
  // cleanup socket resources (Win32)
  WORD wVersionRequested;
  WSADATA wsaData;
  wVersionRequested = MAKEWORD(2, 2);
  int wsaErr = WSAStartup(wVersionRequested, &wsaData);
  if(wsaErr)
  {
      printf("WSAStartup() failed\n");
      return 1;
  }
#endif

    // network connection
    char            buff[UDP_BUFF_BYTE_COUNT];
    int             socketDesc;
    ssize_t         bytesRead;
    int             flags = 0;
    struct sockaddr from;
    int             fromLen = sizeof(from);

    // execution time information
    time_t          timeStart;
    time_t          timeLast;
    time_t          timeNow;

    // instrument network stats
    uint32_t byteCount       = 0;
    uint32_t byteCountLast   = 0;
    uint32_t packetCountUdp  = 0;

    uint32_t packetCountXipp     = 0;
    uint32_t packetCountXippCfg  = 0;
    uint32_t packetCountXippData = 0;

    uint32_t packetCountXippDataMicro   = 0;
    uint32_t packetCountXippDataSeg     = 0;
    uint32_t packetCountXippDataDig     = 0;
    uint32_t packetCountXippDataAnalog  = 0;

    uint32_t unit1Count = 0;
    uint32_t unit2Count = 0;
    uint32_t unit3Count = 0;
    uint32_t unit4Count = 0;

    // controls UDP pacet reading loop
    bool readInstrumentNet = true;

    printf("\n############################################################\n#\n");
    printf("# XIPP Instrument Network - External Integration Example\n#\n");
    printf("#    Source: %s\n", __FILE__);
    printf("#    Author: Andrew Wilder\n");
    printf("#   Contact: support@rppl.com\n");
    printf("#\n############################################################\n\n");

    // wait for user to pres enter
    printf("Begin test program? n|[y]: ");
    fgets(buff, UDP_BUFF_BYTE_COUNT, stdin);
    if(buff[0] != 'n')
    {
        // attempt to set up a socket to listen to the Instrument Network traffic
        printf("Attempting to connect to Instrument Network ... ");
        if( (socketDesc = CreateXippReceivingSocket(INADDR_ANY, XIPP_NET_DACAR_PORT)) > 0)
        {
            printf("done\n\n");

            // get the start time
            timeStart = time(0);
            timeLast  = timeStart;

            printf("XIPP Instrument Network Stats: (Press any key to quit)\n\n");
            printf("  [UDP]   Pkts    BytesRcvd    Mbps      [XIPP Pkts]  Config   Data [ Total     Micro    Spikes  Ch1(  u1    u2    u3    u4 )    Analog    Digital ]\n");
            printf("  --------------------------------------------------------------------------------------------------------------------------------------------------\n");

            // loop to read incoming UDP packets
            do
            {
                // wait for the next UDP packet
                bytesRead = recvfrom(socketDesc,
                                     (void *)buff,
                                     UDP_BUFF_BYTE_COUNT,
                                     flags,
                                     &from,
                                     (socklen_t*)&fromLen);

                // pull the XIPP packets out of the UDP packet
                int byteIdx = 0;
                while(byteIdx < bytesRead)
                {
                    // interpret the next set of bytes as a XippPacket
                    XippPacket * pPacket = (XippPacket *)(&(buff[byteIdx]));

                    // figure out how long this packet is
                    int packetByteCount = pPacket->header.size*4;

                    /////////////////////////////////
                    // parse the XIPP packet
                    /////////////////////////////////

                    // See if packet is from an NIP
                    if(    (pPacket->header.processor == 1) // these packets are from the NIP (i.e. no Trellis)
                        && (pPacket->header.module != 0) )  // packet is not from NIP process module
                    {
                        // Case 1: Data Packet
                        if(pPacket->header.stream != 0)
                        {
                            // interpret the packet as a  XippData Packet
                            XippDataPacket * pDataPacket = (XippDataPacket *)pPacket;

                            // count packets of different stream types
                            if(pDataPacket->streamType == XIPP_STREAM_SEGMENT)
                            {
                                XippSegmentDataPacket * pSegment = (XippSegmentDataPacket *)pDataPacket;

                                ////////////////////////////////////////////
                                // Insert Segment packet handling code here
                                ////////////////////////////////////////////

                                // [Example]
                                // If a micro or nano front end is plugged into position 1 on
                                // port A then the following will count sorted spikes from
                                // the first channel
                                if(    (pPacket->header.module == 2)   // first front end
                                    && (pPacket->header.stream == 4) ) // first spike stream
                                {
                                    switch(pSegment->classID)
                                    {
                                        case 1: unit1Count++; break;
                                        case 2: unit2Count++; break;
                                        case 3: unit3Count++; break;
                                        case 4: unit4Count++; break;
                                    };
                                }
                                packetCountXippDataSeg++;
                            }
                            else if(pDataPacket->streamType == XIPP_STREAM_LEGACY_DIGITAL)
                            {
                                XippLegacyDigitalDataPacket * pDig = (XippLegacyDigitalDataPacket *)pDataPacket;

                                ////////////////////////////////////////////
                                // Insert Digital packet handling code here
                                ////////////////////////////////////////////

                                packetCountXippDataDig++;
                            }
                            else if(pDataPacket->streamType == XIPP_STREAM_CONTINUOUS)
                            {
                                XippContinousDataPacket * pContin = (XippContinousDataPacket *)pDataPacket;

                                // continuous packets can contain either microelectrode data or analog data
                                if(pDataPacket->header.module == 33)
                                {
                                    ////////////////////////////////////////////
                                    // Insert Analog packet handling code here
                                    ////////////////////////////////////////////

                                    packetCountXippDataAnalog++;
                                }
                                else
                                {
                                    ////////////////////////////////////////////////////
                                    // Insert Microelectrode packet handling code here
                                    ////////////////////////////////////////////////////

                                    packetCountXippDataMicro++;
                                }
                            }

                            packetCountXippData++;
                        }

                        // Case 2: Configuration Packet
                        else
                        {
                            packetCountXippCfg++;

                        }
                    }

                    // figure out where the next XIPP packet starts
                    byteIdx += packetByteCount;

                    // track stats
                    packetCountXipp++;
                }

                // track the UDP statistics
                byteCount += (bytesRead > 0) ? bytesRead : 0;
                packetCountUdp++;

                // get current time
                timeNow = time(0);

                // if greater than one second has elapsed print the Network statistics
                double  elapsedSec = (timeNow - timeLast);
                if(elapsedSec > 0)
                {
                    // continue if user has not hit a key
                    readInstrumentNet = !kbhit();
                    if(readInstrumentNet)
                    {
                        uint32_t newBytes = byteCount - byteCountLast;
                        double dataRate = 8.0*((double)newBytes/1000000.0)/elapsedSec;
                        printf("                                                              \r");
                        printf("  %12d%13u%8.2f%25d%15d%10d%10d%10d%6d%6d%6d%12d%11d\r",
                                   packetCountUdp,
                                   byteCount,
                                   dataRate,
                                   packetCountXippCfg,
                                   packetCountXippData,
                                   packetCountXippDataMicro,
                                   packetCountXippDataSeg,
                                   unit1Count,
                                   unit2Count,
                                   unit3Count,
                                   unit4Count,
                                   packetCountXippDataAnalog,
                                   packetCountXippDataDig);

                        fflush(stdout); // flush stdout because we're not writing newlines (only carriage returns)

                        // mark current values for use next time around
                        timeLast      = timeNow;
                        byteCountLast = byteCount;
                    }
                    else
                    {
                        printf("\n\nExiting Program ... goodbye!\n");
                    }
                }
            }
            while( readInstrumentNet );

            // clean up
            close(socketDesc);
        }
    }

    printf("\n\n\n");

#if defined(_WIN32)
    // cleanup socket resources (Win32)
    WSACleanup();
#endif
}
