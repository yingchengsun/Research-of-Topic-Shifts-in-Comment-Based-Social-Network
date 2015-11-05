// TrainParamsPage.cpp : implementation file
//

#include "stdafx.h"
#include "TextClassify.h"
#include "TrainParamsPage.h"
#include "WzdDrDlg.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTrainParamsPage property page

IMPLEMENT_DYNCREATE(CTrainParamsPage, CPropertyPage)

CTrainParamsPage::CTrainParamsPage() : CPropertyPage(CTrainParamsPage::IDD)
{
	//{{AFX_DATA_INIT(CTrainParamsPage)
	m_txtResultDir = _T("");
	m_txtTrainDir = _T("");
	m_nFSMode = 0;
	m_nWordSize = 2000;
	m_nUKS_k=0.6;
	m_nOpMode = -1;
	m_nSelMode = -1;
	m_bStem = FALSE;
	m_nCS=FALSE;
	m_nRS=FALSE;
	m_nUKS=FALSE;
	m_nURL=FALSE;
	m_Length=FALSE;
	m_ClassifierW=FALSE;
	m_nLanguageType = 0;
	m_nWeightMode = -1;

	//}}AFX_DATA_INIT
}

CTrainParamsPage::~CTrainParamsPage()
{
}

void CTrainParamsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTrainParamsPage)
	DDX_Radio(pDX, IDC_RADIO_FS_IG, m_nFSMode);
	//DDX_Radio(pDX,IDC_RADIO_WEIGHT,m_nWeightMode);
	//DDX_Radio(pDX,IDC_RADIO_WEIGHT2,m_nWeightMode);//xby
	DDX_Text(pDX, IDC_TXT_RESULT, m_txtResultDir);
	DDX_Text(pDX, IDC_TXT_TRAIN_DIR, m_txtTrainDir);
	DDX_Text(pDX, IDC_TXT_WORD_SIZE, m_nWordSize);
	DDX_Text(pDX, IDC_UKS_K,m_nUKS_k);
//	DDX_Check(pDX,IDC_URL,m_nURL);
	DDX_Check(pDX,IDC_WEIGHT1,m_nCS);
	DDX_Check(pDX,IDC_WEIGHT2,m_nRS);
	DDX_Check(pDX,IDC_WEIGHT3,m_nUKS);
	DDX_Check(pDX,IDC_WEIGHT4,m_Length);
	DDX_Check(pDX,IDC_WEIGHT5,m_ClassifierW);

	//DDX_Check(pDX,IDC_WEIGHT3,);
	//}}AFX_DATA_MAP
}
/*	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSegDocDlg)
	DDX_Text(pDX, IDC_EDIT_TARGET, m_strTarget);
	DDX_Text(pDX, IDC_EDIT_SOURCE, m_strSource);
	//DDX_Check(pDX, IDC_CONTAIN_TAG, m_bContainTag);
	DDX_Check(pDX, IDC_MULTI_DOCS, m_bMultiDocs);
	DDX_Check(pDX, IDC_DEL_STOPWORDS, m_bDelStopwords);
	//}}AFX_DATA_MAP
*/
BEGIN_MESSAGE_MAP(CTrainParamsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CTrainParamsPage)
	ON_BN_CLICKED(IDC_BTN_TRAIN, OnBtnTrain)
	ON_BN_CLICKED(IDC_BTN_RESULTS, OnBtnResults)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTrainParamsPage message handlers

void CTrainParamsPage::OnBtnTrain() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);	
	CWzdDirDlg dlg;
	CString dir=dlg.GetDirectory(this);
	if(!dir.IsEmpty())
	{
		m_txtTrainDir=dir;
		UpdateData(FALSE);	
	}	
}

void CTrainParamsPage::OnBtnResults() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	CWzdDirDlg dlg;
	CString dir=dlg.GetDirectory(this);
	if(!dir.IsEmpty())
	{
		m_txtResultDir=dir;
		UpdateData(FALSE);	
	}
}

BOOL CTrainParamsPage::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
//	::EnableWindow((GetDlgItem(IDC_CHK_STEM))->m_hWnd,m_nLanguageType);	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}
