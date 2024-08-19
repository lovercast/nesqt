#pragma once
#ifndef NESQT_H
#define NESQT_H

#include <QException>
#include <QMessageBox>

namespace NESQT {

class NotImplemented : public QException {
public:
    void raise() const override { throw *this; }
    NotImplemented *clone() const override { return new NotImplemented(*this); }
};

enum GridSize {
    None = 0x00,
    OneByOne = 0x01,
    TwoByTwo = 0x02,
    FourByFour = 0x04,
};

enum UndoTypes {
    Nametable = 0,
    CHREdit,
};

void warn(QString msg);

} // namespace NESQT

#endif // NESQT_H
