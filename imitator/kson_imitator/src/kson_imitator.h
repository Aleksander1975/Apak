#ifndef KSON_IMITATOR_H
#define KSON_IMITATOR_H

#include <QMutex>
#include <QMutexLocker>

#include "kson_imitator_global.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"

#include "protocol_params.h"

#include "../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../svlib/SvCRC/1.1/sv_crc.h"

extern "C" {

    KSON_IMITATOR_EXPORT modus::SvAbstractProtocol* create();

}

namespace apak {

  class SvKsonImitator: public modus::SvAbstractProtocol
  // Класс, реализующий программный имитатор сети КСОН.
  {
    Q_OBJECT

  public:
    SvKsonImitator();
    ~SvKsonImitator();

    // Эта функция вызывается серевером "mdserver" для всех имитаторов устройств.
    // Её цель - инициализировать все структуры, необходимые нам для конкретного
    // имитатора (в данном случае, имитатора сети КСОН).
    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;

    // Эта функция вызывается сервером "mdserver" для всех сигналов имитатора КСОН.
    // Её цель состоит в получении всех параметров для каждого сигнала и заполнинии в соответствии
    // с ними структур данных (словари "m_signal_by_byte", "m_params_by_signal",
    // "m_relevance_by_group"), необходимых для формирования кадра от сети КСОН к системе АПАК.
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:

    // Cодержит параметр протокола обмена сети КСОН с системой АПАК
    // (период поступления данных в мс от КСОН в систему АПАК).
    kson::ProtocolParams  m_params;

    // Словарь, который каждому смещению (в байтах) значения сигнала относительно начала информационного
    // блока, передаваемого из КСОН в АПАК, ставит в соответствие указатель на этот сигнал.
    QMap<uint8_t, modus::SvSignal*> m_signals_by_byte;

    // Список указателей на сигналы, отсортированный по возрастанию смещений
    // значений сигналов (в байтах) относительно начала информационного блока,
    // передаваемого из КСОН в АПАК.
    QList <modus::SvSignal*> m_signals;

    // Словарь, который каждому сигналу ставит в соответствие структуру SignalParams,
    // хранящую параметры для этого сигнала:
    QMap<modus::SvSignal*, kson::SignalParams> m_params_by_signal;

    // Битовый массив, который для каждого номера группы сигналов (номер - это индекс в этом массиве),
    // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны).
    // В этом массиве АКТУАЛЬНОСТЬ сигналов группы определяется, исходя из наличия сигналов с
    // параметром, указывающим на принадлежность к данной группе, в файле сигналов устройства КСОН.
    QBitArray m_relevance_by_group;

    // Битовый массив, который для каждого номера группы сигналов (номер - это индекс в этом массиве),
    // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны).
    // В этом массиве АКТУАЛЬНОСТЬ сигналов группы определяется, исходя из актуальности и соответствия
    // указанному в сигнале типу содержимого переменной "m_value" типа QVariant в классе сигнала.
    QBitArray m_relevanceConcrete_by_group;

    // Массив данных для передачи от КСОН в систему АПАК в соответствии с протоколом обмена.
    QByteArray m_send_data;

    // Таймер, по таймауту которого, мы посылаем данные от имитатора КСОН в систему АПАК.
    QTimer* m_timer;

  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;

    // В этой функции мы осуществляем подключение вызова функции send() к наступлению таймаута таймера m_timer.
    void start() override;

  private slots:

    // В этой функции мы формируем и помещаем в массив байт "m_send_data" пакет для передачи от имитатора
    // сети КСОН в систему АПАК (в соответствии с протоколом обмена) и инициируем передачу
    // этого пакета от протокольной к интерфейcной части имитатора (для передачи по линии связи).
    void send();
  };
}


#endif // KSON_IMITATOR_H
