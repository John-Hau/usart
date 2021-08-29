#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "blink.h"
#include "led.h"


void blink(void)
{
	for(int i=0;i<5;i++)
	{
		printf("test led %d\n",i);
		test_led();

	}


}
