####################################################################################################
#
# @description xippmin is a series of C header files and C source files that demonstrate how to
#              communicate with and control the Ripple Grapevine NIP and Trellis software.
#
# @files
#   - xippmin.h
#   - xippmin_functions.h
#   - XippOperatorTypes.h
#   - count_nip_packets.h 
#   - control_trellis_recording.c
#
# @author Andrew Wilder
# @email support@rppl.com
#
####################################################################################################

This set of files contains two programs: count_nip_packets.c and control_trellis_recording.h. Both
programs can be compiled in Windows (with GCC via MinGW) and Mac and Linux (with GCC).

[Compile and run on Windows]
------------------------------------
To compile the programs on Windows you must first download and install MinGW and put MinGW\bin in 
your system PATH. Then open a command prompt, navigate to the directory containing the files, and 
type either of the following lines (depending on which program you want to generate):

 1) gcc count_nip_packets.c -o count.exe -lws2_32
 2) gcc control_trellis_recording.c -o record.exe -lws2_32
 
To run the programs type record.exe or count.exe at the command prompt


[Compile and run on Mac/Linux]
------------------------------------
To compile the programs on Mac or Linux open a terminal, navigate to the directory containing the files, and 
type either of the following lines (depending on which program you want to generate):

 1) gcc count_nip_packets.c -o count
 2) gcc control_trellis_recording.c -o record
 
To run the programs type ./count or ./record at the command prompt

