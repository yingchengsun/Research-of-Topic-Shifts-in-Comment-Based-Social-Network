#include<iostream>
using namespace std;
#include "fstream"
#include "string"
#include <cstdio>
#include <list>

//��sortͨ���㷨���������������е�Ԫ��
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
using namespace std;

class log{
private://����ʱ��\t�û�ID\t[��ѯ��]\t��URL�ڷ��ؽ���е�����\t�û������˳���\t�û������URL
//���У��û�ID�Ǹ����û�ʹ�������������������ʱ��Cookie��Ϣ�Զ���ֵ����ͬһ��ʹ�����������Ĳ�ͬ��ѯ��Ӧͬһ���û�ID��
	string time;
	double userID;
	string text;
	int rank;
	double order;
	string URL;
public:
	void setTime(string time){
		this->time=time;
	}
	string getTime(){
		return this->time;
	}
	void setUserID(double userID){
		this->userID=userID;
	}
	double getUserID(){
		return this->userID;
	}
	void setText(string text){
		this->text=text;
	}
	string getText(){
		return this->text;
	}

	void setRank(int rank){
		this->rank=rank;
	}
	int getRank()
	{
		return this->rank;
	}
	void setOrder(string order){
		this->order=order;
	}
	double getOrder()
	{
		return this->order;
	}
	void setURL(string URL){
		this->URL=URL;
	}
	string getURL(){
		return this->URL;
	}
	bool operator <   (const log& l )   const //��������ʱ����д�ĺ���
	{ 

	   if( time != l.time)

		 return time < l.time; 

		 else //if(userID == l.userID)
			 			  		 
			 return userID < l.userID; //return text < l.text;
	  
	}

	bool operator >   (const log& l )   const //��������ʱ����д�ĺ���
	{ 

	  if(time != l.time){

			return time > l.time;

	   }

	  else 
	   return userID > l.userID; //return text > l.text;
	}

	void shaixuan()
	{
	 
	
	
	}

	unsigned  int  ELFHash( string  str)
	{
         unsigned  int  hash  =   0 ;
         unsigned  int  x     =   0 ;

          while  ( * str)
          {
                 hash  =  (hash  <<   4 )  +  ( * str ++ );
                  if  ((x  =  hash  &   0xF0000000L )  !=   0 )
                   {
                         hash  ^=  (x  >>   24 );
                         hash  &=   ~ x;
                 } 
         } 

          return  (hash  &   0x7FFFFFFF );
	}

};
/*
#include <hash_map> 
#include <string> 
using namespace std; 
int main(){         
	hash_map<string,int> mymap;       	
	mymap["�Ʋ���������"]=9527;         
	mymap["�����̵�����"]=1000000;         
	mymap[]=10000;              
		if(mymap.find("����Ĺ��ʵ���") != mymap.end())
		{                          
		}
}*/
void LogPurify(){

}