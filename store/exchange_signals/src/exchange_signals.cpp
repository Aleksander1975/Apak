#include "exchange_signals.h"


inline QDataStream& operator<< (QDataStream &stream, const modus::SvSignal &signal)
// Сериализация необходимой для передачи на другой "mdserver" информации из класса "SvSignal".
// Функция расширяемая: при необходимости список передаваемых полей сигнала может быть
// легко откорректирован.
{
    // Сохраняем необходимые поля из структуры "m_config":
    stream << *signal.config();

    // Сохраняем время последнего обновления сигнала:
    stream << signal.lastUpdate();

    // Сохраняем значение сигнала:
    stream << signal.value();

    return stream;
}


inline QDataStream& operator<< (QDataStream &stream, const modus::SignalConfig &signalCfg)
// Сериализация необходимой для передачи на другой "mdserver" информации
// из структуры "SignalConfig". Функция расширяемая: при необходимости список
// передаваемых полей сигнала может быть легко откорректирован.
{
    // Сохраняем идентификатор сигнала:
    stream << signalCfg.id;

    return stream;
}


inline QDataStream& operator>> (QDataStream &stream, QMap <int, modus::SvSignal*> signals_by_id)
// Десериализация информации из класса "SvSignal", принятой от другого "mdserver"а.
// Функция расширяемая: при необходимости список принимаемых полей класса
// "SvSignal" может быть легко откорректирован.
{
    // Переменная для чтения из потока "stream" конфигурации сигнала:
    modus::SignalConfig signalCfg;

    // Переменная для чтения из потока "stream" значения сигнала:
    QVariant value;

    // Переменная для чтения из потока "stream" времени последнего обновления сигнала:
    QDateTime dataTime;

    // Читаем поля структуры "SignalConfig":
    stream >> signalCfg;

    // Проверяем, что в словаре "signals_by_id" существует сигнал
    // с принятым в структуре "SignalConfig" идентификатором:
    if (signals_by_id.contains(signalCfg.id) == false)
    { // Если такого сигнала нет ->

        // - Сдвинем указатель чтения в потоке "stream", "прочтя" все данные, относящиеся
        //   к отсутствующему в словаре "signals_by_id" сигналу:

        // Читаем время последнего обновления сигнала:
        stream >> dataTime;

        // Читаем значение сигнала:
        stream >> value;

        // - Выбрасываем исключение с сообщением об ошибке:
        throw SvException(QString("МОС: Принятый сигнал с идентификатором %1 не привязан к МОС").arg(signalCfg.id));
    }

    // По идентификатору сигнала, содержащемуся в структуре "SignalConfig", мы из словаря
    // "signals_by_id" получаем указатель на этот сигнал:
    modus::SvSignal *signal = signals_by_id.value(signalCfg.id);

    // Читаем время последнего обновления сигнала:
    stream >> dataTime;

    // Читаем значение сигнала:
    stream >> value;

    // Устанавливаем поля "значения сигнала" и "времени последнего обновления сигнала":
    signal -> setValue(value, dataTime);

    return stream;
}


inline QDataStream& operator>> (QDataStream &stream, modus::SignalConfig& signalCfg)
// Десериализация информации из структуры "SignalConfig", принятой от другого "mdserver"а.
// Функция расширяемая: при необходимости список принимаемых полей структуры
// "SignalConfig" может быть легко откорректирован.
{
    // Читаем идентификатор сигнала:
    stream >> signalCfg.id;

    return stream;
}


apak::SvExchangeSignals::SvExchangeSignals():
  modus::SvAbstractProtocol()
// Инициализация структур данных, необходимых для формирования пакета К другому модулю МОС
// и обработки пактов ОТ другого МОС.

{
    // === ИНИЦИАЛИЗИРУЕМ СТРУКТУРУ ДАННЫХ, НЕОБХОДИМУЮ ДЛЯ ПРИЁМА ИНФОРМАЦИИ О СИГНАЛАХ ОТ ДРУГОГО МОС ====:
    // Очистим словарь, который каждому идентификатору сигнала ставит в соответствие сигнал:
    m_signal_by_id.clear();

    // === ИНИЦИАЛИЗИРУЕМ СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ ПЕРЕДАЧИ ИНФОРМАЦИИ О СИГНАЛАХ ДРУГОМУ МОС ====:
    // Очистим список сигналов для передачи:
    m_signals_to_transmit.clear();
}


