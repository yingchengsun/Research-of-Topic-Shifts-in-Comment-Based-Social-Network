// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "TextClassify.h"
#include "Message.h"

#include "MainFrm.h"
#include "LeftView.h"
#include "TextClassifyView.h"


#include "SegDocDlg.h"
#include "WordSegment.h"
#include "TrainParamsPage.h"
#include "TestParamsDlg.h"
#include "classifier.h"
#include "svmparamspage.h"
#include <direct.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame
CString CMainFrame::m_strResultDir;

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_MESSAGE(WM_COMPUTATION_FINISH,OnComputationFinish)
	ON_COMMAND(ID_TOOLS_WORDSEG, OnToolsWordseg)
	ON_WM_CLOSE()
	ON_WM_TIMER()
	ON_COMMAND(ID_MAIN_TRAINKNN, OnMainTrainknn)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TRAINKNN, OnUpdateMainTrainknn)
	ON_COMMAND(ID_MAIN_TRAINSVM, OnMainTrainsvm)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TRAINSVM, OnUpdateMainTrainsvm)
	ON_COMMAND(ID_MAIN_TRAINBAYES,OnMainTrainbayes)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TRAINBAYES,OnUpdateMainTrainbayes)
	ON_COMMAND(ID_MAIN_OPENMODEL, OnMainOpenmodel)
	ON_UPDATE_COMMAND_UI(ID_MAIN_OPENMODEL, OnUpdateMainOpenmodel)
	ON_COMMAND(ID_MAIN_RESULTS, OnMainResults)
	ON_UPDATE_COMMAND_UI(ID_MAIN_RESULTS, OnUpdateMainResults)
	ON_COMMAND(ID_MAIN_TEST, OnMainTest)
	ON_UPDATE_COMMAND_UI(ID_MAIN_TEST, OnUpdateMainTest)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator	
//	ID_INDICATOR_CAPS,
//	ID_INDICATOR_NUM,
//	ID_INDICATOR_SCRL,
//	ID_INDICATOR_TIME,
};

//unsigned int LPVOID是任意类型指针
UINT CMainFrame::TrainThreadProc(LPVOID pParam)
{	
	theClassifier.m_theSVM.com_param.running=1;
	theClassifier.Train(theClassifier.m_paramClassifier.m_nClassifierType);
	theClassifier.m_theSVM.com_param.running=0;
	::PostMessage((HWND)pParam,WM_COMPUTATION_FINISH,0,0);
	return 0;
}

UINT CMainFrame::TestThreadProc(LPVOID pParam)
{
	if(theClassifier.m_paramClassifier.m_nDocFormat==CClassifierParam::nDF_Directory)
	{
		//if(theClassifier.Classify())
		if(theClassifier.Classify_Query())
			CMainFrame::m_strResultDir = theClassifier.m_paramClassifier.m_strResultDir;
	}
	else if(theClassifier.m_paramClassifier.m_nDocFormat==CClassifierParam::nDF_Smart)
	{
		if(theClassifier.ClassifySmart())
			CMainFrame::m_strResultDir = theClassifier.m_paramClassifier.m_strResultDir;
	}
	::PostMessage((HWND)pParam,WM_COMPUTATION_FINISH,0,0);
	return 0;
}
/*
UINT CMainFrame::ConvertThreadProc(LPVOID pParam)
{
	CTime startTime;
	CTimeSpan totalTime;

	CMessage::PrintInfo(_T("开始进行文档格式转换，请稍候..."));
	startTime=CTime::GetCurrentTime();
	theSaxFileHandler.Convert();
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("文档格式转换结束，耗时")+totalTime.Format("%H:%M:%S"));
	CMessage::PrintStatusInfo("");

	::PostMessage((HWND)pParam,WM_COMPUTATION_FINISH,0,0);
	ExitThread(0);
	return 0;
}

UINT CMainFrame::SeldocsThreadProc(LPVOID pParam)
{
	CTime startTime;
	CTimeSpan totalTime;
	long lDocNum;
	CString str;

	CMessage::PrintInfo(_T("开始选择训练文档和测试文档，请稍候..."));
	startTime=CTime::GetCurrentTime();
	lDocNum=theSelectDocs.SelectDocs();
	totalTime=CTime::GetCurrentTime()-startTime;
	str.Format("共处理了%d篇文档!",lDocNum);
	CMessage::PrintInfo(str);
	CMessage::PrintInfo(_T("选择文档结束，耗时")+totalTime.Format("%H:%M:%S"));
	CMessage::PrintStatusInfo("");

	::PostMessage((HWND)pParam,WM_COMPUTATION_FINISH,0,0);
	ExitThread(0);
	return 0;
}
*/
UINT CMainFrame::SegDocsThreadProc(LPVOID pParam)
{
	CTime startTime;
	CTimeSpan totalTime;
	startTime=CTime::GetCurrentTime();
	CMessage::PrintInfo(_T("分词程序初始化，请稍候..."));	
	if(!g_wordSeg.InitWorgSegment(theApp.m_strPath.GetBuffer(0)))
	{
		CMessage::PrintError(_T("分词程序初始化失败!"));
		return 0;
	}
	g_wordSeg.SetSegSetting(CWordSegment::uPlace);
	CMessage::PrintInfo(_T("正在进行分词，请稍候..."));	

	if(theSegDocsParam.m_bMultiDocs)
	{
		CString strTarget,strSource;
		HANDLE hFinder;
		LPWIN32_FIND_DATA lpFindFileData;	
		lpFindFileData  = new WIN32_FIND_DATA;
		hFinder = ::FindFirstFile(theSegDocsParam.m_strSource+"\\*.*",lpFindFileData );
		while(::FindNextFile(hFinder,lpFindFileData))
		{
			if( !strcmp(lpFindFileData->cFileName,".") || !strcmp(lpFindFileData->cFileName,"..") )
				continue;

			if(!(lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				strTarget=theSegDocsParam.m_strTarget+"\\";
				strTarget+=lpFindFileData->cFileName;
				strSource=theSegDocsParam.m_strSource+"\\";
				strSource+=lpFindFileData->cFileName;
				g_wordSeg.SegmentFile(strSource.GetBuffer(0),
									strTarget.GetBuffer(0),
									theSegDocsParam.m_bDelStopwords);
				CMessage::PrintStatusInfo(lpFindFileData->cFileName);
			}
		}
		delete	lpFindFileData;
	}
	else
		g_wordSeg.SegmentFile(theSegDocsParam.m_strSource.GetBuffer(0),
							theSegDocsParam.m_strTarget.GetBuffer(0),
							theSegDocsParam.m_bDelStopwords);

	g_wordSeg.FreeWordSegment();
	totalTime=CTime::GetCurrentTime()-startTime;
	CMessage::PrintInfo(_T("分词结束，耗时")+totalTime.Format("%H:%M:%S"));
	CMessage::PrintStatusInfo("");

	::PostMessage((HWND)pParam,WM_COMPUTATION_FINISH,0,0);
	ExitThread(0);
	return 0;
}




/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	m_pThread=NULL;
	m_strModel.Empty();
	m_strResultDir.Empty();
	m_nRunning=0;
	m_bPaused=false;
	m_nClassifierType=0;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;


	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}
	
	CMessage::SetStatusWnd(&m_wndStatusBar);

	return 0;
}

