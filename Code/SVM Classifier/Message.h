// Message.h: interface for the CMessage class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MESSAGE_H__D829B8E8_BDA2_40FC_8272_6DD9EF78A38B__INCLUDED_)
#define AFX_MESSAGE_H__D829B8E8_BDA2_40FC_8272_6DD9EF78A38B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CMessage  
{
public:
	CMessage();
	virtual ~CMessage();
public:
	static void ClearInfo();
	static void SetOutputWnd(CWnd *);
	static void SetStatusWnd(CWnd *);
	static void PrintInfo(CString);
	static void PrintError(CString);
	static void PrintStatusInfo(CString);
private:
	static CWnd *m_pWndInfo;
	static CWnd *m_pWndStatusInfo;
};

#endif // !defined(AFX_MESSAGE_H__D829B8E8_BDA2_40FC_8272_6DD9EF78A38B__INCLUDED_)
