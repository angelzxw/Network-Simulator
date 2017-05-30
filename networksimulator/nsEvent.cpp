#include "nsEvent.h"

nsEventManager::nsEventManager()
{
	global_order = 0;
}

void nsEventManager::addEvent(nsEvent * e)
{
	e->order = global_order++;
	queue.push(e);
}

bool nsEventManager::empty()
{
	return queue.empty();
}

nsEvent * nsEventManager::peek()
{
	return queue.top();
}

void nsEventManager::pop()
{
	queue.pop();
}