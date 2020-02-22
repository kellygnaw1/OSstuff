// exception.cc 
//  Entry point into the Nachos kernel from user programs.
//  There are two kinds of things that can cause control to
//  transfer back to here from user code:
//
//  syscall -- The user code explicitly requests to call a procedure
//  in the Nachos kernel.  Right now, the only function we support is
//  "Halt".
//
//  exceptions -- The user code does something that the CPU can't handle.
//  For instance, accessing memory that doesn't exist, arithmetic errors,
//  etc.  
//
//  Interrupts (which can also cause control to transfer from user
//  code into the Nachos kernel) are handled elsewhere.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "machine.h"
#include "pcb.h"

#define MAX_FILENAME_LEN 128
#define USER_READ 0 // passed as type for userReadWrite
#define USER_WRITE 1 // passed as type for userReadWrite

void IncrementPC(void);
int forkImpl(void);
void copyStateBack(int forkPC);
void yieldImpl(void);
void exitImpl(void);
int joinImpl(void);
SpaceId execImpl(char* filename);
void execHelper(void);
void readFilenameFromUsertoKernel(char* filename);
void createImpl(char* filename);
char* copyString(char* oldStr);
int openImpl(char* filename);
int userReadWrite(int virtAddr, char* buffer, int size, int type);
void writeImpl(void);
int readImpl(void);
void closeImpl(void);

//----------------------------------------------------------------------
// ExceptionHandler
//  Entry point into the Nachos kernel.  Called when a user program
//  is executing, and either does a syscall, or generates an addressing
//  or arithmetic exception.
//
//  For system calls, the following is the calling convention:
//
//  system call code -- r2
//    arg1 -- r4
//    arg2 -- r5
//    arg3 -- r6
//    arg4 -- r7
//
//  The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//  "which" is the kind of exception.  The list of possible exceptions 
//  are in machine.h.
//----------------------------------------------------------------------

void ExceptionHandler(ExceptionType which)
{
    int newProcessPID = 0;
    int otherProcessStatus = 0;
    int result = 0;
    //int arg1 = 0;
    //int arg2 = 0;
    //int arg3 = 0;
    char* filename = new char[MAX_FILENAME_LEN];

    int type = machine->ReadRegister(2);
    PCB* pcb = (currentThread->space)->getPCB();

    if (which == SyscallException) {

        switch (type) { /* Find out what type of syscall we're dealing with */

            case SC_Halt:
                fprintf(stderr, "System Call: %d invoked Halt\n", pcb->getPID());
                interrupt->Halt();
                break;
            case SC_Fork:
                fprintf(stderr, "System Call: %d invoked Fork\n", pcb->getPID());
                newProcessPID = forkImpl();
                machineLock->Acquire();
                machine->WriteRegister(2, newProcessPID);
                machineLock->Release();
                break;
            case SC_Yield:
                fprintf(stderr, "System Call: %d invoked Yield\n", pcb->getPID());
                yieldImpl();
                break;
            case SC_Exec:
                fprintf(stderr, "System Call: %d invoked Exec\n", pcb->getPID());
                readFilenameFromUsertoKernel(filename);
                newProcessPID = execImpl(filename);
                machineLock->Acquire();
                machine->WriteRegister(2, newProcessPID);
                machineLock->Release();
                break;
            case SC_Join:
                fprintf(stderr, "System Call: %d invoked Join\n", pcb->getPID());
                otherProcessStatus = joinImpl();
                machineLock->Acquire();
                machine->WriteRegister(2, otherProcessStatus);
                machineLock->Release();
                break;
            case SC_Exit:
                fprintf(stderr, "System Call: %d invoked Exit\n", pcb->getPID());
                exitImpl();
                break;
            case SC_Create:
                fprintf(stderr, "System Call: %d invoked Create\n", pcb->getPID());
                readFilenameFromUsertoKernel(filename);
                createImpl(filename);
                break;
            case SC_Open:
                fprintf(stderr, "System Call: [%d] invoked Open\n", pcb->getPID());
                readFilenameFromUsertoKernel(filename);
                copyString(filename); // copy bytes to a fresh piece of memory
                result = openImpl(filename); 
                machine->WriteRegister(2, result);
                break;
            case SC_Write:
                fprintf(stderr, "System Call: %d invoked Write\n", pcb->getPID());
                writeImpl();
                break;
            case SC_Read:
                fprintf(stderr, "System Call: %d invoked Read\n", pcb->getPID());
                result = readImpl();
                machine->WriteRegister(2, result);
                break;
            case SC_Close:
                fprintf(stderr, "System Call: %d invoked Close\n", pcb->getPID());
                closeImpl();
                break;
            default:
                fprintf(stderr, "System Call: %d invoked an unknown syscall!\n", 
                    pcb->getPID());
                ASSERT(FALSE);
        }
        
        IncrementPC();

    } else {
        fprintf(stderr, "Unexpected user mode exception %d %d\n", which, type);
        ASSERT(FALSE);
    }
}