BOOL CMainFrame::OnCreateClient(LPCREATESTRUCT /*lpcs*/,
	CCreateContext* pContext)
{
	// create splitter window
	if (!m_wndSplitter.CreateStatic(this, 1, 2))
		return FALSE;

	if (!m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CLeftView), CSize(100, 100), pContext) ||
		!m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(CTextClassifyView), CSize(100, 100), pContext))
	{
		m_wndSplitter.DestroyWindow();
		return FALSE;
	}
	CMessage::SetOutputWnd((CWnd *)m_wndSplitter.GetPane(0, 1));
	return TRUE;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

/*
CSvmclsView* CMainFrame::GetRightPane()
{
	CWnd* pWnd = m_wndSplitter.GetPane(0, 1);
	CSvmclsView* pView = DYNAMIC_DOWNCAST(CSvmclsView, pWnd);
	return pView;
}

CLeftView* CMainFrame::GetLeftView()
{
	CWnd* pWnd = m_wndSplitter.GetPane(0, 0);
	CLeftView* pView = DYNAMIC_DOWNCAST(CLeftView, pWnd);
	return pView;
}

void CMainFrame::OnUpdateViewStyles(CCmdUI* pCmdUI)
{
	// TODO: customize or extend this code to handle choices on the
	// View menu.

	CSvmclsView* pView = GetRightPane(); 

	// if the right-hand pane hasn't been created or isn't a view,
	// disable commands in our range

	if (pView == NULL)
		pCmdUI->Enable(FALSE);
	else
	{
		DWORD dwStyle = pView->GetStyle() & LVS_TYPEMASK;

		// if the command is ID_VIEW_LINEUP, only enable command
		// when we're in LVS_ICON or LVS_SMALLICON mode

		if (pCmdUI->m_nID == ID_VIEW_LINEUP)
		{
			if (dwStyle == LVS_ICON || dwStyle == LVS_SMALLICON)
				pCmdUI->Enable();
			else
				pCmdUI->Enable(FALSE);
		}
		else
		{
			// otherwise, use dots to reflect the style of the view
			pCmdUI->Enable();
			BOOL bChecked = FALSE;

			switch (pCmdUI->m_nID)
			{
			case ID_VIEW_DETAILS:
				bChecked = (dwStyle == LVS_REPORT);
				break;

			case ID_VIEW_SMALLICON:
				bChecked = (dwStyle == LVS_SMALLICON);
				break;

			case ID_VIEW_LARGEICON:
				bChecked = (dwStyle == LVS_ICON);
				break;

			case ID_VIEW_LIST:
				bChecked = (dwStyle == LVS_LIST);
				break;

			default:
				bChecked = FALSE;
				break;
			}

			pCmdUI->SetRadio(bChecked ? 1 : 0);
		}
	}
}

*/
void CMainFrame::OnComputationFinish(WPARAM wParam,LPARAM lParam)
{
	CeaseComputation();
}


void CMainFrame::CeaseComputation()
{	
	m_wndStatusBar.SetPaneText(0,"运算全部结束!");
	m_nRunning=0;
	m_pThread=NULL;
	m_bPaused=false;
	KillTimer(m_nTimer);
}

void CMainFrame::BeginComputation()
{
	m_nRunning=1;
	m_nTimer=SetTimer(1001,500,NULL);
	m_tmBegin=CTime::GetCurrentTime();
}
/*
void CMainFrame::OnTimer(UINT nIDEvent) 
{
	CTimeSpan t=CTime::GetCurrentTime()-m_tmBegin;
	m_wndStatusBar.SetPaneText(1,t.Format("耗时%H:%M:%S"),TRUE);
	CFrameWnd::OnTimer(nIDEvent);
}

void CMainFrame::OnClose() 
{
	GetActiveDocument()->SetModifiedFlag(false);
	if(m_nRunning>0)
	{
		if(AfxMessageBox("正在进行计算，真的要停止吗?",MB_YESNO)==IDYES)
		{
			StopComputation();
			CFrameWnd::OnClose();
		}
	}
	else
		CFrameWnd::OnClose();
}
*/
void CMainFrame::Train()
{
	CMessage::ClearInfo();
	BeginComputation();
	m_pThread=AfxBeginThread(TrainThreadProc,GetSafeHwnd(),THREAD_PRIORITY_NORMAL);
}

void CMainFrame::Test() 
{
	CMessage::ClearInfo();
	BeginComputation();
	m_pThread=AfxBeginThread(TestThreadProc,GetSafeHwnd(),THREAD_PRIORITY_NORMAL);
}

