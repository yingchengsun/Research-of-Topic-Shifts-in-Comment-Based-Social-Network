#include<iostream>
using namespace std;
#include "fstream"
#include "string"
#include <cstdio>
#include <list>

//用sort通用算法来排序向量容器中的元素
#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
using namespace std;

class log{
private://访问时间\t用户ID\t[查询词]\t该URL在返回结果中的排名\t用户点击的顺序号\t用户点击的URL
//其中，用户ID是根据用户使用浏览器访问搜索引擎时的Cookie信息自动赋值，即同一次使用浏览器输入的不同查询对应同一个用户ID。
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
	bool operator <   (const log& l )   const //升序排序时必须写的函数
	{ 

	   if( time != l.time)

		 return time < l.time; 

		 else //if(userID == l.userID)
			 			  		 
			 return userID < l.userID; //return text < l.text;
	  
	}

	bool operator >   (const log& l )   const //降序排序时必须写的函数
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
	mymap["唐伯虎点秋香"]=9527;         
	mymap["百万富翁的生活"]=1000000;         
	mymap[]=10000;              
		if(mymap.find("白领的工资底线") != mymap.end())
		{                          
		}
}*/
void LogPurify(){

}