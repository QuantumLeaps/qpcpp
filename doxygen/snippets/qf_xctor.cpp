namespace DPP {

// local extended-thread objects .............................................
static void Thread1_run(QP::QXThread * const me); // run routine for Thread1
static QP::QXThread l_thread1(&Thread1_run, 0U);  // Thread1 instance

QP::QXThread * const XT_Thread1 = &l_thread1; // global pointer to the thread

} // namespace DPP
