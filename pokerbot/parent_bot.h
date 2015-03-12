#ifndef PARENT_BOT_H
#define PARENT_BOT_H

#include "thread_utils.h"
#include <tchar.h>

int main_bot_start();
s_thread * main_bot_get_thread(); 
int main_bot_terminate();
void main_bot_cleanup();

#endif