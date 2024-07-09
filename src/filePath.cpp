
#include <filePath.h>
#include <SD.h>
#include <resizeBuff.h>
#include <strTools.h>


// If you look for a filename ending in '/'. 9 out of 10 times SD will crash your program.
// So, we make sure the silly things are clipped off before asking.
void clipTrialingSlash(char* instr) {

	int index;
	
	if (instr) {								// If we got something..													
		index = 0;								// Set our index to zero.
		while(instr[index]!='\0') {		// Go find the end of the string!
			index++;								// Zoom zoom zoom!
		}											//
		if (index>1) {							// If the end of the string is past the second char..
			if (instr[index-1]=='/') {		// If the char before the end is.. SLASH.
				instr[index-1]='\0';			// Pop a '\0' on that nasty slash.
			}										// Our work is done here.
		}
	}
}

	

//****************************************************************************************
// pathItem:
//
// This will have the default actions. For all the different types pf path items. Root,
// folder, file. This pathItem is pure virtual so it'll never be able to be created.
//****************************************************************************************


// Create a base pathItem.
pathItem::pathItem(void)
	:dblLinkListObj() {
	
	ourType		= noType;	// We have no type.
	name[0]		= '\0';		// We have no name.
}


// What the hell is a grand item?
pathItem::pathItem(pathItem* aGrandItem)
	:dblLinkListObj() {
	
	ourType = aGrandItem->ourType;	
	strcpy(name,aGrandItem->name);
}


// No matter what we become, we'll need to dump this..
pathItem::~pathItem(void) { }
	

// Everyone will have a path type. This is how we get around not having dynamic_cast.	
pathItemType pathItem::getType(void) { return ourType; }


// The name of this, Root, folder or file. The name you would use for a file listing.
char* pathItem::getName(void) { return name; }


// How many chars do we use for our path name? Default is just the chars for our name.
// Folders will be more. But again, this is a good default.
int pathItem::getNumPathChars(void) {  return strlen(name); }

	
// Return a pointer to our parent object.
pathItem* pathItem::getParent(void) { return (pathItem*)dllPrev; }


// Return a pointer to our "next" or "child" object.
pathItem* pathItem::getNext(void) { return (pathItem*)dllNext; }



//****************************************************************************************
// rootItem:
//
// All paths start with one. Basically the SD card.
//****************************************************************************************	


// Create a root item..
rootItem::rootItem(void)
	:pathItem() {
	
	ourType = rootType;	// Our type is root.
	strcpy(name,"/");		// Our only choice of name is '/';
}

	
// Nothing to dispose of.	
rootItem::~rootItem(void) {  }
	

// We only have one choice of name as a root and that's '/'. So the answer is 1.
int rootItem::getNumPathChars(void) { return 1; }


// We are building a text path. The buffer is big enough, just add yourself to the end.
// But NO! Root is the beginning of the path. You don't add the root item to the path. You
// start the path WITH the root item. Notice strcpy() instead of strcat() as used for the
// others.
void rootItem::addNameToPath(char* path) { strcpy(path,name); }



//****************************************************************************************
// fileItem:
//
// These are so useful. You can store stuff in 'em!
//****************************************************************************************


// Create a file item..
fileItem::fileItem(const char* fileName)
	:pathItem() {
	
	ourType = fileType;		// Our type will be file type.
	strcpy(name,fileName);	// And our name is passed in to us.
	upCase(name);				// For now we use uppercase.
}
	

// Files don't have children. So, do nothing.	
fileItem::~fileItem(void) { }


// We are building a text path. The buffer is big enough, just add yourself to the end.
void fileItem::addNameToPath(char* path) { strcat(path,name); }


	
//****************************************************************************************	
// folderItem:
//
// Lets create a folder item. These can be a little more complex..
//****************************************************************************************


// Create a file item..	
folderItem::folderItem(const char* folderName)
	:pathItem() {
	
	ourType = folderType;		// Our type will be folder type.
	strcpy(name,folderName);	// And our name is passed in to us. (Just name, no slash!)
	upCase(name);					// For now we use uppercase.
}


// Nothing special to delete here..
folderItem::~folderItem(void) {  }
	

