#include "exchange_signals_debug.h"


apak::SvExchangeSignalDebug::SvExchangeSignalDebug():
  modus::SvAbstractProtocol()
// Очистка структур данных, необходимых для:
// - установки типов и значений сигналов, передаваемых модулем МОС на другой "mdserver",
// - отображения оператору типов и значений сигналов, принятых МОС от другого "mdserver"a.
{
    // === ОЧИСТИМ СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ УСТАНОВКИ ТИПОВ И ЗНАЧЕНИЙ ===
    //           === СИГНАЛОВ, ПЕРЕДАВАЕМЫХ МОС НА ДРУГОЙ "mdserver" ====:
    // Очистим список указателей на сигналы, подлежащие передаче:
    m_signals_to_send.clear();

    // Очистим словарь, который каждому указателю на сигнал (КЛЮЧ словаря) ставит в
    // соответствие структуру "SignalParams", хранящую параметры для этого
    // сигнала(ЗНАЧЕНИЕ словаря):
    m_params_by_signal.clear();

    // ==== ОЧИСТИМ СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ ОТОБРАЖЕНИЯ ОПЕРАТОРУ ТИПОВ и ЗНАЧЕНИЙ ===
    //            == СИГНАЛОВ, ПРИНЯТЫХ МОС ОТ ДРУГОГО "mdserver"a ====:
    // Очистим список указателей на сигналы, подлежащие приёму:
    m_signals_to_receive.clear();
}


apak::SvExchangeSignalDebug::~SvExchangeSignalDebug()
{
  deleteLater();
}


bool apak::SvExchangeSignalDebug::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
// Эта функция вызывается серевером "mdserver" для всех устройств.
// Её цель - инициализировать все структуры, необходимые нам для сигналов конкретного
// устройства (в данном случае, модуля МП МОС).
{
    try
    {
        p_config = config;
        p_io_buffer = iobuffer;

        // Заполняем структуру "m_params" параметрами модуля МП МОС (периодом установки
        // значений сигналов, подлежащих передаче через МОС и периодом чтения значений
        // и параметров сигналов, принятых через МОС):
        m_params = apak::ProtocolParams::fromJson(p_config->protocol.params);

        return true;
    }
    catch (SvException& e)
    {
        // Отображаем оператору сообщение о месте ошибке. Сообщение о самой ошибке
        // хранится в исключении (e.error) и будет передано в "mdserver" через "p_last_error":
        emit message(QString("МП МОС: Исключение в функции \"configure\""), sv::log::llError, sv::log::mtError);
        qDebug() << "МП МОС: Исключение в функции \"configure\"";

        p_last_error = e.error;
        return false;
    }
}


bool apak::SvExchangeSignalDebug::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
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
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);


    if(r)
    {
        if(binding.mode == modus::Master)
        { // Сигналы, передаваемые на другой "mdserver":

            // Добавляем указатель на сигнал в список "m_signals_to_send":
            m_signals_to_send.append(signal);

            // Параметры сигналов описываются в подразделе "params", следующем за номером устройства,
            // к которому они привязаны.
            apak::SignalParams signal_params = apak::SignalParams::fromJson(binding.params);

            // Вставляем в словарь "m_params_by_signal", пару: КЛЮЧ - указатель на сигнал,
            // ЗНАЧЕНИЕ - структура "SignalParams", хранящая параметры для этого сигнала:
            m_params_by_signal.insert (signal, signal_params);
      } // if(binding.mode == modus::Master)
      else
      { // Сигналы, поступающие от другого "mdserver"a - не имеют параметров.

            // Добавляем указатель на сигнал в список "m_signals_to_receive":
            m_signals_to_receive.append(signal);
      }
    } // if(r)

    return r;
  }

  catch(SvException& e) {
    p_last_error = e.error;

    //Получаем имя сигнала:
    QString signalName = signal ->config() ->name;

    // Отображаем оператору сообщение о месте ошибке. Сообщение о самой ошибке
    // хранится в исключении (e.error) и будет передано в "mdserver" через "p_last_error":
    emit message(QString("МП МОС: Исключение в функции \"bindSignal\" на сигнале: %1").arg(signalName), sv::log::llError, sv::log::mtError);
    qDebug() << QString ("МП МОС: Исключение в функции \"bindSignal\" на сигнале %1").arg(signalName);

    return false;
  }
}

