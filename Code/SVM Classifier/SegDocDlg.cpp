// SegDocDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TextClassify.h"
#include "SegDocDlg.h"
#include "WZDDRDLG.H"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSegDocDlg dialog

SegDocsParam theSegDocsParam;

CSegDocDlg::CSegDocDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSegDocDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSegDocDlg)
	m_strTarget = _T("");
	m_strSource = _T("");
	m_bContainTag = FALSE;
	m_bMultiDocs = FALSE;
	m_bDelStopwords = FALSE;
	//}}AFX_DATA_INIT
}


void CSegDocDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSegDocDlg)
	DDX_Text(pDX, IDC_EDIT_TARGET, m_strTarget);
	DDX_Text(pDX, IDC_EDIT_SOURCE, m_strSource);
	//DDX_Check(pDX, IDC_CONTAIN_TAG, m_bContainTag);
	DDX_Check(pDX, IDC_MULTI_DOCS, m_bMultiDocs);
	DDX_Check(pDX, IDC_DEL_STOPWORDS, m_bDelStopwords);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSegDocDlg, CDialog)
	//{{AFX_MSG_MAP(CSegDocDlg)
	ON_BN_CLICKED(IDC_BTN_BROWSE1, OnBtnBrowse1)
	ON_BN_CLICKED(IDC_BTN_BROWSE2, OnBtnBrowse2)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSegDocDlg message handlers

void CSegDocDlg::OnOK() 
{
	// TODO: Add extra validation here
	
	UpdateData(TRUE);
	if(m_strSource.IsEmpty()||m_strTarget.IsEmpty())
	{
		AfxMessageBox("目标路径或结果路径不能为空!");
		return;
	}
	if(m_bMultiDocs)
	{
		if(!SetCurrentDirectory(m_strSource))
		{
			AfxMessageBox("源文档目录不存在!");
			return;
		}
		if(m_strTarget.Find(m_strSource)>=0)
		{
			AfxMessageBox("\"结果\"不能是\"目标\"的子目录!");
			return;
		}
		if(!SetCurrentDirectory(m_strTarget))
		{
			if(!CreateDirectory(m_strTarget,NULL))
			{
				AfxMessageBox("无法创建结果文档目录!");
				return;
			}
		}	
	}
	else
	{
		FILE *file;
		if((file=fopen(m_strSource,"r"))==NULL)
		{
			AfxMessageBox("文件"+m_strSource+"不存在!");
			return;
		}
		fclose(file);
	}
	theSegDocsParam.m_strSource=m_strSource;
	theSegDocsParam.m_strTarget=m_strTarget;
	theSegDocsParam.m_bContainTag=m_bContainTag;
	theSegDocsParam.m_bMultiDocs=m_bMultiDocs;
	theSegDocsParam.m_bDelStopwords=m_bDelStopwords;

	CDialog::OnOK();
}

void CSegDocDlg::OnBtnBrowse1() 
{
	// TODO: Add your control notification handler code here
	UpdateData(TRUE);
	if(m_bMultiDocs)
	{
		UpdateData(TRUE);	
		CWzdDirDlg dlg;
		CString dir=dlg.GetDirectory(this,m_strSource);
		if(!dir.IsEmpty())
		{
			m_strSource=dir;
			UpdateData(FALSE);	
		}	
	}
	else
	{
		static char BASED_CODE szFilter[] = "All Files (*.*)|*.*||";
		CFileDialog cfd(TRUE,szFilter,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter,NULL);
		if (cfd.DoModal()==IDOK) 
		{
			m_strSource=cfd.GetPathName();
			m_strTarget=m_strSource+".seg";
			UpdateData(FALSE);	
		}
	}
}

void CSegDocDlg::OnBtnBrowse2() 
{
	// TODO: Add your control notification handler code here
	if(m_bMultiDocs)
	{
		UpdateData(TRUE);	
		CWzdDirDlg dlg;
		CString dir=dlg.GetDirectory(this,m_strTarget);
		if(!dir.IsEmpty())
		{
			m_strTarget=dir;
			UpdateData(FALSE);	
		}	
	}
	else
	{
		static char BASED_CODE szFilter[] = "All Files (*.*)|*.*||";
		CFileDialog cfd(FALSE,szFilter,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szFilter,NULL);
		if (cfd.DoModal()==IDOK) 
		{
			m_strTarget=cfd.GetPathName();
			UpdateData(FALSE);	
		}
	}
}
