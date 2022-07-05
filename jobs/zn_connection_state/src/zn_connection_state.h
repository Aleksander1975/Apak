#ifndef ZN_CONNECTION_STATE_H
#define ZN_CONNECTION_STATE_H

#include <QSet>

#include "zn_connection_state_global.h"

#include "../../../../Modus/global/job/sv_abstract_job.h"
#include "../../../../Modus/global/signal/sv_signal.h"

#include "zn_connection_state_defs.h"
#include "supported_signal_tags.h"

extern "C" {

  ZN_CONNECTION_STATE_EXPORT modus::SvAbstractJob* create();

}

namespace zncs {


  const QSet<QString> SupportedSignalTags = {
    TAG_ZN_State_data1,
    TAG_ZN_State_data11,
    TAG_101_DataType_1,
    TAG_105_DataType_1
  };


  class ZnConnectionState: public modus::SvAbstractJob
  {

  public:
    ZnConnectionState();

    bool configure(modus::JobConfig* config) override;
    bool signalIsSupported(modus::SvSignal* signal) override;

    void start() override;

  private:
    zncs::Params       m_params;

  public slots:

    virtual void signalUpdated(modus::SvSignal* signal) override;
    virtual void signalChanged(modus::SvSignal* signal) override;

  };


}

#endif // ZN_CONNECTION_STATE_H
