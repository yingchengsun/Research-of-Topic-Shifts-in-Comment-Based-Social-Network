// TestParamsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TextClassify.h"
#include "TestParamsDlg.h"
#include "ClassifierParam.h"
#include "WZDDRDLG.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestParamsDlg dialog


CTestParamsDlg::CTestParamsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestParamsDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestParamsDlg)
	m_nClassifyType = 0;
	m_nKNN = 35;
	m_strResultDir = _T("");
	m_strTestDir = _T("");
	m_nDocFormat = 0;
	m_bCopyFiles = FALSE;
	m_dThreshold = 0.0;
	m_bEvaluation = FALSE;
	//}}AFX_DATA_INIT
	m_nClassifierType=0;
}


void CTestParamsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestParamsDlg)
	DDX_Radio(pDX, IDC_CLASSIFYTYPE, m_nClassifyType);
	DDX_Text(pDX, IDC_EDIT_KNN, m_nKNN);
	DDV_MinMaxUInt(pDX, m_nKNN, 1, 100);
	DDX_Text(pDX, IDC_TXT_RESULT, m_strResultDir);
	DDX_Text(pDX, IDC_TXT_TEST_DIR, m_strTestDir);
	DDX_Text(pDX, IDC_THRESHOLD, m_dThreshold);
	DDV_MinMaxDouble(pDX, m_dThreshold, -100., 100.);
	DDX_Check(pDX, IDC_EVALUATE, m_bEvaluation);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTestParamsDlg, CDialog)
	//{{AFX_MSG_MAP(CTestParamsDlg)
	ON_BN_CLICKED(IDC_BTN_RESULTS, OnBtnResults)
	ON_BN_CLICKED(IDC_BTN_TEST, OnBtnTest)
	ON_BN_CLICKED(IDC_EVALUATE, OnEvaluate)
	ON_BN_CLICKED(IDC_CLASSIFYTYPE, OnClassifytype)
	ON_BN_CLICKED(IDC_MULTI, OnMulti)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestParamsDlg message handlers


void CTestParamsDlg::OnBtnResults() 
{
	UpdateData(TRUE);	
	CWzdDirDlg dlg;
	CString dir=dlg.GetDirectory(this,m_strResultDir);
	if(!dir.IsEmpty())
	{
		m_strResultDir=dir;
		UpdateData(FALSE);	
	}		
}

void CTestParamsDlg::OnBtnTest() 
{
	UpdateData(TRUE);
	if(m_nDocFormat==CClassifierParam::nDF_Smart)
	{
		static char BASED_CODE szFilter[] = "All Files (*.*)|*.*||";
		CFileDialog cfd(TRUE,szFilter,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter,NULL);
		if (cfd.DoModal()==IDOK) 
		{
			m_strTestDir=cfd.GetPathName();
			UpdateData(FALSE);	
		}
	}
	else if(m_nDocFormat==CClassifierParam::nDF_Directory)
	{
		CWzdDirDlg dlg;
		CString dir=dlg.GetDirectory(this,m_strTestDir);
		if(!dir.IsEmpty())
		{
			m_strTestDir=dir;
			UpdateData(FALSE);	
		}	
	}
}

void CTestParamsDlg::SetClassifierType(int nType)
{
	m_nClassifierType=nType;
}

BOOL CTestParamsDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	if(m_nClassifierType==CClassifierParam::nCT_KNN)
		::EnableWindow((GetDlgItem(IDC_EDIT_KNN))->m_hWnd,TRUE);
	else 
		::EnableWindow((GetDlgItem(IDC_EDIT_KNN))->m_hWnd,FALSE);	

	
	SetControlStatus();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTestParamsDlg::OnEvaluate() 
{
	// TODO: Add your control notification handler code here
	SetControlStatus();
}

void CTestParamsDlg::OnClassifytype() 
{
	// TODO: Add your control notification handler code here
	SetControlStatus();
}

void CTestParamsDlg::OnMulti() 
{
	// TODO: Add your control notification handler code here
	SetControlStatus();
}

void CTestParamsDlg::SetControlStatus()
{
	UpdateData(TRUE);
	if(m_nClassifyType==CClassifierParam::nFT_Multi&&m_bEvaluation)
	{
//		::EnableWindow((GetDlgItem(IDC_DOCFORMAT))->m_hWnd,FALSE);
//		::EnableWindow((GetDlgItem(IDC_REUTERS))->m_hWnd,FALSE);
		m_nDocFormat=1;
	}
	else
	{
//		::EnableWindow((GetDlgItem(IDC_DOCFORMAT))->m_hWnd,TRUE);
//		::EnableWindow((GetDlgItem(IDC_REUTERS))->m_hWnd,TRUE);
	}

	if(m_nClassifyType==CClassifierParam::nFT_Single)
		::EnableWindow((GetDlgItem(IDC_THRESHOLD))->m_hWnd,FALSE);
	else
		::EnableWindow((GetDlgItem(IDC_THRESHOLD))->m_hWnd,TRUE);

	UpdateData(FALSE);	
}

void CTestParamsDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CTestParamsDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	CDialog::OnOK();
}
