#pragma once

/*
scantree.hpp

Сканирование текущего каталога и, опционально, подкаталогов на
предмет имен файлов
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include "bitflags.hpp"
#include "array.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <map>
#include <set>


enum
{
	// эту фигню может ставить плагин (младшие 8 бит)
	FSCANTREE_RETUPDIR         = FRS_RETUPDIR,
	// FSCANTREE_RETUPDIR causes GetNextName() to return every directory twice:
	// 1. when scanning its parent directory 2. after directory scan is finished
	FSCANTREE_RECUR            = FRS_RECUR,
	FSCANTREE_SCANSYMLINK      = FRS_SCANSYMLINK,

	// в младшем слове старшие 8 бита служебные!
	FSCANTREE_SECONDPASS       = 0x00002000, // то, что раньше было было SecondPass[]
	FSCANTREE_SECONDDIRNAME    = 0x00004000, // set when FSCANTREE_RETUPDIR is enabled and directory scan is finished
	FSCANTREE_INSIDEJUNCTION   = 0x00008000, // - мы внутри симлинка?

	// здесь те флаги, которые могут выставляться в 3-м параметре SetFindPath()
	FSCANTREE_FILESFIRST       = 0x00010000, // Сканирование каталга за два прохода. Сначала файлы, затем каталоги
	FSCANTREE_NOFILES          = 0x00020000, // Don't return files
	FSCANTREE_NODEVICES        = 0x00040000, // Don't return devices
	FSCANTREE_NOLINKS          = 0x00080000, // Don't return symlinks
	FSCANTREE_CASE_INSENSITIVE = 0x00100000 // Currently affects only english characters
};

struct ScanTreeData
{
	BitFlags Flags;
	FindFile* Find;
	FARString RealPath;
	ScanTreeData(): Find(nullptr) { }
	~ScanTreeData()
	{
		if(Find)
		{
			delete Find;
		}
	}
};

class ScannedINodes
{
	struct Map : std::map<uint64_t, std::set<uint64_t> > {} _map;
public:
	bool Put(uint64_t d, uint64_t ino)
	{
		return _map[d].insert(ino).second;
	}
};

class ScanTree
{
	private:
		BitFlags Flags;
		TPointerArray<ScanTreeData> ScanItems;
		FARString strFindPath;
		FARString strFindMask;

	public:
		ScanTree(int RetUpDir,int Recurse=1,int ScanJunction=-1);

	public:
		// 3-й параметр - флаги из старшего слова
		void SetFindPath(const wchar_t *Path,const wchar_t *Mask, const DWORD NewScanFlags = FSCANTREE_FILESFIRST);
		bool GetNextName(FAR_FIND_DATA_EX *fdata, FARString &strFullName);

		void SkipDir();
		int IsDirSearchDone() {return Flags.Check(FSCANTREE_SECONDDIRNAME);};
		int InsideJunction()   {return Flags.Check(FSCANTREE_INSIDEJUNCTION);};
		
		bool IsSymlinksScanEnabled() {return Flags.Check(FSCANTREE_SCANSYMLINK); }
};
