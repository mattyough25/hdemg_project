// $Id: XippOperatorTypes.h 4253 2016-08-18 14:22:45Z amwilder $
//
//  XippOperatorTypes.h
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
//  Copyright © 2011 Ripple, LLC
//  contact: support@rppl.com
//

#if defined(__cplusplus)
  #include <vector>
#endif

#ifndef XIPPOPERATORTYPES_H
#define XIPPOPERATORTYPES_H

//! \defgroup RecordingTrialStatus
//! \{
//! \brief set of possible values for Recording Trial status
static const uint32_t RECORDING_TRIAL_STATUS_STOPPED    = 1;
static const uint32_t RECORDING_TRIAL_STATUS_RECORDING  = 2;
static const uint32_t RECORDING_TRIAL_STATUS_PAUSED     = 3;

static const uint32_t RECORDING_TRIAL_STATUS_STOP_REQUESTED     = 4;
static const uint32_t RECORDING_TRIAL_STATUS_START_REQUESTED    = 5;
static const uint32_t RECORDING_TRIAL_STATUS_PAUSE_REQUESTED    = 6;
static const uint32_t RECORDING_TRIAL_STATUS_UNPAUSE_REQUESTED  = 7;
//! \}

//! \defgroup TrellisRecordingSignalTypes
//! \{
//! \brief Types of Grapevine Signal that can be recorded by the Trellis operator
static const uint32_t SIGNAL_TYPE_SPIKE             = 1;
static const uint32_t SIGNAL_TYPE_DIGITAL           = 2;
static const uint32_t SIGNAL_TYPE_CONTINUOUS_LFP    = 3;
static const uint32_t SIGNAL_TYPE_CONTINUOUS_RAW    = 4;
static const uint32_t SIGNAL_TYPE_CONTINUOUS_ANALOG = 5;

static const char * SignalTypeLabels[] =
{
    "Undefined",
    "Spikes",
    "Digital Events",
    "LFP",
    "Raw",
    "Analog"
};
//! \}

//! Port on which IP4 connections are made between multiple trellis operators attempting to 
//! synchronize file savers
static const uint16_t OperatorRecSyncPort = 3000;

//! Recording formats
static const uint32_t  RECORDING_FORMAT_NEV2PT2  = 0;

//! Constants for querying XIPP Operators
static const uint8_t XIPP_OPERATOR_ALL            = 255;
static const uint8_t XIPP_OPERATOR_PROCESS_MAIN   = 0;

//
// Trellis specific properties
//

//! Property schema version indicators
static const uint32_t  TRELLIS_PROPERTY_SCHEMA_VERSION_MAJOR = 3; //!< indicates non binary compatible changes
static const uint32_t  TRELLIS_PROPERTY_SCHEMA_VERSION_MINOR = 0; //!< indicates binary compatible changes

//
// Properties defined for this version of the Trellis property schema
//

//! \brief Id of the Operator Descriptor property. All XIPP operators must provide this property at this Id.
static const uint16_t OPERATOR_PROPERTY_PROCESS_DESCRIPTOR = 0;

//! \brief prop set descriptor (points to all the trial descriptors)
static const uint16_t OPERATOR_PROPERTY_RECORDING_TRIAL_LIST = 1;

//! \brief [AMW-2015/10/23] HACK to allow xippmex and xippmin to find the first trial descriptor
static const uint16_t TRELLIS_PROPERTY_RECORDING_TRIAL_DESCRIPTOR = 0x7FFF;

