#ifndef TIMERTASK_HPP
#define TIMERTASK_HPP

#include <uCOS-III_Due.h>
#include "qpcpp.hpp"  // QP-C++ framework


/*
  Example class that holds a UCOS3
  task which generates a signal after an
  OS Delay and posts the event to an active object
*/
class TimerTask
{
  public:
    TimerTask(QP::QActive * _act);
    OS_ERR Create(OS_PRIO priority);
  private:
    static constexpr int stack_size = 256;
    QP::QActive * act;
    CPU_STK  stack[256];
    OS_TCB  tcb;
    int task_no;
    static void  Task(void * args);
    static int totalTasks;
    bool created = false;
};

#endif