//----------------------------------------------------------------------
// Fork system call implementation
//----------------------------------------------------------------------

int forkImpl() {

    // Create a new kernel thread
    Thread* childThread = new Thread("user-level child process");

    int newProcessPC = machine->ReadRegister(4);

    // BEGIN HINT
    // Use current-Thread->space to get the current PID
    // See addrspace.cc on how to find PCB, and pcb.cc on how to find PID.
    // Use processManager to get a new PID. 
    // Construct new PCB. See pcb.cc on how to create a new PCB.
    // Set the new  process as P_RUNNING 
    // Associate the new user process with this childThread
    // Add this newly created process to processManager using addProcess.
    // END  HINT

    
    int currPID = currentThread->space->getPCB()->getPID();//changed. 
    int newPID = processManager->getPID();//changed
    PCB* newpcb = new PCB(newPID,  currPID);
    newpcb->status = P_RUNNING;
    newpcb->process = childThread;
    processManager->addProcess(newpcb, newPID);
    if (newPID == -1) {
          fprintf(stderr, "Process %d is unable to fork a new process\n", currPID);
          return -1;
    }
    
   
  
    // BEGIN HINT
    // Make a copy of the address space using AddrSpace::AddrSpace()
    // END HINT
    AddrSpace* newspace = new AddrSpace(currentThread->space, newpcb);
    childThread->space = newspace;

    int childNumPages = childThread->space->getNumPages();
    if (childThread->space->pageTable == NULL) {
        fprintf(stderr, "Process %d Fork: start at address 0x%x with %d pages"
               " memory failed\n", currPID, newProcessPC, childNumPages);
        return -1;
    }

    // BEGIN HINT
    // Save states of the new created process using SaveState.
    // Save states/registers of the corresponding childThread for context switch.
    // See addrspace.cc and thread.cc on how to save the states.
    // END HINT
    childThread->space->SaveState();
    childThread->SaveUserState();

    // Mandatory printout of the forked process
    //PCB* parentPCB = currentThread->space->getPCB();
    //PCB* childPCB = childThread->space->getPCB();
    fprintf(stderr, "Process %d Fork: start at address 0x%x with %d pages memory\n",
        currPID, newProcessPC, childNumPages);
    
    // Set up the function that new process will run and yield
    childThread->Fork(copyStateBack, newProcessPC);
    currentThread->Yield(); 
    return newPID;
}

//----------------------------------------------------------------------
// copyStateBack
//      The "dummy" function from 2.1.1 of NACHOS implementation that 
//      copies back the machine registers, PC and return registers saved
//      from before the yield was performed.
//      
//----------------------------------------------------------------------

void copyStateBack(int forkPC) {

    // Get fresh registers, but use copy of addr space
    currentThread->RestoreUserState();

    // Set the PC and run
    machineLock->Acquire();
    machine->WriteRegister(PCReg, forkPC);
    machine->WriteRegister(PrevPCReg, forkPC - 4);
    machine->WriteRegister(NextPCReg, forkPC + 4);
    machineLock->Release();

    currentThread->space->RestoreState();
    machine->Run();
    ASSERT(FALSE); // should never reach here
}

//----------------------------------------------------------------------
// Yield system call implementation.
//----------------------------------------------------------------------

void yieldImpl() {

    //BEGIN HINTS
    //Save the corresponding user process's register states.
    //This kernel thread yields using currentThread->Yield() to accomplish the context switch
    //Once  this process is resumed for exectuion after yielding,
    //restore the corresponding user process's states (registers using Thread::RestoreUserState() 
    //and page table using AddrSpace::RestoreState()).
    //See addrspace.cc and thread.cc on how to save and restore states.
    //END HINTS
    currentThread->space->SaveState();
    currentThread->Yield();
    currentThread->space->RestoreState();
    currentThread->RestoreUserState();
}

//----------------------------------------------------------------------
// Exit system call implementation.
//----------------------------------------------------------------------