//! File Saver property schema
//! \note The following values are offsets from the first property in the file saver block
static const uint16_t TRELLIS_PROP_OFFSET_REC_TRIAL_DESC               = 0;
static const uint16_t TRELLIS_PROP_OFFSET_REC_OVERRUN_CNT              = 1;
static const uint16_t TRELLIS_PROP_OFFSET_REC_DATA_GAP_CNT             = 2;
static const uint16_t TRELLIS_PROP_OFFSET_REC_DATA_SRC_ERR_CNT         = 3;
static const uint16_t TRELLIS_PROP_OFFSET_REC_REMOTE_SYNC_DESC         = 4;
static const uint16_t TRELLIS_PROP_OFFSET_REC_PLAY_TONE_ON_CHANGE      = 5;
static const uint16_t TRELLIS_PROP_OFFSET_REC_REMOTE_CTRL_ENABLED      = 6;
static const uint16_t TRELLIS_PROP_OFFSET_REC_DIGITAL_TRIG_ENABLED     = 7;
static const uint16_t TRELLIS_PROP_OFFSET_REC_SAVE_INST_CONFIG_ENABLED = 8;
static const uint16_t TRELLIS_PROP_OFFSET_REC_EXT_INFO_BLOCK_DESC      = 9;  // 4 extended info props
static const uint16_t TRELLIS_PROP_OFFSET_REC_FILE_NAMES_BLOCK_DESC    = 10; // 16 file name props
static const uint16_t TRELLIS_PROP_OFFSET_REC_PREV_PATHS_BLOCK_DESC    = 11; // 128 prev paths props
static const uint16_t TRELLIS_PROP_OFFSET_REC_SIG_SELECT_BLOCK_DESC    = 12; // 16 signal selection props
static const uint16_t TRELLIS_PROP_OFFSET_REC_DIGITAL_TRIG_BLOCK_DESC  = 13; // 4 trig props
//! \note block properties follow at this point

#pragma pack(push, 1)

// Parts of Xipp.h that are referenced here
#if !defined(XIPP_H)
  #if defined(__cplusplus)
    //! \brief Charcter array lengths for various fields (all lengths include null termination character).
    static const uint32_t XIPP_STRLEN_LABEL = 128;
  #else
    #define XIPP_STRLEN_LABEL 128
  #endif
#endif

// Only define these if NOT compiling against XippTypes.h
// [AMW-2015/10/23] Should these be removed from XippTypes.h and permanently located here?
#if !defined(XIPPTYPES_H)
    
    //! \todo [AMW-2012/07/28] convert these to TRELLIS_TYPEs with corresponding IDs
    // Trellis property type information
    static const uint8_t   XIPP_TYPE_PROPERTY_BLOCK               = 19;
    static const uint8_t   XIPP_TYPE_OPERATOR_DESCRIPTOR          = 20;
    static const uint8_t   XIPP_TYPE_RECORDING_TRIAL_DESCRIPTOR   = 21;
    static const uint8_t   XIPP_TYPE_SIGNAL_SELECTION_DESCRIPTOR  = 22;

    //! \todo [AMW-2012/07/30] decide if it makes sense to have all trellis property
    //!       types 128 and greater (i.e. like the processor/operator id division)
    static const uint8_t   TRELLIS_TYPE_GV_DIGITAL_TRIGGER_DESCRIPTOR = 128;
    static const uint8_t   TRELLIS_TYPE_RECORDING_SYNC_CMD            = 129;
    static const uint8_t   TRELLIS_REMOTE_RECORDING_SYNC_CONFIG       = 130;
    static const uint8_t   TRELLIS_TYPE_GV_FE_SIGNAL_SELECTION_DESC   = 131;
    static const uint8_t   TRELLIS_TYPE_RECORDING_SYNC_HEARTBEAT      = 132;
#endif

typedef struct
{
  XippPropertyHeader header;
  uint16_t first;        //!< ID of first property in the block
  uint16_t count;        //!< number of properties in block
  uint16_t circStart;    //!< index of start item (when used as a circular list)
  uint16_t propertyType; //!< type of properties in the block
  char description[128]; //!< description of block contents

#if defined(__cplusplus)

  //! \brief convenience method for testing block membership
  int Contains(uint16_t propID) { return (propID >= first) && (propID < first + count); }

#endif

} XippPropertyBlock;

