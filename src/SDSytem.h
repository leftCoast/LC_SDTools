#ifndef SDSytem_h
#define SDSytem_h


#include <lists.h>
#include <SDVolume.h>



// **************************************************
// *****************  SDSlot ************************
// **************************************************


class SDSlot	: public linkListObj {

	public:
				SDSlot(int inCS);
	virtual	~SDSlot(void);
	
				int				getCS(void);
				bool				begin(void);
				bool				hasVolume(void);
				const char*		getVolName(void);
				unsigned long	getVolSize(void);
				unsigned long	getVolBytes(void);
				
	protected:
				int				CS;
				SDVolume*		currVolume;
};



// **************************************************
// *****************  SDSytem ***********************
// **************************************************


class SDSytem : public linkList {

	public:
				SDSytem(void);
	virtual	~SDSytem(void);
	
				int				findByCS(int CS);
				bool 				addSlot(SDSlot* aSlot);
				int				getNumSlots(void);
				bool				hasVolume(int slotIndex);
				const char*		getVolName(int slotIndex);
				unsigned long	getVolSize(int slotIndex);
				unsigned long	getVolBytes(int slotIndex);
};

#endif