void exitImpl() {

    IntStatus oldLevel = interrupt->SetLevel(IntOff);
    int status = machine->ReadRegister(4);
    int currPID = currentThread->space->getPCB()->getPID();
    
    fprintf(stderr, "Process %d exits with %d\n", currPID, status);


    //BEGIN HINTS 
    //Set the exit status in the PCB of this process using  currentThread->space->getPCB() 
    //Also let other processes  know this process  exits through  processManager. 
    //See pcb.cc on how to get the exit code and see processmanager.cc on the above notification.
    //END HINTS
    currentThread->space->getPCB()->status = status;
    //currentThread->space->getPCB()->status = P_BLOCKED;
    processManager->broadcast(currPID);
    
   

    //Delete the current space of this process
    delete currentThread->space;
    currentThread->space = NULL;
    processManager->clearPID(currPID);

    (void) interrupt->SetLevel(oldLevel);
    currentThread->Finish();
}

//----------------------------------------------------------------------
// Join system call implementation.
//----------------------------------------------------------------------

int joinImpl() {

    int otherPID = machine->ReadRegister(4);
    currentThread->space->getPCB()->status = P_BLOCKED;

   // BEGIN HINTS 
   // If the other process has  already exited, then just return its status
   // Use proessManager to wait for the completion of  otherPID.
   // Change the status of this process  in its PCB as P_RUNNING.
   // END HINTS
   //
    
   if(processManager->getStatus(otherPID) != -1){
        processManager->join(otherPID);
        currentThread->space->getPCB()->status = P_RUNNING;
   }
    return processManager->getStatus(otherPID);
}

//----------------------------------------------------------------------
// Exec system call helper.
//----------------------------------------------------------------------

void execHelper(int uselessArg) {

    AddrSpace* currAddrSpace = currentThread->space;
    currAddrSpace->InitRegisters();
    currAddrSpace->SaveState();
    currAddrSpace->RestoreState();
    machine->Run();
}

//----------------------------------------------------------------------
// Exec system call implementation.
//----------------------------------------------------------------------

SpaceId execImpl(char* filename) {

    int currPID = currentThread->space->getPCB()->getPID();

    // Open the file
    OpenFile* fileToExecute = fileSystem->Open(filename);
    if (fileToExecute == NULL) {
        fprintf(stderr, "Unable to open file %s for execution. Terminating.\n", filename);
        return -1;
    }

    // Create a PCB for the new process, along with its running thread
    Thread* newThread = new Thread("Executing process");
    int newPID = processManager->getPID();
    PCB* newpcb = new PCB(newPID, currPID);
    newpcb->status = P_RUNNING;
    newpcb->process = newThread;

    // Give it an address space
    AddrSpace* newSpace = new AddrSpace(fileToExecute, newpcb);
    if (!newSpace->isValid()) {
        fprintf(stderr, "Exec Program: %d loading %s failed\n", currPID, filename);
        delete fileToExecute;
        delete newSpace;
        return -1;
    }
    newThread->space = newSpace;
    processManager->addProcess(newpcb, newPID);

    // Close file and execute new process
    delete fileToExecute;
    fprintf(stderr, "Exec Program: %d loading %s\n", currPID, filename);
    newThread->Fork(execHelper, 0);
    currentThread->Yield();
    return newPID;
}

//----------------------------------------------------------------------
// Helper function that brings bytes from memory to kernel space.
//----------------------------------------------------------------------

int moveBytesFromMemToKernel(int virtAddr, char* buffer, int size) {

    int physAddr = 0;
    int remainingFromPage = 0;
    int bytesCopied = 0;
    int bytesToCopy = 0;
    
    while (size > 0) {
        physAddr = currentThread->space->Translate(virtAddr);
        remainingFromPage = PageSize - physAddr % PageSize;
        bytesToCopy = remainingFromPage < size ? remainingFromPage : size;
        bcopy(machine->mainMemory + physAddr, buffer + bytesCopied, bytesToCopy);
        size -= bytesToCopy;
        bytesCopied += bytesToCopy;
        virtAddr += bytesToCopy;
    }

    return bytesCopied;
}

//----------------------------------------------------------------------
// Helper function to bring a string that represents a filename from 
// user space to kernel space.
//----------------------------------------------------------------------

void readFilenameFromUsertoKernel(char* filename) {

    int currPosition = 0;
    int filenameArg = machine->ReadRegister(4);

    do {
        moveBytesFromMemToKernel(filenameArg, filename + currPosition, 1);
        filenameArg++;
    } while (filename[currPosition++] != 0);

    filename[currPosition] = 0; // terminate C string
}

//----------------------------------------------------------------------
// Create file system call implementation.
//----------------------------------------------------------------------

