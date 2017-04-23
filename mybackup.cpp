#include <iostream>
#include <string>
#include <fstream>
#include <windows.h>
#include <direct.h>
#include <shellapi.h>
#include <atlstr.h>
#include <tchar.h>
using namespace std;

void CompareTime(TCHAR* src, TCHAR* dest);
int CopyDir(TCHAR* src, TCHAR* dest);
string UpDataTime(TCHAR* src);
void write_text_to_log_file(TCHAR *fileName);

int _tmain(int argc, TCHAR *argv[]) {
	if (argc != 3) {
		cout << "exe생성!\n";
		return 1;
	}

	char src_path[MAX_PATH];
	WideCharToMultiByte(CP_ACP, 0, argv[1], MAX_PATH, src_path, MAX_PATH, NULL, NULL);

	if (!_chdir(src_path)) {
		cout << "complete change work directory\n\n";
	}
	else {
		cout << "fail to change work directory\n\n";
		return 1;
	}

	TCHAR* src = argv[1];
	TCHAR* dest = argv[2];

	if (!CreateDirectory((LPCWSTR)dest, NULL)) {
		int n = GetLastError();
		if (n == ERROR_ALREADY_EXISTS) {
			cout << "exist directory ===> Update check" << endl;
			CompareTime(src, dest);
		}
		else if (n == ERROR_PATH_NOT_FOUND) {
			cout << "error for path\n";
			return 1;
		}
	}
	else {
		cout << "File Copy First time" << endl;
		int nD = 0;
		nD = CopyDir(src, dest);

		if (nD == 0) {
			cout << "complete copy directory\n";
		}
		else {
			cout << "failed copy\n";
			return 1;
		}
	}
	return 0;
}

int CopyDir(TCHAR* src, TCHAR* dest) {
	int result = 1;

	TCHAR* src_path = new TCHAR[_tcslen(src) + 2]();
	TCHAR* dest_path = new TCHAR[_tcslen(dest) + 2]();

	_tcscat(src_path, src);
	_tcscat(dest_path, dest);

	cout << "In CopyDir" << endl;
	
	SHFILEOPSTRUCT sf;
	sf.hwnd = NULL;
	sf.wFunc = FO_COPY;
	sf.pFrom = (PCZZWSTR)src_path;
	sf.pTo = (PCZZWSTR)dest_path;
	sf.fFlags = FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR | FOF_NOERRORUI | FOF_SILENT;
	sf.fAnyOperationsAborted = false;
	sf.hNameMappings = NULL;
	sf.lpszProgressTitle = NULL;

	result = SHFileOperation(&sf);   //success : 0, non-success : !0

	cout << result << "  (success : 0, non-success : !0) " << endl;

	return result;
}

string UpDataTime(TCHAR* src) {

	TCHAR* fname = new TCHAR[_tcslen(src) + 2]();
	_tcscat(fname, src);

	//temporary storage for file sizes
	DWORD dwFileSize;
	DWORD dwFileType;
	DWORD dwErr = 0;
	DWORD dwRetries = 0;
	BOOL bSuccess = FALSE;

	//the files handle
	HANDLE hFile1;
	FILETIME ftCreate, ftAccess, ftWrite;
	SYSTEMTIME stUTC, stLocal, stUTC1, stLocal1, stUTC2, stLocal2;

	hFile1 = CreateFile((LPCTSTR)fname,	//file to open 
		GENERIC_READ | GENERIC_WRITE, //open for reading 
		FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, //share for reading 
		NULL, //default security 
		OPEN_ALWAYS, //existing file only 
		FILE_FLAG_BACKUP_SEMANTICS, // normal file 
		NULL); //no attribute template 

	if (hFile1 == INVALID_HANDLE_VALUE) {
		printf("Could not open %s file, error %d\n", fname, GetLastError());
		dwErr = GetLastError();
	}

	//Retrieve the file times for the file.
	if (!GetFileTime(hFile1, &ftCreate, &ftAccess, &ftWrite)) {
		printf("Something wrong lol!\n");
		return FALSE;
	}

	//Convert the last-write time to local time.
	FileTimeToSystemTime(&ftWrite, &stUTC2);
	SystemTimeToTzSpecificLocalTime(NULL, &stUTC2, &stLocal2);

	
	printf("Last written: %d/%02d/%02d %02d:%02d\n", stLocal2.wYear, stLocal2.wMonth,
		stLocal2.wDay, stLocal2.wHour, stLocal2.wMinute);

	string data = to_string(stLocal2.wYear);
	if (stLocal2.wMonth < 10) {	data = data + "0" + to_string(stLocal2.wMonth);}
	else { data = data + to_string(stLocal2.wMonth);}
	if (stLocal2.wDay < 10) { data = data + "0" + to_string(stLocal2.wDay); }
	else { data = data + to_string(stLocal2.wDay); }
	if (stLocal2.wHour < 10) { data = data + "0" + to_string(stLocal2.wHour); }
	else { data = data + to_string(stLocal2.wHour); }
	if (stLocal2.wMinute < 10) { data = data + "0" + to_string(stLocal2.wMinute); }
	else { data = data + to_string(stLocal2.wMinute); }

	CloseHandle(hFile1);

	return data;
}

//src와 dest의 시간을 비교. **특정 파일만 확인하고 업데이트 하는 것이 아니라
//경로 안의 모든 파일 다 업데이트..
void CompareTime(TCHAR* src, TCHAR* dest) {
	string updatefile = UpDataTime(src);
	string backupfile = UpDataTime(dest);
	
	long double updateTime = atof(updatefile.c_str());
	long double backupTime = atof(backupfile.c_str());

	if (updateTime > backupTime) { //파일을 다시 업데이트
		cout << "**Need to update**" << endl;
		CopyDir(src, dest);
		cout << endl << "**Update completed**" << endl;
		write_text_to_log_file(src);
	}
	else { //업데이트 필요x
		cout << "**No need to update**" << endl;
	}
}

void write_text_to_log_file(TCHAR *fileName)
{
	ofstream log_file("mybackup.log");

	//TCHAR to char
	char fName[MAX_PATH];
	WideCharToMultiByte(CP_ACP, //Code page to use in performing the conversion
		0, //dwFlags is conversion type
		fileName, //lpWideCharStr
		MAX_PATH, //cccWideChar
		fName, //lpMultiByteStr
		MAX_PATH, //cbMultiByte
		NULL, //lpDefaultChar
		NULL);//lpUsedDefaultChar
	
	log_file << fName << endl;
	log_file.close();
}