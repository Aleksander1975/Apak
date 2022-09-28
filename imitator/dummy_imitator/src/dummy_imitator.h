#ifndef DUMMY_IMITATOR_H
#define DUMMY_IMITATOR_H

#include <QMutex>
#include <QMutexLocker>

#include "dummy_imitator_global.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"

#include "protocol_params.h"

#include "../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../svlib/SvCRC/1.1/sv_crc.h"

extern "C" {

    DUMMY_IMITATOR_EXPORT modus::SvAbstractProtocol* create();

}

namespace apak {

  class SvDummyImitator: public modus::SvAbstractProtocol
  // Класс, реализующий программную "заглушку" для отладки библиотек, реализующих TCP-интерфейс
  {
    Q_OBJECT

  public:
    SvDummyImitator();
    ~SvDummyImitator();

    // Эта функция вызывается серевером "mdserver" для всех имитаторов устройств.
    // Её цель - инициализировать все структуры, необходимые нам для конкретного
    // имитатора (для "заглушки" единственная структура данных - это номер посылки "m_transferNumber").
    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;

    // Эта функция вызывается сервером "mdserver" для всех сигналов "заглушки".
    // Так как у "заглушки" нет сигналов, то эта функция вызыватья не будет.
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:
    // Номер посылки: (каждый байт посылки будет содержать этот номер. Изменяется в цикле от 0 до 255).
    uchar m_transferNumber;

    // Cодержит параметр протокола обмена "заглушки" с системой АПАК (период поступления данных в мс от "заглушки" в систему АПАК).
    Dummy::ProtocolParams  m_params;


    // Массив данных для передачи от "заглушки" в систему АПАК в соответствии с протоколом обмена.
    QByteArray m_send_data;

    // Таймер, по таймауту которого, мы посылаем данные от "заглушки" в систему АПАК.
    QTimer* m_timer;

  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;

    // В этой функции мы осуществляем подключение вызова функции send() к наступлению таймаута таймера m_timer.
    void start() override;

  private slots:

    // В этой функции мы формируем и помещаем в массив байт m_send_data пакет для передачи от
    // заглушки в систему АПАК (в соответствии с протоколом обмена) и инициируем передачу
    // этого пакета от протокольной к интерфейcной части имитатора (для передачи по линии связи).
    void send();
  };
}


#endif // DUMMY_IMITATOR_H
