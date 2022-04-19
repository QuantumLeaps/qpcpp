int main(int arc, char *argv[]) {
    . . .

    if (!QS_INIT(argv)) {         // Initialize QS target component
        return -1;                // Unable to initialize QSpy
    }

    // apply the global QS filters...
    // NOTE: global filters start as being all OFF
    QS_GLB_FILTER(QP::QS_QF_RECORDS); // turn QF-group ON
    QS_GLB_FILTER(-QP::QS_QF_TICK);   // turn #QS_QF_TICK OFF

    // apply the local QS filters...
    // NOTE: local filters start as being all ON
    QS_LOC_FILTER(-QP::QS_EP_IDS); // turn EP (Event-Pool) group  OFF
    QS_LOC_FILTER(3);              // turn AO with priority 3 ON

    // start the active objects...
    . . .

    // NOTE: the following will work only after AO_Table has been started
    QS_LOC_FILTER(-AO_Table->prio); // turn AO_Table OFF
    . . .
}