/*! \brief List of properties (possibly of a particular type). If entries in the list are 0 they
 *  to not point to a property.
 */
typedef struct
{
  XippPropertyHeader header;
  uint8_t  vendor;   //!< vendor of the properties in the block
  uint8_t  type;     //!< type of the properties in the block (e.g. XippString, XippInt ... etc)
  uint16_t len;      //!< length of the props array
  uint16_t propIDs[0]; //!< array of property IDs
  
#if defined(__cplusplus)

  //! \brief Returns true if the property is in the list. Else, returns false.
  bool Contains(uint16_t propID)
  { 
    for(int i=0; i<len; ++i) {
      if(propIDs[i] == propID) return true; 
    }
    return false; 
  }
  
  /*! \brief If the specified property is within the specified distance of a property in this list
   *  the id for that property is returned. Else, 0 is returned.
   */  
  uint16_t StartOfRange(uint16_t propID, uint16_t propCnt)
  {
    for(int i=0; i<len; ++i) {
      if((propID >= propIDs[i]) && (propID < (propIDs[i] + propCnt))) return propIDs[i]; 
    }
    return 0;
  }

#endif
  
} XippPropertyList;
    
/*! \brief XippOperatorProcessDesc contains information about entities on the Instrument Network
 *  that are functioning as processes on Xipp Operator machines. Xipp Operator processes are
 *  involved in various activites such as launching and managing threads/processes for visualizing,
 *  analyzing or recording data.
 * 
 *  Providing access to a valid XippOperatorProcessDesc property is the minimum requirement an
 *  entiy on the Instrument Network must satisfy to be considered a XIPP operator process.
 *  Operator processes may provide access to other properties \sa propertyCount that can be
 *  queried and manipulated by other entites on the Instruemnt Network to control various aspects
 *  of Operator process state/behavior (e.g. data Recording).
 */
typedef struct
{
  XippPropertyHeader header;
  uint16_t propCount;
  uint8_t  propSchemaMajor;
  uint8_t  propSchemaMinor;
  char vendor[16];
  char label[16];
  char version[16];
  
  //! \todo [AMW-2015/05/06] figure out if I should add a size field (like XippModuleDesc)
  
} XippOperatorProcessDesc;

// [Recording Trial]
/*!
 * \note A Trellis Recording Trial is a set of NIP data that are recorded over a period
 * of time. A trial may consist of more than one file (i.e. one file may contain
 * extracted spike waveforms, and another continuous sample data). A Recording Trial
 * can be in one of several states (Stopped | Recording | Paused).
 * \sa RECORDING_TRIAL_STATUS_STOPPED etc.
 */

/*!
 * \brief XippRecordingTrialDescriptor contains information about a Trellis Recording Trial.
 * Some of the fields are read-only and describe the status of the current Trial.
 * Other fields can be changed to control the behavior of subsequent trials. While a
 * Trial is in progress the only descriptor field that can be modified is status. It
 * can be set to RECORDING_TRIAL_STATUS_STOPPED or RECORDING_TRIAL_STATUS_PAUSED.
 */
