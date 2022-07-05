#ifndef KRAB_IMITATOR_H
#define KRAB_IMITATOR_H

#include <QMutex>
#include <QMutexLocker>

#include "krab_imitator_global.h"

#include "../../../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../../../Modus/global/signal/sv_signal.h"

#include "params.h"

#include "../../../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../../../svlib/SvCRC/1.0/sv_crc.h"

extern "C" {

    KRAB_IMITATOR_EXPORT modus::SvAbstractProtocol* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

namespace apak {

  class SvKrabImitator: public modus::SvAbstractProtocol
  {
    Q_OBJECT

  public:
    SvKrabImitator();
    ~SvKrabImitator();

    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;

    bool bindSignal(modus::SvSignal *signal, modus::SignalBinding binding) override;

  public slots:
    void start() override;

    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;

  private:
    hmi::ProtocolParams                       m_params;

    QMap<modus::SvSignal*, hmi::SignalParams> m_input_signals;
    QMap<modus::SvSignal*, hmi::SignalParams> m_params_by_signals;
//    QMap<quint8, modus::SvSignal*> m_signals_by_byte_number;

    std::map<uint8_t, modus::SvSignal*> m_signals_by_byte_number;

    quint16 m_max_byte;

  private slots:
    void putout();

  };
}


#endif // UNIVERSAL_PACK_H
