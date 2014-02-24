#include "headmotor.h"
main(void)
{
	int ID=1;
	int angle=get_pos(ID);
	printf("%d",angle);
}