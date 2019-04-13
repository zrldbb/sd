#include<iostream>
#include<errno.h>
#include"tcpsever.h"

int main()
{
	Tcpsever ser;

	//运行服务器
	ser.run();

	return 0;
}
