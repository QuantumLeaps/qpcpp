#ifndef EXAMPLE_AO_HPP
#define EXAMPLE_AO_HPP

#include "qpcpp.hpp"  // QP-C++ framework

enum Signals
{
  SIG_TIMER = QP::Q_USER_SIG,
};

class ExampleActiveObject: public QP::QActive
{
  public:
    ExampleActiveObject(): QActive(Q_STATE_CAST(Initial)){}
  private:
    Q_STATE_DECL(Initial);
    Q_STATE_DECL(State1);
    Q_STATE_DECL(State2);

};

#endif
