#include "qpcpp.hpp"  // QP-C++ framework
#include <Arduino.h>

//#define QS_ON

constexpr int QsTXBuffer_KB = 10; //Units are in Kilobytes
constexpr int QsRXBuffer_B  = 500;//Units are in bytes
bool QP::QS::onStartup(void const * arg)
{

  static uint8_t qsTxBuf[QsTXBuffer_KB * 1024]; // buffer for QS transmit channel
  static uint8_t qsRxBuf[QsRXBuffer_B];    // buffer for QS receive channel
  initBuf  (qsTxBuf, sizeof(qsTxBuf));
  rxInitBuf(qsRxBuf, sizeof(qsRxBuf));
#ifdef QS_ON
  QS_GLB_FILTER(QP::QS_AO_RECORDS); // active object records
  QS_GLB_FILTER(QP::QS_SM_RECORDS); // state machine records
#endif
  return true; // return success
}

void QP::QS::onCommand(uint8_t cmdId, uint32_t param1,
                       uint32_t param2, uint32_t param3)
{
  (void)cmdId;
  (void)param1;
  (void)param2;
  (void)param3;
  //Handling of User Commands from Qspy should go here
}


void QP::QS::onCleanup(void)
{

}

QP::QSTimeCtr QP::QS::onGetTime(void)
{
  return millis();
}

void QP::QS::onFlush(void)
{
#ifdef QS_ON
  uint16_t len = 0xFFFFU; // big number to get as many bytes as available
  uint8_t const *buf = QS::getBlock(&len); // get continguous block of data
  while (buf != nullptr)
  { // data available?
    Serial.write(buf, len); // might poll until all bytes fit
    len = 0xFFFFU; // big number to get as many bytes as available
    buf = QS::getBlock(&len); // try to get more data
  }
  Serial.flush(); // wait for the transmission of outgoing data to complete
#endif
}

void QP::QS::onReset(void)
{
  NVIC_SystemReset();
}
