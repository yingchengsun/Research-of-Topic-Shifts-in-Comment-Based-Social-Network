// TextClassifyDoc.cpp : implementation of the CTextClassifyDoc class
//

#include "stdafx.h"
#include "TextClassify.h"

#include "TextClassifyDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTextClassifyDoc

IMPLEMENT_DYNCREATE(CTextClassifyDoc, CDocument)

BEGIN_MESSAGE_MAP(CTextClassifyDoc, CDocument)
	//{{AFX_MSG_MAP(CTextClassifyDoc)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextClassifyDoc construction/destruction

CTextClassifyDoc::CTextClassifyDoc()
{
	// TODO: add one-time construction code here

}

CTextClassifyDoc::~CTextClassifyDoc()
{
}

BOOL CTextClassifyDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	((CEditView*)m_viewList.GetHead())->SetWindowText(NULL);

	// TODO: add reinitialization code here
	// (SDI documents will reuse this document)

	return TRUE;
}



/////////////////////////////////////////////////////////////////////////////
// CTextClassifyDoc serialization

void CTextClassifyDoc::Serialize(CArchive& ar)
{
	// CEditView contains an edit control which handles all serialization
	((CEditView*)m_viewList.GetHead())->SerializeRaw(ar);
}

/////////////////////////////////////////////////////////////////////////////
// CTextClassifyDoc diagnostics

#ifdef _DEBUG
void CTextClassifyDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTextClassifyDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CTextClassifyDoc commands