apak::SvExchangeSignals::~SvExchangeSignals()
{
  deleteLater();
}


bool apak::SvExchangeSignals::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
// Эта функция вызывается серевером "mdserver" для всех устройств.
// Её цель - инициализировать все структуры, необходимые нам для сигналов конкретного
// устройства (в данном случае, модуля МОС).
{
    try
    {
        p_config = config;
        p_io_buffer = iobuffer;

        // Определяем размер буфера между протокольной и интерфейсной частями:
        m_send_maxLen = iobuffer->output->size;

        // Заполняем структуру "m_params" параметрами протокола обмена модуля обмена
        // сигналами (МОС). Параметры протокола описаны в файле "protokol_params.h"
        m_params = apak::ProtocolParams::fromJson(p_config->protocol.params);

        // На основе параметров протокола из структуры "m_params" заполняем
        // заголовок пакета, для ПЕРЕДАЧИ на другой МОС "m_sendHeader":

        // Магическое число в заголовке посылки между МОС:
        m_sendHeader.magic_number = m_params.magic_number;

        // Номер версии протокола обмена между МОС:
        m_sendHeader.protocol_version = m_params.protocol_version;

        // Номер версии формата сериализации данных библиотеки Qt:
        m_sendHeader.version_Qt = m_params.version_Qt;

        // Порядок байт:
        m_sendHeader.byte_order = m_params.byte_order;

        // Точность значений с плавающей точкой:
        m_sendHeader.fp_precision = m_params.fp_precision;

        return true;
    }
    catch (SvException& e)
    {
        // Отображаем оператору сообщение о месте ошибке. Сообщение о самой ошибке
        // хранится в исключении (e.error) и будет передано в "mdserver" через "p_last_error":
        emit message(QString("МОС: Исключение в функции \"configure\""), sv::log::llError, sv::log::mtError);
        qDebug() << "МОС: Исключение в функции \"configure\"";

        p_last_error = e.error;
        return false;
    }
}


bool apak::SvExchangeSignals::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
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
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    if(r)
    {
        if(binding.mode == modus::Master)
        { // Если привязка сигнала - "master", то это сигналы, принимаемые от
            // другого "mdserver"а.

            // Заполняем структуру "SignalParams" параметрами принимаемых от
            // другого "mdserver"а сигналов:

            // Параметры сигналов описываются в подразделе "params", следующем за номером устройства,
            // к которому они привязаны.
            // apak::SignalParams signal_params = apak::SignalParams::fromJson(binding.params);

            // Получаем идентификатор сигнала:
            int id = signal -> config()-> id;

            // Вставляем в словарь "m_signal_by_id" пару: КЛЮЧ - это идентификатор сигнала,
            // ЗНАЧЕНИЕ - это указатель на сигнал:
            m_signal_by_id.insert(id, signal);
      } // if(binding.mode == modus::Master)
      else
      { // Привязка сигнала - "binding", то есть это сигналы, передаваемые
            // на другой "mdserver".

            // Заполняем структуру "SignalParams" параметрами передаваемых
            // на другой "mdserver" сигналов:

            // Параметры сигналов описываются в подразделе "params", следующем за номером устройства,
            // к которому они привязаны.
            //apak::SignalParams signal_params = apak::SignalParams::fromJson(binding.params);

            // Вставляем сигнал в список сигналов для передачи "m_signals_to_transmit":
            m_signals_to_transmit.append(signal);
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
    emit message(QString("МОС: Исключение в функции \"bindSignal\" на сигнале: %1").arg(signalName), sv::log::llError, sv::log::mtError);
    qDebug() << QString ("МОС: Исключение в функции \"bindSignal\" на сигнале %1").arg(signalName);

    return false;
  }
}


void apak::SvExchangeSignals::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}


void apak::SvExchangeSignals::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}


