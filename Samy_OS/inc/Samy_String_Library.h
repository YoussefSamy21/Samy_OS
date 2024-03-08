/*
 * Samy_String_Library.h
 *
 *  Created on: August 20, 2023
 *      Author: Youssef Samy
 */

#ifndef INC_SAMY_STRING_LIBRARY_H_
#define INC_SAMY_STRING_LIBRARY_H_

#include "Platform_Types.h"

/************************ My Previous String Library Created in Unit2 C-Programming ************************/
//void Get_String(uint8 *ptr);

uint32 String_Length(uint8 *ptr);

void Reverse_String(uint8 *ptr);

//void Print_String(uint8 *ptr);

void String_Copy(uint8 *ptr1 , uint8 *ptr2);

void String_Concatenate(uint8 *ptr1 , uint8 *ptr2);

void String_Lower_Case(uint8 *ptr); // changes all string letters to the Lower case

void String_Upper_Case(uint8 *ptr);	// changes all string letters to the Upper case

uint32 String_to_Integer(uint8 *ptr); // EX: "120KG" ==> 120  , "Cost is 30" ==> 30  ,  "1024" ==> 1024

uint32 String_Compare(uint8 *ptr1 , uint8 *ptr2);

/*
	String_Compare function compares two strings and produces one of three results:
	1- if the two string are identical it gives 0
	2- if the First string is lower in the alphabetic order it gives -1
	3- if the First string is higher in the alphabetic order it gives 1

	Remark:
	This function differentiate between Capital and Small Letters ,
	Where the ASCII code of the Capital letters is lower than the ASCII code of the Small letters , EX: MAGED is less than ahmed

	To Solve this Problem , we can convert both strings to the same case first (using String_Lower_Case or String_Upper_Case ), then Compare them.
*/

#endif /* INC_SAMY_STRING_LIBRARY_H_ */
