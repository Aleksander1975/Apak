#ifndef IV1_IMITATOR_H
#define IV1_IMITATOR_H

#include <QMutex>
#include <QMutexLocker>

#include "iv1_imitator_global.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"

#include "protocol_params.h"

#include "../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../svlib/SvCRC/1.1/sv_crc.h"

extern "C" {

    IV1_IMITATOR_EXPORT modus::SvAbstractProtocol* create();

}

namespace apak {

  class SvIv1Imitator: public modus::SvAbstractProtocol
  // Класс, реализующий программный имитатор извещателя влажности ИВ-1
  {
    Q_OBJECT

  public:
    SvIv1Imitator();
    ~SvIv1Imitator();

    // Эта функция вызывается серевером "mdserver" для всех имитаторов устройств.
    // Её цель - инициализировать все структуры, необходимые нам для конкретного
    // имитатора (в данном случае, имитатора ИВ-1).
    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;

    // Эта функция вызывается сервером "mdserver" для всех сигналов имитатора устройства ИВ-1.
    // (Структура SignalBinding описывает привязку ОДНОГО сигнала к ОДНОМУ устройству).
    // Информация об этой привязке берётся из json-файлa сигналов для имитатора устройства ИВ-1 -
    // (файл: iv1.json).
    // Замечание: вообще, каждый сигнал может быть привязан к одному устройству, которое устанавливает его
    // значение (эти устройства в файле сигналов называются "master"), и к нескольким устройствам,
    // которые "читают" его значение (такие устройства называют "bindings").
    // В данном случае функция функция bindSignal() будет вызываться по одному разу для каждого из 42-ух
    // сигналов имитатора ИВ-1 (первый аргумент функции) и binding - привязке этого сигнала к
    // устройству ИВ-1 (второй аргумент).
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:

    // Cодержит параметр протокола обмена ИВ-1 с системой АПАК (период поступления данных в мс от ИВ-1 в систему АПАК).
    iv1::ProtocolParams  m_params;

    // Словарь, который каждому сигналу ставит в соответствие структуру SignalParams, хранящую параметры для этого сигнала.
    QMap<modus::SvSignal*, iv1::SignalParams> m_params_by_signal;

    // Словарь, который каждому порядковому номеру датчика ставит в соответствие сигнал, содержащий значение
    // температуры от данного датчика.
    QMap <uint8_t, modus::SvSignal*> m_temperature;

    // Словарь, который каждому порядковому номеру датчика ставит в соответствие сигнал, содержащий значение
    // влажности от данного датчика.
    QMap<uint8_t, modus::SvSignal*> m_moisture;

    // Массив данных для передачи от ИВ-1 в систему АПАК в соответствии с протоколом обмена.
    QByteArray m_send_data;

    // Таймер, по таймауту которого, мы посылаем данные от имитатора ИВ-1 в систему АПАК.
    QTimer* m_timer;

  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;

    // В этой функции мы осуществляем подключение вызова функции send() к наступлению таймаута таймера m_timer.
    void start() override;

  private slots:

    // В этой функции мы формируем и помещаем в массив байт m_send_data пакет для передачи от имитатора
    // устройства ИВ-1 в систему АПАК (в соответствии с протоколом обмена) и инициируем передачу
    // этого пакета от протокольной к интерфейcной части имитатора (для передачи по линии связи).
    void send();
  };
}


#endif // IV1_IMITATOR_H
