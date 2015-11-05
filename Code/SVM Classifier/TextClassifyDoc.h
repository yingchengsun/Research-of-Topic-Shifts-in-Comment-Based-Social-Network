// TextClassifyDoc.h : interface of the CTextClassifyDoc class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_TEXTCLASSIFYDOC_H__BE26CECA_4FAB_4A1D_A4A2_0D9AD7819BD1__INCLUDED_)
#define AFX_TEXTCLASSIFYDOC_H__BE26CECA_4FAB_4A1D_A4A2_0D9AD7819BD1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class CTextClassifyDoc : public CDocument
{
protected: // create from serialization only
	CTextClassifyDoc();
	DECLARE_DYNCREATE(CTextClassifyDoc)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTextClassifyDoc)
	public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CTextClassifyDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	//{{AFX_MSG(CTextClassifyDoc)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXTCLASSIFYDOC_H__BE26CECA_4FAB_4A1D_A4A2_0D9AD7819BD1__INCLUDED_)
