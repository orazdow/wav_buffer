### CS575 Final
#### Audio Synchronization Project

##### Building:
\- Requires PortAudio (windows dll and headers are provided in the portaudio_win directory)\
\- edit Makefile or w32build.bat to point to PortAudio include and build directories\
\- G++: run Makefile\
\- MSVC: run w32build.bat

##### Running:

The prepoccesor definitions LOCKING, LOCK_FREE and EXTRA_WORK\
can be all be toggled in main.cpp to test different modes:

LOCKING: -use locks to synchronize threads\
LOCK_FREE -use PortAudio's ring buffer instead of simple ring buffer\
EXTRA_WORK: -adds extra simulated load time

load ~.wav -loads a new wav file\
quit -exits program 