// Folders in paths have trailing '/'. So we add this in.	
void folderItem::addNameToPath( char* path) {  

	strcat(path,name);
	strcat(path,"/");
}	

	
// Special case for number of path chars. Num chars in name + one for the '/'.	
int folderItem::getNumPathChars(void) { return strlen(name) + 1; }



//****************************************************************************************
// filePath:
//
// And now we get to the actual filePath class. This is the bit the user will typically
// interact with. And the bit that will interact with the SD card's file system as well.
//
//****************************************************************************************



filePath::filePath(void) {

	pathList		= NULL;
	childList	= NULL;
	path			= NULL;
}


filePath::~filePath(void) { reset(); }


// Same effect as just dumping everything. In fact, the destructor uses it.
void filePath::reset(void) {
	
	dumpChildList();					// Loose the chile list, if we have one.
	if (pathList) {					// If we have a non NULL path list..
		pathList->dumpList();		// We dump all the nodes attached to this list node.
		delete(pathList);				// Then we delete the list pointer node.
		pathList = NULL;				// And NULL out the list pointer.
	}
	resizeBuff(0,&path);				// And our temporary path buffer is recycled.
}
 

// Returns the total number of path bytes needed to hold the path string.
int filePath::numPathBytes(void) {

 	pathItem*	trace;
 	int			numBytes;
 	
 	numBytes = 0;														// Zero out the counter.
 	trace = pathList;													// Move trace to the head of our path list.
 	while(trace) {														// While trace is not NULL, do..
 		numBytes = numBytes + trace->getNumPathChars();		// Add this node's num chars to the counter.
		trace = (pathItem*)trace->dllNext;						// Jump to the next node.
	}
	numBytes++;															// Add one for the '\0'. (Even empty lists need '\0')
	return numBytes;													// Return the number of chars..
 }
 

// Paths point at things. Directories, files.. Root? This passes back what kind of thing
// that this is a path to.
pathItemType filePath::getPathType(void) { return getCurrItem()->getType(); }



// Paths point at things. Directories, files.. Root? This passes back the name of what
// this is a path to.
char* filePath::getPathName(void) { return getCurrItem()->getName(); }

	
// Ok, kinda' a special here. -IF- we were to add this name to our current pathList.. What
// do we end up with? A file? A folder? Nothing at all? Find out and return the answer.
pathItemType filePath::checkPathPlus(const char* inPath) {

	File				testFile;
	int				numBytes;
	char*				testPath;
	pathItem*		currentItem;
	pathItemType	theType;
	
	testPath = NULL;											// NULL out the string we'll be allocating.
	theType = noType;											// Set default type as non-type.
	currentItem = getCurrItem();							// Grab the current "last" item on our path.
 	if(currentItem) {											// If we have a current item..
		if (currentItem->getType()==fileType) {		// If the current itme is a file..
			return theType;									// We bail!
		}															// If we're here we have a current time and its not a file.
		numBytes = numPathBytes() + strlen(inPath) + 1;	// Calc. the total number of bytes needed.
		if (resizeBuff(numBytes,&testPath)) {			// If we can grab the memory for this path..
			strcpy(testPath,getPath());					// Grab the path we have.
			strcat(testPath,inPath);						// Add the inPath we got.
			clipTrialingSlash(testPath);					// Loose any stray slashes.
			testFile = SD.open(testPath,FILE_READ);	// Try opening the constructed path.
			if (testFile) {									// If this constructed path exists..
				if (testFile.isDirectory()) {				// If the returned file is a directory..
					theType = folderType;					// We set our type to folderType.
				} else {											// Else, not a folder..
					theType = fileType;						// We set our type to fileType.
				}													//
				testFile.close();								// Close the file/folder.
			}														//
			resizeBuff(0,&testPath);						// And recycle the path string.
		}															//
	}																//
	return theType;											// And return our results.
}


