#ifndef RADUGA_GLOBAL_STATE_H
#define RADUGA_GLOBAL_STATE_H

#include <QSet>

#include "raduga_global_state_global.h"

#include "../../../../../Modus/global/job/sv_abstract_job.h"
#include "../../../../../Modus/global/signal/sv_signal.h"

#include "raduga_global_state_defs.h"
#include "supported_signal_tags.h"

extern "C" {

  RADUGA_GLOBAL_STATE_EXPORT modus::SvAbstractJob* create();

}

#define STATE_BAB_CONNECTION    0
#define STATE_BAD_AUTHORIZATION 1
#define STATE_BAD_RECORDING     2
#define STATE_OK                3

namespace apak {

  const QList<quint16> badStates = {STATE_BAB_CONNECTION, STATE_BAD_AUTHORIZATION, STATE_BAD_RECORDING  };

  class RadugaGlobalState: public modus::SvAbstractJob
  {

  public:
    RadugaGlobalState();

    bool configure(modus::JobConfig* config) override;
    bool signalIsSupported(modus::SvSignal* signal) override;
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

    void start() override;

  private:
    apak::rdga::ProtocolParams  m_params;
    modus::SvSignal*            m_global_state_signal = nullptr;

    QMap<modus::SvSignal*, apak::rdga::SignalParams> m_params_by_signals;


  public slots:

    virtual void signalUpdated(modus::SvSignal* signal) override;
    virtual void signalChanged(modus::SvSignal* signal) override;

  private slots:
    void updateGlobalState();

  };


}

#endif // RADUGA_GLOBAL_STATE_STATE_H
