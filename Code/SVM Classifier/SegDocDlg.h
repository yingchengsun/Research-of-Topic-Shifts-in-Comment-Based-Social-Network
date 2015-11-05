#if !defined(AFX_SEGDOCDLG_H__C8AC14EA_C0C3_4027_B692_384E5C88AAB1__INCLUDED_)
#define AFX_SEGDOCDLG_H__C8AC14EA_C0C3_4027_B692_384E5C88AAB1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SegDocDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSegDocDlg dialog

struct SegDocsParam
{
	CString	m_strTarget;
	CString	m_strSource;
	BOOL	m_bContainTag;
	BOOL	m_bMultiDocs;
	BOOL	m_bDelStopwords;
};


class CSegDocDlg : public CDialog
{
// Construction
public:
	CSegDocDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CSegDocDlg)
	enum { IDD = IDD_WORDSEG };
	BOOL	m_bDelStopwords;
	CString	m_strSource;
	CString	m_strTarget;
	BOOL	m_bMultiDocs;
	//}}AFX_DATA
	BOOL	m_bContainTag;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSegDocDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CSegDocDlg)
	virtual void OnOK();
	afx_msg void OnBtnBrowse1();
	afx_msg void OnBtnBrowse2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
extern SegDocsParam theSegDocsParam;

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SEGDOCDLG_H__C8AC14EA_C0C3_4027_B692_384E5C88AAB1__INCLUDED_)
