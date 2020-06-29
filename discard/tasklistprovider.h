#ifndef TASKLISTPROVIDER_H
#define TASKLISTPROVIDER_H

#include "datalist.h"

class TaskItem;
class TaskListProvider
{
public:
    virtual ~TaskListProvider() = 0;
    virtual DataList<TaskItem> getTaskList(int start, int count) const = 0;

};

#endif // TASKLISTPROVIDER_H
