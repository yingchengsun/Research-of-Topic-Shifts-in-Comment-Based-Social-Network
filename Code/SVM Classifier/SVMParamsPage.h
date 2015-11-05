#if !defined(AFX_SVMPARAMSPAGE_H__183CFA07_C733_4C0C_BAE5_092D3B0F4D7A__INCLUDED_)
#define AFX_SVMPARAMSPAGE_H__183CFA07_C733_4C0C_BAE5_092D3B0F4D7A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SVMParamsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSVMParamsPage dialog

class CSVMParamsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CSVMParamsPage)

// Construction
public:
	CSVMParamsPage();
	~CSVMParamsPage();

// Dialog Data
	//{{AFX_DATA(CSVMParamsPage)
	enum { IDD = IDD_SVMPARAMS };
	CComboBox	m_ComboKernel;
	BOOL	m_bB;
	BOOL	m_bI;
	BOOL	m_bX;
	double	m_fC;
	double	m_dC;
	long	m_iD;
	double	m_fE;
	double	m_fG;
	long	m_IgIterationTime;
	double	m_dJ;
	long	m_fK;
	double	m_fM;
	long	m_lgNewVariable;
	double	m_fO;
	double	m_fP;
	long	m_lgMaxQPSize;
	double	m_fS;
	CString	m_strU;
	BOOL	m_bFinal;
	int		m_classifier_type;
	int		m_iKernelType;
	long	m_lgIterationTime;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CSVMParamsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CSVMParamsPage)
	afx_msg void OnEditchangeComboT();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSelchangeComboT();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SVMPARAMSPAGE_H__183CFA07_C733_4C0C_BAE5_092D3B0F4D7A__INCLUDED_)