// We'll try to add this path segment to our existing path. If it works out? This new path
// will be our path. If not? No change.
bool filePath::addPath(const char* inPath) {

	tempStr	tempPath(inPath);
	int		numBytes;
	char*		pathBuff;
	bool		success;
	
	success = false;													// Well, it ain't a success yet.									
	if (checkPathPlus(tempPath.getStr())) {					// If this checks out..
		numBytes = numPathBytes() + tempPath.numChars() + 1;	// Calc. num bytes.
		if (resizeBuff(numBytes,&pathBuff)) {					// If we can grab some RAM..
			strcpy(pathBuff,getPath());							// Stuff in our path.
			strcat(pathBuff,tempPath.getStr());					// Add the inPath text.
			success = setPath(pathBuff);							// This better work, already checked.
         resizeBuff(0,&pathBuff);								// Recycle the RAM.
      }																	//
   }																		//
   return success;													// return our result.
}


// This is used for setting the initial path for browsing. Paths must start with '/'
// because they all start at root. They Must also fit in 8.3 file names, because SD is
// brain dead and we're stuck with it for now. If the path is NOT found on the SD card,
// this fails and gives back a false. This does NOT fill out the childList. Only the
// pathList. HA! Look at the botton of the code. There is a call to refreshChildList()
// upon success. So it DOES fill out the child list.
bool filePath::setPath(const char* inPath) {

	rootItem*	theRoot;
	folderItem*	aFolder;
	fileItem*	aFile;
	char			itemName[13];
	int			nIndex;
	int			pIndex;
	bool			fail;
	
	fail = false;													// We've not failed. Yet..
	reset();															// Clear out and reset everything.
	if (inPath) {													// If we have been given a non-NULL path..
		if (inPath[0]=='/') {									// Of said path string starts with '/'..
			theRoot = new rootItem();							// First thing is to grab a new root node.
			if (theRoot) {											// If we got the root node..
				pathList = theRoot;								// Start the list with it.
				pIndex = 1;											// Set pIndex for reading out chars.
			} else {													// Else, no root node? Ran out of RAM!
				return false;										// Return false, we failed.
			}															//
		} else {														// Else, there's no leading '/'?
			return false;											// Seriously, no leading '/'? We can't work with these people!
		}																//
	} else {															// Else, they handed us a NULL string?!?
		return false;												// Where are all these Bozos coming from?
	}																	// Getting here we have at least a root setup.
	nIndex = 0;														// Set nIndex for reading in a name.
	while(inPath[pIndex]!='\0'&&!fail) {					// While we have chars to read and we have not failed..
		if (inPath[pIndex]=='/') {								// If we have run across a '/'..
			switch (checkPathPlus(itemName)) {				// itemName has the folder name, see if it exists and is of type..
				case folderType :									// Folder! All is well!
					aFolder = new folderItem(itemName);		// Have a go at creating a folderItem.	
					if (aFolder) {									// If we got our folderItem..
						aFolder->linkToEnd(pathList);			// Link it to the end of our list.
						nIndex = 0;									// And reset the itemName index.
					} else {											// Else, allocation failed..
						fail = true;								// We can't get the RAM, we fail!
					}													//
				break;												// Done adding a folder to our path.
				case rootType	:									// Root is dealt with above. Should NEVER get to here.
				case fileType	:									// Preceding a '/' must not be a file type.
				case noType		:  fail = true;	break;	// And noType means the path did not exist.
			}															//
			pIndex++;												// And we increment the text path index.
		} else {														// Else, it wasn't a '/' so it "must" be a name char..
			if (nIndex<12) {										// If we've not run out of name chars..								
				itemName[nIndex]=inPath[pIndex];				// Pop the char into the name string.
				nIndex++;											// Bump up the name string index.
				itemName[nIndex]='\0';							// Pop in a 0 char just in case this is the last one.
				pIndex++;											// Bump up the path string index.
			} else {													// Else, we are being asked to put in too long of a name.
				fail = true;										// Not doing this today, call foul!
			}															//
		}																//
	}																	// At this point we've gone through all the folders. We MAY have a file at the end. We'll check.
	if (!fail) {													// If we did not fail..
		if (nIndex>0) {											// If we have a non empty name string..
			clipTrialingSlash(itemName);						// Loose any stray slashes. (just in case..)
			switch (checkPathPlus(itemName)) {				// See what we find..
				case folderType :									// If its a folder? (Really we should fail, but for now, we'll fix it.)
					aFolder = new folderItem(itemName);		// Have a go at creating a folderItem.	
					if (aFolder) {									// If we got our folderItem..
						aFolder->linkToEnd(pathList);			// Link it to the end of our list.
					} else {											// Else, we were unable to allocate a folder item.
						fail = true;								// We can't get the RAM, we fail!
					}													//
				break;												// Last folder is done, bolt!
				case fileType	:									// Its a file..
					aFile = new fileItem(itemName);			// Have a go at creating a fileItem.	
					if (aFile) {									// If we got our fileItem..
						aFile->linkToEnd(pathList);			// Link it to the end of our list.
					} else {											// Else, we were unable to allocate a file item.
						fail = true;								// We can't get the RAM, we fail!
					}													//
				break;												// Last file is complete.
				case rootType	:									// We started with root. Adding another is a fail.
				case noType		:  fail = true;	break;	// And noType means the path did not exist. Its a fail.
			}															//
		}																//
	}																	//
	if (fail) {														// If we were NOT successful..
		reset();														// Clear out and reset everything.
	} else {															//
		refreshChildList();										// You need this for a success.
	}																	//
	return !fail;													// In any case we return our result.
}


