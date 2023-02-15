// $Id: xippmin_functions.h 3007 2015-11-27 22:05:03Z amwilder $
//
//  xippmin_funcions.h
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

#ifndef XIPPMINFUNCTIONS_H
#define XIPPMINFUNCTIONS_H

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#if defined(_WIN32)
  # include <winsock2.h>
  typedef int socklen_t;
  typedef const char * sockOptSetValPtr_t;
  typedef char * sockOptGetValPtr_t;
#else
  #include <sys/types.h>
  #include <sys/time.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  #include <ctype.h>
  typedef int * sockOptSetValPtr_t;
  typedef int * sockOptGetValPtr_t;
#endif

// set up some convenience types
#if !defined(__cplusplus)
    typedef int bool; // not defined in c
    #define true  1
    #define false 0
#endif

#include "xippmin.h"
#include "XippOperatorTypes.h"

// various constants
static const int UDP_BUFF_BYTE_COUNT = 1500;
static const int XIPP_UDP_RCVBUF_SIZE_BYTES = 2000000;

/**
    Creates a socket configured for broadcasting IP/UDP packets

    \return socket resource id
  */
int
CreateXippSendingSocket()
{
    int outSocket = 0;

    // set up the sending socket
    if ( (outSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 )
    {
        printf("ERROR: could not initialize the sending socket.\n");
        return 0;
    }

    int optionValue = 1;

    // the sending socket must be configured for broadcasting
    if( setsockopt(outSocket,
                   SOL_SOCKET,
                   SO_BROADCAST,
                   (sockOptSetValPtr_t)(&optionValue),
                   (socklen_t)(sizeof(optionValue))) == -1 )
    {
        printf("ERROR: could not configure the socket for broadcasting\n");
        return 0;
    }

    return outSocket;
}

/**
    Creats a socket for receiving IP/UDP packets, binds to it and then
    sets the receive buffer size.

    \arg host - IP4 address in compact form
    \arg port - port that this socket should be bound to
  */
int
CreateXippReceivingSocket(uint32_t host, uint16_t port)
{
    struct sockaddr_in addr;
    int                sockDesc;

    // setup the parameters of the address we will bind this socket to
    memset((char *) &addr, 0, sizeof(addr));
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(port);
    addr.sin_addr.s_addr = host;

    // get a socket
    if ( (sockDesc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1 )
    {
        /// [AMW-2015/10/22] win only
        int errCode = 0;
#if defined(__MACH__) || defined(__linux__) || defined(__unix__)
        errCode = 0;
#else
        errCode = WSAGetLastError();
#endif
        int errOptLen = sizeof(errCode);
        int errOptVal = 0;
        int errOk = getsockopt(sockDesc, SOL_SOCKET, SO_RCVBUF, (sockOptGetValPtr_t)(&errOptVal), (socklen_t *)(&errOptLen));

        printf("ERROR: failed to create a UDP socket. err[%d]\n", errCode);
        if(errOk == -1) {
          printf("Error code could not be recovered.\n");
        }
        return 0;
    }

    // set the receive buffer size
    int receiveBuffSizeInBytes = XIPP_UDP_RCVBUF_SIZE_BYTES;
    if( setsockopt(sockDesc,
                   SOL_SOCKET,
                   SO_RCVBUF,
                   (sockOptSetValPtr_t)(&receiveBuffSizeInBytes),
                   (socklen_t)(sizeof(receiveBuffSizeInBytes))) == -1)
    {
        printf("ERROR: could not set the receive buffer size to [%d] bytes\n", receiveBuffSizeInBytes);
        close(sockDesc);
        return 0;
    }

    // get the size that the buffer is now
    int optLen = sizeof(receiveBuffSizeInBytes);
    int optVal = 0;
    if( getsockopt(sockDesc,
                   SOL_SOCKET,
                   SO_RCVBUF,
                   (sockOptGetValPtr_t)(&optVal),
                   (socklen_t *)(&optLen)) == -1)
    {
        printf("ERROR: could not read the receive buffer size\n");
        close(sockDesc);
        return 0;
    }

    // check that the buffer is actually the right size
    if(receiveBuffSizeInBytes != XIPP_UDP_RCVBUF_SIZE_BYTES)
    {
        printf("ERROR: receive buffer size is incorrect expected[%d] actual[%d]\n", XIPP_UDP_RCVBUF_SIZE_BYTES, receiveBuffSizeInBytes);
        close(sockDesc);
        return 0;
    }

    // the receive socket must be configured for multiple users
    optVal = 1;
    if( setsockopt(sockDesc,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   (sockOptSetValPtr_t)(&optVal),
                   (socklen_t)(sizeof(optVal))) == -1 )
    {
        printf("ERROR: could not configure the socket for multiple users\n");
        return 0;
    }

    // see if the socket is configured for multiple users
    optLen = sizeof(optVal);
    optVal = -1;
    if( getsockopt(sockDesc,
                   SOL_SOCKET,
                   SO_REUSEADDR,
                   (sockOptGetValPtr_t)(&optVal),
                   (socklen_t *)(&optLen)) == -1)
    {
        printf("ERROR: could not read the SO_REUSEADDR socket option\n");
        close(sockDesc);
        return 0;
    }

    // check that the buffer is actually the right size
    if(optVal <= 0)
    {
        printf("ERROR: SO_REUSEADDR is not set. value [%d]", optVal);
        close(sockDesc);
        return 0;
    }

    // attempt to bind to the socket
    if( bind(sockDesc, (struct sockaddr *)&addr, sizeof(addr)) != 0 )
    {
        printf("ERROR: failed to bind to the UDP socket to address[%d] port[%d].", host, port);
        close(sockDesc);
        return 0;
    }

    printf("leaving [%s]",__func__);
    return sockDesc;
}


/**
    Sends a XippConfigPacket to the specific target using the specified socket.
    Tries three times before giving up.

    \arg pConfigPkt - pointer to the XippConfigPacket that is to be sent
    \arg socketDesc - the socket that is to be used to do the sending
    \arg pTarget    - pointer to struct containing information on where to send the packet
    \arg targetLen  - size in bytes of the pTarget structure

    \return true if packet is succesfully sent else false
  */
bool
SendXippConfigPacket(XippConfigPacket * pConfigPkt,
                     int                socketDesc,
                     struct sockaddr *  pTarget,
                     int                targetLen)
{
    if(!pConfigPkt)
        return false;

    // send the config packet
    int sendCount     = 0;
    int pktByteCount  = pConfigPkt->header.size*4;
    int sentByteCount = 0;
    while(    (sentByteCount = sendto(socketDesc,
                                      (const void *)pConfigPkt,
                                      pktByteCount,
                                      0,
                                      pTarget,
                                      targetLen)) != pktByteCount
           && (sendCount < 3) ) // try three times then give up
    {
        printf("\nSent [%d] bytes", sentByteCount);
        sendCount++;
    }

    // quit if query was not successfully sent
    if(sentByteCount != pktByteCount)
    {
        printf("\nERROR: could not send query packet!\n\n");
        return false;
    }

    return true;
}

/**
    Convenience function for repurposing config acknowledgment XippConfigPackets for
    use as anonymous config request packets.

    \arg pCfgPkt - pointer to the XippConfigPacket that is to be repurposed
  */
void
ResetConfigPacketHeaderToAnonymous(XippConfigPacket * pCfgPkt)
{
    if(!pCfgPkt)
        return;

    // overwrite the from info in the header because we will use
    // this packet to make configuration requests and it must be from an
    // apropriate config reqester (i.e. an operator - anonymous or named)
    pCfgPkt->header.processor = 0;
    pCfgPkt->header.module    = 0;
    pCfgPkt->header.stream    = 0;
    pCfgPkt->header.time      = 0; // time is not necessary for config requests
}

/**
    Convenience function for printing out the fields of a
    XippRecordingTrialDescriptor struct.

    \arg pTrial - the trial to be printed
  */
void
PrintRecordingTrial(XippRecordingTrialDescriptor * pTrial)
{
    if(!pTrial)
        return;

    char statusStr[128];
         if(pTrial->status == RECORDING_TRIAL_STATUS_PAUSED)            strcpy(statusStr, "Paused");
    else if(pTrial->status == RECORDING_TRIAL_STATUS_RECORDING)         strcpy(statusStr, "Recording");
    else if(pTrial->status == RECORDING_TRIAL_STATUS_STOP_REQUESTED)    strcpy(statusStr, "Stopping...");
    else if(pTrial->status == RECORDING_TRIAL_STATUS_START_REQUESTED)   strcpy(statusStr, "Starting...");
    else if(pTrial->status == RECORDING_TRIAL_STATUS_PAUSE_REQUESTED)   strcpy(statusStr, "Pausing...");
    else if(pTrial->status == RECORDING_TRIAL_STATUS_UNPAUSE_REQUESTED) strcpy(statusStr, "Unpausing...");
    else                                                                strcpy(statusStr, "Stopped");

    printf("\n Recording Trial Info\n");
    printf(" -------------------------------------------------------\n");
    printf("                 File Format: %d\n",   pTrial->format);
    printf("          Auto Timeout (sec): %d\n",   pTrial->autoStopTime);

    printf("              Auto Increment: %s\n",   pTrial->autoIncrEnabled ? "Yes" : "No");
    printf("        Auto Increment Value: %04d\n", pTrial->autoIncrNumber);

    printf("   Extended Trial Info Block: %d\n",   pTrial->extInfoBlock);
    printf("            File Names Block: %d\n",   pTrial->fileNamesBlock);
    printf("      Signal Selection Block: %d\n",   pTrial->sigSelectionBlock);

    printf("              File Path Base: %s\n",   pTrial->filePathBase);

    printf("                        - - - - -\n");

    printf("          Elapsed Time (sec): %.2f\n", (pTrial->trialElapsed)/1000.0F);
    printf("              File Size (MB): %.2f\n", (pTrial->trialSize)/1000000.0F);

    printf("                        - - - - -\n");

    printf("               Segment Count: %d\n",   pTrial->segCnt);
    printf("  Segment Elapsed Time (sec): %.2f\n", (pTrial->segElapsed)/1000.0F);
    printf("           Segment Size (MB): %.2f\n", (pTrial->segSize)/1000000.0F);

    printf("                        - - - - -\n");

    printf("                      Status: %s\n",   statusStr);

    printf(" -------------------------------------------------------\n");
}

/**
    Convenience function for printing out the fields of a
    XippPropertyBlock struct.

    \arg blockIdx  - index of the property block (among all property blocks)
    \arg pTrial    - the block to be printed
    \arg pItemPkts - the block items
  */
void
PrintPropertyBlock(int blockIdx, XippPropertyBlock * pBlock, XippConfigPacket ** pItemPkts)
{
    if(!pBlock)
        return;

    printf("   %s\n", pBlock->description);
    printf("   -------------------------------------\n");

    if(pItemPkts)
    {
        if(pBlock->count == 0)
            printf("     0 items\n");

        int i;
        for(i=0; i<pBlock->count; ++i)
        {
            if(!pItemPkts[i])
            {
                printf("     item[%d] is invalid\n", i+1);
                continue;
            }

            XippPropertyHeader * pHdr = (XippPropertyHeader *)(pItemPkts[i]->config);

            // [Case 1] - item is a XippRecordedSignalTypeDescriptor
            if( pHdr->type == TRELLIS_TYPE_GV_FE_SIGNAL_SELECTION_DESC )
            {
/// [AMW-2015/10/22] fix this for the new signal selection descriptor
//                XippSignalSelectionDescriptor * pSelectionDesc =
//                        (XippSignalSelectionDescriptor *)(pItemPkts[i]->config);
//
//                uint32_t guid = pSelectionDesc->guid;
//
//                // turn the GUID into a string
//                char guidStr[16];
//                sprintf(guidStr+0,  "%03d.", (guid&0xFF000000)>>24 );
//                sprintf(guidStr+4,  "%03d.", (guid&0x00FF0000)>>16 );
//                sprintf(guidStr+8,  "%03d.", (guid&0x0000FF00)>>8 );
//                sprintf(guidStr+12, "%03d",  (guid&0x000000FF) );
//                guidStr[15] = '\0';
//
//                printf("     %03d - GUID:%s [%s]\n",  i+1,
//                                                      guidStr,
//                                                     !pSelectionDesc->available ? "Unavailable"
//                                                                                : (pSelectionDesc->selected ? "Selected"
//                                                                                                            : "Unselected") );
            }

            // [Case 2] - item is a XippString
            else if( pHdr->type == XIPP_TYPE_STRING )
            {
                XippString * pString = (XippString *)(pItemPkts[i]->config);

                // HACK!!! using this to distinguish between two different blocks on the
                // XippRecordingTrialDescriptor (i.e. extendedInfoBlock and selectedSignalTypesBlock)
                if(blockIdx == 1)
                    printf("     Comment: %s\n", pString->value);
                else
                    printf("     File [%d]: %s\n", i+1, pString->value);
            }

            // [Default] is to print the item number and the property type
            else
            {
                printf("     [Item %d] type[%d]\n", i+1, pHdr->type);
            }
        }
    }
    else
    {
        printf("   0 of the %d Items in this block were retrieved\n", pBlock->count);
    }

    printf("   -------------------------------------\n");

}


/**
    Detects if any key has been pressed

    \return 1 if a keypress is detected else 0
  */
int
kbhit()
{
#if defined(_WIN32)
    return _kbhit();
#else
    struct timeval tv = { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
#endif
}

#define NS_INADDRSZ  4
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ   2

/*! \brief Parses an IP4 address string of form xxx.xxx.xxx.xxx and writes the information into the
 *  destination pointer (assumed to be a in_addr structure)
 */
int inet_pton4(const char *src, char *dst)
{
    uint8_t tmp[NS_INADDRSZ], *tp;

    int saw_digit = 0;
    int octets = 0;
    *(tp = tmp) = 0;

    int ch;
    while ((ch = *src++) != '\0')
    {
        if (ch >= '0' && ch <= '9')
        {
            uint32_t n = *tp * 10 + (ch - '0');

            if (saw_digit && *tp == 0)
                return 0;

            if (n > 255)
                return 0;

            *tp = n;
            if (!saw_digit)
            {
                if (++octets > 4)
                    return 0;
                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
                return 0;
            *++tp = 0;
            saw_digit = 0;
        }
        else
            return 0;
    }
    if (octets < 4)
        return 0;

    memcpy(dst, tmp, NS_INADDRSZ);

    return 1;
}
#endif // XIPPMINFUNCTIONS_H
