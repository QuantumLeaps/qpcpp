#include "ExampleAO.hpp"
#include <Arduino.h>

Q_STATE_DEF(ExampleActiveObject, Initial)
{
  Serial.println("initial state");
  return tran(&State1);
}

Q_STATE_DEF(ExampleActiveObject, State1)
{
  QP::QState status_;
  switch (e->sig)
  {
    case Q_ENTRY_SIG:
      Serial.println("entry state1");
      status_ = Q_RET_HANDLED;
      break;
    case SIG_TIMER:
      Serial.println("Transition to State 2");
      status_ = tran(&State2);
      break;
    default:
      status_ = super(&top);
      break;
  }
  return status_;
}

Q_STATE_DEF(ExampleActiveObject, State2)
{
  QP::QState status_;
  switch (e->sig)
  {
    case Q_ENTRY_SIG:
      Serial.println("entry state2");
      status_ = Q_RET_HANDLED;
      break;
    case SIG_TIMER:
      Serial.println("Transition State 1");
      status_ = tran(&State1);
      break;
    default:
      status_ = super(&top);
      break;
  }
  return status_;
}
