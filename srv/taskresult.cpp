#include "taskresult.h"

#pragma execution_character_set("utf-8")

TaskResult::TaskResult(int id):TaskResult(id, -1)
{

}

TaskResult::TaskResult(int id, int type):_id(id), _type(type)
{

}
