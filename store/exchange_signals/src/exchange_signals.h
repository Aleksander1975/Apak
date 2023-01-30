#ifndef EXCHANGE_SIGNALS_H
#define EXCHANGE_SIGNALS_H

#include <QDataStream>

#include "exchange_signals_global.h"

#include "frame_header.h"
#include "protocol_params.h"
//#include "signal_params.h"

#include "../../../global_apak_defs.h"

#include "../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../Modus/global/signal/sv_signal.h"
#include "../../../../svlib/SvAbstractLogger/1.2/sv_abstract_logger.h"
#include "../../../../svlib/SvException/1.1/sv_exception.h"
#include "../../../../svlib/SvCRC/1.0/sv_crc.h"

#define LIB_SHORT_INFO \
  "Модуль обмена сигналами (МОС) между двумя системами АПАК. Интерфейсная библиотека Modus. Версия " LIB_VERSION ".\n"

#define LIB_DESCRIPTION \
  LIB_SHORT_INFO \
    "1. С интервалом, заданным параметром " P_SEND_INTERVAL ", каждый из двух, участвующих в обмене " \
    "модулей МОС, посылает другому модулю пакет с информацией о сигналах, которые имеют привязку " \
    " \"binding\" к этому (посылающему пакет) модулю. Информация о каждом сигнале включает его " \
    "идентификатор, значение и время последнего обновления.\n" \
    "2. Получив такой пакет, модуль МОС, находящийся на другом АПАК, последовательно считывает " \
    "из него информацию обо всех находящихся в нём сигналах. Прочитав идентификатор сигнала, он проверяет, " \
    "привязан ли сигнал с таким идентификатором к нему, как к мастеру. Если привязан, то модуль МОС " \
    "устанавливает в этом сигнале поля \"значения сигнала\" и \"времени последнего обновления сигнала\" из " \
    "полученного пакета. Если не привязан, то модуль МОС выдаёт оператору в утилите \"logview\" сообщение " \
    "об ошибке. \n" \
    "Автор " LIB_AUTHOR

extern "C" {

     EXCHANGE_SIGNALS_EXPORT modus::SvAbstractProtocol* create();

     EXCHANGE_SIGNALS_EXPORT const char* getVersion();
     EXCHANGE_SIGNALS_EXPORT const char* getParams();
     EXCHANGE_SIGNALS_EXPORT const char* getInfo();
     EXCHANGE_SIGNALS_EXPORT const char* getDescription();
}

namespace apak {


