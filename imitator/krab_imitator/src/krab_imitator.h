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
#include "../../../../svlib/SvCRC/1.1/sv_crc.h"

extern "C" {

    KRAB_IMITATOR_EXPORT modus::SvAbstractProtocol* create();

}

namespace apak {

  class SvKrabImitator: public modus::SvAbstractProtocol
  // Класс, реализующий программный имитатор изделия КРАБ
  {
    Q_OBJECT

  public:
    SvKrabImitator();
    ~SvKrabImitator();

    // Эта функция вызывается серевером "mdserver" для всех имитаторов устройств.
    // Её цель - инициализировать все структуры, необходимые нам для конкретного
    // имитатора (в данном случае, имитатора КРАБ).
    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;

    // Эта функция вызывается сервером "mdserver" для всех сигналов имитатора КРАБ.
    // Её цель состоит в получении всех параметров для каждого сигнала и заполнинии в соответствии
    // с ними структур данных (словари m_signal_by_bitNumberInDataField и
    // m_bitNumberInSignal_by_bitNumberInDataField), необходимых для формирования пакета
    // запроса на запись от КРАБ'а к АПАК.
    bool bindSignal(modus::SvSignal *signal, modus::SignalBinding binding) override;


  private:

    // Cодержит параметр протокола обмена КРАБ с системой АПАК (период поступления
    // данных в мс от КРАБ в систему АПАК).
    krab::ProtocolParams m_params;

    // Битовый массив размером m_params.data_len*8 битов, в котором формируется "поле данных"
    //(длиной m_params.data_len байтов) запроса на запись от КРАБа к АПАК:
    QBitArray m_bitsDataField;

    // Байтовый массив размером m_params.data_len байтов, в котором хранится "поле данных"
    //(длиной m_params.data_len байтов) запроса на запись от КРАБа к АПАК:
    QByteArray m_byteDataField;

    // Пакет запроса на запись для передачи от КРАБ в систему АПАК в соответствии с протоколом обмена:
    QByteArray m_send_data;

    // Словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsDataField" ставит в соответствие сигнал, бит которого должен быть записан
    // в массив "m_bitsDataField":
    QMap<uint16_t, modus::SvSignal*> m_signal_by_bitNumberInDataField;

    // Словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsDataField", ставит в соответствие номер бита сигнала, который
    // должен быть записан в массив "m_bitsDataField":
    QMap<uint16_t, uint8_t> m_bitNumberInSignal_by_bitNumberInDataField;

    // Таймер, по таймауту которого, мы посылаем данные от имитатора КРАБ в систему АПАК.
    QTimer* m_timer;

   public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;

    // В этой функции мы осуществляем подключение вызова функции send() к наступлению таймаута таймера m_timer.
    void start() override;

  private slots:

    // В этой функции мы формируем и помещаем в массив байт m_send_data пакет для передачи от имитатора
    // устройства КРАБ в систему АПАК (в соответствии с протоколом обмена) и инициируем передачу
    // этого пакета от протокольной к интерфейcной части имитатора (для передачи по линии связи).
    void send();
  };
}


#endif // KRAB_IMITATOR_H

