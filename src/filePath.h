#ifndef filePath_h
#define filePath_h

#include <lists.h>

extern void clipTrialingSlash(char* instr);


// NOTE : What is this filePath stuff used for?
//
// File path is all about locating files and folders. A valid path, in it's universe, is
// one that it can actually find. To that end, it checks every path given to it, to make
// sure it can find it. Be it root, a folder or a file. Otherwise, it returns errors or
// nothing. Either the path had invalid name in it, or it just couldn't find the item you
// asked for.


//****************************************************************************************
// pathItem:
//
// This will have the default actions. For all the different types pf path items. Root,
// folder, file. This pathItem is pure virtual so it'll never be able to be created.
//****************************************************************************************


enum pathItemType { noType, rootType, folderType, fileType };

class pathItem : public dblLinkListObj {

	public:
				pathItem(void);
				pathItem(pathItem* aGrandItem);
	virtual	~pathItem(void);
				
				void				copyItem(pathItem* aGrandItem);
				bool				setName(const char* inName);
				pathItemType	getType(void);
	virtual	char* 			getName(void);
	virtual	int				getNumPathChars(void);
	virtual	void				addNameToPath(char* path);
	virtual	pathItem*		getParent(void);
				pathItem*		getNext(void);
				
				pathItemType	ourType;
				char				name[13];
};



//****************************************************************************************
// rootItem:
//
// All paths start with one. Basically the SD card.
//****************************************************************************************


class rootItem : public	pathItem {
	
	public:
				rootItem(void);
	virtual	~rootItem(void);
	
	virtual	int	getNumPathChars(void);
	virtual	void	addNameToPath(char* path);
};



//****************************************************************************************
// fileItem:
//
// These are so useful. You can store stuff in 'em!
//****************************************************************************************


class fileItem : public	pathItem {
	
	public:
				fileItem(const char* fileName);
	virtual	~fileItem(void);
	
	virtual	void addNameToPath(char* path);
};



//****************************************************************************************	
// folderItem:
//
// A little more complicated than a silly old file.
//****************************************************************************************


class folderItem : public	pathItem {
	
	public:
				folderItem(const char* folderName);
	virtual	~folderItem(void);
	
	virtual	int	getNumPathChars(void);
	virtual	void	addNameToPath(char* path);
};



//****************************************************************************************
// filePath:
//
// And now we get to the actual filePath class. This is the bit the user will typically
// interact with. And the bit that will interact with the SD card's file system as well.
//
//****************************************************************************************


class filePath {

	public:
				filePath(void);
	virtual	~filePath(void);
	
	virtual	void				reset(void);
				int				numPathBytes(void);
				pathItemType	getPathType(void);
				char*				getPathName(void);
				pathItemType	checkPathPlus(const char* inPath);
				bool				addPath(const char* inPath);
	virtual	bool				setPath(const char* inPath);
	virtual	char*				getPath(void);
				pathItem*		getCurrItem(void);
				char*				getCurrItemName(void);
				void				dumpChildList(void);
				void				refreshChildList(void);
				int				numChildItems(void);
				pathItem*		getChildList(void);
				pathItem*		getChildItemByName(const char* name);
				bool				pushChildItemByName(const char* name);
	virtual	void				popItem(void);
				bool				clearDirectory(void);
				bool				deleteCurrentItem(void);
			
	protected:
	virtual	bool				pushItem(pathItem* theNewGuy); // Why is this virtual?
				
				pathItem*	pathList;
				pathItem*	childList;
				char*			path;
};

#endif