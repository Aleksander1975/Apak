#ifndef MOXA_GLOBAL_STATE_H
#define MOXA_GLOBAL_STATE_H

#include <QSet>

#include "moxa_global_state_global.h"

#include "../../../../Modus/global/job/sv_abstract_job.h"
#include "../../../../Modus/global/signal/sv_signal.h"

#include "moxa_global_state_defs.h"
#include "supported_signal_tags.h"

extern "C" {

  MOXA_GLOBAL_STATE_EXPORT modus::SvAbstractJob* create();

}

#define STATE_LINK_DOWN 0
#define STATE_LINK_UP   1
#define STATE_DISABLED  2
#define STATE_NO_PORT   0xFFFF


namespace apak {

  namespace moxa {

    const QList<quint16> badStates = {STATE_LINK_DOWN, STATE_DISABLED, STATE_NO_PORT  };
  }

  class MoxaGlobalState: public modus::SvAbstractJob
  {

  public:
    MoxaGlobalState();

    bool configure(modus::JobConfig* config) override;
    bool signalIsSupported(modus::SvSignal* signal) override;
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

    void start() override;

  private:
    apak::moxa::ProtocolParams    m_params;
    modus::SvSignal*        m_global_state_signal = nullptr;

    QMap<modus::SvSignal*, apak::moxa::SignalParams> m_params_by_signals;


  public slots:

    virtual void signalUpdated(modus::SvSignal* signal) override;
    virtual void signalChanged(modus::SvSignal* signal) override;

  private slots:
    void updateGlobalState();

  };


}

#endif // ZN_CONNECTION_STATE_H
