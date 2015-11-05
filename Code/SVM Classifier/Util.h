/*
 * HashMapUtil.h
 *
 *  Created on: 2009-4-22
 *      Author: Administrator
 */

#ifndef HASHMAPUTIL_H_
#define HASHMAPUTIL_H_
static int prime[28] =	{57,        97,         193,		389,		769,		//list all the integer number no less than 57 total number is 28
						 1543,	  	3079,		6151,		12289,		24593,		//And each number is about half of its next number
						 49157,	  	98317,		196613,		393241,		786433,
						 1572869,   3145739,	6291469,	12582917,	25165843,
						 50331653,  100663319,	201326611,  402653189,  805306457,
						 1610612741};
class HashMapUtil
{
public:
			static int find_NextPrimeNumber( int current )
			{/*Find the next prime number by searching in the prime number list.*/
				int i = 0;
				for( ; i < 28 ; i++ )
				if ( current < prime[i] )
					break;
				return prime[i];     //return the next larger prime.
			}
};

#endif /* HASHMAPUTIL_H_ */
