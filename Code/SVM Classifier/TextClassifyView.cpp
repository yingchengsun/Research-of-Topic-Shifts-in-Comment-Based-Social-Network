// TextClassifyView.cpp : implementation of the CTextClassifyView class
//

#include "stdafx.h"
#include "TextClassify.h"

#include "TextClassifyDoc.h"
#include "TextClassifyView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextClassifyView

IMPLEMENT_DYNCREATE(CTextClassifyView, CEditView)

BEGIN_MESSAGE_MAP(CTextClassifyView, CEditView)
	//{{AFX_MSG_MAP(CTextClassifyView)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextClassifyView construction/destruction

CTextClassifyView::CTextClassifyView()
{
	// TODO: add construction code here

}

CTextClassifyView::~CTextClassifyView()
{
}

BOOL CTextClassifyView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	BOOL bPreCreated = CEditView::PreCreateWindow(cs);
	cs.style &= ~(ES_AUTOHSCROLL|WS_HSCROLL);	// Enable word-wrapping

	return bPreCreated;
}

/////////////////////////////////////////////////////////////////////////////
// CTextClassifyView drawing

void CTextClassifyView::OnDraw(CDC* pDC)
{
	
	CTextClassifyDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	// TODO: add draw code for native data here
}

/////////////////////////////////////////////////////////////////////////////
// CTextClassifyView diagnostics

#ifdef _DEBUG
void CTextClassifyView::AssertValid() const
{
	CEditView::AssertValid();
}

void CTextClassifyView::Dump(CDumpContext& dc) const
{
	CEditView::Dump(dc);
}

CTextClassifyDoc* CTextClassifyView::GetDocument() // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTextClassifyDoc)));
	return (CTextClassifyDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextClassifyView message handlers
