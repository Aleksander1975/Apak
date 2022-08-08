#ifndef DATA_TYPE_1_H
#define DATA_TYPE_1_H

#include <QObject>
#include <QList>

#include "raduga_defs.h"

namespace raduga {

  struct SignalType1 {

    SignalType1(modus::SvSignal* signal, raduga::SignalParams params):
      signal(signal), params(params)
    {  }

    modus::SvSignal*      signal;
    raduga::SignalParams  params;

  };

  class DataType1 : public SvAbstractSignalCollection
  {
    Q_OBJECT
  public:
    explicit DataType1();

    void addSignal(modus::SvSignal* signal); // throw (SvException);

    void updateSignals(const raduga::DATA* data = nullptr);

  private:
    QList<SignalType1> m_signals;

  };
}


#endif // DATA_TYPE_1_H