// We'll allocate this, but we'll also reuse the memory. So be careful. It won't last.
char* filePath::getPath(void) {

	pathItem*	trace;
	
	resizeBuff(0,&path);										// Clear the last path string.
	if (pathList) {											// If we have a path list..
		if (resizeBuff(numPathBytes(),&path)) {		// Grab the RAM needed to store the path string.
			trace = pathList;									// Gab the root of the path.
			while(trace) {										// While we're pointing to a path node..
				trace->addNameToPath(path);				// Add this nodes name to the string.
				trace = (pathItem*)trace->dllNext;		// And jump to the next node.
			}														//
		}															//
	}																//
	return path;												// Return the result. 
}


// Lets see what's at the end of the list we currently have. This will pass back a pointer
// to it for you to have a peek at it. Maybe ask what kind of thing it is, possibly
// what's it's name? DON'T DELETE IT!
pathItem* filePath::getCurrItem(void) {

	if (pathList) {									// Sanity check, we have a list right?
		return (pathItem*)pathList->getLast();	// Grab the one at the end and return its address.
	}														// If we got here? No list.
	return NULL;										// Return NULL.
}


// If we have a current item. Pass back a pointer to it's name. (Don't mess with it!)
char* filePath::getCurrItemName(void) {

	pathItem*	currItem;
	
	currItem = getCurrItem();			// Grab the address of the last node on our list.
	if (currItem) {						// Got one?
		return currItem->getName();	// Return the pointer to its name string.
	}											// If we got here here is no list.
	return NULL;							// So return NULL.
}


// There are times when we need to clear out the child list. So it's here.
void filePath::dumpChildList(void) {
	
	if (childList) {					// If we currently have a child list..
		childList->dumpList();		// Dump the tail of the list.
		delete(childList);			// Dump what the list is pointing to.
		childList = NULL;				// Null out the list variable.
	}
}


// We need to refresh the current child list. Dump what we have and fill it with what we
// got. What we're saying is : If you are a folder, refresh your list of items you hold.
void filePath::refreshChildList(void) {

	tempStr		ourPath;
	File			dir;
	File			entry;
	pathItem*	newItem;
	bool			done;
	
	dumpChildList();																		// Clear out the child list.
	ourPath.setStr(getPath());															// Save off a copy of our current path.
	if (ourPath.getStr()) {																// If we got our path string..
		dir = SD.open(ourPath.getStr());												// Try opening the directory.
		if (dir) {																			// If our directory exists..
			if (dir.isDirectory()) {													// If it is a directory..
				dir.rewindDirectory();                 							// Rewind it to the first entry.
				done = false;                          							// We ain't done yet.
				do {                                   							// Start looping through the entries.
					entry = dir.openNextFile();										// Grab an entry.
					if (entry) {															// If we got an entry..
						if (strlen(entry.name()) <13) {								// If the entry name is 8.3 or shorter..
							if (entry.isDirectory()) {									// If it is a directory..
								newItem = new folderItem((char*)entry.name());	// Create the folderItem.
							} else {															// Else its a file..
								newItem = new fileItem((char*)entry.name());		// Create the new fileItem.
							}																	//
							if (newItem) {													// We actually got a new item..
								if (childList) { 											// If there is a list..
									newItem->linkToEnd(childList);					// Link yourself to the end of this list.
								} else {														// Else there is no list.. (yet)
									childList = newItem;									// You are the list!
								}																//
							} else {															// Else, it seems we've run out of RAM..
								done = true;												// We're done with this.
								dumpChildList();											// Our world is crumbling. Clear out the child list.
							}																	//
						}																		//
						entry.close();                								// And we close the entry.	
					} else {                            							// Else, we didn't get an entry from above.
						done = true;                     							// No entry means, we are done here.
					}																			//
				} while (!done);                       							// And we do this loop, over and over, while we are not done.
			}																					//
			dir.close();                              							// Looping through entries is done, close up the original file.
		}																						//
	} else {																					// Else, we can't even open the path we have?
		Serial.println("In refreshChildList()");									//
		Serial.print("Can't open path [");											// This is bad, it actually warrants an error message.
		Serial.print(ourPath.getStr());
		Serial.println("] ");
		Serial.println("All paths should, at least, be valid!");
	}
}


