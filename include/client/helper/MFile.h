#pragma  once
namespace dspeak{
	namespace util{
		//copy from cloudy
class FileMapping{
private:
	HANDLE hFile_, hMapping_;
	char* pData_;
	unsigned fsize_;
public:
	FileMapping()
	{
		hFile_ = INVALID_HANDLE_VALUE;
		hMapping_ = NULL;
		pData_ = NULL;
		fsize_ = 0;
	}
	~FileMapping()
	{ close(); }
	unsigned size() { return fsize_; }
	const char* data() { return pData_; }
	HANDLE FileHandle() { return hFile_; }
	void close()
	{
		fsize_ = 0;
		if (pData_ != NULL)
		{
			UnmapViewOfFile(pData_);
			pData_ = NULL;
		}
		if (hMapping_ != NULL)
		{
			CloseHandle(hMapping_);
			hMapping_ = NULL;
		}
		if (hFile_ != INVALID_HANDLE_VALUE)
		{
			CloseHandle(hFile_);
			hFile_ = INVALID_HANDLE_VALUE;
		}
	}
	bool open(LPCTSTR fname)
	{
		close();
		hFile_ = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile_ == INVALID_HANDLE_VALUE) {return false;}
		fsize_ = GetFileSize(hFile_, NULL);
		if (fsize_ == 0) {return true;}
		hMapping_ = CreateFileMapping(hFile_, NULL, PAGE_READONLY, 0, 0, NULL);
		if (hMapping_ == NULL) {return false;}
		pData_ = (char*)MapViewOfFile(hMapping_, FILE_MAP_READ, 0, 0, 0);
		if (pData_ == NULL) {return false;}
		return true;
	}
};
	}
}