/*
void CMainFrame::OnMainClassifySinglefile() 
{
	static char BASED_CODE szTextFilter[] = "Text Files (*.txt)|*.txt|All Files (*.*)|*.*||";
	CFileDialog cfd(TRUE,szTextFilter,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szTextFilter,NULL);
	if (cfd.DoModal()==IDOK) 
	{
		CString strFile=cfd.GetPathName();
		short id;
		theClassifier.m_paramClassifier.m_dThreshold     =   60;
		theClassifier.m_paramClassifier.m_nKNN           =   35;
		theClassifier.m_paramClassifier.m_nClassifyType  =   CClassifierParam::nFT_Single;
		theClassifier.m_paramClassifier.m_bEvaluation    =   FALSE;
		theClassifier.m_paramClassifier.m_bCopyFiles     =   FALSE;
		if(theClassifier.m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_KNN)
			id=theClassifier.KNNCategory(strFile.GetBuffer(0));
		else if(theClassifier.m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_SVM)
			id=theClassifier.SVMCategory(strFile.GetBuffer(0));
		else
		{
			CMessage::PrintError("分类器类型不正确!");
			return;
		}
		CMessage::ClearInfo();
		if(id>=0) theClassifier.m_lstTrainCatalogList.GetCataName(id,strFile);
		else strFile="无法识别";
		CMessage::PrintInfo("文档的类别为: "+strFile);
	}	
}
*/

void CMainFrame::OnMainTest() 
{
	CTestParamsDlg testParamsDlg;
	
	if(theClassifier.m_paramClassifier.m_strTestDir.IsEmpty())
		theClassifier.m_paramClassifier.m_strTestDir=theApp.m_strPath+"\\sample\\test";
	if(theClassifier.m_paramClassifier.m_strResultDir.IsEmpty())
		theClassifier.m_paramClassifier.m_strResultDir=theClassifier.m_paramClassifier.m_txtResultDir;
	if((theClassifier.m_paramClassifier.m_nClassifierType==CClassifierParam::nCT_SVM)||
		(theClassifier.m_paramClassifier.m_dThreshold>0))
	{
		theClassifier.m_paramClassifier.m_nKNN=35;
		theClassifier.m_paramClassifier.m_dThreshold=0;
	}

	testParamsDlg.m_strTestDir    = theClassifier.m_paramClassifier.m_strTestDir;
	testParamsDlg.m_strResultDir  = theClassifier.m_paramClassifier.m_strResultDir;
	testParamsDlg.m_nDocFormat    = theClassifier.m_paramClassifier.m_nDocFormat;
	testParamsDlg.m_dThreshold    = theClassifier.m_paramClassifier.m_dThreshold;
	testParamsDlg.m_nKNN          = theClassifier.m_paramClassifier.m_nKNN;
	testParamsDlg.m_nClassifyType = theClassifier.m_paramClassifier.m_nClassifyType;
	testParamsDlg.m_bEvaluation   = theClassifier.m_paramClassifier.m_bEvaluation;
	testParamsDlg.m_bCopyFiles    = theClassifier.m_paramClassifier.m_bCopyFiles;

	testParamsDlg.SetClassifierType(theClassifier.m_paramClassifier.m_nClassifierType);

	if(testParamsDlg.DoModal()==IDOK)
	{
		if((testParamsDlg.m_nDocFormat==CClassifierParam::nDF_Directory)&&
			!CheckPath(testParamsDlg.m_strTestDir)) return;
		if(!CheckPath(testParamsDlg.m_strResultDir,true)) return;

		theClassifier.m_paramClassifier.m_strTestDir     =   testParamsDlg.m_strTestDir;
		theClassifier.m_paramClassifier.m_strResultDir   =   testParamsDlg.m_strResultDir;
		theClassifier.m_paramClassifier.m_nDocFormat     =   testParamsDlg.m_nDocFormat; 
		theClassifier.m_paramClassifier.m_dThreshold     =   testParamsDlg.m_dThreshold;
		theClassifier.m_paramClassifier.m_nKNN           =   testParamsDlg.m_nKNN ;
		theClassifier.m_paramClassifier.m_nClassifyType  =   testParamsDlg.m_nClassifyType;
		theClassifier.m_paramClassifier.m_bEvaluation    =   testParamsDlg.m_bEvaluation;
		theClassifier.m_paramClassifier.m_bCopyFiles     =   testParamsDlg.m_bCopyFiles;
		Test();
	}
}

/*
void CMainFrame::StopComputation()
{
	if((m_nRunning>=1)&&(m_pThread!=NULL))
	{
		if(TerminateThread(m_pThread->m_hThread,1))
		{
			CeaseComputation();
		}
		else
			AfxMessageBox("无法停止计算过程!");
	}
}

void CMainFrame::OnUpdateMainReadFeatures(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning<=0);
}
*/

void CMainFrame::OnUpdateMainTrainknn(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning<=0);	
}

void CMainFrame::OnUpdateMainTrainsvm(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning<=0);
}

void CMainFrame::OnUpdateMainTrainbayes(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_nRunning<=0);
}

void CMainFrame::OnUpdateMainTest(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning<=0&&!m_strModel.IsEmpty());	
}
/*
void CMainFrame::OnUpdateMainClassify(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning<=0&&!m_strModel.IsEmpty());	
}

void CMainFrame::OnUpdateMainClassifySinglefile(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning<=0&&!m_strModel.IsEmpty());	
}
*/

void CMainFrame::OnUpdateMainResults(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning<=0&&!m_strResultDir.IsEmpty());	
}

  
void CMainFrame::OnUpdateMainOpenmodel(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning<=0);	
}


/*
void CMainFrame::OnUpdateMainStoprunning(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning);	
}

void CMainFrame::OnUpdateMainPause(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning&&!m_bPaused);		
}

void CMainFrame::OnUpdateMainResume(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning&&m_bPaused);
}

void CMainFrame::OnUpdateToolsConvert(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning<=0);	
}

void CMainFrame::OnUpdateToolsResults(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(m_nRunning<=0);	
}

void CMainFrame::OnMainStoprunning() 
{
	if(AfxMessageBox("正在进行计算，真的要停止吗?",MB_YESNO)==IDYES)
		StopComputation();	
}

*/
void CMainFrame::OnMainOpenmodel() 
{
	static char BASED_CODE szModelFilter[] = "Model Files (*.prj)|*.prj|All Files (*.*)|*.*||";
	CFileDialog cfd(TRUE,szModelFilter,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szModelFilter,NULL);
	if (cfd.DoModal()==IDOK) 
	{
		m_strModel=cfd.GetPathName();
		CMessage::ClearInfo();
		if(!theClassifier.OpenModel(m_strModel)) m_strModel.Empty();
	}		
}

