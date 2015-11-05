// Message.cpp: implementation of the CMessage class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Message.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

CWnd* CMessage::m_pWndInfo;
CWnd* CMessage::m_pWndStatusInfo;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMessage::CMessage()
{

}

CMessage::~CMessage()
{

}

void CMessage::SetStatusWnd(CWnd *pWnd)
{
	m_pWndStatusInfo=pWnd;
}

void CMessage::SetOutputWnd(CWnd *pWnd)
{
	m_pWndInfo=pWnd;
}

void CMessage::PrintError(CString strInfo)
{
	if(m_pWndInfo!=NULL)
	{
		CString str;
		m_pWndInfo->GetWindowText(str);
		str+="------------------error-----------------\r\n";
		str=str+strInfo+"\r\n";
		m_pWndInfo->SetWindowText(str);
		CEditView *pView=(CEditView*)m_pWndInfo;
		CEdit& edit=pView->GetEditCtrl();
		edit.LineScroll(edit.GetLineCount(), 0);
	}
}

void CMessage::PrintInfo(CString strInfo)
{
	if(m_pWndInfo!=NULL)
	{
		CString str;
		m_pWndInfo->GetWindowText(str);
		str=str+strInfo+"\r\n";
		m_pWndInfo->SetWindowText(str);
		CEditView *pView=(CEditView*)m_pWndInfo;
		CEdit& edit=pView->GetEditCtrl();
		edit.LineScroll(edit.GetLineCount(), 0);
	}
}

void CMessage::PrintStatusInfo(CString strInfo)
{
	if (m_pWndStatusInfo!=NULL)
	{
		m_pWndStatusInfo->SetWindowText(strInfo);
		CEditView *pView=(CEditView*)m_pWndInfo;
		CEdit& edit=pView->GetEditCtrl();
		edit.LineScroll(edit.GetLineCount(), 0);
	}
}

void CMessage::ClearInfo()
{
	if (m_pWndStatusInfo!=NULL)
		m_pWndStatusInfo->SetWindowText("");
	if(m_pWndInfo!=NULL)
		m_pWndInfo->SetWindowText("");
}
