/*
 * Arduino Due Example to test the UCOS-III QPCPP Port
 * To test this example UCOS-III port for arduino is needed
 * https://github.com/vChavezB/uc-os3-arduino-due
 * 
 * 
 */
#include <uCOS-III_Due.h>

#include "TimerTask.hpp"
#include "ExampleAO.hpp"

ExampleActiveObject exampleAO;
TimerTask timer_task(&exampleAO);

const unsigned long cpu_clock = 84000000; //Arduino Due Clock
const int TimerTaskPrio = 1;


void setup() {
  Serial.begin(115200);
  OS_ERR  err;
  QS_INIT(0);
  QP::QF::init();                               /* initialize the framework and UC OSIII */
  CPU_Init();                                  /* Initialize uC/CPU services.                          */
  OS_CPU_SysTickInitFreq(cpu_clock);            /* Init uC/OS periodic time src (SysTick).              */
  err = timer_task.Create(TimerTaskPrio);
  OS_ASSERT(err, OS_ERR_NONE);

  static QP::QEvt const * exampleQueueSto[20];
  static CPU_STK exampleStack[100];
  OS_OPT task_opt = OS_OPT_TASK_STK_CLR | OS_OPT_TASK_STK_CHK;
  exampleAO.setAttr(QP::TASK_NAME_ATTR, "Example AO"); //TODO add
  exampleAO.setAttr(QP::TASK_OPT_ATTR, static_cast<void*>(&task_opt));
  exampleAO.start(   TimerTaskPrio + 1,                // Less priority than the task that produces the signal
                     &exampleQueueSto[0],        // storage for the AO's queue
                     Q_DIM(exampleQueueSto), // queue's length [events]
                     &exampleStack,          // stack storage
                     sizeof(exampleStack),// stack size [bytes]
                     (void *)0
                 );
  OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
  OS_ASSERT(err, OS_ERR_NONE);
  while (1U) ; /* Code should never enter here */
}

void loop() {
  /* do nothing */
}

extern "C"  Q_NORETURN Q_onAssert(char const * const module, int location)
{
  Serial.print("Assert module:");
  Serial.print(module);
  Serial.print(" ");
  Serial.println(location);
  for (;;) {

  }
}
