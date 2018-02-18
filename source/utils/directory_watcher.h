#ifndef INC_DIRECTORY_WATCHER_H_
#define INC_DIRECTORY_WATCHER_H_

#include <thread>
#include <windows.h>
#include <sys/stat.h>  

class CDirectoyWatcher {
	HANDLE    hDirectory;
	HWND      hWnd;					// To should we notify
	char      folder_name[MAX_PATH];

public:

	static const UINT WM_FILE_CHANGED = WM_USER + 0;

	void start(const char* new_folder, HWND new_hWnd) {
		hWnd = new_hWnd;
		strcpy(folder_name, new_folder);

		if (!hDirectory) {
			hDirectory = CreateFile(folder_name,
				FILE_LIST_DIRECTORY | GENERIC_READ,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				0,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
				0
			);
		}

		std::thread* th = new std::thread([this] {

			DWORD b;
			union {
				FILE_NOTIFY_INFORMATION i;
				char d[sizeof(FILE_NOTIFY_INFORMATION) + MAX_PATH];
			} fni;

			// For ever..
			while (true) {
				memset(&fni, 0x00, sizeof(fni));

				// This call is blocking...
				BOOL rc = ReadDirectoryChangesW(hDirectory,
					&fni,
					sizeof(fni),
					TRUE,           // Watch subtrees also
					FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME,
					&b,
					nullptr,
					0);

				if (fni.i.Action != 0) {
					char name[MAX_PATH];
					size_t n = wcstombs(name, fni.i.FileName, MAX_PATH);

					// Allocate memory because we are using POST msg and 
					// we don't known when will be used.
					char* full_name = new char[MAX_PATH];
					sprintf(full_name, "%s/%s", folder_name, name);

					// Replace '\' by '/'
					char* p = full_name;
					while (*p) {
						if (*p == '\\')
							*p = '/';
						++p;
					}

					struct _stat buf;
					if (_stat(full_name, &buf) == 0) {
						if (buf.st_size != 0) {

							::Sleep(500);

							::PostMessage(hWnd, WM_FILE_CHANGED, fni.i.Action, LPARAM(full_name));
						}
					}

					fni.i.Action = 0;
				}
			}

		});

	}
};

#endif