void apak::SvExchangeSignals::start(void)
// В этой функции мы:
// 1. Осуществляем привязку вызова функции "sendSignals", в которой мы
// формируем пакет с информацией о значениях сигналов к другому МОС, к наступлению таймаута
// таймера посылки "m_sendTimer".
// 2. Запускаем таймер посылки "m_sendTimer" с периодом,равным интервалу посылки
// пакетов, заданному  в конфигурационном файле
// "config_apak.json", как параметр протокола МОС.
// 3. Привязываем вызов функции "receiveSignals" к сигналу "modus::IOBuffer::dataReaded".
// Этот сигнал испускается интерфейсной частью по приходу данных от интерефейса.
// В этой функции мы обрабатываем принятый от другого МОС пакет сигналов.

{
    // 1. Привязываем вызов функции "sendSignals", в которой мы
    // формируем пакет с информацией о сигналах к другому "mdserver"у, к наступлению таймаута
    // таймера посылки "m_sendTimer":
    m_sendTimer = new QTimer;
    connect(m_sendTimer, &QTimer::timeout, this, &SvExchangeSignals::sendSignals);

    // 2. Запускаем таймер посылки "m_sendTimer" с периодом,
    // равным интервалу посылки пакетов, заданному  в конфигурационном файле
    // "config_apak.json", как параметр протокола модуля МОС.
    m_sendTimer->start(m_params.send_interval);

    // 3. Привязываем вызов функции "receiveSignals" к сигналу "modus::IOBuffer::dataReaded".
    // Этот сигнал испускается интерфейсной частью по приходу данных от интерефейса.
    // В этой функции мы обрабатываем принятый от другого МОС пакет сигналов.
    connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &SvExchangeSignals::receiveSignals);

    emit message(QString("МОС: Размер буфера между протокольной и интерфейсной частями: %1").arg(m_send_maxLen), sv::log::llInfo, sv::log::mtInfo);
    qDebug() << QString("МОС: Размер буфера между протокольной и интерфейсной частями: %1").arg(m_send_maxLen);

    p_is_active = bool(p_config) && bool(p_io_buffer);
}


