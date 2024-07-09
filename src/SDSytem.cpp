#include <SDSytem.h>



// **************************************************
// *****************  SDSlot ************************
// **************************************************


SDSlot::SDSlot(int inCS)
	: linkListObj() {
	
	currVolume = NULL;
	CS = inCS;
}
	
	 
SDSlot::~SDSlot(void) { if (currVolume) delete(currVolume); }


int SDSlot::getCS(void) { return CS; }


// Not really sure what I need to do here yet.	
bool SDSlot::begin(void) {
	
	return true;
}


bool SDSlot::hasVolume(void) { return currVolume!=NULL; }


char* SDSlot::getVolName(void) {

	if (currVolume) {
		return currVolume->getName();
	}
	return NULL;
}


unsigned long SDSlot::getVolSize(void) {
	
	if (currVolume) {
		return currVolume->getSize();
	}
	return 0;
}


unsigned long SDSlot::getVolBytes(void) {
	
	if (currVolume) {
		return currVolume->getNumBytes();
	}
	return 0;
}



// **************************************************
// *****************  SDSytem ***********************
// **************************************************


class SDSytem : public linkList {

SDSytem::SDSytem(void)
	: linkList() {  }


SDSytem::~SDSytem(void) {  }


int SDSytem::findByCS(int CS) {

	SDSlot*	slotPtr;
	int		index;
	
	index = 0;
	slotPtr = (SDSlot*)getFirst();
	while(slotPtr) {
		if (slotPtr->getCS()==CS) {
			return index;
		}
		slotPtr = (SDSlot*)slotPtr->getNext();
		index++;
	}
	return -1;
}

	
bool SDSytem::addSlot(int CS) {

	SDSlot* slotPtr;
	
	if (findByCS(CS)==-1) {				// If we don't already have this one..
		slotPtr = new SDSlot(CS);		// Have a go at creating one.
		if (slotPtr) {						// We got one?
			if (slotPtr->begin()) {		// Se if we can initialize it.
				addToEnd(slotPtr);		// Good! Add it to our list.
				return true;				// And beam out with a success!
			} else {							// Else initialization failed?
				delete(slotPtr);			// Recycle the bad slot.
			}									//
		}										//
	}											//
	return false;							// We get here? We failed.
}


int SDSytem::getNumSlots(void) { return getCount(); }


bool SDSytem::hasVolume(int slotIndex) {

	SDSlot* slotPtr;
	
	slotPtr = (SDSlot*)getByIndex(slotIndex);
	if (slotPtr) {
		return slotPtr->hasVolume();
	}
	return false;
}
	
	
const char* SDSytem::getVolName(int slotIndex) {

	SDSlot* slotPtr;
	
	slotPtr = (SDSlot*)getByIndex(slotIndex);
	if (slotPtr) {
		return slotPtr->getVolName();
	}
	return NULL;
}
	
	 
unsigned long SDSytem::getVolSize(int slotIndex) {

	SDSlot* slotPtr;
	
	slotPtr = (SDSlot*)getByIndex(slotIndex);
	if (slotPtr) {
		return slotPtr->getVolSize();
	}
	return 0;
}


unsigned long SDSytem::getVolBytes(int slotIndex) {

	SDSlot* slotPtr;
	
	slotPtr = (SDSlot*)getByIndex(slotIndex);
	if (slotPtr) {
		return slotPtr->getVolBytes();
	}
	return 0;
}

