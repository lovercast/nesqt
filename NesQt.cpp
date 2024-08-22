#include "NesQt.h"

void NESQT::warn(QString msg)
{
    QMessageBox::warning(nullptr, QMessageBox::tr("NESST"), msg);
}

void NESQT::not_implemented() {
    std::fputs("NesQt: Not implemented\n", stderr);
    std::abort();
}