void apak::SvExchangeSignals::sendSignals(void)
// В этой функции мы:
// 1. Формируем и помещаем в массив байт "m_send_data" пакет со значениями сигналов
// для передачи на другой МОС.
// 2. Инициируем передачу этого пакета от протокольной к интерфейcной части
// (для передачи по линии связи).
{
    // qDebug() << "МОС: Посылка пакета: Вызов sendSignals()";

    if (p_is_active == false)
        return;

    // Поскольку размеры пакета для посылки на другой МОС ограничены размером буфера
    // между протокольной и интерфейсной частями, то сериализовывать сигналы
    // мы будем последовательно. После сериализации каждого сигнала мы будем оценивать
    // возможность его добавления к пакету. В случае невозможности - мы будем выдавать
    // сообщение оператору о том, что не все сигналы поместились в пакет.

    // Очистим массив в котором будем формировать пакет:
    m_send_data.clear();

    // Текущий размер пакета со значениями сигналов для передачи на другой МОС:
    unsigned send_currentLen = 0;

    // Массив байт для сериализации элементов пакета (заголовка и сигналов):
    QByteArray sendHeaderArray;

    // Поток для сериализации заголовка пакета:
    QDataStream sendHeaderStream (&sendHeaderArray, QIODevice::WriteOnly);

    // Cначала сериализуем заголовок пакета. УСЛОВИМСЯ, что В ЗАГОЛОВКЕ:
    // - последовательность байт ВСЕГДА "BigEndian":
    sendHeaderStream.setByteOrder(QDataStream::BigEndian);

    // - версия формата сериализации данных библиотеки Qt: всегда "5.5":
    sendHeaderStream.setVersion(QDataStream::Qt_5_5);

    sendHeaderStream << m_sendHeader;

    //qDebug() << "Размер заголовка: " << sendHeaderArray.length();
    //qDebug() << "Содержание заголовка" << sendHeaderArray;

    // Добавим заголовок к пакету:
    m_send_data.append(sendHeaderArray);

    // Вычислим текущий размер пакета:
    send_currentLen = sendHeaderArray.length();

    // В цикле пройдём по всем сигналам, намеченным к передаче.

    // Сначала:
    //   - определим количество сигналов для передачи:
    uint numberOfSignals = m_signals_to_transmit.size();   
    //qDebug() << "Количество сигналов для передачи: " << numberOfSignals;


    for (uint i = 0; i < numberOfSignals; i++)
    {
        // 1. Получаем указатель на текущий сериализуемый сигнал:
        modus::SvSignal* signal = m_signals_to_transmit.at (i);

        // 2. Массив байт для сериализации текущего (в цикле) сигнала:
        QByteArray sendSignalArray;

        // 3. Поток для сериализации текущего (в цикле) сигнала:
        QDataStream sendSignalStream (&sendSignalArray, QIODevice::WriteOnly);

        // 4. Установим параметры потока сериализации, согласно заданным
        //     в конфигурационном файле для модуля МОС:

        // 4.1. Параметр "номер версии формата сериализации данных библиотеки Qt":
        sendSignalStream.setVersion(m_sendHeader.version_Qt);

        // 4.2. Параметр "порядок байт":
        sendSignalStream.setByteOrder((QDataStream::ByteOrder)m_sendHeader.byte_order);

        // 4.3. Параметр "точность значений с плавающей точкой":
        sendSignalStream.setFloatingPointPrecision((QDataStream::FloatingPointPrecision)m_sendHeader.fp_precision);

        // 5. Сериализуем текущий сигнал:
        sendSignalStream << *signal;

        // 6. Определяем размер сериализованного сигнала, и решаем хватит ли на него
        // места в буфере между протокольной и интерфейсной частями:
        unsigned signalLen = sendSignalArray.length();

        // Получаем имя сигнала:
        QString signalName = signal ->config() ->name;

        emit message(QString("MOC: Сигнал: %1 занимает %2 байт").arg(signalName).arg(signalLen), sv::log::llInfo, sv::log::mtInfo);
        qDebug() << QString("MOC: Сигнал: %1 занимает %2 байт").arg(signalName).arg(signalLen);

        if (send_currentLen + signalLen > m_send_maxLen)
        { // Если текущий сигнал не помещается в буфер:

            // Выдаём сообщение оператору:
            qDebug() << QString ("МОС: Пакет с сигналами для передачи на другой МОС не помещается в буфер");
            emit message(QString ("МОС: Пакет с сигналами для передачи на другой МОС не помещается в буфер"), sv::log::llError, sv::log::mtError);

            // Выходим из цикла:
            break;
        }

        // Добавляем текущий сигнал к пакету:
        m_send_data.append(sendSignalArray);

        //qDebug() << "Сериализация сигнала: " << signalName << "Длина: " << signalLen << "Содержание: " << sendSignalArray.toHex();

        // Вычисляем текущий размер пакета:
        send_currentLen += signalLen;
    } // for

    // Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
    transferToInterface (m_send_data);

    return;
}


