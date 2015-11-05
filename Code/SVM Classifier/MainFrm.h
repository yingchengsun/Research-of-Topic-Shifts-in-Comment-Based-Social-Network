// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__4B0150F3_E99C_408E_B376_DF641CD2EFCF__INCLUDED_)
#define AFX_MAINFRM_H__4B0150F3_E99C_408E_B376_DF641CD2EFCF__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CTextClassifyView;
class CLeftView;
const WM_COMPUTATION_FINISH =WM_USER+100;
class CMainFrame : public CFrameWnd
{
	
protected: // create from serialization only
	CMainFrame();
	DECLARE_DYNCREATE(CMainFrame)

// Attributes
protected:
	CSplitterWnd m_wndSplitter;
public:
	CWinThread *m_pThread;
	int m_nTimer;
	CTime m_tmBegin;
	int m_nTimeIndex;
	int m_nRunning;
	bool m_bPaused;
	CString m_strModel;
	static CString m_strResultDir;
	//0 没有打开分类器; 1 平面分类器; 2 层次分类器
	int m_nClassifierType;
// Operations
public:
private:
	static UINT SeldocsThreadProc(LPVOID pParam);
	void BeginComputation();
	void CeaseComputation();
	void StopComputation();
	void Train();
	void Test();
	bool CheckPath(CString &strPath, bool bCreateDir=false);
	static UINT TrainThreadProc(LPVOID pParam);
	static UINT TestThreadProc(LPVOID pParam);
	static UINT ConvertThreadProc(LPVOID pParam);
	static UINT SegDocsThreadProc(LPVOID pParam);
// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	public:
	virtual BOOL OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CMainFrame();	
	CTextClassifyView* GetRightPane();
	CLeftView* GetLeftView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnComputationFinish(WPARAM wParam,LPARAM lParam);
	afx_msg void OnToolsWordseg();
	afx_msg void OnMainTrainknn();
	afx_msg void OnUpdateMainTrainknn(CCmdUI* pCmdUI);
	afx_msg void OnMainTrainsvm();
	afx_msg void OnUpdateMainTrainsvm(CCmdUI* pCmdUI);
	afx_msg void OnMainTrainbayes();
	afx_msg void OnUpdateMainTrainbayes(CCmdUI* pCmdUI);
	afx_msg void OnMainOpenmodel();
	afx_msg void OnUpdateMainOpenmodel(CCmdUI* pCmdUI);
	afx_msg void OnMainTest();
	afx_msg void OnUpdateMainTest(CCmdUI* pCmdUI);
	afx_msg void OnUpdateMainResults(CCmdUI* pCmdUI);
	afx_msg void OnMainResults();
	//}}AFX_MSG
	afx_msg void OnUpdateViewStyles(CCmdUI* pCmdUI);
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__4B0150F3_E99C_408E_B376_DF641CD2EFCF__INCLUDED_)
