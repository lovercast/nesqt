#include "NesQt.h"

void NESQT::warn(QString msg)
{
    QMessageBox::warning(nullptr, QMessageBox::tr("NESST"), msg);
}
