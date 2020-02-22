Your work is to fill code following detailed hints written in "BEGIN HINTS" and "END HINTS", and 
there are about 50 lines of code to fill in total.
 
To find where they are:   grep HINTS *.cc

** Notice that during the Nachos initialization (threads/main.cc), the following objects are allocated (threads/system.cc), 

MemoryManager *memoryManager;       // memory manager for a set of free memory pages

ProcessManager *processManager;     // process manager for a set of free PIDs, and PCBs.
        Each PCB also points to a set of openned files (UserOpenFile) and each of them points to a system wide
        SysOpenFile object (See below), plus a file offset. Make sure to move this offset during Read() and Write().

OpenFileManager *openFileManager;   // open file manager for a set of SysOpenFile objects, namely all opened file descriptors
char diskBuffer[PageSize];          // Use this disk buffer for  buffer all of your user-level file I/O. 
Lock *diskBufferLock;              //  Use this Lock for accessing diskBuffer if needed. 