typedef struct
{
    XippPropertyHeader  header;

    //! used to synchronize access to this object
    //! (can only be set by FileSaver that owns this trial descriptor)
    uint32_t locked;
    //! Number of times this descriptor or any another trial propery has changed.
    //! The purpose is to provide a means for UIs etc. to know when to refresh property values
    //! Note: changes to nipTime, elapsed, and size fields are not counted
    uint32_t  chngCnt; //!< [Read Only]
    // trial configuration
    uint32_t  format;          //!< [Read/Write] file format (e.g. RECORDING_FORMAT_NEV2PT2)
    uint32_t  autoStopTime;    //!< [Read/Write] 0 => record until stop signal received
    uint32_t  autoSegEnabled;  //!< [Read/Write] indicates whether recording should be broken into segments
    float     autoSegSize;     //!< [Read/Write] size of recording segments
    int32_t   autoSegUnits;    //!< [Read/Write] recording segment size units (megabytes, min, hrs, days)
    uint32_t  autoIncrEnabled; //!< [Read/Write] indicates whether auto increment of file suffix is enabled (NOTE: will be more usefull when triggering is implemented)
    uint32_t  autoIncrNumber;  //!< [Read/Write] current increment number
    // blocks of trial configuration properties
    uint16_t  extInfoBlock;      //!< [Read Only] prop id of block desc pointing to file format specific trial info properties
    uint16_t  fileNamesBlock;    //!< [Read Only] prop id of block desc pointing to file name properties
    uint16_t  prevPathsBlock;    //!< [Read Only] prop id of block desc pointing to previous file path properties
    uint16_t  sigSelectionBlock; //!< [Read Only] prop id of block desc pointing to signal selection properties
    uint16_t  digTrigBlock;      //!< [Read Only] prop id of block desc pointing to digital trigger properties
    
    // trial status information
    uint32_t  status;     //!< [Read/Write] state of recording [ stopped | recording | paused ]
    float     dataRate;   //!< [Read Only]  aggregate data rate (bytes per second)
    float     remDiskMB;  //!< [Read Only]  remaining disk space for the file save operation
    // trial information
    uint32_t  trialStart;   //!< [Read/Write] nip time to start/stop recording (or, if recording is active or paused, time recording was started/paused)
    uint32_t  trialPause;   //!< [Read/Write] nip time to start/stop recording (or, if recording is active or paused, time recording was started/paused)
    uint32_t  trialStop;    //!< [Read/Write] nip time to start/stop recording (or, if recording is active or paused, time recording was started/paused)
    uint32_t  trialElapsed; //!< [Read Only]  milliseconds elapsed since recording started
    float     trialSize;    //!< [Read Only]  aggregate size (in bytes) of all files generated as part of this trial
    // segment status information [Note: these are only updated if segmenting is enabled]
    uint32_t  segCnt;     //!< [Read Only]  number of segments created (including the one currently being recorded) 
    uint32_t  segStart;   //!< [Read/Write] nip time to start/stop recording (or, if recording is active or paused, time recording was started/paused)
    uint32_t  segPause;   //!< [Read/Write] nip time to start/stop recording (or, if recording is active or paused, time recording was started/paused)
    uint32_t  segStop;    //!< [Read/Write] nip time to start/stop recording (or, if recording is active or paused, time recording was started/paused)
    uint32_t  segElapsed; //!< [Read Only]  milliseconds elapsed since this segment started
    float     segSize;    //!< [Read Only]  aggregate size (in bytes) of all files generated during this segment

    char filePathBase[512]; //!< [Read/Write] base file name (no extension) including path
    char fileErrMsg[256]; //!< [Read Only] set by file save instance if file path base is invalid
    
} XippRecordingTrialDescriptor;
typedef XippRecordingTrialDescriptor XippRecordTrialDesc;

/*! \brief Contains selection state for all signals/streams associated with a single stream type
 *  (e.g. raw, lfp, spike) from a single front end. When the stream contains events, the guid is 
 *  the first of a continuous sequence of 32 streams, each with a GUID one greater than the previous.
 */
