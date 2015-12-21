#include "gui.h"

char GUI::getDirection()
{
	if(isEmpty())
		return 'w';
	char direction = buffer[0];;
	buffer = buffer.substr(1);
	return direction;
}

void GUI::putDirection(char dir)
{
	direction = dir;
	buffer.push_back(dir);
}

bool GUI::isEmpty()
{
	buffer.empty();
}