/*
void CMainFrame::OnMainPause() 
{
	if(!m_bPaused)
	{
		m_pThread->SuspendThread();
		m_wndStatusBar.SetPaneText(0,"运算被用户暂停了!");
		m_bPaused=true;
	}
}


void CMainFrame::OnMainResume() 
{
	if(m_bPaused)
	{
		m_pThread->ResumeThread();
		m_wndStatusBar.SetPaneText(0,"恢复正常运算了!");
		m_bPaused=false;
	}
}
*/

void CMainFrame::OnMainResults() 
{
	if(theClassifier.m_paramClassifier.m_bEvaluation)
		theClassifier.Evaluate(theApp.m_strPath);
	else
	{
		CMessage::ClearInfo();
		CFile fin;
		CString fname=theClassifier.m_paramClassifier.m_strResultDir+"\\results.txt";
		if(!fin.Open(fname,CFile::modeRead))
		{
			CMessage::PrintError(_T("分类结果文件")+fname+"不存在!");
			return;
		}
		DWORD flen=fin.GetLength();
		char *buffer=new char[flen+1];
		flen=fin.ReadHuge(buffer,flen);
		buffer[flen]='\0';
		fin.Close();
		CMessage::PrintInfo("----------------------分类结果----------------------\r\n");
		CMessage::PrintInfo("文档ID\t名称\t\t所属类别\r\n");
		CMessage::PrintInfo(buffer);
		delete[] buffer;			
	}
}

/*
void CMainFrame::OnToolsResults() 
{
	CString strFileName=theApp.m_strPath;
	strFileName+="\\multieval.exe";
	if(WinExec(strFileName,SW_SHOWNORMAL)<32)
		AfxMessageBox("分类结果评测程序不存在!");
}

void CMainFrame::OnMainReadFeatures() 
{
	static char BASED_CODE szModelFilter[] = "Features Files (*.txt)|*.txt|All Files (*.*)|*.*||";
	CFileDialog cfd(TRUE,szModelFilter,NULL,OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,szModelFilter,NULL);
	if (cfd.DoModal()==IDOK) 
	{
		CString strFileName;
		strFileName=cfd.GetPathName();
		CMessage::ClearInfo();
		theClassifier.m_lstTrainWordList.GetListFromFile(strFileName);
	}
}
*/
void CMainFrame::OnMainTrainknn() 
{
	CPropertySheet paramsSheet;
	CTrainParamsPage trainParamsPage;
	
	if(theClassifier.m_paramClassifier.m_txtTrainDir.IsEmpty())
		theClassifier.m_paramClassifier.m_txtTrainDir=theApp.m_strPath+"\\sample\\train";
	if(theClassifier.m_paramClassifier.m_txtResultDir.IsEmpty())
		theClassifier.m_paramClassifier.m_txtResultDir=theApp.m_strPath+"\\sample\\knn";
	trainParamsPage.m_txtTrainDir  = theClassifier.m_paramClassifier.m_txtTrainDir;
	trainParamsPage.m_txtResultDir = theClassifier.m_paramClassifier.m_txtResultDir;
	trainParamsPage.m_nFSMode      = theClassifier.m_paramClassifier.m_nFSMode;
	trainParamsPage.m_nWordSize    = theClassifier.m_paramClassifier.m_nWordSize;
	trainParamsPage.m_nUKS_k       = theClassifier.m_paramClassifier.m_nUKS_k;
	trainParamsPage.m_nSelMode     = theClassifier.m_paramClassifier.m_nSelMode;
	trainParamsPage.m_nOpMode      = theClassifier.m_paramClassifier.m_nOpMode;
	trainParamsPage.m_nLanguageType= theClassifier.m_paramClassifier.m_nLanguageType;
	trainParamsPage.m_bStem        = theClassifier.m_paramClassifier.m_bStem;
    trainParamsPage.m_nCS    	   = theClassifier.m_paramClassifier.m_nCS ;
	trainParamsPage.m_nRS          = theClassifier.m_paramClassifier.m_nRS;
	trainParamsPage.m_Length       = theClassifier.m_paramClassifier.m_Length;
	trainParamsPage.m_nUKS         = theClassifier.m_paramClassifier.m_nUKS; 
	trainParamsPage.m_ClassifierW  = theClassifier.m_paramClassifier.m_ClassifierW;
    trainParamsPage.m_nURL         = theClassifier.m_paramClassifier.m_nURL;
	trainParamsPage.m_nWeightMode  = theClassifier.m_paramClassifier.m_nWeightMode;

	//paramsSheet.SetWizardMode();
	paramsSheet.SetTitle("Parameter settings");
	paramsSheet.AddPage(&trainParamsPage);
	if(paramsSheet.DoModal()==IDOK)
	{
		if(!CheckPath(trainParamsPage.m_txtTrainDir)||
			!CheckPath(trainParamsPage.m_txtResultDir,true))
			return;
		
		theClassifier.m_paramClassifier.m_txtTrainDir    =   trainParamsPage.m_txtTrainDir;
		theClassifier.m_paramClassifier.m_txtResultDir   =   trainParamsPage.m_txtResultDir;
		theClassifier.m_paramClassifier.m_nFSMode        =   trainParamsPage.m_nFSMode;
		theClassifier.m_paramClassifier.m_nWordSize      =   trainParamsPage.m_nWordSize;
		theClassifier.m_paramClassifier.m_nUKS_k         =   trainParamsPage.m_nUKS_k;
		theClassifier.m_paramClassifier.m_nSelMode       =   trainParamsPage.m_nSelMode;
		theClassifier.m_paramClassifier.m_nOpMode        =   trainParamsPage.m_nOpMode;
		theClassifier.m_paramClassifier.m_nLanguageType  =   trainParamsPage.m_nLanguageType;
		theClassifier.m_paramClassifier.m_bStem          =   trainParamsPage.m_bStem;
    	theClassifier.m_paramClassifier.m_nCS            =   trainParamsPage.m_nCS;
        theClassifier.m_paramClassifier.m_nRS            =   trainParamsPage.m_nRS;
		theClassifier.m_paramClassifier.m_Length         =   trainParamsPage.m_Length;
	    theClassifier.m_paramClassifier.m_nUKS           =   trainParamsPage.m_nUKS;
	    theClassifier.m_paramClassifier.m_ClassifierW    =   trainParamsPage.m_ClassifierW;
        theClassifier.m_paramClassifier.m_nURL           =   trainParamsPage.m_nURL;
		theClassifier.m_paramClassifier.m_nWeightMode    =   trainParamsPage.m_nWeightMode;
		theClassifier.m_paramClassifier.m_nClassifierType=   CClassifierParam::nCT_KNN;

		Train();
		m_strModel=theClassifier.m_paramClassifier.m_txtResultDir+"\\model.prj";
		theClassifier.m_paramClassifier.m_strModelFile="model";
	}
}

