#ifndef GAMMA_IMITATOR_H
#define GAMMA_IMITATOR_H

#include <QMutex>
#include <QMutexLocker>

#include "gamma_imitator_global.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"

#include "protocol_params.h"

#include "../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../svlib/SvCRC/1.0/sv_crc.h"

extern "C" {

    GAMMA_IMITATOR_EXPORT modus::SvAbstractProtocol* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

namespace apak {

  class SvGammaImitator: public modus::SvAbstractProtocol
  {
    Q_OBJECT

  public:
    SvGammaImitator();
    ~SvGammaImitator();

    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:
    apak::ProtocolParams  m_params;

    modus::SvSignal*      m_send_signal;

    QByteArray send_data;

    QTimer* m_timer;
    QSemaphore sem;

  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;
    void start() override;

  private slots:
    void send();

  };
}


#endif // GAMMA_IMITATOR_H
