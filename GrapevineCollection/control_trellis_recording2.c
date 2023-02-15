// $Id: control_trellis_recording.c 3595 2016-04-29 15:07:33Z amwilder $
//
//  XippRemoteRecordingControlExample.c
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

//
// uncomment the following enable out debugging output
//
//#define VERBOSE

#define STDIN_BUFF_BYTE_COUNT     4096
#define SELECTION_DESC_COUNT_MAX  2048

typedef XippConfigPacket *  XippConfigPktPtr;

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
    char                 udpBuff[UDP_BUFF_BYTE_COUNT];
    char                 stdinBuff[STDIN_BUFF_BYTE_COUNT];
    char                 fileDirPath[512];
    char                 file[100];
    int                  inSocket;
    int                  outSocket;
    ssize_t              bytesRead;
    uint32_t             byteCount      = 0;
    uint32_t             packetCountUdp = 0;
    int                  flags = 0;
    struct  sockaddr     from;
    struct  sockaddr_in  target;
    struct  sockaddr *   pTarget    = (struct sockaddr *)&target;
    int                  fromLen    = sizeof(from);
    int                  targetLen  = sizeof(target);

    // execution time information
    time_t          timeStart;
    time_t          timeLast;
    time_t          timeNow;

    // flags for detecting state
    bool operatorQueried  = false;
    bool operatorDetected = false;

    bool operatorQueryInProgress = false;
    bool trialQueryComplete      = true;
    bool trialDescriptorReceived = false;

    bool extInfBlkQueried        = false;
    bool extInfBlkItmsQueried    = false;
    bool sigSlctnsBlkQueried     = false;
    bool sigSlctnsBlkItmsQueried = false;
    bool fileNameBlkQueried      = false;
    bool fileNameBlkItmsQueried  = false;

    int  trialBlocksReceived     = 0;
    int  trialBlockItemsReceived = 0;
    bool trialConfigInProgress   = false;
    bool trialInfoValid          = false;

    bool userPromptedForTrialQuery  = false;
    bool userPromptedForTrialAction = false;

    int recordingRequested = 0;

    // Operator Descriptor
    char opDescPktRaw[1024];
    XippConfigPacket *pOpDescPkt = (XippConfigPacket*)opDescPktRaw;
    XippOperatorProcessDesc *pOpDesc = (XippOperatorProcessDesc*)(pOpDescPkt->config);

    // Recording Trial Descriptor
    char trialDescPktRaw[1024];
    XippConfigPacket *pTrialDescPkt = (XippConfigPacket*)trialDescPktRaw;
    XippRecordingTrialDescriptor *pTrial = (XippRecordingTrialDescriptor*)(pTrialDescPkt->config);

    // Recording Trial Extended Info Block
    char extInfBlkPktRaw[1024];
    XippConfigPacket *pExtInfBlkPkt = NULL;
    XippPropertyBlock *pExtInfBlock = NULL;

    // Recording Trial Selected SignalTypes Block
    char sigSlctnDescBlockPktRaw[1024];
    XippConfigPacket *pSigSlctnDescBlockPkt = NULL;
    XippPropertyBlock *pSigSlctnDescBlock = NULL;

    // Recording Trial File Names Block
    char fileNamesBlkPktRaw[1024];
    XippConfigPacket *pFileNamesBlkPkt = NULL;
    XippPropertyBlock *pFileNamesBlk = NULL;

    // array of pointers to xipp packets that store recording trial extended info
    XippConfigPacket ** pExtInfBlkItems = NULL;

    // array of pointers to xipp packets that store signal selection properties
    XippConfigPacket * pSigSlctnDescBlkPkts[SELECTION_DESC_COUNT_MAX];
    memset((void *)pSigSlctnDescBlkPkts, 0, SELECTION_DESC_COUNT_MAX * sizeof(XippConfigPacket *));

    // array of pointers to xipp packets that store names of files generated in the
    // recording trial. NOTE: for NevNsx2.2 there are currently only 6 kinds of files
    // that can be generated duting a recording trial.
    XippConfigPacket * pFileNamesBlkItems[6];
    memset((void *)pFileNamesBlkItems, 0, sizeof(pFileNamesBlkItems));

    // Query Packet
    //
    // This just a XippConfigPacket without any config. It can be used to query a property.
    // That is, if the module and property fields specify one of its properties, an operator,
    // upon receiving this packet will broadcast the property.
    XippConfigPacket xippQueryPkt;

    // populate query packe with initial values
    xippQueryPkt.header.processor = 0;                                      // indicates that this query is from an anonymous agent
    xippQueryPkt.header.module    = 0;                                      // indicates that this query is from an anonymous agent
    xippQueryPkt.header.stream    = 0;                                      // indicates that this is a config query
    xippQueryPkt.header.size      = 12/4;                                   // packet consists of only header (i.e. no payload)
    xippQueryPkt.header.time      = 0;                                      // not necessary in this context
    xippQueryPkt.target.processor = XIPP_OPERATOR_ALL;                      // target query to all operators
    xippQueryPkt.target.module    = XIPP_OPERATOR_PROCESS_MAIN;             // target query to the main operator process
    xippQueryPkt.target.property  = OPERATOR_PROPERTY_PROCESS_DESCRIPTOR;   // target the process descriptor property

    // controls UDP pacet reading loop
    bool readNetwork = true;

    printf("\n##############################################################\n#\n");
    printf("# XIPP Instrument Network - External Recording Control Example\n#\n");
    printf("#    Source: %s\n", __FILE__);
    printf("#    Author: Andrew Wilder\n");
    printf("#   Contact: support@rppl.com\n");
    printf("#\n##############################################################\n\n");

    // wait for user to pres enter
    printf("Begin test program? n|[y]: ");
    fgets(stdinBuff, STDIN_BUFF_BYTE_COUNT, stdin);
    if(stdinBuff[0] != 'n')
    {
        bool pathEntered = false;
        // get the directory the user wants to save files to
        do
        {
            /*printf("\nEnter the filename:\n");
            fgets(stdinBuff, STDIN_BUFF_BYTE_COUNT, stdin);

            // check input
            if(strlen(stdinBuff) <= 0)
            {
              printf("Invalid input.");
              continue;
            }
            strcpy(file, stdinBuff);*/

            printf("\nEnter the directory on the remote machine where Trellis should save files:\n");
            fgets(stdinBuff, STDIN_BUFF_BYTE_COUNT, stdin);

            // check input
            if(strlen(stdinBuff) <= 0)
            {
              printf("Invalid input.");
              continue;
            }
            strcpy(fileDirPath, stdinBuff);

            printf("\nYou have entered: %sIs that correct? n|[y]", fileDirPath);
            fgets(stdinBuff, STDIN_BUFF_BYTE_COUNT, stdin);
            pathEntered = stdinBuff[0] != 'n';
            //

        }
        while(!pathEntered);

        bool networkInitialized = true;

        // set up the sending socket
        if( (outSocket = CreateXippSendingSocket()) == 0 )
            networkInitialized = false;

        // initialize the broadcast address
        memset((char *) &target, 0, sizeof(target));
        target.sin_family = AF_INET;
        target.sin_port   = htons(XIPP_NET_CRR_PORT);
#if defined(_WIN32)
        if( inet_pton4(XIPP_NET_INSTRUMENT_INADDR_BROADCAST, (char *)&(target.sin_addr)) == 0 )
#else
        if( inet_aton(XIPP_NET_INSTRUMENT_INADDR_BROADCAST, &(target.sin_addr)) == 0 )
#endif
        {
            printf("ERROR: could not initialize broadcast address!\n");
            networkInitialized = false;
        }

        // target initialized successfully so set up listening socket
        if(networkInitialized)
        {
            // set up a socket to listen to the Instrument Network traffic
            printf("Attempting to connect to Instrument Network ... ");
            if( (inSocket = CreateXippReceivingSocket(INADDR_ANY, XIPP_NET_DACAR_PORT)) > 0)
            {
                printf("done\n\n");

                // get the start time
                timeStart = time(0);
                timeLast  = timeStart;

                printf("Type x+ENTER to quit\n\n");

                /////////////////////////////////////////////////
                //
                //  This is the main execution loop. It does 4 things
                //
                //  1 - Process User Keyboard Input
                //  2 - Test if Trial Configuration is Complete
                //  3 - Figure out The Next Action
                //  4 - Process All New UDP Packets
                //
                /////////////////////////////////////////////////
                do
                {
                    // get current time
                    timeNow = time(0);

                    ///////////////////////////////////////////
                    // [Step 1] - Process User Keyboard Input
                    ///////////////////////////////////////////

                    // if greater than one second has elapsed prompt the user for an action
                    double  elapsedSec = (timeNow - timeLast);
                    if(elapsedSec > 0)
                    {
                        // check for keyborad hit
                        bool userInputPresent = (kbhit() == 0) ? false : true;
                        if(userInputPresent)
                        {
                            char * status = fgets(stdinBuff, sizeof(stdinBuff), stdin);
                            if(status > 0)
                            {

#if defined(VERBOSE)
                                printf("\nyou typed [%s]\n", stdinBuff);
#endif

                                // User wants to exit the program
                                if(stdinBuff[0] == 'x')
                                {
                                    break;
                                }

                                // User has entered some text that we need to process
                                else
                                {
#if defined(VERBOSE)
                                    printf("Processing input...\n");
#endif
                                    // user is responding for request to start/pause/stop recording
                                    if(userPromptedForTrialAction)
                                    {
#if defined(VERBOSE)
                                        printf("Processing response for trial ACTION...\n");
#endif
                                        // tracks what the query state flags should be set to
                                        bool executeFullTrialQueryWithConfig = true;
                                        trialConfigInProgress = true;

                                        char actionStr[128]; // indicates what action is being taken

                                        // The default packet we are about to send is a config packet (i.e. not a query packet)
                                        XippConfigPacket *pPkt = pTrialDescPkt;

                                        //
                                        // figure out what action to take
                                        //

                                        // figure out what to do
                                        char c = stdinBuff[0];
                                        int  stdinLen=0;
                                        bool isNumber = isdigit(c);
//                                        while( ((c = stdinBuff[i++]) != '\n') && (isNumber = isdigit(c)) ) { }
                                        while(c != '\n') {
                                          stdinLen++;
                                          c = stdinBuff[stdinLen];
                                          isNumber = isdigit(c) && isNumber;
                                        }
#if defined(VERBOSE)
                                        printf("STDIN input string len[%d]\n", stdinLen);
#endif
                                        bool executeAction = false;
                                        // Case 1: User entered a comment
                                        if(
                                               (stdinBuff[1] != '\n') // more than just one character
                                            && !isNumber
                                            &&  ( (pTrial->status == RECORDING_TRIAL_STATUS_STOPPED)
                                               || (pTrial->status == RECORDING_TRIAL_STATUS_STOP_REQUESTED) ) )
                                        {
                                            XippString *pComment = (XippString *)(pExtInfBlkItems[0]->config);
#if defined(VERBOSE)
                                            printf("Handling comment...\n");
                                            printf("Current comment length is[%d]\n", pComment->length);
#endif
                                            // remove return character from comment
                                            char comment[256];
                                            memcpy(comment, stdinBuff, stdinLen);
                                            int len = (stdinLen < pComment->maxLength) ? stdinLen : pComment->maxLength;
#if defined(VERBOSE)
                                            printf("Copying the stdin comment[%s] with len[%d] to"
                                                    " field with len[%d] using len[%d]",
                                                    comment, stdinLen, pComment->maxLength, len);
#endif
                                            memcpy((void *)(pComment->value), (const void *)comment, len);
                                            pComment->length = len;

#if defined(VERBOSE)
                                            char tmpStr[128];
                                            sprintf(tmpStr, "ADD [%d char] COMMENT [%s] TO", (int)strlen(pComment->value), pComment->value);
                                            strcpy(actionStr, tmpStr);
#else
                                            strcpy(actionStr, "ADD COMMENT TO");
#endif
                                            pPkt = pExtInfBlkItems[0];

                                            // mark trial query as incomplete so program knows to wait for expected XippConfigPacket
                                            trialBlockItemsReceived--;
                                            trialQueryComplete = false;
                                            executeFullTrialQueryWithConfig = false;
                                            trialConfigInProgress = false;
                                            executeAction = true;
                                        }

                                        // Case 2: User wants to start the recording
                                        else if (    (stdinBuff[0] == 'r')
                                                  &&  ( (pTrial->status == RECORDING_TRIAL_STATUS_STOPPED)
                                                     || (pTrial->status == RECORDING_TRIAL_STATUS_STOP_REQUESTED) ) )
                                        {
                                            strcpy(actionStr, "START");

                                            // generate a random file name and set the file base name field on the Trial property
                                            char fileNameStr[512];
                                            // this assumes that the folder created in the user's home directory
                                            // during the Trellis install is still intact.

                                            int lastChar = strlen(fileDirPath)-1;
                                            // get rid of new line character
                                            if(fileDirPath[lastChar] == '\n')
                                              fileDirPath[lastChar] = '\0';
                                            lastChar -= 1;
                                            bool hasSep = false;
                                            char lastCh = fileDirPath[lastChar];
#if defined(__MACH__) || defined(__linux__) || defined(__unix__)
                                            hasSep = (lastCh == '/');
                                            sprintf(fileNameStr, "%s", fileDirPath); //, hasSep ? '_' : '/', packetCountUdp);
#else
                                            hasSep = (fileDirPath[lastChar] == '\\');
                                            sprintf(fileNameStr, "%s", fileDirPath); //, hasSep ? '_' : '\\',  packetCountUdp);
#endif
                                            strcpy(pTrial->filePathBase, fileNameStr);

                                            // uncomment the following line to ativate recording auto stop
                                            // pTrial->autoStopTime = 5; // seconds

                                            pTrial->status = RECORDING_TRIAL_STATUS_START_REQUESTED;

                                            // trial starts when Trellis gets this command (i.e. no specific time)
                                            pTrial->trialStart = 0;
#if defined(VERBOSE)
                                            printf("\n\n*** STARTING TRIAL - TIME[%d] ***\n\n", pTrial->trialStart);

#endif
                                            recordingRequested = true;
                                            executeAction = true;
                                        }

                                        // Case 3: User wants to pause the recording
                                        else if( (stdinBuff[0] == 'p') &&
                                                  ( (pTrial->status == RECORDING_TRIAL_STATUS_RECORDING)
                                                 || (pTrial->status == RECORDING_TRIAL_STATUS_START_REQUESTED)
                                                 || (pTrial->status == RECORDING_TRIAL_STATUS_UNPAUSE_REQUESTED) ) )
                                        {
                                            strcpy(actionStr, "PAUSE");
                                            pTrial->status = RECORDING_TRIAL_STATUS_PAUSE_REQUESTED;
                                            executeAction = true;
                                        }

                                        // Case 4: User wants to unpause the recording
                                        else if( (stdinBuff[0] == 'u') &&
                                                  ( (pTrial->status == RECORDING_TRIAL_STATUS_PAUSED)
                                                 || (pTrial->status == RECORDING_TRIAL_STATUS_PAUSE_REQUESTED)) )

                                        {
                                            strcpy(actionStr, "UNPAUSE");
                                            pTrial->status = RECORDING_TRIAL_STATUS_UNPAUSE_REQUESTED;
                                            executeAction = true;
                                        }

                                        // Case 5: User wants to stop the recording
                                        else if( (stdinBuff[0] == 's') &&
                                                  ( (pTrial->status != RECORDING_TRIAL_STATUS_STOPPED)
                                                 && (pTrial->status != RECORDING_TRIAL_STATUS_STOP_REQUESTED)) )

                                        {
                                            strcpy(actionStr, "STOP");
                                            pTrial->status = RECORDING_TRIAL_STATUS_STOP_REQUESTED;
                                            executeAction = true;
                                        }

                                        // Case 6: User wants to query the recording status
                                        else if(stdinBuff[0] == 'q')
                                        {
                                            strcpy(actionStr, "QUERY");
                                            xippQueryPkt.target.property = TRELLIS_PROPERTY_RECORDING_TRIAL_DESCRIPTOR;

                                            // change which packet is sent
                                            pPkt = &xippQueryPkt;

                                            // delete all existing signal selection descriptors
                                            int i;
                                            for(i=0; i<SELECTION_DESC_COUNT_MAX; ++i)
                                            {
                                                if(pSigSlctnDescBlkPkts[i])
                                                    free(pSigSlctnDescBlkPkts[i]);
                                            }
                                            // reinitialize the array to NULL pointers
                                            memset((void *)pSigSlctnDescBlkPkts, 0, SELECTION_DESC_COUNT_MAX * sizeof(XippConfigPacket *));

                                            trialConfigInProgress = false;
                                            executeAction = true;
                                        }

                                        // Case 7: User wants to select/unselect a signal type
                                        else if(       ( (pTrial->status == RECORDING_TRIAL_STATUS_STOPPED)
                                                      || (pTrial->status == RECORDING_TRIAL_STATUS_STOP_REQUESTED) )
                                                   && (pSigSlctnDescBlkPkts) )
                                        {
                                            int itemIdx = atoi(stdinBuff) - 1;
                                            if(isdigit(stdinBuff[0]) && (itemIdx <SELECTION_DESC_COUNT_MAX))
                                            {
                                                // make sure the packet exists
                                                if(pSigSlctnDescBlkPkts[itemIdx])
                                                {
/// \todo [AMW-2015/10/23] update to reflect changes to file save property schema
//                                                    strcpy(actionStr, "CONFIGURE");
//
//                                                    // change which packet is sent
//                                                    pPkt = pSigSlctnDescBlkPkts[itemIdx];
//
//                                                    // get a pointer to the signal property
//                                                    XippSignalSelectionDescriptor * pSelectionDesc =
//                                                            (XippSignalSelectionDescriptor *)(pPkt->config);
//
//                                                    // toggle the value of the property
//                                                    pSelectionDesc->selected = pSelectionDesc->available && !pSelectionDesc->selected;
//
//                                                    // mark trial query as incomplete so program knows to wait for expected XippConfigPacket
//                                                    trialBlockItemsReceived--;
//                                                    trialQueryComplete = false;
//                                                    executeFullTrialQueryWithConfig = false;
//                                                    trialConfigInProgress = false;
//                                                    executeAction = true;
                                                }
                                            }
                                        }

                                        //
                                        // Execute the action
                                        //
                                        if(executeAction)
                                        {
                                            printf("\n\n[Action] - Attempting to %s Recording Trial ... ", actionStr);
                                            if( SendXippConfigPacket(pPkt, outSocket, pTarget, targetLen) )
                                            {
                                                printf("done\n");

                                                // If this is a full Recording Trial query set the state tracking flags
                                                if( (stdinBuff[0] == ('q')) || (executeFullTrialQueryWithConfig) )
                                                {
#if defined(VERBOSE)
                                                    printf("Executing query for full File Save config.");
#endif
                                                    // These are the status flags that must be set to track full Trial queries
                                                    trialDescriptorReceived = false;
                                                    trialBlocksReceived     = 0;
                                                    trialBlockItemsReceived = 0;

                                                    extInfBlkQueried        = false;
                                                    extInfBlkItmsQueried    = false;
                                                    sigSlctnsBlkQueried     = false;
                                                    sigSlctnsBlkItmsQueried = false;
                                                    fileNameBlkQueried      = false;
                                                    fileNameBlkItmsQueried  = false;

                                                    trialQueryComplete      = false;
                                                }
                                            }
                                            else
                                            {
                                                printf("ERROR: sending XIPP ConfigPacket over UDP!\n");
                                            }

                                            userPromptedForTrialAction = false;
                                        }
                                    }

                                    // the user is responding to the intial prompt whether to execute a recording trial query
                                    else if(userPromptedForTrialQuery)
                                    {
#if defined(VERBOSE)
                                       printf("Processing response for trial QUERY...\n");
#endif
                                        // user has responded affirmatively
                                        if( (stdinBuff[0] == 'q') || (stdinBuff[0] == '\n') )
                                        {
                                            // query info on the current recording Trial
                                            xippQueryPkt.target.property = TRELLIS_PROPERTY_RECORDING_TRIAL_DESCRIPTOR;

                                            // send property query packet
                                            printf("\n\n[Action] - Attempting to QUERY Recording Trial Information ... ");
                                            if( SendXippConfigPacket(&xippQueryPkt, outSocket, pTarget, targetLen) )
                                            {
                                                printf("done\n");
                                                trialDescriptorReceived = false;
                                                trialBlocksReceived     = 0;
                                                trialBlockItemsReceived = 0;

                                                extInfBlkQueried        = false;
                                                extInfBlkItmsQueried    = false;
                                                sigSlctnsBlkQueried     = false;
                                                sigSlctnsBlkItmsQueried = false;
                                                fileNameBlkQueried      = false;
                                                fileNameBlkItmsQueried  = false;

                                                trialQueryComplete      = false;
                                            }
                                            else
                                            {
                                                printf("ERROR!\n");
                                            }
                                        }
                                        userPromptedForTrialQuery = false;
                                    }
                                }
                            }
                        }

                        // NOTE: this is an action that gets executed every ~1 sec
                        // If current Trial is recording or paused query its status
                        if( trialInfoValid && !trialConfigInProgress && (pTrial->status != RECORDING_TRIAL_STATUS_STOPPED) )
                        {
                            xippQueryPkt.target.property = TRELLIS_PROPERTY_RECORDING_TRIAL_DESCRIPTOR;
                            if( !SendXippConfigPacket(&xippQueryPkt, outSocket, pTarget, targetLen) )
                            {
                                printf("ERROR: querying recording Trial!\n");
                            }
                        }

                        // mark current values for use next time around
                        timeLast = timeNow;
                    }

                    ///////////////////////////////////////////////////////
                    // [Step 2] - Test if Trial Configuration is Complete
                    ///////////////////////////////////////////////////////

                    // trial query is complete
                    if(    !trialQueryComplete
                        && trialDescriptorReceived
                        && (trialBlocksReceived == 3)
                        && (trialBlockItemsReceived == (pExtInfBlock->count + pSigSlctnDescBlock->count + pFileNamesBlk->count) ) )
                    {
                        PrintRecordingTrial(pTrial);
                        printf("\n");

                        // print out the trial block properties
                        if(pTrial->status == RECORDING_TRIAL_STATUS_STOPPED)
                        {
                            /// \todo [AMW-2015/10/23] add support for printing signal selection descriptors
                            //PrintPropertyBlock(2, pSigSlctnDescBlock, pSigSlctnDescBlkPkts);
                            PrintPropertyBlock(1, pExtInfBlock, pExtInfBlkItems);
                        }
                        else
                        {
                            PrintPropertyBlock(1, pExtInfBlock, pExtInfBlkItems);
                            PrintPropertyBlock(3, pFileNamesBlk, pFileNamesBlkItems);
                        }

                        trialQueryComplete         = true;
                        userPromptedForTrialAction = false;
                        trialConfigInProgress      = false;

#if defined(VERBOSE)
                        printf("\n\n");
                        printf("#########################################\n");
                        printf("#            QUERY COMPLETE\n");
                        printf("#########################################\n\n");
#endif
                    }


                    ////////////////////////////////////
                    // [Step 3] - Figure Out Next Action
                    ////////////////////////////////////

                    // if we haven't querried the Operator do so
                    if(!operatorQueried)
                    {
                        printf("\n[Step 1] - Searching the network for an Operator ... ");

                        // send property query packet
                        if( !SendXippConfigPacket(&xippQueryPkt, outSocket, pTarget, targetLen) )
                            break;

                        printf("done\n");
                        operatorQueried = true;
                        operatorQueryInProgress = true;
                    }
                    else if(!trialInfoValid && !operatorQueryInProgress && !userPromptedForTrialQuery && trialQueryComplete)
                    {
                        printf("\n1) Choose an action: Exit(x)|[Query(q)]: ");
                        userPromptedForTrialQuery = true;
                    }
                    else if(trialInfoValid && trialQueryComplete && !userPromptedForTrialAction && !trialConfigInProgress)
                    {
                        char statusStr[1024];
                        char actionStr[1024];
                        if(pTrial->status == RECORDING_TRIAL_STATUS_STOPPED)
                        {
                            strcpy(statusStr, "STOPPED");
                            /// [AMW-2015/10/23] \todo update to reflect new schema structure
                            //strcpy(actionStr, "1) Enter the number of a Signal to toggle its selection state.\n"
                            strcpy(actionStr, "1) Type a comment string and press ENTER.\n"
                                              "2) Choose an action: Exit(x)|Query(q)|Record(r)");
                        }
                        else if(pTrial->status == RECORDING_TRIAL_STATUS_RECORDING)
                        {
                            strcpy(statusStr, "RECORDING");
                            strcpy(actionStr, "Choose an action: Exit(x)|Query(q)|Pause(p)|Stop(s)");
                        }
                        else if(pTrial->status == RECORDING_TRIAL_STATUS_PAUSED)
                        {
                            strcpy(statusStr, "PAUSED");
                            strcpy(actionStr, "Choose an action: Exit(x)|Query(q)|Unpause(u)|Stop(s)");
                        }
                        // else file save is in transition... only allow requery
                        else if(pTrial->status == RECORDING_TRIAL_STATUS_START_REQUESTED)
                        {
                            strcpy(statusStr, "STARTING...");
                            strcpy(actionStr, "Choose an action: Exit(x)|Query(q)|Pause(p)|Stop(s)");
                        }
                        else if(pTrial->status == RECORDING_TRIAL_STATUS_PAUSE_REQUESTED)
                        {
                            strcpy(statusStr, "PAUSING...");
                            strcpy(actionStr, "Choose an action: Exit(x)|Query(q)|Unpause(u)|Stop(s)");
                        }
                        else if(pTrial->status == RECORDING_TRIAL_STATUS_UNPAUSE_REQUESTED)
                        {
                            strcpy(statusStr, "UNPAUSING...");
                            strcpy(actionStr, "Choose an action: Exit(x)|Query(q)|Pause(p)|Stop(s)");
                        }
                        else if(pTrial->status == RECORDING_TRIAL_STATUS_STOP_REQUESTED)
                        {
                            strcpy(statusStr, "STOPPING...");
                            strcpy(actionStr, "Choose an action: Exit(x)|Query(q)");
                        }

                        // prompt user what to do with the trial
                        printf("Recording Trial Is currently %s\n\n", statusStr);
                        printf("%s\n", actionStr);

                        userPromptedForTrialAction = true;
                    }
//#if defined(VERBOSE)
//                    else
//                    {
//                        printf("UDP Packet Count: [%d]\r", packetCountUdp);
//                    }
//#endif

                    ////////////////////////////////////
                    // Process UDP Packets
                    ////////////////////////////////////

                    // wait for the next UDP packet
                    bytesRead = recvfrom(inSocket, (void *)udpBuff, UDP_BUFF_BYTE_COUNT, flags, &from, (socklen_t*)(&fromLen));

                    // pull the XIPP packets out of the UDP packet
                    int byteIdx = 0;
                    while(byteIdx < bytesRead)
                    {
                        // interpret the next set of bytes as a XippPacket
                        XippPacket * pPacket = (XippPacket *)(&udpBuff[byteIdx]);

                        // figure out how long this packet is
                        int packetByteCount = pPacket->header.size*4;

                        /////////////////////////////////
                        // parse the XIPP packet
                        /////////////////////////////////

                        // Case 1: Data Packet
                        if(pPacket->header.stream != 0)
                        {
                            // Ignore
                        }

                        // Case 2: Configuration Packet
                        else
                        {
                            ////////////////////////////
                            // Process XippConfigPacket
                            ////////////////////////////

                            XippConfigPacket * pCfgPkt = (XippConfigPacket *)(&udpBuff[byteIdx]);

                            // Case A: config packet is from an NIP
                            if(pCfgPkt->header.processor < 128)
                            {
                                // Ignore these
                            }

                            // Case B: config packet is from an Operator
                            else
                            {
#if defined(VERBOSE)
                                XippPropertyHeader * pPropHdr = (XippPropertyHeader *)&(pCfgPkt->config);
                                printf("\n\nreceived a XippConfigPacket from an Operator\n");
                                printf("target operator [%d]\n", pCfgPkt->target.processor);
                                printf("target process  [%d]\n", pCfgPkt->target.module);
                                printf("target property [%d]\n", pCfgPkt->target.property);
                                printf("property type   [%d]\n", pPropHdr->type);
                                printf("packet time     [%d]\n", pCfgPkt->header.time);
#endif
                                // figure out what property this packet contains
                                uint16_t propID = pCfgPkt->target.property;

                                // Operator Descriptor
                                if(propID == OPERATOR_PROPERTY_PROCESS_DESCRIPTOR)
                                {
                                    if(!operatorDetected)
                                    {
                                        // make a copy of the operator descriptor packet
                                        memcpy(opDescPktRaw, &udpBuff[byteIdx], pCfgPkt->header.size*4);

                                        // print out the operator's info
                                        printf("\n Detected an Operator on the network\n");
                                        printf(" -----------------------------------\n");
                                        printf("               ID: %d\n",    pCfgPkt->target.processor);
                                        printf("             Type: %s\n",    pOpDesc->label);
                                        printf("           Vendor: %s\n",    pOpDesc->vendor);
                                        printf("          Version: %s\n",    pOpDesc->version);
                                        printf("  Property Schema: %d.%d\n", pOpDesc->propSchemaMajor,
                                                                             pOpDesc->propSchemaMinor);
                                        printf(" -----------------------------------\n");

                                        // check that the schema matches the one this program is compiled against
                                        if(    (pOpDesc->propSchemaMajor != TRELLIS_PROPERTY_SCHEMA_VERSION_MAJOR)
                                            || (pOpDesc->propSchemaMinor != TRELLIS_PROPERTY_SCHEMA_VERSION_MINOR)
                                          )
                                        {
                                            printf("Trellis Property schema not consistent with current schema %d.%d",
                                                   TRELLIS_PROPERTY_SCHEMA_VERSION_MAJOR,
                                                   TRELLIS_PROPERTY_SCHEMA_VERSION_MINOR);
                                            break;
                                        }

                                        // flag that we've found the operator
                                        operatorDetected = true;

                                        // change the processor field on the query packet so
                                        // from hence forch we only interact with this operator
                                        xippQueryPkt.target.processor = pCfgPkt->target.processor;

                                        operatorQueryInProgress = false;
                                    }
                                }

                                // Recording Format Descriptor
                                else if(propID == TRELLIS_PROPERTY_RECORDING_TRIAL_DESCRIPTOR)
                                {
                                    trialDescriptorReceived = true;

                                    // make a copy of the recording trial descriptor packet
                                    memcpy((void *)pTrialDescPkt, (const void *)pCfgPkt, pCfgPkt->header.size*4);

                                    // convert the packet to a config request
                                    ResetConfigPacketHeaderToAnonymous(pTrialDescPkt);

#if defined(VERBOSE)
                                    // print out the recording trial info
                                    PrintRecordingTrial(pTrial);
#endif
                                    // if we have a valid trial descriptor and are not tring to cinfigure a trial
                                    if(trialQueryComplete && !trialConfigInProgress)
                                    {
                                        // if we have requested a recording see if the trial has stopped
                                        if( recordingRequested
//                                         && (pTrial->status != RECORDING_TRIAL_STATUS_RECORDING)
//                                         && (pTrial->status != RECORDING_TRIAL_STATUS_PAUSED)
                                            )
                                        {
                                            printf("\n\n");

                                            // print out the recording trial info
                                            PrintRecordingTrial(pTrial);

                                            userPromptedForTrialAction = false;
                                            recordingRequested = (recordingRequested == 1) ? 2 : false;
                                        }
                                        // if we are in the middle of a recording trial just print out the aggregate data size
                                        else
                                        {
                                            printf("File Size: %.2f MB\r", pTrial->trialSize/1000000.0F);
                                        }
                                    }

                                    // if we are in the process of a trial query or config query the blocks
                                    else if( !trialQueryComplete || trialConfigInProgress )
                                    {
                                        if(!extInfBlkQueried)
                                        {
#if defined(VERBOSE)
                                            printf("Querying Extended Trial Info. ...\n");
#endif
                                          // query the extended info block
                                          xippQueryPkt.target.property = pTrial->extInfoBlock;
                                          if( SendXippConfigPacket(&xippQueryPkt, outSocket, pTarget, targetLen) )
                                              extInfBlkQueried = true;
                                          else
                                              printf("ERROR: could not query Extended Recording Trial Information!");
                                        }
                                        if(!sigSlctnsBlkQueried)
                                        {
#if defined(VERBOSE)
                                            printf("Querying Trial Selected Signal Types ...\n");
#endif
                                          // query the selected signal type block
                                          xippQueryPkt.target.property = pTrial->sigSelectionBlock;
                                          if( SendXippConfigPacket(&xippQueryPkt, outSocket, pTarget, targetLen) )
                                              sigSlctnsBlkQueried = true;
                                          else
                                              printf("ERROR: could not query Selected Signal Types for this Recording Trial!");
                                        }
                                        if(!fileNameBlkQueried)
                                        {
#if defined(VERBOSE)
                                          printf("Querying Trial File Names ...\n");
#endif
                                          // query the file names block
                                          xippQueryPkt.target.property = pTrial->fileNamesBlock;
                                          if( SendXippConfigPacket(&xippQueryPkt, outSocket, pTarget, targetLen) )
                                              fileNameBlkQueried = true;
                                          else
                                              printf("ERROR: could not query Recording Trial File Name Information!");
                                        }
                                    }

                                    // set the state flags
                                    trialInfoValid = true;
                                }

                                //
                                // XIPP packet is a Recording Trial Property Block
                                //

                                // Recording Trial Extended Information Block
                                else if( trialInfoValid && !extInfBlkItmsQueried && (propID == pTrial->extInfoBlock) )
                                {
                                    // copy the config packet
                                    memcpy((void *)extInfBlkPktRaw, (const void *)pCfgPkt, pCfgPkt->header.size*4);

                                    // set the Trial Extended Info Block pointer
                                    pExtInfBlkPkt = (XippConfigPacket *)extInfBlkPktRaw;
                                    pExtInfBlock    = (XippPropertyBlock *)(pExtInfBlkPkt->config);

                                    // convert the packet to a config request
                                    ResetConfigPacketHeaderToAnonymous(pExtInfBlkPkt);

                                    // create the array to store the extended trial info packets
                                    if(!pExtInfBlkItems)
                                    {
                                        pExtInfBlkItems = (XippConfigPacket**)( malloc( pExtInfBlock->count * sizeof(XippConfigPacket*) ) );
                                        memset((void *)pExtInfBlkItems, 0, pExtInfBlock->count * sizeof(XippConfigPacket *));
                                    }

#if defined(VERBOSE)
                                    printf("Querying Extended Trial Info Block Properties. Expecting [%d] properties",
                                               pExtInfBlock->count);
                                    if(pExtInfBlock->count)
                                        printf(" [%d-%d]", pExtInfBlock->first, pExtInfBlock->first + pExtInfBlock->count-1);
                                    printf(" ...\n");
#endif

                                    // query the block items
                                    int i;
                                    for(i=0; i<pExtInfBlock->count; ++i)
                                    {
                                        xippQueryPkt.target.property = pExtInfBlock->first + i;
                                        if( !SendXippConfigPacket(&xippQueryPkt, outSocket, pTarget, targetLen) )
                                            printf("ERROR: could not query Recording Trial Block Item!");
                                    }

                                    // keep track of the number of blocks received
                                    trialBlocksReceived++;
                                    extInfBlkItmsQueried = true;
                                }
                                // Recording Trial Signal Selection Block
                                else if( trialInfoValid && !sigSlctnsBlkItmsQueried && (propID == pTrial->sigSelectionBlock) )
                                {
                                    // copy the config packet
                                    memcpy((void *)sigSlctnDescBlockPktRaw, (const void *)pCfgPkt, pCfgPkt->header.size*4);

                                    // set the Recording Trial Signal Selection Block pointer
                                    pSigSlctnDescBlockPkt = (XippConfigPacket *)sigSlctnDescBlockPktRaw;
                                    pSigSlctnDescBlock    = (XippPropertyBlock *)(pSigSlctnDescBlockPkt->config);

                                    // convert the packet to a config request
                                    ResetConfigPacketHeaderToAnonymous(pSigSlctnDescBlockPkt);

#if defined(VERBOSE)
                                    printf("Querying Selected Signal Type Block Properties. Expecting [%d] properties",
                                               pSigSlctnDescBlock->count);

                                    if(pSigSlctnDescBlock->count)
                                        printf(" [%d-%d]",
                                               pSigSlctnDescBlock->first,
                                               pSigSlctnDescBlock->first + pSigSlctnDescBlock->count-1);
                                    printf(" ...\n");
#endif

                                    // query the block items
                                    int i;
                                    for(i=0; i<pSigSlctnDescBlock->count; ++i)
                                    {
                                        xippQueryPkt.target.property = pSigSlctnDescBlock->first + i;
                                        if( !SendXippConfigPacket(&xippQueryPkt, outSocket, pTarget, targetLen) )
                                            printf("ERROR: could not query Recording Trial Block Item!");
                                    }
                                    sigSlctnsBlkItmsQueried = true;

                                    // keep track of the number of blocks received
                                    trialBlocksReceived++;
                                }
                                // Recording Trial File Names Block
                                else if( trialInfoValid && !fileNameBlkItmsQueried && (propID == pTrial->fileNamesBlock) )
                                {
                                    // copy the config packet
                                    memcpy((void *)fileNamesBlkPktRaw, (const void *)pCfgPkt, pCfgPkt->header.size*4);

                                    // set the Recording Trial File Names Block pointer
                                    pFileNamesBlkPkt = (XippConfigPacket *)fileNamesBlkPktRaw;
                                    pFileNamesBlk    = (XippPropertyBlock *)(pFileNamesBlkPkt->config);

                                    // convert the packet to a config request
                                    ResetConfigPacketHeaderToAnonymous(pFileNamesBlkPkt);

#if defined(VERBOSE)
                                    printf("Querying File Names Block Properties. Expecting [%d] properties",
                                               pFileNamesBlk->count);

                                    if(pFileNamesBlk->count)
                                        printf(" [%d-%d]", pFileNamesBlk->first, pFileNamesBlk->first + pFileNamesBlk->count-1);
                                    printf(" ...\n");
#endif

                                    // query the block items
                                    int i;
                                    for(i=0; i<pFileNamesBlk->count; ++i)
                                    {
                                        xippQueryPkt.target.property = pFileNamesBlk->first + i;
                                        if( !SendXippConfigPacket(&xippQueryPkt, outSocket, pTarget, targetLen) )
                                            printf("ERROR: could not query Recording Trial Block Item!");
                                    }
                                    fileNameBlkItmsQueried = true;

                                    // keep track of the number of blocks received
                                    trialBlocksReceived++;
                                }

                                //
                                // XIPP packet is a Recording Trial Property Block Item
                                //

                                // Item from Recording Trial Extended Info Block (i.e. member of the block)
                                else if(    trialInfoValid
                                         && pExtInfBlock
                                         && pExtInfBlock->count // i.e. there are properties in the block
                                         && ( (propID >= pExtInfBlock->first) && (propID < (pExtInfBlock->first + pExtInfBlock->count)) )
                                         && pExtInfBlkItems )
                                {
                                    uint32_t itemIdx = propID - pExtInfBlock->first;

                                    // make sure we have memory to store the packet
                                    if(pExtInfBlkItems[itemIdx] == NULL)
                                    {
                                        pExtInfBlkItems[itemIdx] = (XippConfigPacket *)( malloc( pCfgPkt->header.size*4) );
                                    }
                                    // copy over the new value of the packet
                                    memcpy((void *)(pExtInfBlkItems[itemIdx]), pCfgPkt, pCfgPkt->header.size*4);

                                    // convert the packet to a config request
                                    ResetConfigPacketHeaderToAnonymous(pExtInfBlkItems[itemIdx]);

                                    // keep track of how many recording trial block items are received
                                    trialBlockItemsReceived++;
                                }


                                // Item from Recording Trial Selected Signal Types Block (i.e. member of the block)
                                else if(    trialInfoValid
                                         && pSigSlctnDescBlock
                                         && pSigSlctnDescBlock->count // i.e. there are properties in the block
                                         && ( (propID >= pSigSlctnDescBlock->first) && (propID < (pSigSlctnDescBlock->first + pSigSlctnDescBlock->count)) ) )
                                {
                                    uint32_t itemIdx = propID - pSigSlctnDescBlock->first;

                                    // make sure we have memory to store the packet
                                    if(pSigSlctnDescBlkPkts[itemIdx] == NULL)
                                        pSigSlctnDescBlkPkts[itemIdx] = (XippConfigPacket *)( malloc( pCfgPkt->header.size*4) );

                                    // copy over the new value of the packet
                                    memcpy((void *)(pSigSlctnDescBlkPkts[itemIdx]), pCfgPkt, pCfgPkt->header.size*4);

                                    // convert the packet to a config request
                                    ResetConfigPacketHeaderToAnonymous(pSigSlctnDescBlkPkts[itemIdx]);

                                    // keep track of how many recording trial block items are received
                                    trialBlockItemsReceived++;
                                }

                                // Item from Recording Trial File Names Block (i.e. member of the block)
                                else if(    trialInfoValid
                                         && pFileNamesBlk
                                         && pFileNamesBlk->count // i.e. there are properties in the block
                                         && ( (propID >= pFileNamesBlk->first) && (propID < (pFileNamesBlk->first + pFileNamesBlk->count)) ) )
                                {
                                    uint32_t itemIdx = propID - pFileNamesBlk->first;

#if defined(VERBOSE)
                                    printf("Received File Name item [%d]\n", itemIdx);
#endif
                                    // make sure we have memory to store the packet
                                    if(pFileNamesBlkItems[itemIdx] == NULL  )
                                        pFileNamesBlkItems[itemIdx] = (XippConfigPacket *)( malloc( pCfgPkt->header.size*4) );

                                    // copy over the new value of the packet
                                    memcpy((void *)(pFileNamesBlkItems[itemIdx]), pCfgPkt, pCfgPkt->header.size*4);

                                    // convert the packet to a config request
                                    ResetConfigPacketHeaderToAnonymous(pFileNamesBlkItems[itemIdx]);

                                    // keep track of how many recording trial block items are received
                                    trialBlockItemsReceived++;
                                }
                            }
                        }

                        // figure out where the next XIPP packet starts
                        byteIdx += packetByteCount;
                    }

                    // track the UDP statistics
                    byteCount += (bytesRead > 0) ? bytesRead : 0;
                    packetCountUdp += (bytesRead > 0) ? 1 : 0;

                    fflush(stdout); // flush stdout because we're not always writing newlines (only carriage returns)
                }
                while( readNetwork );

                // clean up
                close(inSocket);
                close(outSocket);
            }
        }
    }
    printf("\n\nExiting Program ... goodbye!\n");
    printf("\n\n\n");

#if defined(_WIN32)
    // cleanup socket resources (Win32)
    WSACleanup();
#endif
}