/*
void CMainFrame::OnMainTrainsvm() 
{
	// TODO: Add your command handler code here
	CPropertySheet paramsSheet;
	CSVMParamsPage   svmParamsPage;
	CTrainParamsPage trainParamsPage;
	
	if(theClassifier.m_paramClassifier.m_txtTrainDir.IsEmpty())
		theClassifier.m_paramClassifier.m_txtTrainDir=theApp.m_strPath+"\\sample\\train";
	if(theClassifier.m_paramClassifier.m_txtResultDir.IsEmpty())
		theClassifier.m_paramClassifier.m_txtResultDir=theApp.m_strPath+"\\sample\\svm";
	trainParamsPage.m_txtTrainDir  = theClassifier.m_paramClassifier.m_txtTrainDir;
	trainParamsPage.m_txtResultDir = theClassifier.m_paramClassifier.m_txtResultDir;
	trainParamsPage.m_nFSMode      = theClassifier.m_paramClassifier.m_nFSMode;
	trainParamsPage.m_nWordSize    = theClassifier.m_paramClassifier.m_nWordSize;
	trainParamsPage.m_nSelMode     = theClassifier.m_paramClassifier.m_nSelMode;
	trainParamsPage.m_nOpMode      = theClassifier.m_paramClassifier.m_nOpMode;
	trainParamsPage.m_nLanguageType= theClassifier.m_paramClassifier.m_nLanguageType;
	trainParamsPage.m_bStem        = theClassifier.m_paramClassifier.m_bStem;
	trainParamsPage.m_nWeightMode  = theClassifier.m_paramClassifier.m_nWeightMode;

	svmParamsPage.m_bB=theClassifier.m_theSVM.com_param.biased_Hyperplane;
	svmParamsPage.m_bI=theClassifier.m_theSVM.com_param.remove_inconsitant;
	svmParamsPage.m_dC=theClassifier.m_theSVM.com_param.C;
	svmParamsPage.m_dJ=theClassifier.m_theSVM.com_param.cost_factor;
	svmParamsPage.m_bX=theClassifier.m_theSVM.com_param.loo;
	svmParamsPage.m_fK=theClassifier.m_theSVM.com_param.search_depth;
	svmParamsPage.m_fO=theClassifier.m_theSVM.com_param.rho;
	svmParamsPage.m_fP=theClassifier.m_theSVM.com_param.fraction;
	svmParamsPage.m_fG=theClassifier.m_theSVM.com_param.rbf_gamma;
	svmParamsPage.m_fC=theClassifier.m_theSVM.com_param.poly_c;
	svmParamsPage.m_fS=theClassifier.m_theSVM.com_param.poly_s;
	svmParamsPage.m_iD=theClassifier.m_theSVM.com_param.poly_degree;
	svmParamsPage.m_iKernelType=theClassifier.m_theSVM.com_param.kernel_type;
	svmParamsPage.m_fE=theClassifier.m_theSVM.com_param.epsion;
	svmParamsPage.m_lgIterationTime=theClassifier.m_theSVM.com_param.iteration_time;
	svmParamsPage.m_fM=theClassifier.m_theSVM.com_param.cache_size;
	svmParamsPage.m_lgNewVariable=theClassifier.m_theSVM.com_param.new_variable;
	svmParamsPage.m_lgMaxQPSize=theClassifier.m_theSVM.com_param.maximum_size;
	svmParamsPage.m_bFinal=theClassifier.m_theSVM.com_param.final_test;

	//paramsSheet.SetWizardMode();
	paramsSheet.SetTitle("参数设置");
	paramsSheet.AddPage(&trainParamsPage);
	paramsSheet.AddPage(&svmParamsPage);
	if(paramsSheet.DoModal()==IDOK)
	{
		if(!CheckPath(trainParamsPage.m_txtTrainDir)||
			!CheckPath(trainParamsPage.m_txtResultDir,true))
			return;
		
		theClassifier.m_theSVM.com_param.biased_Hyperplane=svmParamsPage.m_bB ;
		theClassifier.m_theSVM.com_param.remove_inconsitant=svmParamsPage.m_bI;
		theClassifier.m_theSVM.com_param.C                = 	svmParamsPage.m_dC;
		theClassifier.m_theSVM.com_param.cost_factor      = 	svmParamsPage.m_dJ;
					
		theClassifier.m_theSVM.com_param.loo              = 	svmParamsPage.m_bX;
		theClassifier.m_theSVM.com_param.search_depth     = 	svmParamsPage.m_fK;
		theClassifier.m_theSVM.com_param.rho              = 	svmParamsPage.m_fO;
		theClassifier.m_theSVM.com_param.fraction         = 	svmParamsPage.m_fP;

		theClassifier.m_theSVM.com_param.rbf_gamma        = 	svmParamsPage.m_fG  ;
		theClassifier.m_theSVM.com_param.poly_c           = 	svmParamsPage.m_fC;
		theClassifier.m_theSVM.com_param.poly_s           = 	svmParamsPage.m_fS;
		theClassifier.m_theSVM.com_param.poly_degree      = 	svmParamsPage.m_iD  ;
		theClassifier.m_theSVM.com_param.kernel_type      = 	svmParamsPage.m_iKernelType ;

		theClassifier.m_theSVM.com_param.epsion           = 	svmParamsPage.m_fE;
		theClassifier.m_theSVM.com_param.iteration_time   = 	svmParamsPage.m_lgIterationTime;
		theClassifier.m_theSVM.com_param.cache_size       = 	svmParamsPage.m_fM  ;
		theClassifier.m_theSVM.com_param.new_variable     = 	svmParamsPage.m_lgNewVariable;
		theClassifier.m_theSVM.com_param.maximum_size     = 	svmParamsPage.m_lgMaxQPSize;
		theClassifier.m_theSVM.com_param.final_test       = 	svmParamsPage.m_bFinal  ;

		theClassifier.m_paramClassifier.m_txtTrainDir    =   trainParamsPage.m_txtTrainDir;
		theClassifier.m_paramClassifier.m_txtResultDir   =   trainParamsPage.m_txtResultDir;
		theClassifier.m_paramClassifier.m_nFSMode        =   trainParamsPage.m_nFSMode;
		theClassifier.m_paramClassifier.m_nWordSize      =   trainParamsPage.m_nWordSize;
		theClassifier.m_paramClassifier.m_nSelMode       =   trainParamsPage.m_nSelMode;
		theClassifier.m_paramClassifier.m_nOpMode        =   trainParamsPage.m_nOpMode;
		theClassifier.m_paramClassifier.m_nLanguageType  =   trainParamsPage.m_nLanguageType;
		theClassifier.m_paramClassifier.m_bStem          =   trainParamsPage.m_bStem;
		theClassifier.m_paramClassifier.m_nWeightMode    =   trainParamsPage.m_nWeightMode;
		theClassifier.m_paramClassifier.m_nClassifierType=   CClassifierParam::nCT_SVM;

		Train();
		m_strModel=theClassifier.m_paramClassifier.m_txtResultDir+"\\model.prj";
		theClassifier.m_paramClassifier.m_strModelFile="model";
	}	
}*/

