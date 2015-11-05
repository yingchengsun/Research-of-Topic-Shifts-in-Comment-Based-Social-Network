#if !defined(AFX_TRAINPARAMSPAGE_H__CCAE1B32_4661_4A22_9C98_1FA59F55DFF8__INCLUDED_)
#define AFX_TRAINPARAMSPAGE_H__CCAE1B32_4661_4A22_9C98_1FA59F55DFF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TrainParamsPage.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CTrainParamsPage dialog

class CTrainParamsPage : public CPropertyPage
{
	DECLARE_DYNCREATE(CTrainParamsPage)

// Construction
public:
	CTrainParamsPage();
	~CTrainParamsPage();

// Dialog Data
	//{{AFX_DATA(CTrainParamsPage)
	enum { IDD = IDD_TRAINPARAMS };
	int		m_nFSMode;
	CString	m_txtResultDir;
	CString	m_txtTrainDir;
	BOOL	m_nWordSize;

    double     m_nUKS_k;
	int		m_nOpMode;
	int		m_nSelMode;
	BOOL	m_bStem;
	BOOL    m_nCS;
    BOOL    m_nRS;
	BOOL    m_nUKS;
    BOOL    m_nURL;
	BOOL    m_Length;
	BOOL    m_ClassifierW;
	int		m_nLanguageType;
	int		m_nWeightMode;

	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CTrainParamsPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CTrainParamsPage)
	afx_msg void OnBtnTrain();
	afx_msg void OnBtnResults();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TRAINPARAMSPAGE_H__CCAE1B32_4661_4A22_9C98_1FA59F55DFF8__INCLUDED_)
