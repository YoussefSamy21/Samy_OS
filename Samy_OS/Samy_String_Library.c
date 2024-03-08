/*
 * Samy_String_Library.c
 *
 *  Created on: August 20, 2023
 *      Author: Youssef Samy
 */

#include "Samy_String_Library.h"

/************************ My Previous String Library Created in Unit2 C-Programming ************************/

//void Get_String(uint8 *ptr)
//{
//	uint32 i = 0;
//	while(i < MAX_SIZE)
//	{
//		scanf("%c",&ptr[i]);
//		if(ptr[i] == '\n')
//		{
//			ptr[i] = '\0';
//			break;
//		}
//		i++;
//	}
//}

uint32 String_Length(uint8 *ptr)
{
	uint32 i = 0;
	while(ptr[i] != '\0')
	{
		i++;
	}
	return i;
}

void Reverse_String(uint8 *ptr)
{
	uint32 temp;
	uint32 i = 0;
	uint32 j = String_Length(ptr)-1;
	while(i<j)
	{
		temp = ptr[i];
		ptr[i] = ptr[j];
		ptr[j] = temp;
		i++;
		j--;
	}
}

//void Print_String(uint8 *ptr)
//{
//	uint32 i = 0;
//	while(ptr[i] != '\0')
//	{
//		printf("%c",ptr[i]);
//		i++;
//	}
//}


void String_Copy(uint8 *ptr1 , uint8 *ptr2)
{
	uint32 i = 0;
	while(ptr2[i] != '\0')
	{
		ptr1[i] = ptr2[i];
		i++;
	}
	ptr1[i] = '\0';
}

void String_Concatenate(uint8 *ptr1 , uint8 *ptr2)
{
	uint32 i = 0;
	uint32 j = String_Length(ptr1); // j now has the index of '\0' in the destination array
	while(ptr2[i] != '\0')
	{
		ptr1[j] = ptr2[i];
		i++;
		j++;
	}
	ptr1[j] = '\0';
}


void String_Lower_Case(uint8 *ptr)
{
	uint32 i = 0;
	while(ptr[i] != '\0')
	{
		if((ptr[i]>= 'A') && (ptr[i]<= 'Z'))
		{
			ptr[i] = (ptr[i] - 'A') + 'a';
		}
		i++;
	}
}

void String_Upper_Case(uint8 *ptr)
{
	uint32 i = 0;
	while(ptr[i] != '\0')
	{
		if((ptr[i]>= 'a') && (ptr[i]<= 'z'))
		{
			ptr[i] = (ptr[i] - 'a') + 'A';
		}
		i++;
	}
}

uint32 String_to_Integer(uint8 *ptr)
{
	uint32 num = 0 , i = 0;
	while(ptr[i] != '\0')
	{
		if(ptr[i]>= '0' && ptr[i]<= '9')
		{
			num = (num*10)+(ptr[i]- '0');
		}
		i++;
	}
	return num;
}

/*****************************************************************************************************/

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
uint32 String_Compare(uint8 *ptr1 , uint8 *ptr2)
{
	uint32 res = 0 , i = 0;
	// String_Lower_Case(ptr1);
	// String_Lower_Case(ptr2);
	while( (ptr1[i] != '\0') && (ptr2[i] != '\0') )
	{
		if(ptr1[i] < ptr2[i])
		{
			res = -1;
			break;
		}
		else if(ptr1[i] > ptr2[i])
		{
			res = 1;
			break;
		}
		i++;
	}
	return res;
}