void CMainFrame::OnMainTrainsvm() 
{
	// TODO: Add your command handler code here
	CPropertySheet paramsSheet;
	CSVMParamsPage   svmParamsPage;
	CTrainParamsPage trainParamsPage;
	
	if(theClassifier.m_paramClassifier.m_txtTrainDir.IsEmpty())
		theClassifier.m_paramClassifier.m_txtTrainDir=theApp.m_strPath+"\\sample\\train";
	if(theClassifier.m_paramClassifier.m_txtResultDir.IsEmpty())
		theClassifier.m_paramClassifier.m_txtResultDir=theApp.m_strPath+"\\sample\\svm";
	trainParamsPage.m_txtTrainDir  = theClassifier.m_paramClassifier.m_txtTrainDir;
	trainParamsPage.m_txtResultDir = theClassifier.m_paramClassifier.m_txtResultDir;
	trainParamsPage.m_nFSMode      = theClassifier.m_paramClassifier.m_nFSMode;
	trainParamsPage.m_nWordSize    = theClassifier.m_paramClassifier.m_nWordSize;
	trainParamsPage.m_nUKS_k       = theClassifier.m_paramClassifier.m_nUKS_k;
	trainParamsPage.m_nSelMode     = theClassifier.m_paramClassifier.m_nSelMode;
	trainParamsPage.m_nOpMode      = theClassifier.m_paramClassifier.m_nOpMode;
	trainParamsPage.m_nLanguageType= theClassifier.m_paramClassifier.m_nLanguageType;
	trainParamsPage.m_bStem        = theClassifier.m_paramClassifier.m_bStem;
    trainParamsPage.m_nCS   	   = theClassifier.m_paramClassifier.m_nCS;
	trainParamsPage.m_nRS          = theClassifier.m_paramClassifier.m_nRS;
	trainParamsPage.m_Length   	   = theClassifier.m_paramClassifier.m_Length;
	trainParamsPage.m_nUKS         = theClassifier.m_paramClassifier.m_nUKS; 
	trainParamsPage.m_ClassifierW  = theClassifier.m_paramClassifier.m_ClassifierW;
    trainParamsPage.m_nURL         = theClassifier.m_paramClassifier.m_nURL;
	trainParamsPage.m_nWeightMode  = theClassifier.m_paramClassifier.m_nWeightMode;

	svmParamsPage.m_bB=theClassifier.m_theSVM.com_param.biased_Hyperplane;
	svmParamsPage.m_bI=theClassifier.m_theSVM.com_param.remove_inconsitant;
	svmParamsPage.m_dC=theClassifier.m_theSVM.com_param.C;
	svmParamsPage.m_dJ=theClassifier.m_theSVM.com_param.cost_factor;
	svmParamsPage.m_bX=theClassifier.m_theSVM.com_param.loo;
	svmParamsPage.m_fK=theClassifier.m_theSVM.com_param.search_depth;
	svmParamsPage.m_fO=theClassifier.m_theSVM.com_param.rho;
	svmParamsPage.m_fP=theClassifier.m_theSVM.com_param.fraction;
	svmParamsPage.m_fG=theClassifier.m_theSVM.com_param.rbf_gamma;
	svmParamsPage.m_fC=theClassifier.m_theSVM.com_param.poly_c;
	svmParamsPage.m_fS=theClassifier.m_theSVM.com_param.poly_s;
	svmParamsPage.m_iD=theClassifier.m_theSVM.com_param.poly_degree;
	svmParamsPage.m_iKernelType=theClassifier.m_theSVM.com_param.kernel_type;
	svmParamsPage.m_fE=theClassifier.m_theSVM.com_param.epsion;
	svmParamsPage.m_lgIterationTime=theClassifier.m_theSVM.com_param.iteration_time;
	svmParamsPage.m_fM=theClassifier.m_theSVM.com_param.cache_size;
	svmParamsPage.m_lgNewVariable=theClassifier.m_theSVM.com_param.new_variable;
	svmParamsPage.m_lgMaxQPSize=theClassifier.m_theSVM.com_param.maximum_size;
	svmParamsPage.m_bFinal=theClassifier.m_theSVM.com_param.final_test;

	//paramsSheet.SetWizardMode();
	paramsSheet.SetTitle("Parameter settings");
	paramsSheet.AddPage(&trainParamsPage);
	paramsSheet.AddPage(&svmParamsPage);
	if(paramsSheet.DoModal()==IDOK)
	{
		if(!CheckPath(trainParamsPage.m_txtTrainDir)||
			!CheckPath(trainParamsPage.m_txtResultDir,true))
			return;
		
		theClassifier.m_theSVM.com_param.biased_Hyperplane=svmParamsPage.m_bB ;
		theClassifier.m_theSVM.com_param.remove_inconsitant=svmParamsPage.m_bI;
		theClassifier.m_theSVM.com_param.C                = 	svmParamsPage.m_dC;
		theClassifier.m_theSVM.com_param.cost_factor      = 	svmParamsPage.m_dJ;
					
		theClassifier.m_theSVM.com_param.loo              = 	svmParamsPage.m_bX;
		theClassifier.m_theSVM.com_param.search_depth     = 	svmParamsPage.m_fK;
		theClassifier.m_theSVM.com_param.rho              = 	svmParamsPage.m_fO;
		theClassifier.m_theSVM.com_param.fraction         = 	svmParamsPage.m_fP;

		theClassifier.m_theSVM.com_param.rbf_gamma        = 	svmParamsPage.m_fG  ;
		theClassifier.m_theSVM.com_param.poly_c           = 	svmParamsPage.m_fC;
		theClassifier.m_theSVM.com_param.poly_s           = 	svmParamsPage.m_fS;
		theClassifier.m_theSVM.com_param.poly_degree      = 	svmParamsPage.m_iD  ;
		theClassifier.m_theSVM.com_param.kernel_type      = 	svmParamsPage.m_iKernelType ;

		theClassifier.m_theSVM.com_param.epsion           = 	svmParamsPage.m_fE;
		theClassifier.m_theSVM.com_param.iteration_time   = 	svmParamsPage.m_lgIterationTime;
		theClassifier.m_theSVM.com_param.cache_size       = 	svmParamsPage.m_fM  ;
		theClassifier.m_theSVM.com_param.new_variable     = 	svmParamsPage.m_lgNewVariable;
		theClassifier.m_theSVM.com_param.maximum_size     = 	svmParamsPage.m_lgMaxQPSize;
		theClassifier.m_theSVM.com_param.final_test       = 	svmParamsPage.m_bFinal  ;

		theClassifier.m_paramClassifier.m_txtTrainDir    =   trainParamsPage.m_txtTrainDir;
		theClassifier.m_paramClassifier.m_txtResultDir   =   trainParamsPage.m_txtResultDir;
		theClassifier.m_paramClassifier.m_nFSMode        =   trainParamsPage.m_nFSMode;
		theClassifier.m_paramClassifier.m_nWordSize      =   trainParamsPage.m_nWordSize;
		theClassifier.m_paramClassifier.m_nUKS_k         =   trainParamsPage.m_nUKS_k;
		theClassifier.m_paramClassifier.m_nSelMode       =   trainParamsPage.m_nSelMode;
		theClassifier.m_paramClassifier.m_nOpMode        =   trainParamsPage.m_nOpMode;
		theClassifier.m_paramClassifier.m_nLanguageType  =   trainParamsPage.m_nLanguageType;
		theClassifier.m_paramClassifier.m_bStem          =   trainParamsPage.m_bStem;
		theClassifier.m_paramClassifier.m_nWeightMode    =   trainParamsPage.m_nWeightMode;
		theClassifier.m_paramClassifier.m_nClassifierType=   CClassifierParam::nCT_SVM;
	    theClassifier.m_paramClassifier.m_nCS            =   trainParamsPage.m_nCS;
        theClassifier.m_paramClassifier.m_nRS            =   trainParamsPage.m_nRS;
		theClassifier.m_paramClassifier.m_Length         =   trainParamsPage.m_Length;
	    theClassifier.m_paramClassifier.m_nUKS           =   trainParamsPage.m_nUKS;
	    theClassifier.m_paramClassifier.m_ClassifierW    =   trainParamsPage.m_ClassifierW;
		theClassifier.m_paramClassifier.m_nURL           =   trainParamsPage.m_nURL;
		Train();
		m_strModel=theClassifier.m_paramClassifier.m_txtResultDir+"\\model.prj";
		theClassifier.m_paramClassifier.m_strModelFile="model";
	}	
}

