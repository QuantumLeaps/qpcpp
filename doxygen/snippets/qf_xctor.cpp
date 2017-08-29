namespace DPP {

// local extended-thread objects .............................................
static void Thread1_run(QP::QXThread * const me); // run routine for Thread1

static QP::QXThread l_test1(&Thread1_run, 0U); //<== QXThread::QXThread() ctor
. . .
} // namespace DPP