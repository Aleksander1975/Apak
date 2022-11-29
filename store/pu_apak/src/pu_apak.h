#ifndef PU_APAK_H
#define PU_APAK_H


#include "pu_apak_global.h"

#include "protocol_params.h"
#include "signal_params.h"

#include "../../../global_apak_defs.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"
#include "../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../svlib/SvCRC/1.0/sv_crc.h"

extern "C" {

     PU_APAK_EXPORT modus::SvAbstractProtocol* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

namespace apak {


  class Sv_PU_APAK: public modus::SvAbstractProtocol
 // Класс, реализующий библиотеку АПАК для синхронизации сведений между ПУ_АПАК-1 и
 // ПУ_АПАК-2 о состоянии подключённых к ним устройств.
  {
    Q_OBJECT

  public:
    // Инициализация структур данных, необходимых для посылки РОST-запроса на
    // сервер другого ПУ АПАК:
    Sv_PU_APAK();
    ~Sv_PU_APAK();

    // Эта функция вызывается серевером "mdserver" для библиотек всех устройств.
    // Её цель - инициализировать все структуры, необходимые нам для работы
    // с конкретным устройстовом (в данном случае c устройством ПУ_АПАК).
    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;

    // Эта функция вызывается сервером "mdserver" для всех сигналов, связанных с данным ПУ АПАК.
    // Цель этой функции состоит в получении всех параметров для каждого сигнала и заполнении в соответствии
    // с ними структур данных (перечисленных в файле "pu_apak.h"), необходимых для формирования
    // POST-запроса к серверу другого ПУ АПАК.
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:

    // Cодержит параметры протокола обмена между ПУ АПАК-1 и ПУ АПАК-2
    // (эти параметры описаны в файле "protokol_param.h").
    apak::ProtocolParams  m_params;

    // ==== Структуры данных для обмена между АПАК ПУ:====
    // 1. Список указателей на сигналы, подлежащие передаче на другой ПУ АПАК:
    QList <modus::SvSignal*> m_signals;
    // 2. Cловарь, который каждому сигналу, подлежащему передаче на другой ПУ АПАК,
    //ставит в соответствие структуру SignalParams, хранящую параметры для этого сигнала:
    QMap <modus::SvSignal*, apak::SignalParams> m_params_by_signal;

    // "Таймер посылки", по таймауту которого, мы посылаем запрос POST к другому ПУ АПАК:
    QTimer* m_sendTimer;

    // Указатель на сигнал - "состояние ПУ АПАК":
    modus::SvSignal*      m_state_signal;

  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;

    // В этой функции мы
    // 1. Привязываем к наступлению таймаута таймера посылки "m_sendTimer"
    // вызова функции "sendResponse_POST". В функции "sendResponse_POST" мы устанавливаем в "1"
    // сигнал состояния ПУ АПАК и выполняем формирование и передачу
    // интерфейсной части запроса POST к другому ПУ АПАК.
    // 2. Привязываем вызов функции "getReply_POST", к сигналу "modus::IOBuffer::dataReaded".
    // Этот сигнал испускается интерфейсной частью по приходу данных от интерефейса
    // (в нашем случае - от сервера другого ПУ АПАК).
    // В функции "getReply_POST" мы выводим ответ сервера на посланный к нему POST-запрос.
    void start(void) override;

  private slots:
    // В этой функции мы:
    // 1. Устанавливаем в "1" сигнал состояния ПУ АПАК
    // 2. Выполняем формирование и передачу интерфейсной части запроса POST к другому ПУ АПАК.
    void sendResponse_POST();

    // Эта функция выводит на консоль и в утилиту "logview" ответ сервера другого ПУ АПАК
    // на посланный нами POST-запрос. Это делается для целей отладки и контроля работы.
    void getReply_POST(modus::BUFF* buffer);

    // Функция передаёт данные от протокольной к интерфейcной части (для передачи по линии связи).
    // Аргумент: "data" - массив байт для передачи
    void transferToInterface (QByteArray data);
   };
}

#endif // PU_APAK_H
