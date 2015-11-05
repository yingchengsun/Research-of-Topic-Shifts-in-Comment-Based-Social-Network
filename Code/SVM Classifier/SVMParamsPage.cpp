// SVMParamsPage.cpp : implementation file
//

#include "stdafx.h"
#include "TextClassify.h"
#include "SVMParamsPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSVMParamsPage property page

IMPLEMENT_DYNCREATE(CSVMParamsPage, CPropertyPage)

CSVMParamsPage::CSVMParamsPage() : CPropertyPage(CSVMParamsPage::IDD)
{
	//{{AFX_DATA_INIT(CSVMParamsPage)
	m_bB = FALSE;
	m_bI = FALSE;
	m_bX = FALSE;
	m_fC = 0.0;
	m_dC = 0.0;
	m_iD = 0;
	m_fE = 0.0;
	m_fG = 0.0;
	m_IgIterationTime = 0;
	m_dJ = 0.0;
	m_fK = 0;
	m_fM = 0.0;
	m_lgNewVariable = 0;
	m_fO = 0.0;
	m_fP = 0.0;
	m_lgMaxQPSize = 0;
	m_fS = 0.0;
	m_strU = _T("");
	m_bFinal = FALSE;
	m_classifier_type = -1;
	m_iKernelType = 0;
	//}}AFX_DATA_INIT
}

CSVMParamsPage::~CSVMParamsPage()
{
}

void CSVMParamsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSVMParamsPage)
	DDX_Control(pDX, IDC_COMBO_T, m_ComboKernel);
	DDX_Check(pDX, IDC_CHECK_B, m_bB);
	DDX_Check(pDX, IDC_CHECK_I, m_bI);
	DDX_Check(pDX, IDC_CHECK_X, m_bX);
	DDX_Text(pDX, IDC_EDIT_C, m_fC);
	DDX_Text(pDX, IDC_EDIT_C1, m_dC);
	DDX_Text(pDX, IDC_EDIT_D, m_iD);
	DDX_Text(pDX, IDC_EDIT_E, m_fE);
	DDX_Text(pDX, IDC_EDIT_G, m_fG);
	DDX_Text(pDX, IDC_EDIT_H, m_IgIterationTime);
	DDX_Text(pDX, IDC_EDIT_J, m_dJ);
	DDX_Text(pDX, IDC_EDIT_K, m_fK);
	DDX_Text(pDX, IDC_EDIT_M, m_fM);
	DDX_Text(pDX, IDC_EDIT_N, m_lgNewVariable);
	DDX_Text(pDX, IDC_EDIT_O, m_fO);
	DDX_Text(pDX, IDC_EDIT_P, m_fP);
	DDX_Text(pDX, IDC_EDIT_Q, m_lgMaxQPSize);
	DDX_Text(pDX, IDC_EDIT_S, m_fS);
	DDX_Text(pDX, IDC_EDIT_U, m_strU);
	DDX_Check(pDX, IDC_FINAL_CHECK, m_bFinal);
	DDX_Radio(pDX, IDC_RADIO_CLASSIFIER_TYPE, m_classifier_type);
	DDX_CBIndex(pDX, IDC_COMBO_T, m_iKernelType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSVMParamsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CSVMParamsPage)
	ON_CBN_EDITCHANGE(IDC_COMBO_T, OnEditchangeComboT)
	ON_WM_SHOWWINDOW()
	ON_CBN_SELCHANGE(IDC_COMBO_T, OnSelchangeComboT)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSVMParamsPage message handlers

void CSVMParamsPage::OnEditchangeComboT() 
{
	// TODO: Add your control notification handler code here
	
}

void CSVMParamsPage::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	switch (m_ComboKernel.GetCurSel()) 
	{
	case 0://linear kernel 
		
		GetDlgItem(IDC_EDIT_G)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_S)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_C)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_U)->EnableWindow(FALSE);
		break;
	case 1://polynomial
		GetDlgItem(IDC_EDIT_D)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_G)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_S)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_C)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_U)->EnableWindow(FALSE);
		break;

	case 2://rbf kernel
		GetDlgItem(IDC_EDIT_D)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_G)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_S)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_C)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_U)->EnableWindow(FALSE);
		break;
	case 3:
		GetDlgItem(IDC_EDIT_D)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_G)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_S)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_C)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_U)->EnableWindow(FALSE);
		break;
	case 4:
		GetDlgItem(IDC_EDIT_D)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_G)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_S)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_C)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_U)->EnableWindow(TRUE);
		break;
	}	
	// TODO: Add your message handler code here
	
}

void CSVMParamsPage::OnSelchangeComboT() 
{
	// TODO: Add your control notification handler code here
	switch (m_ComboKernel.GetCurSel()) 
	{
	case 0://linear kernel 
		
		GetDlgItem(IDC_EDIT_G)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_S)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_C)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_U)->EnableWindow(FALSE);
		break;
	case 1://polynomial
		GetDlgItem(IDC_EDIT_D)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_G)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_S)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_C)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_U)->EnableWindow(FALSE);
		break;

	case 2://rbf kernel
		GetDlgItem(IDC_EDIT_D)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_G)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_S)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_C)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_U)->EnableWindow(FALSE);
		break;
	case 3:
		GetDlgItem(IDC_EDIT_D)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_G)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_S)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_C)->EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_U)->EnableWindow(FALSE);
		break;
	case 4:
		GetDlgItem(IDC_EDIT_D)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_G)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_S)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_C)->EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_U)->EnableWindow(TRUE);
		break;
	}
}