void apak::SvExchangeSignalDebug::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvExchangeSignalDebug::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvExchangeSignalDebug::start(void)
// В этой функции мы осуществляем:
// 1. Привязываем вызов функции "setSignals" к наступлению таймаута
// таймера установки "m_setSignalTimer". В этой функции мы будем устанавливать типы и значения
// сигналов, которые подлежат передаче модулем МОС.
// 2. Запускаем таймер установки "m_setSignalTimer".
// 3. Привязываем вызов функции "readSignals" к наступлению таймаута
// таймера чтения "m_readSignalTimer". В этой функции мы будем считывать и выводить оператору
// типы и значения сигналов, которые принимаются модулем МОС.
// 4. Запускаем таймер чтения "m_readSignalTimer".
{
    // 1. Привязываем вызов функции "setSignals" к наступлению таймаута
    // таймера установки "m_setSignalTimer". В этой функции мы будем устанавливать типы и значения
    // сигналов, которые подлежат передаче модулем МОС.
    m_setSignalTimer = new QTimer;
    connect(m_setSignalTimer, &QTimer::timeout, this, &SvExchangeSignalDebug::setSignals);

    // 2. Запускаем таймер установки "m_setTimer".
    m_setSignalTimer->start(m_params.set_signal_interval);

    // 3. Привязываем вызов функции "readSignals" к наступлению таймаута
    // таймера чтения "m_readSignalTimer". В этой функции мы будем считывать и выводить оператору
    // типы и значения сигналов, которые принимаются модулем МОС.
    m_readSignalTimer = new QTimer;
    connect(m_readSignalTimer, &QTimer::timeout, this, &SvExchangeSignalDebug::readSignals);

    // 4. Запускаем таймер чтения "m_readTimer".
    m_readSignalTimer->start(m_params.read_signal_interval);

    p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvExchangeSignalDebug::setSignals(void)
// Мы устанавливаем типы и значения сигналов, которые подлежат передаче
// модулем МОС. Эти сигналы в конфигурационном файле модуля проверки модуля
// МОС (МП МОС) имеют привязку "master". Значения и типы сигналов указаны в
// этом файле, как параметры сигналов.
{
    // qDebug() << "МП МОС: Установка значений и типов сигналов: Вызов setSignals()";

    if (p_is_active == false)
        return;

    // В цикле проходим по списку сигналов "m_signals_to_send", предназначенных для передачи.
    // Для каждого сигнала из словаря "m_params_by_signal" получаем его параметры:
    // тип и значение. Устанавливаем их.
    foreach (modus::SvSignal * signal, m_signals_to_send)
    {
        // Получаем имя сигнала:
        QString signalName = signal ->config() ->name;

        // Получаем параметры сигнала:
        if (m_params_by_signal.contains(signal) == false)
        {
            // Выводим оператору сообщение об ошибке:
            emit message(QString("МП МОС: Для передаваемого через МОС сигнала: %1 нет записи в словаре \"m_params_by_signal\"").arg(signalName), sv::log::llError, sv::log::mtError);
            qDebug() << QString("МП МОС: Для передаваемого через МОС сигнала: %1 нет записи в словаре \"m_params_by_signal\"").arg(signalName);
        }

        SignalParams signal_params = m_params_by_signal.value (signal);

        // Подготавливаем значение сигнала:
        QVariant signalValue = signal_params.data_value.toVariant();

        // Подготавливаем тип сигнала:
        if (signalValue.convert(signal_params.data_type) == false)
        {
            // Выводим оператору сообщение об ошибке:
            emit message(QString("МП МОС: Для передаваемого через МОС сигнала: %1 невозможно привести значение сигнала: %2 к указанному типу: %3").arg(signalName).arg(signal_params.data_value.toString()).arg(signal_params.data_type), sv::log::llError, sv::log::mtError);
            qDebug() << QString("МП МОС: Для передаваемого через МОС сигнала: %1 невозможно привести значение сигнала: %2 к указанному типу: %3").arg(signalName).arg(signal_params.data_value.toString()).arg(signal_params.data_type);
        }

        // Устанавливаем значение сигнала:
        signal->setValue(signalValue);

        // Определяем имя типа сигнала:
        const char *signalType = QVariant::typeToName(signal_params.data_type);
        QString signalTypeString = QString (signalType);

        // Выводим оператору сообщение об установленных типе и значении сигнала:
        emit message(QString("МП МОС: Передаваемый черес МОС сигнал: %1 типа: %2 имеет значение: %3").arg(signalName).arg(signalTypeString).arg(signalValue.toString()), sv::log::llInfo, sv::log::mtInfo);
        qDebug() << QString("МП МОС: Передаваемый черес МОС сигнал: %1 типа: %2 имеет значение: %3").arg(signalName).arg(signalTypeString).arg(signalValue.toString());
    } // foreach

    return;
}


void apak::SvExchangeSignalDebug::readSignals(void)
// Мы читаем типы и значения сигналов, принятых модулем МОС. Эти сигналы в
// конфигурационном файле модуля проверки модуля МОС (МП МОС) имеют привязку "binding".
{
    //qDebug() << "МП МОС: Чтение значений и типов сигналов: Вызов readSignals()";

    if(p_is_active == false)
        return;

    // Проходим по списку сигналов "m_signals_to_receive", принимаемых модулем МОС.
    // Для каждого сигнала отображаем оператору его тип и значение.
    foreach (modus::SvSignal * signal, m_signals_to_receive)
    {
        // Получаем имя сигнала:
        QString signalName = signal ->config() ->name;

        // Получаем значение сигнала:
        QVariant signalValue = signal ->value();

        // Получаем время последнего обновления сигнала:
        QDateTime signalLastUpdate = signal->lastUpdate();

        // Проверяем валидно ли значение сигнала:
        if( signalValue.isValid() == false)
        {
            // Выводим оператору сообщение о невалидности значения сигнала:
            emit message(QString("МП МОС: Cигнал: %1 имеет НЕВАЛИДНОЕ значение").arg(signalName), sv::log::llInfo, sv::log::mtInfo);
            qDebug() << QString("МП МОС: Cигнал: %1 имеет НЕВАЛИДНОЕ значение").arg(signalName);

            continue;
        }

        // Проверяем, что значение сигнала NULL:
        if ( signalValue.isNull() == true)
        {
            // Выводим оператору сообщение о том, что значение сигнала NULL:
            emit message(QString("МП МОС: Cигнал: %1 имеет значение NULL").arg(signalName), sv::log::llInfo, sv::log::mtInfo);
            qDebug() << QString("МП МОС: Cигнал: %1 имеет значение NULL").arg(signalName);

            continue;
        }

        // Получаем тип сигнала:
        const char * signalType = signalValue.typeName();
        QString signalTypeString = QString (signalType);

        // Выводим оператору сообщение о типе, значении и времени последнего обновления сигнала:
        emit message(QString("МП МОС: Полученный черес МОС сигнал: %1 типа: %2 имеет значение: %3, время последнего обновления: %4").arg(signalName).arg(signalTypeString).arg(signalValue.toString()).arg(signalLastUpdate.toString(Qt::ISODate)), sv::log::llInfo, sv::log::mtInfo);
        qDebug() << QString("МП МОС: Полученный черес МОС сигнал: %1 типа: %2 имеет значение: %3,  время последнего обновления: %4").arg(signalName).arg(signalTypeString).arg(signalValue.toString()).arg(signalLastUpdate.toString(Qt::ISODate));
    } // foreach

    return;
}


/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvExchangeSignalDebug();
  return protocol;
}

