#ifndef EXCHANGE_SIGNALS_DEBUG_H
#define EXCHANGE_SIGNALS_DEBUG_H


#include "exchange_signals_debug_global.h"

#include "protocol_params.h"
#include "signal_params.h"

#include "../../../global_apak_defs.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"
#include "../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../svlib/SvCRC/1.0/sv_crc.h"

extern "C" {

     EXCHANGE_SIGNALS_DEBUG_EXPORT modus::SvAbstractProtocol* create();

//    VIRTUAL_DEVICESHARED_EXPORT QString defaultDeviceParams();
//    VIRTUAL_DEVICESHARED_EXPORT QString defaultIfcParams(const QString& ifc);
//    VIRTUAL_DEVICESHARED_EXPORT QList<QString> availableInterfaces();

}

namespace apak {


  class SvExchangeSignalDebug: public modus::SvAbstractProtocol
 // Класс, реализующий модуль библиотеки АПАК для отладки и проверки модуля МОС (МП МОС).
 // Модуль позволяет устанавливать значения и ТИПЫ сигналов.
  {
    Q_OBJECT

  public:
    // Очистка структур данных, необходимых для:
    // - установки типов и значений сигналов, передаваемых модулем МОС на другой "mdserver",
    // - отображения оператору типов и значений сигналов, принятых МОС от другого "mdserver"a.
    SvExchangeSignalDebug();
    ~SvExchangeSignalDebug();

    // Эта функция вызывается серевером "mdserver" для всех устройств.
    // Её цель - инициализировать все структуры, необходимые нам для сигналов конкретного
    // устройства (в данном случае, модуля МП МОС).
    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;

    // Эта функция вызывается сервером "mdserver" для всех сигналов, связанных с модулем МП МОС.
    //
    // Эта функция вызывается сервером "mdserver" для всех сигналов, связанных с модулем МП МОС.
    // Сигналы, передаваемые соответствующим ему МОС на ДРУГОЙ "mdserver" будут
    // иметь привязку к его модулю МОС - "binding", а к нему (МП МОС) - "master".
    // Сигналы, принимаемые соответствующим ему МОС от ДРУГОГО "mdserver"а будут
    // иметь привязку к его модулю МОС - "master", а к нему (МП МОС) - "binding".

    // Цель этой функции состоит в получении всех параметров для каждого сигнала и заполнении
    // в соответствии с ними структур данных (перечисленных в файле "exchange_signals_debug.h"),
    // необходимых для установки типов и значений передаваемых на другой "mdserver" сигналов и
    // отображения типов и значений принимаемых от другого "mdserver"а сигналов.

    // Параметры (для модуля МП МОС) передаваемых на другой "mdserver" сигналов: их тип и значение.
    // Сигналы, принимаемые с другого "mdserver"а никаких параметров (для модуля МП МОС) не имеют.
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:

    // Cодержит параметры модуля МП МОС
    // (эти параметры описаны в файле "protokol_param.h").
    apak::ProtocolParams  m_params;

    // "Таймер установки сигналов" - таймер, отсчитывающий период установки значений,
    // сигналов, подлежащих передаче:
    QTimer* m_setSignalTimer;

    // "Таймер чтения сигналов" - таймер, отсчитывающий период чтения значений
    // и типов принятых сигналов:
    QTimer* m_readSignalTimer;

    // === СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ УСТАНОВКИ ТИПОВ И ЗНАЧЕНИЙ ===
    //           === СИГНАЛОВ, ПЕРЕДАВАЕМЫХ МОС НА ДРУГОЙ "mdserver" ====:
    // 1И. Список указателей на сигналы, подлежащие передаче:
    QList<modus::SvSignal*> m_signals_to_send;

    // 2И. Словарь, который каждому указателю на сигнал (КЛЮЧ словаря) ставит в
    //  соответствие структуру "SignalParams", хранящую параметры для этого
    //  сигнала(ЗНАЧЕНИЕ словаря):
    QMap<modus::SvSignal*, apak::SignalParams> m_params_by_signal;


    // ==== СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ ОТОБРАЖЕНИЯ ОПЕРАТОРУ ТИПОВ и ЗНАЧЕНИЙ ===
    //            == СИГНАЛОВ, ПРИНЯТЫХ МОС ОТ ДРУГОГО "mdserver"a ====:
    // 1И. Список указателей на сигналы, подлежащие приёму:
    QList<modus::SvSignal*> m_signals_to_receive;


  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;

    // В этой функции мы осуществляем:
    // 1. Привязываем вызов функции "setSignals" к наступлению таймаута
    // таймера установки "m_setSignalTimer". В этой функции мы будем устанавливать типы и значения
    // сигналов, которые подлежат передаче модулем МОС.
    // 2. Запускаем таймер установки "m_setSignalTimer".
    // 3. Привязываем вызов функции "readSignals" к наступлению таймаута
    // таймера чтения "m_readSignalTimer". В этой функции мы будем считывать и выводить оператору
    // типы и значения сигналов, которые принимаются модулем МОС.
    // 4. Запускаем таймер чтения "m_readSignalTimer".
    void start(void) override;

  private slots:
    // Мы устанавливаем типы и значения сигналов, которые подлежат передаче
    // модулем МОС. Эти сигналы в конфигурационном файле модуля проверки модуля
    // МОС (МП МОС) имеют привязку "master". Значения и типы сигналов указаны в
    // этом файле, как параметры сигналов.
    void setSignals(void);

    // Мы читаем типы и значения сигналов, принятых модулем МОС. Эти сигналы в
    // конфигурационном файле модуля проверки модуля МОС (МП МОС) имеют привязку "binding".
    void readSignals(void);
  };
}

#endif // EXCHANGE_SIGNALS_DEBUG_H
