#ifndef KRAB_IMITATOR_H
#define KRAB_IMITATOR_H

#include <QMutex>
#include <QMutexLocker>
#include <QBitArray>

#include "krab_imitator_global.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"

#include "protocol_params.h"

#include "../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../svlib/SvCRC/1.0/sv_crc.h"

extern "C" {

    KRAB_IMITATOR_EXPORT modus::SvAbstractProtocol* create();


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


  private:
    krab::ProtocolParams m_params;

    // Битовый массив размером 66*8 битов, в котором формируется "поле данных"
    //(длиной 66 байтов) запроса на запись от КРАБа к АПАК:
    QBitArray m_bitsPacket;

    // Словарь, который каждому номеру бита (от 0 до 66*8-1) из битового массива
    // "m_bitsPacket" ставит в соответствие сигнал, бит которого должен быть записан
    // в массив "m_bitsPacket":
    QMap<uint8_t, modus::SvSignal*> m_signal_by_bitNumberInPacket;

    // Словарь, который каждому номеру бита (от 0 до 66*8-1) из битового массива
    // "m_bitsPacket" ставит в соответствие номер бита сигнала, который
    // должен быть записан в массив "m_bitsPacket":
    QMap<uint8_t, uint8_t> m_bitNumberInSignal_by_bitNumberInPacket;

    // Переменная, которая содержит количество байт в "поле данных" запроса на запись
    // от Краба к АПАК. В данной версии протокола это значение - 66 байт. Предполагается внести
    // этот параметр в конфигурационный json-файл для сервера, в раздел:
    // "devices" -> "KRAB" -> "protokol" / "interface" -> "params".
    quint16 m_max_byte;

    // Таймер, по таймауту которого, мы посылаем данные от имитатора КРАБ в систему АПАК.
       QTimer* m_timer;

  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;

    // В этой функции мы осуществляем подключение вызова функции send() к наступлению таймаута таймера m_timer.
    void start() override;

  private slots:
    void send();
  };
}


#endif // KRAB_IMITATOR_H

