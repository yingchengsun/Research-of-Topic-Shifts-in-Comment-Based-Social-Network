#include "stdafx.h"
#include "WzdDrDlg.h"
#include "Shlobj.h"

CString CWzdDirDlg::m_sRootDir;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWzdDirDlg::CWzdDirDlg()
{

}

CWzdDirDlg::~CWzdDirDlg()
{

}

CString CWzdDirDlg::GetDirectory(CWnd *pParent,LPCSTR lpszRoot,LPCSTR lpszTitle)
{
	CString str;
	BROWSEINFO bi;
    bi.hwndOwner=pParent->m_hWnd;	
    bi.pidlRoot=0;					
    bi.pszDisplayName=0;			
    bi.lpszTitle=lpszTitle;			
	bi.lpfn = BrowseCallbackProc;	
    bi.lParam=0;					
    bi.ulFlags=
		BIF_RETURNONLYFSDIRS |		
		BIF_STATUSTEXT |			
		0; 
	m_sRootDir=lpszRoot;

	LPITEMIDLIST lpItemId=::SHBrowseForFolder(&bi); 
	if (lpItemId)
	{
		LPTSTR szBuf=str.GetBuffer(MAX_PATH);
		::SHGetPathFromIDList(lpItemId, szBuf);
		::GlobalFree(lpItemId);
		str.ReleaseBuffer();
	}

	return str;
}

int CALLBACK BrowseCallbackProc(HWND hwnd,UINT msg,LPARAM lp, LPARAM pData)
{
	TCHAR buf[MAX_PATH];

	switch(msg) 
	{
		case BFFM_INITIALIZED: 
			strcpy(buf,CWzdDirDlg::m_sRootDir);
			::SendMessage(hwnd,BFFM_SETSELECTION,TRUE,(LPARAM)buf);
			break;

		case BFFM_SELCHANGED:
			if (::SHGetPathFromIDList((LPITEMIDLIST) lp ,buf)) 
				SendMessage(hwnd,BFFM_SETSTATUSTEXT,0,(LPARAM)buf);
			break;
	}
	return 0;
}
