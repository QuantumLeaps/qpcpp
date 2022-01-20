#include "TimerTask.hpp"
#include "ExampleAO.hpp"
#include <Arduino.h>

int TimerTask::totalTasks = 0;

const QP::QEvt TimerEvt = {.sig = SIG_TIMER, .poolId_ = 0, .refCtr_ = 0};

TimerTask::TimerTask(QP::QActive * _act): act(_act)
{

}

OS_ERR TimerTask::Create(OS_PRIO priority)
{
  OS_ERR err;
  if (created == false)
  {
    created = false;
    OSTaskCreate((OS_TCB     *) & (this->tcb),
                 (CPU_CHAR   *)"TimerTask",
                 (OS_TASK_PTR )this->Task,
                 (void       *)this,
                 (OS_PRIO     )priority,
                 (CPU_STK    *) & (this->stack[0]),
                 (CPU_STK_SIZE)stack_size / 10,
                 (CPU_STK_SIZE)stack_size,
                 (OS_MSG_QTY  )5,
                 (OS_TICK     )10,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK),
                 (OS_ERR     *)&err);
    if (err == OS_ERR_NONE)
    {
      task_no = totalTasks++;
    }
  }
  return err;
}

void  TimerTask::Task(void *p_arg)
{
  TimerTask * pTimerTask = (TimerTask * )p_arg;
  OS_ERR      err;
  int count = 0;
  int task_no = pTimerTask->task_no;
  Serial.print("Timer Task ");
  Serial.print(task_no);
  Serial.println(" started");
  while (1)
  {
    Serial.print("Timer Task ");
    Serial.print(task_no);
    Serial.print(" execution #");
    Serial.println(count++);
    pTimerTask->act->POST(&TimerEvt, pTimerTask);
    OSTimeDlyHMSM((CPU_INT16U) 0,
                  (CPU_INT16U) 0,
                  (CPU_INT16U) 1,   //Seconds
                  (CPU_INT32U)500, //Milliseconds
                  (OS_OPT )OS_OPT_TIME_DLY,
                  (OS_ERR *)&err);
  }
}
