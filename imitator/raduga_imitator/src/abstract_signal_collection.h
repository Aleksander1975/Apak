#ifndef ABSTRACT_SIGNAL_COLLECTION
#define ABSTRACT_SIGNAL_COLLECTION

#include "/home/user/job/Modus/global/signal/sv_signal.h"

namespace raduga {

  class SvAbstractSignalCollection: public QObject
  {
    Q_OBJECT

  public:
    SvAbstractSignalCollection()
    {  }

    virtual ~SvAbstractSignalCollection()
    {  }

    virtual void addSignal(modus::SvSignal* signal, modus::SignalBinding binding, quint16 bufsize) = 0;

    virtual void updateSignals(const raduga::DATA* data = nullptr) = 0;

    virtual void updateOutput(const modus::BUFF* data = nullptr) = 0;

  };

}

#endif // ABSTRACT_SIGNAL_COLLECTION

