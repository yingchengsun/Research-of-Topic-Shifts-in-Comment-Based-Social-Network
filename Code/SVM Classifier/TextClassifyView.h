// TextClassifyView.h : interface of the CTextClassifyView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTCLASSIFYVIEW_H__9EB90FB9_AFBA_442C_9B4A_F3E35847C89C__INCLUDED_)
#define AFX_TEXTCLASSIFYVIEW_H__9EB90FB9_AFBA_442C_9B4A_F3E35847C89C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTextClassifyView : public CEditView
{
protected: // create from serialization only
	CTextClassifyView();
	DECLARE_DYNCREATE(CTextClassifyView)

// Attributes
public:
	CTextClassifyDoc* GetDocument();

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextClassifyView)
	public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTextClassifyView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTextClassifyView)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // debug version in TextClassifyView.cpp
inline CTextClassifyDoc* CTextClassifyView::GetDocument()
   { return (CTextClassifyDoc*)m_pDocument; }
#endif

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTCLASSIFYVIEW_H__9EB90FB9_AFBA_442C_9B4A_F3E35847C89C__INCLUDED_)
