/*
    Eng.Youssef Mohamed Samy
*/

#ifndef PLATFORM_TYPES_H_
#define PLATFORM_TYPES_H_


typedef unsigned char			uint8;
typedef signed char 			sint8;
typedef signed short 			sint16;
typedef unsigned short 			uint16;
typedef signed int 				sint32;
typedef unsigned int 			uint32;
typedef signed long long int 	sint64;
typedef unsigned long long int 	uint64;

typedef volatile unsigned char			  vuint8;
typedef volatile signed char 			  vsint8;
typedef volatile signed short 			  vsint16;
typedef volatile unsigned short 		  vuint16;
typedef volatile signed int 			  vsint32;
typedef volatile unsigned int 			  vuint32;
typedef volatile signed long long int 	  vsint64;
typedef volatile unsigned long long int   vuint64;

typedef float 		float32;
typedef double 		float64;

#define NULL ((void*)0)

#endif /* PLATFORM_TYPES_H_ */