// Lest see how many kids we have..
int filePath::numChildItems(void) {

	if (childList) {								// If the pointer is Non NULL..
		return childList->countTail() + 1;	// Gotta' add one for the first one.
	}													//
	return 0;										// Otherwise we pass back a big zero.
}


// We want to SEE the child with this name. This does NOT unhook the child. It just gives
// you a pointer to it. You can look, but don't mess with it!
pathItem*  filePath::getChildItemByName(const char* name) {

	pathItem*	trace;
	int			numBytes;
	char*			tempName;
	char			fileName[13];
	bool			success;
	
	success = false;											// Not a success yet.
	tempName = NULL;											// Start tempName at NULL.
	numBytes = strlen(name)+1;								// How many bytes will we need to copy the passed in name?
	if (resizeBuff(numBytes,&tempName)) {				// If we can allocate a buffer for the passed in name..
		strcpy(tempName,name);								// Make a local copy of it..
		upCase(tempName);										// Capitalize it.
		trace = childList;									// Point trace out the child list.
		while(trace && !success) {							// While trace is non_NULL, and we ain't found the named node..
			strcpy(fileName,trace->name);					// Make local copy of the node's name.
			upCase(fileName);									// Capitalize it.
			if (!strcmp(fileName,tempName)) {			// If the names match..
				success = true;								// Then we are a success!
			} else {												// Else, no match..
				trace = (pathItem*)trace->dllNext;		// So, jump to the next node to check.
			}														//
		}															//
		resizeBuff(0,&tempName);							// Done with our copy of the file name. recycle it.
	}																//
	if (success) {												// In the end if we had success.
		return trace;											// Trace is pointing to the node, so return trace.
	}																//
	return NULL;												// If we got here we didn't find it. Pass back NULL.
}
			
	
// If one has a list of child items to choose from. This grabs the one with this passed
// in name, copies it, then adds the copy to the end of the path list.	
bool filePath::pushChildItemByName(const char* name) {
	
	tempStr		theName(name);
	pathItem*	theChild;
	pathItem*	newItem;
	bool			success;
	
	success = false;												// Not a success yet.
	theChild = getChildItemByName(theName.getStr());	// Try grabbing a child by this name.
	newItem = NULL;												// We start newItem at NULL.
	if (theChild) {												// If we were able to find such child node..
		switch(theChild->getType()) {							// Check the type..
			case noType			:									// No type? Not possible.
			case rootType		: break;							// Root?! Root can NOT be a child! Fail!
			case fileType		:									// File type? This is good.
				newItem = new fileItem(name);					// Create the new item.
			break;													// Jump out.
			case folderType	:									// Folder type? This is also good.
				newItem = new folderItem(name);				// Create the new item.
			break;													// And jump.
		}																//
		if (newItem) {												// If we were able to create the new item..
			success = pushItem(newItem);						// Try putting the new item at the end of the pathList. Save off the result.
			if (!success) {										// If, for some reason, we couldn't push it on there..
				delete(newItem);									// Make sure to delete it. No leaking!
			}															//
		}																//
	}																	//
	return success;												// report or success. (Or failure)
}


