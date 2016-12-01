/*---------------------------------------------------------------------------------------------------------*/
/*                                                                                                         */
/* Copyright (c) Nuvoton Technology Corp. All rights reserved.                                             */
/*                                                                                                         */
/*---------------------------------------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#include "M051Series.h"				// For M051 series

#define LED0_GPIO_GRP	P3
#define LED0_GPIO_BIT	4
#define LED1_GPIO_GRP	P3
#define LED1_GPIO_BIT	5
#define LED2_GPIO_GRP	P3
#define LED2_GPIO_BIT	6
#define LED3_GPIO_GRP	P3
#define LED3_GPIO_BIT	7

extern "C"{
	void main_cpp(int n);
}

int main(){

	/* Open LED GPIO for testing */
	_GPIO_SET_PIN_MODE(LED0_GPIO_GRP, LED0_GPIO_BIT, GPIO_PMD_OUTPUT);
	_GPIO_SET_PIN_MODE(LED1_GPIO_GRP, LED1_GPIO_BIT, GPIO_PMD_OUTPUT);
	_GPIO_SET_PIN_MODE(LED2_GPIO_GRP, LED2_GPIO_BIT, GPIO_PMD_OUTPUT);
	_GPIO_SET_PIN_MODE(LED3_GPIO_GRP, LED3_GPIO_BIT, GPIO_PMD_OUTPUT);

	main_cpp(3);
	
	return 0;
}


