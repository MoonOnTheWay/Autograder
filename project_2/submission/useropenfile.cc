// useropenfile.cc
//

#include "useropenfile.h"


UserOpenFile::UserOpenFile(char* filename, int fileTableIndex, int currentPosition)
{
	this->filename = filename;
	this->fileTableIndex = fileTableIndex;
	this->currentPosition = currentPosition;
}

UserOpenFile::UserOpenFile(const UserOpenFile* other){
	this->filename = other->filename;
	this->fileTableIndex = other->fileTableIndex;
	this->currentPosition = other->currentPosition;
}