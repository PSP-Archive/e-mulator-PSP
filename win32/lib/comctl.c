
#pragma comment(lib, "COMCTL32.LIB")
#pragma comment(lib, "COMDLG32.LIB")

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <commdlg.h>


char *gui_getRomPath(HWND hWnd)
{
	static OPENFILENAME ofn;				// common dialog box structure
	static char			szFile[260];		// buffer for file name
	HWND				hwnd;				// owner window
	HANDLE				hf;					// file handle
	static char			oldDir[1024]="";

	InitCommonControls();

	szFile[0]=0;
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.hInstance=NULL;
	// Display the Open dialog box. 

	ofn.lpstrFilter = 
		"ALL\0"
		    "*.nes;"
			"*.gb;"
			"*.gbc;"
			"*.sgb;"
			"*.npc;"
			"*.ngp;"
			"*.gg;"
			"*.sms;"
			"*.zip;"
			"*.ws;"
			"*.wsc;"
			"*.pce;"
			"*.sgx;"
			"*.toc;"
			"*.lnx;"
		"\0";

	if (GetOpenFileName(&ofn)==TRUE)
		return(ofn.lpstrFile);

	return(NULL);
}


char *gui_getSavePath(HWND hWnd,int bSave)
{
	static OPENFILENAME ofn;				// common dialog box structure
	static char			szFile[260];		// buffer for file name
	HWND				hwnd;				// owner window
	HANDLE				hf;					// file handle
	static char			oldDir[1024]="";

	InitCommonControls();

	szFile[0]=0;
	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(OPENFILENAME));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.hInstance=NULL;
	// Display the Open dialog box. 

	ofn.lpstrFilter = 
		"SAV(*.s*)\0*.s*\0"
		"ALL(*.*)\0*.*\0"
		"\0";

	if(bSave){
		if (GetSaveFileName(&ofn)==TRUE)
			return(ofn.lpstrFile);
	}
	else {
		if (GetOpenFileName(&ofn)==TRUE)
			return(ofn.lpstrFile);
	}

	return(NULL);
}