void CMainFrame::OnMainTrainbayes()
{
	CPropertySheet paramsSheet;
	CTrainParamsPage trainParamsPage;
	
	if(theClassifier.m_paramClassifier.m_txtTrainDir.IsEmpty())
		theClassifier.m_paramClassifier.m_txtTrainDir=theApp.m_strPath+"\\sample\\train";
	if(theClassifier.m_paramClassifier.m_txtResultDir.IsEmpty())
		theClassifier.m_paramClassifier.m_txtResultDir=theApp.m_strPath+"\\sample\\bayes";
	trainParamsPage.m_txtTrainDir  = theClassifier.m_paramClassifier.m_txtTrainDir;
	trainParamsPage.m_txtResultDir = theClassifier.m_paramClassifier.m_txtResultDir;
	trainParamsPage.m_nFSMode      = theClassifier.m_paramClassifier.m_nFSMode;
	trainParamsPage.m_nWordSize    = theClassifier.m_paramClassifier.m_nWordSize;
	trainParamsPage.m_nUKS_k       = theClassifier.m_paramClassifier.m_nUKS_k;
	trainParamsPage.m_nSelMode     = theClassifier.m_paramClassifier.m_nSelMode;
	trainParamsPage.m_nOpMode      = theClassifier.m_paramClassifier.m_nOpMode;
	trainParamsPage.m_nLanguageType= theClassifier.m_paramClassifier.m_nLanguageType;
	trainParamsPage.m_bStem        = theClassifier.m_paramClassifier.m_bStem;
	trainParamsPage.m_nWeightMode  = theClassifier.m_paramClassifier.m_nWeightMode;
    trainParamsPage.m_nCS   	   = theClassifier.m_paramClassifier.m_nCS;
	trainParamsPage.m_nRS          = theClassifier.m_paramClassifier.m_nRS;
	trainParamsPage.m_Length       = theClassifier.m_paramClassifier.m_Length;
	trainParamsPage.m_nUKS         = theClassifier.m_paramClassifier.m_nUKS; 
	trainParamsPage.m_ClassifierW  = theClassifier.m_paramClassifier.m_ClassifierW;
	trainParamsPage.m_nURL         = theClassifier.m_paramClassifier.m_nURL;
	//paramsSheet.SetWizardMode();
	paramsSheet.SetTitle("Parameter settings");
	paramsSheet.AddPage(&trainParamsPage);
	if(paramsSheet.DoModal()==IDOK)
	{
		if(!CheckPath(trainParamsPage.m_txtTrainDir)||
			!CheckPath(trainParamsPage.m_txtResultDir,true))
			return;
		
		theClassifier.m_paramClassifier.m_txtTrainDir    =   trainParamsPage.m_txtTrainDir;
		theClassifier.m_paramClassifier.m_txtResultDir   =   trainParamsPage.m_txtResultDir;
		theClassifier.m_paramClassifier.m_nFSMode        =   trainParamsPage.m_nFSMode;
		theClassifier.m_paramClassifier.m_nWordSize      =   trainParamsPage.m_nWordSize;
		theClassifier.m_paramClassifier.m_nUKS_k         =   trainParamsPage.m_nUKS_k;
		theClassifier.m_paramClassifier.m_nSelMode       =   trainParamsPage.m_nSelMode;
		theClassifier.m_paramClassifier.m_nOpMode        =   trainParamsPage.m_nOpMode;
		theClassifier.m_paramClassifier.m_nLanguageType  =   trainParamsPage.m_nLanguageType;
		theClassifier.m_paramClassifier.m_bStem          =   trainParamsPage.m_bStem;
		theClassifier.m_paramClassifier.m_nWeightMode    =   trainParamsPage.m_nWeightMode;
		theClassifier.m_paramClassifier.m_nCS            =   trainParamsPage.m_nCS;
        theClassifier.m_paramClassifier.m_nRS            =   trainParamsPage.m_nRS;
		theClassifier.m_paramClassifier.m_Length         =   trainParamsPage.m_Length;
	    theClassifier.m_paramClassifier.m_nUKS           =   trainParamsPage.m_nUKS;
	    theClassifier.m_paramClassifier.m_ClassifierW    =   trainParamsPage.m_ClassifierW;
		theClassifier.m_paramClassifier.m_nURL           =   trainParamsPage.m_nURL;
		theClassifier.m_paramClassifier.m_nClassifierType=   CClassifierParam::nCT_BAYES;

		Train();
		m_strModel=theClassifier.m_paramClassifier.m_txtResultDir+"\\model.prj";
		theClassifier.m_paramClassifier.m_strModelFile="model";
	}
}