typedef struct
{
  uint32_t guid; //!< GUID of the continuous stream or the guid of the first of 32 event streams
  uint32_t count; //!< Number of signals of this type (for the front end this selection set belongs to)
  uint32_t selectns;

#if defined(__cplusplus)
  
  //! \brief Returns the stream ID of the continuous stream (or the first spike stream in the sequence)
  int FirstStrmID() { return ((guid&0xFF00)>>8); }
		
  //! \brief Continuous streams are stored with 0x0 in the signal portion of the guid.
  inline bool IsEvent() { return (guid & 0xFF) != 0x0; }
  
  //! \brief Returns the GUID of the signal at the specified index, else 0
  inline uint32_t SigGUID(uint32_t idx)
  {
    uint32_t sigGUID = 0;
    if(idx<count) {
      uint32_t baseGUID = guid & (IsEvent() ? 0xFFFF0000 : 0xFFFFFF00);
      int shift = IsEvent() ? 8 : 0;
      sigGUID = baseGUID | ((idx+1)<<shift); // streams are 1-indexed
    }
    return sigGUID;
  }

  /*! \brief Returns true if this selection descriptor pertains to the specified signal,
   *  else false.
   */
  inline bool HasSignal(uint32_t sigGUID)
  {
    bool hasSignal = false;
    // get the stream for the supplied signal
    uint32_t sigStrm = (sigGUID & 0xFFFFFF00)>>8;
    // get the base stream for this next signal type selection
    uint32_t thisStrm = (guid & 0xFFFFFF00)>>8;
    // event stream sequence
    if(IsEvent()) {
      hasSignal = (sigStrm >= thisStrm) && (sigStrm < (thisStrm+count));
    }
    // contin stream
    else {
      hasSignal = (sigStrm == thisStrm);
    }
    return hasSignal;
  }

  /*! \brief Returns true if this selection descriptor pertains to the specified signal,
   *  else false.
   */
  inline bool IsSelected(uint32_t sigGUID)
  {
    bool isSelected = false;
    if(HasSignal(sigGUID)) {
      int idx = IsEvent() ? (((sigGUID & 0x0000FF00)>>8)-FirstStrmID()) : (sigGUID & 0x000000FF);
      isSelected = selectns & (0x1<<idx);
    }
    return isSelected;
  }

  /*! \brief generates a list of the signals this struct represents
   */
  inline std::vector<uint32_t> Signals()
  {
     std::vector<uint32_t> sigs;
      uint32_t guidBase = guid & (IsEvent() ? 0xFFFF0000 : 0xFFFFFF00);
      int shift = IsEvent() ? 8 : 0;
      int offset = IsEvent() ? FirstStrmID() : 0; // streams sequence starts at first stream, signal sequence starts at 0
      for(int i=0; i<count; ++i) {
        uint32_t sigGUID = guidBase | ((i+offset)<<shift);
        sigs.push_back(sigGUID);
      }
      return sigs;
  }

  //! \brief convenience method.
  inline bool HasStream(uint32_t strmGUID) { return HasSignal(strmGUID); }

  /*! \brief Clears all selections
   */
  inline void ClearSelections() { this->selectns = 0; }

  /*! \brief If this selection descriptor pretains to the specified signal, the signal is selected
   *  and true is returned, else false is returned.
   */
  inline bool SetSelection(uint32_t sigGUID, bool isSelected)
  {
    bool selctnIsSet = false;
    if(HasSignal(sigGUID)) {
      // figure out the position of the signal
      int idx = IsEvent() ? (((sigGUID & 0xFF00) - (guid & 0xFF00)) >> 8) : (sigGUID & 0xFF);
      // generate a selector by shifting a single bit to the appropriate position
      uint32_t selector = 0x1 << idx;
      // make the selection/unselection
      selectns = isSelected ? (selectns | selector) : (selectns & ~selector);
      selctnIsSet = true;
    }
    return selctnIsSet;
  }

  /*! \brief If the specified stream corresponds to this selection descriptor, the stream/signal
   *  selection states are set to the specified states and true is returned, else false is returned.
   *
   *  \arg [in] strmGUID - GUID of the (continuous) stream that contains the signals, or guid of
   *  the first (event) stream in a 16/32 stream sequence.
   *
   *  \arg [in] selections - selection state for the 16/32 channels related to this stream type
   *  low bit corresponds to first signal.
   */
  inline bool SetSelections(uint32_t strmGUID, uint32_t selections)
  {
    bool sigSelected = false;
    if((sigSelected = HasStream(strmGUID))) {
      selectns = selections;
    }
    return sigSelected;
  }

  /*! \brief sets all the signals to the specified selection state
   */
  inline void SetAllSelected(bool isSelected)
  {
    //! \todo [AMW-2016/08/16] verify that this works for FEs with less than 32 signals
    selectns = isSelected ? 0xFFFFFFFF : 0;
  }

#endif  
} SignalTypeSelections;
    