void createImpl(char* filename) {
    bool successfulCreate = fileSystem->Create(filename, 0);
    ASSERT(successfulCreate); // should never fail
}

//----------------------------------------------------------------------
// Helper function to copy a string to a fresh piece of memory so we
// don't run into funky pointer stuff.
//----------------------------------------------------------------------

char* copyString(char* oldStr) {
    char* newStr = new char[MAX_FILENAME_LEN];
    for (int i = 0; i < MAX_FILENAME_LEN; i++) {
        newStr[i] = oldStr[i];
        if (oldStr[i] == '\0') {
            break;
        }
    }
    return newStr;
}

//----------------------------------------------------------------------
// Open file system call implementation.
//----------------------------------------------------------------------

int openImpl(char* filename) {
    
    int index = 0;
    SysOpenFile* currSysFile = openFileManager->getFile(filename, index);

    if (currSysFile == NULL) { // if no process currently has it open
        OpenFile* openFile = fileSystem->Open(filename);
        if (openFile == NULL) {
            fprintf(stderr, "Unable to open the file %s\n", filename);
            return -1;
        }
        SysOpenFile curSysFile;
        curSysFile.file = openFile;
        curSysFile.numProcessesAccessing = 1;
        curSysFile.filename = copyString(filename);
        index = openFileManager->addFile(curSysFile);
    }
    else { // the file is already open by another process
        currSysFile->numProcessesAccessing++;
    }

    // Either way, add it to the current PCB's open file list
    UserOpenFile currUserFile;
   //BEGIN HINTS 
   //Set up this UserOpenFile data structure
   // currUserFile.indeInSysOpenFileList should point to the index from openFileManager.
   // currUserFile.currOffsetInFile is the offset position of the current file openned
   // END HINTS
   // See useropenfile.h and pcb.cc on UserOpenFile class and its methods.
   // See sysopenfile.h and openfilemanager.cc for SysOpenFile class and its methods.
    currUserFile.fileName = copyString(filename);
    currUserFile.indexInSysOpenFileList = index;
    currUserFile.currOffsetInFile = 0;
  
 
    int currFileID = currentThread->space->getPCB()->addFile(currUserFile);
    return currFileID;
}

//----------------------------------------------------------------------
// Helper function that either reads from main mem into a buffer or 
// writes from a buffer into main mem.
//----------------------------------------------------------------------

int userReadWrite(int virtAddr, char* buffer, int size, int type) {

    int physAddr = 0;
    int numBytesFromPSLeft = 0;
    int numBytesCopied = 0;
    int numBytesToCopy = 0;

    if (type == USER_READ) { // from buffer to main memory
        while (size > 0) {
            physAddr = currentThread->space->Translate(virtAddr);
            numBytesFromPSLeft = PageSize - physAddr % PageSize;
            numBytesToCopy = (numBytesFromPSLeft < size) ? numBytesFromPSLeft : size;
            bcopy(buffer + numBytesCopied, machine->mainMemory + physAddr, numBytesToCopy);
            numBytesCopied += numBytesToCopy;
            size -= numBytesToCopy;
            virtAddr += numBytesToCopy;
        }
    }
    else if (type == USER_WRITE) { // from main memory to buffer
        while (size > 0) {
            physAddr = currentThread->space->Translate(virtAddr);
            numBytesFromPSLeft = PageSize - physAddr % PageSize;
            numBytesToCopy = (numBytesFromPSLeft < size) ? numBytesFromPSLeft : size;
            bcopy(machine->mainMemory + physAddr, buffer + numBytesCopied, numBytesToCopy);
            numBytesCopied += numBytesToCopy;
            size -= numBytesToCopy;
            virtAddr += numBytesToCopy;
        }
    }
    else {
        //printf ("Invalid type passed in.\n");
        ASSERT(FALSE);
    }
    return numBytesCopied; 
}