void apak::SvExchangeSignals::receiveSignals(modus::BUFF* buffer)
// Обрабатываем пакет с информацией о сигналах, принятый от другого модуля МОС:
{
    //qDebug() << "MOC: receiveSignals";

    if(p_is_active == false)
        return;

    if(!buffer->isReady())
    { // Если принятых данных нет, то не в чем и разбираться (выходим из функции):
        return;
    }

    // Блокируем  мьютекс, чтобы интерфейсная часть не имела доступа к входному буферу
    // пока мы не разберёмся с тем, что находится там сейчас.
    // Разблокировка мьютикса "buffer -> mutex" будет происходить автоматически ПРИ ВЫХОДЕ
    // из этой функции. Таким образом мы не допустим ситуации повторного вызова
    // этой функции до тех пор, пока она не закончит выполняться первый раз.
    QMutexLocker(&(buffer->mutex));

    // Скопируем пакет, пришедший от другого МОС, в массив "m_receive_data":
    m_receive_data = QByteArray(buffer->data, buffer->offset);

    //qDebug() << "МОС: Длина пакета, принятого от другого МОС: " << m_receive_data.length();

    buffer->reset();


    // Разбираемся с пакетом, принятым от другого МОС:
    // Создаём поток для десериализации пакета:
    QDataStream receiveStream (&m_receive_data, QIODevice::ReadOnly);

    // Cначала десериализуем заголовок пакета. УСЛОВИМСЯ, что В ЗАГОЛОВКЕ:
    // - последовательность байт ВСЕГДА "BigEndian":
    receiveStream.setByteOrder(QDataStream::BigEndian);

    // - версия формата сериализации данных библиотеки Qt: всегда "5.5":
    receiveStream.setVersion(QDataStream::Qt_5_5);


    receiveStream >> m_receiveHeader;


    // Разбираемся с параметрами, принятыми в заголовке:
    // 1. Параметр "магическое число":
    if (m_receiveHeader.magic_number != m_sendHeader.magic_number)
    { // Если не совпало "магическое число" в заголовке пакета:

        // Выводим сообщение оператору:
        qDebug() << QString ("МОС: Магическое число в заголовке принятого пакета: %1 не совпало с числом, указанным в конфигурационном файле: %2. Принимать сигналы не будем.").arg(m_receiveHeader.magic_number).arg(m_sendHeader.magic_number);
        emit message(QString ("МОС: Магическое число в заголовке принятого пакета: %1 не совпало с числом, указанным в конфигурационном файле: %2. Принимать сигналы не будем.").arg(m_receiveHeader.magic_number).arg(m_sendHeader.magic_number), sv::log::llError, sv::log::mtError);

        // Дальше этот пакет не обрабатываем:
        return;
    }


    // 2. Параметр "версия протокола обмена между МОС" - пока не обрабатываем.

    // 3. Параметр "номер версии формата сериализации данных библиотеки Qt":
    receiveStream.setVersion(m_receiveHeader.version_Qt);

    // 4. Параметр "порядок байт":
    receiveStream.setByteOrder((QDataStream::ByteOrder) m_receiveHeader.byte_order);

    // 5. Параметр "точность значений с плавающей точкой":
    receiveStream.setFloatingPointPrecision((QDataStream::FloatingPointPrecision) m_receiveHeader.fp_precision);

    // В цикле, пока не достигнут конец потока, десериализуем из потока сигналы:
    while (receiveStream.atEnd() == false)
    {
        try
        { // При выполнении чтения значений сигналов из принятого от другого МОС пакета
            // может возникнуть исключение: сигнала с принятым идентификатором нет в списке
            // принимаемых сигналов (это ошибка несоответствия конфигурационных файлов
            // передающего и принимающего модулей МОС).

            receiveStream >> m_signal_by_id;
        }
        catch (SvException& e)
        { // Перехватываем это исключение и выводим сообщение об ошибке пользователю
          // (обычно эта ошибка заключается в том, что десериализованный сигнал не привязан к МОС):

            emit message(e.error, sv::log::llError, sv::log::mtError);
            qDebug() << e.error;
        }
    } // while

    return;
}



void apak::SvExchangeSignals::transferToInterface (QByteArray data)
// Функция передаёт данные от протокольной к интерфейcной части (для передачи по линии связи).
// Аргумент: "data" - массив байт для передачи
{
    p_io_buffer->output->mutex.lock();

    if (p_io_buffer->output->isReady())
    {   // Если нам надо записать в буфер сообщение,
        // а интерфейсная часть ещё не прочла предыдущее сообщение (флаг "is_ready" - установлен),
        // то вместо функции "setData", мы используем фунцкцию "append", чтобы не "затирать"
        // предыдущее сообщение, а дополнить его. Но предварительно проверяем, есть ли в буфере
        // место на новые данные:

        // Если в буфере нет места на новые данные, то очищаем его содержимое и
        // сбрасываем флаг "is_ready":
        if(p_io_buffer->output->offset + data.length() > p_config->bufsize)
            p_io_buffer->output->reset();

        // Дополняем буфер новыми данными:
        p_io_buffer ->output->append(data);
    }
    else
    {   // Записываем в буфер сообщение и устанавливаем флаг готовности буфера "is_ready":
        p_io_buffer->output->setData(data);
    }

    emit message(QString(m_send_data.toHex()), lldbg, sv::log::mtNew);

    p_io_buffer->output->setReady(true);

    // Испускаем для интерфейсной части сигнал "readyWrite", по получению которого она
    // должна начать передачу данных из буфера в интерфейс:
    emit p_io_buffer->readyWrite(p_io_buffer->output);

    p_io_buffer->output->mutex.unlock();
}