/*!
 *  \brief XippFeSignalSelectionDescriptor contains selection state information for all the
 *  signals generated by a front end.
 */
// Assuming front ends won't have more than 32 stream types (e.g. spike, lfp, raw, etc... )
#if defined(__cplusplus)
  static const int SignalTypeCnt = 32;
#else
  #define SignalTypeCnt 32
#endif
typedef struct
{
  
  XippPropertyHeader  header;
  uint8_t proc; //!< xipp ID of processor
  uint8_t port; //!< port ID (i.e. 1-4)
  uint8_t fe;   //!< front end ID (i.e. 1-4)
  char portChar; //!< character assigned to port (A, B, C, D, etc)
  char feLabel[XIPP_STRLEN_LABEL]; //!< label on the hardware module descriptor
  SignalTypeSelections selectns[SignalTypeCnt];
  
#if defined(__cplusplus)
  
  /*! \brief Returns true if this front end contains the specified signal. Else returns false.
   */
  bool HasSignal(uint32_t sigGUID)
  {
    bool hasSignal = false;
    for(int i=0; (i<SignalTypeCnt) && selectns[i].guid && !hasSignal; ++i) {
      hasSignal = selectns[i].HasSignal(sigGUID);
    }
    return hasSignal;    
  }
		
		/*! \brief Returns true if this front end contains the specified signal and the signal is
		 *  selected. Else returns false.
   */
  bool IsSelected(uint32_t sigGUID)
  {
    bool isSelected = false;
    for(int i=0; (i<SignalTypeCnt) && selectns[i].guid && !isSelected; ++i) {
      isSelected = selectns[i].HasSignal(sigGUID) && selectns[i].IsSelected(sigGUID);
    }
    return isSelected;    
  }
		
  /*! \brief generates a list of the signals this struct represents
   *
   *  \param sigCnt - the number of signals this front end has
   */
  inline std::vector<uint32_t> Signals()
  {
    std::vector<uint32_t> sigs;
    for(int i=0; i<SignalTypeCnt; ++i) {
      if(selectns[i].guid) {
        std::vector<uint32_t> strmSigs = selectns[i].Signals();
        sigs.insert(sigs.end(), strmSigs.begin(), strmSigs.end());
      }
    }
    return sigs;
  }

  /*! \brief Clears all selections for this front end
   */
  void ClearSelections()
  {
    for(int i=0; i<SignalTypeCnt; ++i) {
      selectns[i].ClearSelections();
    }
  }

  /*! \brief If the signal exists, its selection state is set to the specified value true is
   *  returned, else false is returned.
   */
  bool SetSelection(uint32_t sigGUID, bool isSelected)
  {
    bool signalIsSet = false;
    for(int i=0; (i<SignalTypeCnt) && selectns[i].guid && !signalIsSet; ++i) {
      if(selectns[i].HasSignal(sigGUID)) {
        signalIsSet = selectns[i].SetSelection(sigGUID, isSelected);
      }
    }
    return signalIsSet;        
  }

  /*! \brief If the signals exist, their selection state are set to the specified value and true is
   *  returned, else false is returned.
   * 
   *  \arg [in] strmGUID - GUID of the (continuous) stream that contains the signals, or guid of
   *  the first (event) stream in a 16/32 stream sequence.
   * 
   *  \arg [in] selections - selection state for the 16/32 channels related to this stream type
   *  low bit corresponds to first signal.
   */
  bool SetSelections(uint32_t strmGUID, uint32_t selections)
  {
    bool signalsAreSet = false;
    for(int i=0; (i<SignalTypeCnt) && selectns[i].guid && !signalsAreSet; ++i) {
      signalsAreSet = selectns[i].SetSelections(strmGUID, selections);
    }
    return signalsAreSet;        
  }

  /*! \brief Selects all signals published by this front end
   */
  void SetAllSelected(bool isSelected)
  {
    for(int i=0; (i<SignalTypeCnt) && selectns[i].guid; ++i) {
      selectns[i].SetAllSelected(isSelected);
    }
  }

#endif    
} GvFeSignalSelectionDesc;

