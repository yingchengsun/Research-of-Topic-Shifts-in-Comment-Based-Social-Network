// Stemmer.h: interface for the CStemmer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STEMMER_H__F647EC3C_175E_4539_B216_98E2F54BCCFC__INCLUDED_)
#define AFX_STEMMER_H__F647EC3C_175E_4539_B216_98E2F54BCCFC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CStemmer  
{
public:
	void stem(char *p);
	CStemmer();
	virtual ~CStemmer();
	int stem(char * p, int i, int j);
private:
	int cons(int i);
	int m();
	int vowelinstem();
	int doublec(int j);
	int cvc(int i);
	int ends(char * s);
	void setto(char * s);
	void r(char * s);
	void step1ab();
	void step1c();
	void step2();
	void step3();
	void step4();
	void step5();
private:
	char * b;       /* buffer for word to be stemmed */
	int k,k0,j;     /* j is a general offset into the string */
};
extern CStemmer theStemmer;
#endif // !defined(AFX_STEMMER_H__F647EC3C_175E_4539_B216_98E2F54BCCFC__INCLUDED_)