bool CMainFrame::CheckPath(CString &strPath, bool bCreateDir)
{
	if(_chdir(strPath)<0)
	{
		if(bCreateDir)
		{
			if(_mkdir(strPath)<0)
			{
				CMessage::PrintError("无法创建目录"+strPath+"!");
				return false;
			}
		}
		else
		{
			CMessage::PrintError("目录"+strPath+"不存在!");
			return false;
		}
	}
	if(strPath[strPath.GetLength()-1]=='\\')
		strPath=strPath.Left(strPath.GetLength()-1);
	return true;
}

/*
void CMainFrame::OnToolsConvert() 
{
	CConvertDlg dlg;
	if(dlg.DoModal()==IDOK)
	{
		if(dlg.m_nSourceFormat==0&&!CheckPath(dlg.m_strSource)) return;
		if(dlg.m_nTargetFormat==0&&!CheckPath(dlg.m_strTarget,true)) return;

		strcpy(theSaxFileHandler.m_pTarget,dlg.m_strTarget);
		strcpy(theSaxFileHandler.m_pSource,dlg.m_strSource);
		theSaxFileHandler.m_nTargetFormat=dlg.m_nTargetFormat;
		theSaxFileHandler.m_nSourceFormat=dlg.m_nSourceFormat;
		theSaxFileHandler.m_bShowErrors=dlg.m_bShowErrors;
		theSaxFileHandler.m_nSplitType=dlg.m_nSplitType;
		theSaxFileHandler.m_nDocsSetType=dlg.m_nDocsSetType;
		theSaxFileHandler.m_nCategoryNum=dlg.m_nCategoryNum;
		CMessage::ClearInfo();
		BeginComputation();
		m_pThread=AfxBeginThread(ConvertThreadProc,GetSafeHwnd(),THREAD_PRIORITY_NORMAL);
	}	
}

void CMainFrame::OnToolsSeldoc() 
{
	// TODO: Add your command handler code here
	CDocSelDlg dlg;
	dlg.m_strSource=theSelectDocs.m_strSource;
	dlg.m_strTarget=theSelectDocs.m_strTarget;
	dlg.m_nPercent=theSelectDocs.m_nPercent;

	if(dlg.DoModal()==IDOK)
	{
		theSelectDocs.m_strSource=dlg.m_strSource;
		theSelectDocs.m_strTarget=dlg.m_strTarget;
		theSelectDocs.m_nPercent=dlg.m_nPercent;
		CMessage::ClearInfo();
		BeginComputation();
		m_pThread=AfxBeginThread(SeldocsThreadProc,GetSafeHwnd(),THREAD_PRIORITY_NORMAL);
	}
}
*/
void CMainFrame::OnToolsWordseg() 
{
	CSegDocDlg dlg;
	if(dlg.DoModal()==IDOK)
	{
		CMessage::ClearInfo();
		BeginComputation();
		m_pThread=AfxBeginThread(SegDocsThreadProc,GetSafeHwnd(),THREAD_PRIORITY_NORMAL);
	}	
}


