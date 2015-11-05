// TextClassify.h : main header file for the TEXTCLASSIFY application
//

#if !defined(AFX_TEXTCLASSIFY_H__572A2BBF_D7B6_4723_9E04_826E45E6221E__INCLUDED_)
#define AFX_TEXTCLASSIFY_H__572A2BBF_D7B6_4723_9E04_826E45E6221E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CTextClassifyApp:
// See TextClassify.cpp for the implementation of this class
//

class CTextClassifyApp : public CWinApp
{
public:
	CTextClassifyApp();
	CString m_strPath;
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextClassifyApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation
	//{{AFX_MSG(CTextClassifyApp)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
extern CTextClassifyApp theApp;

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTCLASSIFY_H__572A2BBF_D7B6_4723_9E04_826E45E6221E__INCLUDED_)
