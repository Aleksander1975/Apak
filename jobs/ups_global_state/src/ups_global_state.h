#ifndef UPS_GLOBAL_STATE_H
#define UPS_GLOBAL_STATE_H

#include <QSet>

#include "ups_global_state_global.h"

#include "../../../../../Modus/global/job/sv_abstract_job.h"
#include "../../../../../Modus/global/signal/sv_signal.h"

#include "ups_global_state_defs.h"
#include "supported_signal_tags.h"

extern "C" {

  UPS_GLOBAL_STATE_EXPORT modus::SvAbstractJob* create();

}

#define STATE_ON  0
#define STATE_OFF 1


namespace apak {

  namespace ups {

    const QList<quint16> badStates = {STATE_LINK_DOWN, STATE_DISABLED, STATE_NO_PORT  };
  }

  class UPSGlobalState: public modus::SvAbstractJob
  {

  public:
    UPSGlobalState();

    bool configure(modus::JobConfig* config) override;
    bool signalIsSupported(modus::SvSignal* signal) override;
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

    void start() override;

  private:
    apak::ups::ProtocolParams    m_params;
    modus::SvSignal*            m_global_state_signal = nullptr;

    QMap<modus::SvSignal*, apak::ups::SignalParams> m_params_by_signals;


  public slots:

    virtual void signalUpdated(modus::SvSignal* signal) override;
    virtual void signalChanged(modus::SvSignal* signal) override;

  private slots:
    void updateGlobalState();

  };


}

#endif // ZN_CONNECTION_STATE_H
