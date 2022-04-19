#ifdef Q_SPY

QSTimeCtr QS::onGetTime(void) { // invoked with interrupts disabled
    QSTimeCtr ret = DPP::QS_tickTime_ - static_cast<QSTimeCtr>(SysTick->VAL);
    if ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) != 0U) { // flag set?
        ret += DPP::QS_tickPeriod_;
    }
    return ret;
}
. . .

#endif // Q_SPY
