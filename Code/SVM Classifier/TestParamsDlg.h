#if !defined(AFX_TESTPARAMSDLG_H__A52699D4_C4CE_4071_925B_4A6B312255F1__INCLUDED_)
#define AFX_TESTPARAMSDLG_H__A52699D4_C4CE_4071_925B_4A6B312255F1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TestParamsDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTestParamsDlg dialog

class CTestParamsDlg : public CDialog
{
// Construction
public:
	void SetControlStatus();
	void SetClassifierType(int nType);
	CTestParamsDlg(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CTestParamsDlg)
	enum { IDD = IDD_TESTPARAMS };
	int		m_nClassifyType;
	UINT	m_nKNN;
	CString	m_strResultDir;
	CString	m_strTestDir;
	int		m_nDocFormat;
	BOOL	m_bCopyFiles;
	double	m_dThreshold;
	BOOL	m_bEvaluation;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestParamsDlg)
	
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CTestParamsDlg)
	afx_msg void OnBtnOk();
	afx_msg void OnBtnCancel();
	afx_msg void OnBtnResults();
	afx_msg void OnBtnTest();
	virtual BOOL OnInitDialog();
	afx_msg void OnEvaluate();
	afx_msg void OnClassifytype();
	afx_msg void OnMulti();
	virtual void OnCancel();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	//0 代表KNN; 1 代表SVM
	int m_nClassifierType;
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTPARAMSDLG_H__A52699D4_C4CE_4071_925B_4A6B312255F1__INCLUDED_)
