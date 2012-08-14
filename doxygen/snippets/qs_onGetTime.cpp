#ifdef Q_SPY

QSTimeCtr QS::onGetTime(void) {
    return (QSTimeCtr)clock();
}

. . .

#endif // Q_SPY
