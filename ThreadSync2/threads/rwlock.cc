#include<stdio.h>
#include <iostream> //


#include "rwlock.h"

//Implement all methods of RWLock defined in rwlock.h
//Your solution for  Task 3 
//TODO

RWLock::RWLock(){ 
	Active_R = 0; 
	Waiting_R = 0; 
	Active_W = 0; 
	Waiting_W = 0; 
	lock = new Lock("lock");
	okToRead = new Condition("readOK");
	okToWrite = new Condition("writeOK");
}

RWLock::~RWLock(){ 
	delete lock;
	delete okToRead;
	delete okToWrite;
}

void RWLock::startRead(){ 
	lock->Acquire();
	while(Active_W > 0){
		Waiting_R ++;
		okToRead->Wait(lock);
		Waiting_R--;
	}
	Active_R ++;
	lock->Release();
}

void RWLock::doneRead(){ 
	lock->Acquire();
	Active_R--;
	if(Active_R == 0){
		okToWrite->Signal(lock);
	}
	lock->Release();
}

void RWLock::startWrite(){
	lock->Acquire();
	while(Active_R + Active_W > 0){
		Waiting_W++;
		okToWrite->Wait(lock);
		Waiting_W--;
	}
	Active_W++;
	lock->Release();
}

void RWLock::doneWrite(){ 
	lock->Acquire();
	Active_W --; 
	if(Waiting_W > 0){
		okToWrite->Signal(lock);
	}else if(Waiting_R > 0){
		okToRead->Broadcast(lock);
	}
	lock->Release();
}