//----------------------------------------------------------------------
// Write file system call implementation
//----------------------------------------------------------------------
void writeImpl() {
    
    int writeAddr = machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    int fileID = machine->ReadRegister(6);

    char* buffer = new char[size + 1];
    if (fileID == ConsoleOutput) {
        userReadWrite(writeAddr, buffer, size, USER_WRITE);
        buffer[size] = 0; // always terminate
        //printf("%s", buffer);
        openFileManager->consoleWriteLock->Acquire();
        for (int i = 0; i < size; ++i)
            UserConsolePutChar(buffer[i]);
        openFileManager->consoleWriteLock->Release();
    }
    else {
        buffer = new char[size];
       //BEGIN HINTS
       //Fetch data from the user space to this system buffer using  userReadWrite().
       //END HINTS
        int numBytesCopied = userReadWrite(writeAddr, buffer, size, USER_WRITE);
        
        
        UserOpenFile* userFile = currentThread->space->getPCB()->getFile(fileID);
        if (userFile == NULL) {
         return;
        }
        //BEGIN HINTS 
        //Use openFileManager->getFile method  to find the openned file structure (SysOpenFile)
        //Use SysOpenFile->file's writeAt() to write out the above buffer with size listed.
        //Increment the current offset  by the actual number of bytes written.
        //END HINTS 
       // See useropenfile.h and pcb.cc on UserOpenFile class and its methods.
       // See sysopenfile.h and openfilemanager.cc for SysOpenFile class and its methods.
        SysOpenFile* fileStruct = openFileManager->getFile(userFile->indexInSysOpenFileList);
        int numBytesWritten = fileStruct->file->WriteAt(buffer, numBytesCopied, userFile->currOffsetInFile);
        userFile->currOffsetInFile += numBytesWritten;
    }
    delete [] buffer;
}

//----------------------------------------------------------------------
// Read file system call implementation.
//----------------------------------------------------------------------

int readImpl() {

    int readAddr = machine->ReadRegister(4);
    int size = machine->ReadRegister(5);
    int fileID = machine->ReadRegister(6);
    char* buffer = new char[size + 1];
    int numActualBytesRead = size;

    if (fileID == ConsoleInput) {
        int bytesRead = 0;
        while (bytesRead < size) {
            //buffer[bytesRead] = getchar();
            buffer[bytesRead] = UserConsoleGetChar();
            bytesRead++;
        }
    }
    else {
        UserOpenFile* userFile = currentThread->space->getPCB()->getFile(fileID);
        if (userFile == NULL) {
            return 0;
        }

        //BEGIN HINTS
        //Now from openFileManger, find the SystemOpenFile data structure for this userFile.
        //Use ReadAt() to read the file at selected offset to this system buffer buffer[]
        // Adust the offset in userFile to reflect my current position.
        // The above few lines of code are very similar to ones in writeImpl()
        // END HINTS 
        // See useropenfile.h and pcb.cc on UserOpenFile class and its methods.
        // See sysopenfile.h and openfilemanager.cc for SysOpenFile class and its methods.
        SysOpenFile* fileStruct = openFileManager->getFile(userFile->indexInSysOpenFileList);
        int numBytesRead = fileStruct->file->ReadAt(buffer, numActualBytesRead, userFile->currOffsetInFile);
        userFile->currOffsetInFile += numBytesRead;
      
     
    
    }
    //BEGIN HINTS
    //Now copy data from the system buffer to the targted main memory space using userReadWrite()
    //END HINTS
    userReadWrite(readAddr, buffer, size, USER_READ);
    delete [] buffer;
    return numActualBytesRead;
}

//----------------------------------------------------------------------
// Close file system call implementation.
//----------------------------------------------------------------------

void closeImpl() {

    int fileID = machine->ReadRegister(4);
    UserOpenFile* userFile = currentThread->space->getPCB()->getFile(fileID);
    if (userFile == NULL) {
        return;
    } else {
       // BEGIN HINTS
       // Use openFileManager's getFile method to get a pointer to the system-wide SysOpenFile  data structure
       // Call the close method in SysOpenFile
       // Remove the file  in the open file list of this process PCB using PCB::removeFILE().
       // END HINTS
       // See useropenfile.h and pcb.cc on UserOpenFile class and its methods.
       // See sysopenfile.h and openfilemanager.cc for SysOpenFile class and its methods.

       SysOpenFile* filestruct = openFileManager->getFile(userFile->indexInSysOpenFileList);
       filestruct->closedBySingleProcess();
       currentThread->space->getPCB()->removeFile(fileID);
        
       
      
    }
}

//----------------------------------------------------------------------
// IncrementPC
//      Helper function used to increment the values of PCreg, NextPCreg,
//      and PrevPCreg after a syscall.
//----------------------------------------------------------------------

void IncrementPC(void) {

    int currentReg = machine->ReadRegister(PCReg);
    int nextReg = machine->ReadRegister(NextPCReg);
    int prevReg = machine->ReadRegister(PrevPCReg);

    currentReg = currentReg + 4;
    nextReg = nextReg + 4;
    prevReg = prevReg + 4;

    machineLock->Acquire();
    machine->WriteRegister(PCReg, currentReg);
    machine->WriteRegister(NextPCReg, nextReg);
    machine->WriteRegister(PrevPCReg, prevReg);
    machineLock->Release();
}