  class SvExchangeSignals: public modus::SvAbstractProtocol
 // Класс, реализующий модуль МОС.
  {
    Q_OBJECT

  public:
      // Инициализация структур данных, необходимых для формирования пакета К другому МОС
      // и обработки пактов ОТ другого МОС:
    SvExchangeSignals();
    ~SvExchangeSignals();

    // Эта функция вызывается серевером "mdserver" для всех устройств.
    // Её цель - инициализировать все структуры, необходимые нам для сигналов конкретного
    // устройства (в данном случае, модуля обмена сигналами).
    bool configure(modus::DeviceConfig* config, modus::IOBuffer *iobuffer) override;

    // Эта функция вызывается сервером "mdserver" для всех сигналов, связанных с модулем обмена
    // сигналами. Сигналы, передаваемые ЭТИМ модулем обмена сигналами на ДРУГОЙ "mdserver", будут
    // иметь привязку к ЭТОМУ модулю - "binding". Сигналы, принимаемые от ДРУГОГО "mdserver"а,
    // будут иметь привязку к ЭТОМУ модулю - "master".

    // Цель этой функции состоит в получении всех параметров для каждого сигнала и заполнении
    // в соответствии с ними структур данных (перечисленных в файле "exchange_signals.h"),
    // необходимых для передачи информации о сигналах на другой "mdserver"ИЛИ
    // приёма этой информации от другого "mdserver"а.

    // НА ДАННЫЙ МОМЕНТ мы не используем никакие параметры сигналов. Однако в будущем они, возможно,
    // пригодятся (например, период, с которым сигналы передаются на другой МОС, может задаваться
    // различным для разных групп сигналов).
    bool bindSignal(modus::SvSignal* signal, modus::SignalBinding binding) override;

  private:

    // Структура, которая cодержит параметры протокола обмена между МОС:
    // (эти параметры описаны в файле "protokol_param.h").
    apak::ProtocolParams  m_params;

    // "Таймер посылки" - таймер, отсчитывающий период посылки пакетов
    // с информацией о сигналах для передачи на другой МОС:
    QTimer* m_sendTimer;

    // === СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ ПРИЁМА ИНФОРМАЦИИ О СИГНАЛАХ ОТ ДРУГОГО МОС ====:
    // 1В. Cловарь, который каждому идентификатору сигнала ставит в соответствие сигнал:
    QMap <int, modus::SvSignal*> m_signal_by_id;

    // 2В. Структура, описывающая заголовок пакета, ПРИНЯТЫЙ от другого МОС:
    apak::frameHeader m_receiveHeader;

    // 3В. Байтовый массив, в который помещается пакет сигналов, принятый от другого МОС:
    QByteArray m_receive_data;


    // === СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ ПЕРЕДАЧИ ИНФОРМАЦИИ О СИГНАЛАХ ДРУГОМУ МОС ====:
    // 1И. Cписок сигналов для передачи:
    QList <modus::SvSignal*> m_signals_to_transmit;

    // 2И. Структура, описывающая заголовок пакета, для ПЕРЕДАЧИ на другой МОС:
    apak::frameHeader m_sendHeader;

    // 3И. Байтовый массив, в котором формируется пакет со значениями
    // сигналов для передачи на другой МОС:
    QByteArray m_send_data;

    // 4. Размер буфера между протокольной и интерфейсной частью:
    uint m_send_maxLen;

  public slots:
    void signalUpdated(modus::SvSignal* signal) override;
    void signalChanged(modus::SvSignal* signal) override;

    // В этой функции мы:
    // 1. Осуществляем привязку вызова функции "sendSignals", в которой мы
    // формируем пакет с информацией о значениях сигналов к другому МОС, к наступлению таймаута
    // таймера посылки "m_sendTimer".
    // 2. Запускаем таймер посылки "m_sendTimer" с периодом,
    // равным интервалу посылки пакетов, заданному  в конфигурационном файле
    // "config_apak.json", как параметр протокола модуля МОС.
    // 3. Привязываем вызов функции "receiveSignals" к сигналу "modus::IOBuffer::dataReaded".
    // Этот сигнал испускается интерфейсной частью по приходу данных от интерефейса.
    // В этой функции мы обрабатываем принятый от другого МОС пакет сигналов.
    void start(void) override;

  private slots:

    // В этой функции мы:
    // 1. Формируем и помещаем в массив байт "m_send_data" пакет со значениями сигналов
    // для передачи на другой МОС.
    // 2. Инициируем передачу этого пакета от протокольной к интерфейcной части
    // (для передачи по линии связи).
    void sendSignals();

    // Обрабатываем пакет с информацией о сигналах, принятый от другого модуля МОС:
    void receiveSignals(modus::BUFF* buffer);

    // Функция передаёт данные от протокольной к интерфейcной части (для передачи по линии связи).
    // Аргумент: "data" - массив байтов для передачи
    void transferToInterface (QByteArray data);
  };
}

// === ОБЪЯВЛЕНИЕ ФУНКЦИЙ ДЛЯ СЕРИАЛИЗАЦИИ / ДЕСЕРИАЛИЗАЦИИ СИГНАЛОВ: ===

// Сериализация необходимой для передачи на другой "mdserver" информации из класса "SvSignal".
// Функция расширяемая: при необходимости список передаваемых полей сигнала может быть
// легко откорректирован.
inline QDataStream& operator<< (QDataStream &stream, const modus::SvSignal &signal);

// Сериализация необходимой для передачи на другой "mdserver" информации
// из структуры "SignalConfig". Функция расширяемая: при необходимости список
// передаваемых полей сигнала может быть легко откорректирован.
inline QDataStream& operator<< (QDataStream &stream, const modus::SignalConfig &signalCfg);

// Десериализация информации из класса "SvSignal", принятой от другого "mdserver"а.
// Функция расширяемая: при необходимости список принимаемых полей класса
// "SvSignal" может быть легко откорректирован.
inline QDataStream& operator>> (QDataStream &stream, QMap <int, modus::SvSignal*> signals_by_id);

// Десериализация информации из структуры "SignalConfig", принятой от другого "mdserver"а.
// Функция расширяемая: при необходимости список принимаемых полей структуры
// "SignalConfig" может быть легко откорректирован.
inline QDataStream& operator>> (QDataStream &stream, modus::SignalConfig& signalCfg);


#endif // EXCHANGE_SIGNALS_H
