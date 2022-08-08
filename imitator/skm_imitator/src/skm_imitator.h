#ifndef SKM_IMITATOR_H
#define SKM_IMITATOR_H

#include <QMutex>
#include <QMutexLocker>

#include "skm_imitator_global.h"
#include "protocol_params.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"

#include "../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../svlib/SvCRC/1.1/sv_crc.h"

extern "C" {

    SKM_IMITATOR_EXPORT modus::SvAbstractProtocol* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

namespace apak {

  class SvSkmImitator: public modus::SvAbstractProtocol
  {
    Q_OBJECT

  public:
    SvSkmImitator();
    ~SvSkmImitator();

    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:
    skm::ProtocolParams  m_params;

    QMap<quint8, QList<modus::SvSignal*>>     m_signals_by_vin;
    QMap<modus::SvSignal*, skm::SignalParams> m_params_by_signal;

    QByteArray send_data;

    QTimer* m_timer;

  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;
    void start() override;

  private slots:
    void send();

  };
}


#endif // SKM_IMITATOR_H
