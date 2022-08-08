#ifndef MOXA_H
#define MOXA_H

#include <QMutex>
#include <QMutexLocker>

#include "moxa_global.h"

#include "../../../../../../Modus_Libs/APAK/global_apak_defs.h"
#include "../../../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../../../Modus/global/signal/sv_signal.h"

#include "params.h"

#include "../../../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../../../svlib/SvCRC/1.1/sv_crc.h"

extern "C" {

    MOXA_EXPORT modus::SvAbstractProtocol* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

const char REQUEST[8] = {0x01, 0x02, 0x00, 0x00, 0x00, 0x06, 0x01, 0x04};

struct MOXAReplay {
  quint16 id;       // Идентификатор транзакции
  quint16 proto;    // Идентификатор протокола	00	Идентификатор протокола
  quint16 msglen;   // Длина сообщения	00	Длина сообщения
  quint8  address;  // Адрес устройства	01	Адрес устройства
  quint8  func;     // Функциональный код	04	Функциональный код
  quint8  bytecnt;  // Адрес первого регистра Hi байт	04	Количество байт далее
};

namespace apak {

  class SvMoxa: public modus::SvAbstractProtocol
  {
    Q_OBJECT

  public:
    SvMoxa();
    ~SvMoxa();

    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:
    moxa::ProtocolParams  m_params;
    QMap<quint16, modus::SvSignal*> m_master_signals_by_registers;

  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;
    void start() override;

  private slots:
    void parse(modus::BUFF* buffer);
    void sendRequest();

  };
}


#endif // MOXA_H
