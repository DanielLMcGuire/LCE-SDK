#pragma once

class File;

class FileFilter
{
public:
	virtual bool accept(File *dir) = 0;
};