// Have a fileItem? This will add it to the end of your path list. (If possible)	
bool filePath::pushItem(pathItem* theNewGuy) {

	pathItem*	theLastGuy;
	bool			success;
	
	success = false;													// Not a success yet.
	if (theNewGuy) {													// If we got a non-NULL new guy..
		if (pathList) {												// If we have a non-NULL pathList..
			theLastGuy = (pathItem*)pathList->getLast();		// We grab the last item on the list.
			if (theLastGuy->getType()!=fileType) {				// If the last guy is not a file. (Dead end)
				theNewGuy->linkToEnd(pathList);					// Tell the new guy to link hisself to the end of the list.
				success = true;										// Shit howdy! Looks ike we done did it!
			}																//
		} else {															// Else, we had an empty (NULL) pathList..
			if (theNewGuy->getType()==rootType) {				// If this new guy is a rootType..
				pathList = theNewGuy;								// Point the pathList at this new guy.
				success = true;										// And there we go. A success!
			}																//
		}																	//
	}																		//
	refreshChildList();												// This will at least clean out the old list.
	return success;													// And we return our result.
}


// Pop off the last item from your path list. Now, Do we really want to pop off the root
// item? Because now it will. Seems that the list should at lest contain the root. Oh
// well, we'll run it like this and see how it goes.
void filePath::popItem(void) {
	 
	pathItem*	theLastGuy;
	
	if (pathList) {								// If we have a pathList to work with..
		theLastGuy = getCurrItem();			// We grab the last item on the list.
		if (theLastGuy) {							// If we got an item.. (I can't see how we couldn't, really..)
			if (pathList==theLastGuy) {		// If the pathList is actually pointing to the last guy.. IE the root.
				pathList = NULL;					// Set pathList to NULL. Because its going to be empty.
			}											//
			delete (theLastGuy);					// Delete this last node. Wich will unhook it if necessary.
		}												//
	}													//
	refreshChildList();							// This will at least clean out the old list.
}

		
// If we're pointed at a directory, this should clear it out. And yes, it can get
// recursive.
bool filePath::clearDirectory(void) {

	pathItem*	trace;
	
	if(getPathType()==folderType||getPathType()==rootType) {	// Let's see what we're a path to..
		refreshChildList();												// Make sure we have fresh kid list.
		do {																	// For every.. something..
			if (!numChildItems()) {										// We have no kids?
				return true;												// Well that's a success!
			} else {															// Oops, we got kids to clear out.
				trace = childList;										// Grab a pointer to the first child.
				if (pushChildItemByName(trace->getName())) {		// If we can become the path to that child..
					if (!deleteCurrentItem()) {						// If we can't delete that child..
						popItem();											// Back to our old self..
						return false;										// We fail.
					}															//
				} else {														// Can't do the push?
					return false;											// Something broke, fail.
				}																//
			}
		}	while(true);													// We're going to rely on recursion.. 
	} else {																	//
		return true;														// Why? It's clear ain't it?
	}																			//
	return false;															// If we get here? Something bad happened.
} 

			
// Delete what we are pointing at. This is going to get all recursive, so hold on tight.
bool filePath::deleteCurrentItem(void) {
	
	bool success;
	
	success = false;							// Ain't a succes yet.
	switch(getPathType()) {					// Let's see what we're a path to..
		case noType		:						// No type? Not possible.
		case rootType	: 						// Root?! Root can NOT be deleted. Fail!
		break;									// These are fails. Lets go..
		case fileType	:						// File type? This is good.
			if (SD.remove(getPath())) {	// Tell SD to delete the file.
				popItem();						// Pop that item from our path.
				success = true;				// Success!
			}										//
		break;									// Jump out.
		case folderType	:					// Folder type? This is also good.
			if (clearDirectory()) {				// If we can clear this directory..
				if (SD.rmdir(getPath())) {	// Tell SD to delete the file.
					popItem();							// Pop this item from our path.
					success = true;					// Success!
				}
			}											// 							
		break;										// And jump.
	}													//
	return success;								// If we get here? Something didn't work.
}