QString enumBigEndian (int endianType)
{
    switch (endianType)
    {
        case QDataStream::BigEndian:
            return QString("BigEndian");
        case QDataStream::LittleEndian:
            return QString("LittleEndian");
        default:
            return QString("Ошибка");
    }
}

QString enumFloatingPointPrecision (int floatingPointPrecision)
{
    switch(floatingPointPrecision)
    {
        case QDataStream::SinglePrecision:
            return QString ("SinglePrecision");
        case QDataStream::DoublePrecision:
            return QString ("DoublePrecision");
        default:
            return QString("Ошибка");
    }

    return (QString());
}

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvExchangeSignals();
  return protocol;
}


const char* getVersion()
{
  return LIB_VERSION;
}

// Размер массива В БАЙТАХ, хранящего описание параметров модуля МОС:
#define MAX_BYTES_OF_DESCRIPTION_PARAMS 2000

// Массив, который хранит описание параметров модуля МОС:
char usage[MAX_BYTES_OF_DESCRIPTION_PARAMS + 1] = "";

const char* getParams()
{
    QString usageString = QString ("{\"params\": [\n") +
        MAKE_PARAM_STR_3(P_SEND_INTERVAL, P_SEND_INTERVAL_DESC, "quint16", "false",
                         QString("%1").arg(DEFAULT_SEND_INTERVAL), "1 - 65535", ",\n") +

        MAKE_PARAM_STR_3(P_BYTE_ORDER, P_BYTE_ORDER_DESC, "перечисление", "false",
                         enumBigEndian(DEFAULT_BYTE_ORDER), "\'BigEndian\' и \'LittleEndian\'", ",\n") +

        MAKE_PARAM_STR_3(P_FLOATING_POINT_PRECISION, P_FLOATING_POINT_PRECISION_DESC, "перечисление", "false",
                         enumFloatingPointPrecision (DEFAULT_FLOATING_POINT_PRECISION), "\'SinglePrecision\' и \'DoublePrecision\'", ",\n") +

        MAKE_PARAM_STR_3(P_VERSION_QT, P_VERSION_QT_DESC, "перечисление", "false",
                         QString("%1").arg(DEFAULT_VERSION_QT), "Численные значения любых констант из перечисления  \'QDataStream::Version\'", ",\n") +

        MAKE_PARAM_STR_3(P_EXCHANGE_PROTOCOL_VERSION, P_EXCHANGE_PROTOCOL_VERSION_DESC,  "quint16", "false",
                         QString("%1").arg(DEFAULT_EXCHANGE_PROTOCOL_VERSION),  "1 - 65535", ",\n") +

        MAKE_PARAM_STR_3(P_MAGIC_NUMBER, P_MAGIC_NUMBER_DESC, "qint32", "false",
                         QString("%1").arg(DEFAULT_MAGIC_NUMBER), "-2147483648 - 2147483647", "\n") +

        QString("]}");
        //qDebug() << "QString" <<usageString << usageString.length();

        QByteArray usageByteArray = usageString.toUtf8();
        usageByteArray.truncate(MAX_BYTES_OF_DESCRIPTION_PARAMS);
        usageByteArray.append('\0');
        //qDebug() << "QByteArray"<< usageByteArray << usageByteArray.length();

        strcpy(usage, usageByteArray.constData());
        //qDebug() << "char *" << usage << strlen(usage);

    return usage;
}


const char* getInfo()
{
  return LIB_SHORT_INFO;
}


const char* getDescription()
{
  return LIB_DESCRIPTION;
}