/**
    GVDigitalTriggerDesc describes a pattern of digital input that should be
    wathed for. The expected usage is that this pattern will be associated
    with a particular action (e.g. Starting/Stopping Recording). NOTE: This
    structure applies to the Grapevine Digital I/O front end only.
  */
typedef struct
{
  XippPropertyHeader header;
  uint16_t  inputChannel;     //!< indicates which input shoule be monitored (None=0, SMA1=1, SMA2=2, SMA3=3, SMA4=4, parallel=5)
  uint16_t  smaTransition;    //!< indicates trigger transition (hi-lo=0, lo-hi=1) for inputs of type SMA*
  uint16_t  parallelMask;     //!< indicates mask applied to parallel port value
  uint16_t  parallelCompare;  //!< indicates parallel port value to trigger on
} GVDigitalTriggerDesc;

/*! \brief TrellisRecordingSyncCmd is sent from one Trellis instance to another to synchronize
 *  starting, pausing, and stopping recoring.
 */
// allowable actions
static const int RecordingSyncStart = 1; //!< if recording is paused Start resumes is (i.e. == Unpause)
static const int RecordingSyncPause = 2;
static const int RecordingSyncStop  = 3;
    
typedef struct
{
  XippPropertyHeader header;
  uint32_t action; //!< start/stop/pause
  uint32_t time;   //!< nip time to carry out the action
} TrellisRecordingSyncCmd;

/*! \brief Sent between synchronized instances of Trellis to ensure that the synchronization network
 *  link is still active.
 */
typedef struct
{
  XippPropertyHeader header;
  int64_t msecUTC;
} TrellisRecSyncHeartbeat;

/*! \brief Stores information about configuring network resources to enable synching two or more
 *  Trellis file savers. 
 */
typedef struct
{
  XippPropertyHeader header;
  uint8_t  enabled;
  uint8_t  ip4_d;
  uint16_t port;
  uint32_t ip4_subnet;
  uint32_t syncErr; //!< indicates operator<->operator sync status (0 => ok, !0 => error)
  char msg[128]; 
} TrellisRemoteRecSyncCfg;

#pragma pack(pop)


static const int PropBlkSize      = sizeof(XippPropertyBlock) + (sizeof(XippPropertyBlock)%4 ? (4-sizeof(XippPropertyBlock)%4) : 0);
static const int DigTrigSize      = sizeof(GVDigitalTriggerDesc) + (sizeof(GVDigitalTriggerDesc)%4 ? (4-sizeof(GVDigitalTriggerDesc)%4) : 0);
static const int RecTrialSize     = sizeof(XippRecordingTrialDescriptor) + (sizeof(XippRecordingTrialDescriptor)%4 ? (4-sizeof(XippRecordingTrialDescriptor)%4) : 0);
static const int RemoteSyncSize   = sizeof(TrellisRemoteRecSyncCfg) + (sizeof(TrellisRemoteRecSyncCfg)%4 ? (4-sizeof(TrellisRemoteRecSyncCfg)%4) : 0);
static const int FeSigSelctnsSize = sizeof(GvFeSignalSelectionDesc) + (sizeof(GvFeSignalSelectionDesc)%4 ? (4-sizeof(GvFeSignalSelectionDesc)%4) : 0);

#endif // XIPPOPERATORTYPES_H
