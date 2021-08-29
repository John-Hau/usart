#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"./usart/usart.h"
#include"./led/led.h"
#include"./led/blink.h"
//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
int main(int argc,char* argv[])
{

	printf("hello world\n");
	test_usart();
	test_led();
	blink();


	return 0;
}
