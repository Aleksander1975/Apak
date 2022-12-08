#include "kson_packet.h"


// Флаг ошибки структуры информационного кадра:
#define INFO_FRAME_ERROR            1

// Флаг ошибки структуры блока параметрической информации:
#define PARAMETRIC_INFOBLOCK_ERROR  2

// Длина (в байтах) поля "размера данных" в информационном кадре и в пакете подтверждения:
#define DATA_SIZE_FIELD_LENGTH              8

// Длина (в байтах) поля "времени" в информационном кадре и в пакете подтверждения:
#define TIME_FIELD_LENGTH                   8

// Длина (в байтах) поля "доступности групп" в информационном кадре:
#define GROUP_AVAILABILITY_FIELD_LENGTH     2

// Длина (в байтах) поля "статуса" в пакете подтверждения:
#define STATUS_FIELD_LENGTH                 8

// Флаг ошибки структуры информационного кадра:
#define INFO_FRAME_ERROR            1

// Флаг ошибки структуры блока параметрической информации:
#define PARAMETRIC_INFOBLOCK_ERROR  2


apak::SvKsonPacket::SvKsonPacket():
  modus::SvAbstractProtocol(),
  m_data_signal(nullptr),
  m_state_signal(nullptr),
  m_time_signal (nullptr)
{
    // === ИНИЦИАЛИЗИРУЕМ СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ ФОРМИРОВАНИЯ ИНФОРМАЦИОННОГО КАДРА ОТ АПАК К КСОН ====:
    // Очистим словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsInformBlock_to_KSON" ставит в соответствие сигнал, бит которого должен быть записан
    // в массив "m_bitsInformBlock_to_KSON":
    m_signal_by_bitNumberInInformBlock_to_KSON.clear();

    // Очистим словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsInformBlock_to_KSON" ставит в соответствие номер бита сигнала, который
    // должен быть записан в массив "m_bitsInformBlock_to_KSON":
    m_bitNumberInSignal_by_bitNumberInInformBlock_to_KSON.clear();

    // Очистим битовый массив, который для каждого номера группы сигналов от АПАК к КСОН
    // (номер - это индекс в этом массиве),
    // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны):
    m_relevance_by_group_to_KSON.clear();
    m_relevance_by_group_to_KSON.resize(GROUP_AVAILABILITY_FIELD_LENGTH * 8);

    // Установим актуальность всех сигналов от АПАК к КСОН (группы от 0 до 9):
    for (int numberGr = 0; numberGr <= 9; numberGr++ )
        m_relevance_by_group_to_KSON[numberGr] = true;
    for (int numberGr = 10; numberGr <= 12; numberGr++ )
        m_relevance_by_group_to_KSON[numberGr] = false;

    // Конвертируем данные из массива бит "m_relevance_by_group_to_KSON" в массив байт
    // "relevanceGroup_Byte_to_KSON".
    // Для начала заполняем массив байт "relevanceGroup_Byte_to_KSON" нулями:
    m_relevanceGroup_Byte_to_KSON.clear();
    m_relevanceGroup_Byte_to_KSON.resize (GROUP_AVAILABILITY_FIELD_LENGTH);

    // В переменной цикла "bitNumberIn_relevance_by_group_to_KSON" будем хранить текущий номер бита в
    // массиве "relevanceGroup_Byte_to_KSON".
    for(quint16  bitNumberIn_relevance_by_group_to_KSON = 0; bitNumberIn_relevance_by_group_to_KSON <16;
                           bitNumberIn_relevance_by_group_to_KSON++)
    {
        m_relevanceGroup_Byte_to_KSON [bitNumberIn_relevance_by_group_to_KSON / 8] = m_relevanceGroup_Byte_to_KSON.at(bitNumberIn_relevance_by_group_to_KSON / 8) |
                             ((m_relevance_by_group_to_KSON [bitNumberIn_relevance_by_group_to_KSON] ? 1 : 0) << (bitNumberIn_relevance_by_group_to_KSON % 8));
    }

    // === ИНИЦИАЛИЗИРУЕМ СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ ПРОВЕРКИ ИНФОРМАЦИОННОГО КАДРА ОТ КСОН К АПАК ====:
    // Очистим словарь, который каждому смещению (в байтах) значения сигнала относительно начала информационного
    //     блока, передаваемого из КСОН в АПАК, ставит в соответствие указатель на этот сигнал.
    m_signals_by_byte_from_KSON.clear();

    // Очистим список указателей на сигналы, отсортированный по возрастанию смещений
    //     значений сигналов (в байтах) относительно начала информационного блока,
    //     передаваемого из КСОН в АПАК.
    m_signals_from_KSON.clear();

    // Очистим словарь, который каждому сигналу ставит в соответствие структуру SignalParams,
    //     хранящую параметры для этого сигнала:
    m_params_by_signal_from_KSON.clear();

    // Очистим битовый массив, который для каждого номера группы сигналов (номер - это индекс в этом массиве),
    //     определяет актуальны ли её сигналы (false - не актуальны, true - актуальны).
    //     В этом массиве АКТУАЛЬНОСТЬ сигналов группы определяется, исходя из наличия сигналов с
    //     параметром, указывающим на принадлежность к данной группе, в файле сигналов устройства КСОН.
    m_relevance_by_group_from_KSON.clear();
    m_relevance_by_group_from_KSON.resize(GROUP_AVAILABILITY_FIELD_LENGTH * 2);
}


apak::SvKsonPacket::~SvKsonPacket()
{
  deleteLater();
}


bool apak::SvKsonPacket::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
// Эта функция вызывается серевером "mdserver" для всех устройств.
// Её цель - инициализировать все структуры, необходимые нам для сигналов конкретного
// устройства (в данном случае, сети КСОН).
{
    try
    {
        p_config = config;
        p_io_buffer = iobuffer;

        // Заполняем структуру "m_params" параметрами протокола обмена системы АПАК с сетью КСОН
        // (периодом поступления данных в мс от АПАК в сеть КСОН и размером поля данных
        // в информационном кадре от АПАК к КСОН):
        m_params = apak::ProtocolParams::fromJson(p_config->protocol.params);

        return true;
    }
    catch (SvException& e)
    {
        // Отображаем оператору сообщение о месте ошибке. Сообщение о самой ошибке
        // хранится в исключении (e.error) и будет передано в "mdserver" через "p_last_error":
        emit message(QString("АПАК-КСОН: Исключение в функции \"configure\""), sv::log::llError, sv::log::mtError);
        qDebug() << "АПАК-КСОН: Исключение в функции \"configure\"";

        p_last_error = e.error;
        return false;
    }
}


bool apak::SvKsonPacket::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
// Эта функция вызывается сервером "mdserver" для всех сигналов, связанных с сетью КСОН.
// Перечислим эти сигналы:
// 1. "Cигнал данных, поступающих от КСОН". Значение этого сигнала - это весь информационный
//    кадр от КСОН, а не только блок параметрической информации из этого кадра. Сигнал имеет тип
//    "data"и описывается в файле: "data_KSON.json".
// 2. "Сигнал состояния cопряжения сети КСОН с системой АПАК" (0 - данные от КСОН к АПАК НЕ поступают/
//     1 - данные от КСОН к АПАК ПОСТУПАЮТ). Мы устанавливаем этот сигнал в "1" по приходу пакета от КСОН.
//     Сигнал имеет тип "stat" и описывается в файле "state_KSON.json".
// 3. "Сигнал времени" - сигнал содержит время в формате UTC (число секунд с 01.01.1970).
//    Его значение устанавливается нами из первого поля блока параметрической информации,
//    информационного кадра от КСОН к АПАК.
//    Сигнал имеет тип "time" и описывается в файле: "data_KSON.json".
// 4. Сигналы параметрической информации, поступающие в информационном кадре от КСОН к АПАК. Эти сигналы
//    нужны нам только для возможности получения библиотекой "kson_packet" параметров этих сигналов с
//    целью проверки допустимости значений принятых сигналов. По допустимости значений будем делать
//    вывод о наличии ошибок в информционном кадре.
//    Эти сигналы описывается в файле: "data_KSON.json".
// 5. Сигналы от системы АПАК к сети КСОН. Это сигналы о состоянии ПУ1 и ПУ2 системы АПАК и о состоянии
//    сопряжения АПАК с другими устройствами (СКМ, ИВ-1, КРАБ...)
//    Сигналы описываются в файле: "to_KSON.json".

// Цель этой функции состоит в получении всех параметров для каждого сигнала и заполнении в соответствии
// с ними структур данных (перечисленных в файле "kson_packet.h"), необходимых для формирования
// информационного кадра от системы АПАК к сети КСОН и проверки допустимости значений сигналов
// информационного кадра от сети КСОН в систему АПАК.
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);


    if(r)
    {
        if(binding.mode == modus::Master)
        { // Сигналы:
          // 1. данных, поступающих от КСОН (пакет целиком)
          // 2. состояния cопряжения сети КСОН с системой АПАК
          // 3. сигнал времени от сети КСОН
          // 4. сигналы параметрических данных, поступающие от КСОН
          // - имеют привязку "master".
          // Параметры сигналов "данные, поступающие от КСОН" и "состояние сопряжения сети КСОН с системой АПАК"
          // библиотекой "libapak_kson_packet" не используются.

            if(signal->config()->type.toLower() == TYPE_DATA)
            { // Если тип сигнала - "DATA", то это "сигнал данных, поступающих от КСОН". Значение
                // этого сигнала - это весь информационный кадр от КСОН, а не только блок
                // параметрической информации из этого кадра. Сигнал описывается в файле: "data_KSON.json", но
                // тип сигнала ("type": "data") может быть указан не в нём, а в файле "config_apak.json" сразу
                // для всей группы сигналов.

                if(m_data_signal)
                { // Если указатель "m_data_signal" на "сигнал данных" - ненулевой, то это означает,
                    // что функция "bindSignal" для "сигнала данных" уже вызывалась. То есть,
                    // в файлах конфигурации сигналов для устройства КСОН "сигналов данных,
                    // поступающих от сети КСОН в систему АПАК" - НЕСКОЛЬКО, что является ошибкой.

                    p_last_error = TOO_MUCH(p_config->name, TYPE_DATA);
                    return false;
                }

                // Присваиваем указателю "m_data_signal" указатель на сигнал данных:
                m_data_signal = signal;
            } // Если тип сигнала - "DATA"

            else if(signal->config()->type.toLower() == TYPE_STAT)
            {// Если тип сигнала - "STAT", то это сигнал о состоянии cопряжения сети КСОН с системой АПАК
                // (0 - данные от КСОН к АПАК НЕ поступают/ 1 - данные от КСОН к АПАК ПОСТУПАЮТ).
                // Мы устанавливаем этот сигнал в "1" по приходу пакета от КСОН.
                // Сигнал описывается в файле "state_KSON.json", но тип сигнала ("type": "STAT") может
                // быть указан не в нём, а в файле "config_apak.json" сразу
                // для всей группы сигналов.

                if(m_state_signal)
                { // Если указатель "m_state_signal" на "сигнал состояния" - ненулевой, то это означает,
                    // что функция "bindSignal" для "сигнала состояния" уже вызывалась. То есть,
                    // в файлах конфигурации сигналов для устройства КСОН "сигналов состояния
                    // cопряжения сети КСОН с системой АПАК" - НЕСКОЛЬКО, что является ошибкой.

                    p_last_error = TOO_MUCH(p_config->name, TYPE_STAT);                
                    return false;
                }

                // Присваиваем указателю "m_state_signal" указатель на сигнал состояния:
                m_state_signal = signal;
            } // Если тип сигнала - "STAT"

            else if (signal->config()->type.toLower() == TYPE_TIME)
            {// Если тип сигнала - "TIME", то это сигнал времени от сети КСОН.
                // Мы устанавливаем значение этого сигнала из поля времени блока параметрической информации
                // информации информационного кадра от КСОН.
                // Сигнал описывается в файле "date_KSON.json", но тип сигнала ("type": "TIME") может
                // быть указан не в нём, а в файле "config_apak.json" сразу
                // для всей группы сигналов.

                if(m_time_signal)
                { // Если указатель "m_time_signal" на "сигнал времени" - ненулевой, то это означает,
                    // что функция "bindSignal" для "сигнала времени" уже вызывалась. То есть,
                    // в файлах конфигурации сигналов для устройства КСОН "сигналов времени" -
                    // НЕСКОЛЬКО, что является ошибкой.
                    p_last_error = TOO_MUCH(p_config->name, TYPE_TIME);
                    return false;
                }

                // Заполняем структуру "SignalParams" параметрами "сигнала времени" устройства КСОН:         
                // Используемые параметры сигнала времени это:
                // смещение байта, в котором хранится значение сигнала, от начала информацинного
                // блока информационного кадра от АПАК к КСОН (параметр "byte"), смещением области битов,
                // в которой хранится значение сигнала, от начала байта (количество бит от начала байта)
                // (параметр "оffset"), размером области бит, которая хранит значение сигнала (количество бит),
                // в информационном блоке информационного кадра от AПАК к КСОН (параметр "len"), типом
                // данных сигнала (параметр "data_type"), максимальным значением сигнала (параметр "max"),
                // минимальным значением сигнала (параметр "min"), номером группы (параметр "group").

                // Параметры сигналов описываются в подразделе "params", следующем за номером устройства,
                // к которому они привязаны.
                apak::SignalParams signal_params = apak::SignalParams::fromJson(binding.params);

                // Вставляем в словарь "m_signals_by_byte_from_KSON" пару: КЛЮЧ - смещение (в байтах) значения
                // сигнала относительно начала информационного блока, передаваемого из КСОН в АПАК;
                // ЗНАЧЕНИЕ - указатель на сигнал.
                m_signals_by_byte_from_KSON.insert(signal_params.byte, signal);

                // Вставляем в словарь "m_params_by_signal_from_KSON" пару: КЛЮЧ - указатель на сигнал; ЗНАЧЕНИЕ -
                // структуру "signal_params", содержащую параметры этого сигнала.
                m_params_by_signal_from_KSON.insert (signal, signal_params);

                // Присваиваем указателю "m_time_signal" указатель на "сигнал времени":
                m_time_signal = signal;
            } // Если тип сигнала - "TIME"
            else
            { // Если привязка сигнала - "master", но это не "сигнал данных", "сигнал состояния"
              // или "сигнал времени", то это сигнал параметрических данных, поступающих в информационном
              // кадре от КСОН к АПАК. Эти сигналы нужны нам для возможности получить их параметры и заполнить
              // структуры данных, которые необходимы нам для проверки.

                // Заполняем структуру "SignalParams" параметрами сигналов от сети КСОН:
                // Используемые параметры сигналов от сети КСОН это:
                // смещение байта, в котором хранится значение сигнала, от начала информацинного
                // блока информационного кадра от АПАК к КСОН (параметр "byte"), смещением области битов,
                // в которой хранится значение сигнала, от начала байта (количество бит от начала байта)
                // (параметр "оffset"), размером области бит, которая хранит значение сигнала (количество бит),
                // в информационном блоке информационного кадра от AПАК к КСОН (параметр "len"), типом
                // данных сигнала (параметр "data_type"), максимальным значением сигнала (параметр "max"),
                // минимальным значением сигнала (параметр "min"), номером группы (параметр "group").

                // Параметры сигналов описываются в подразделе "params", следующем за номером устройства,
                // к которому они привязаны.
                apak::SignalParams signal_params = apak::SignalParams::fromJson(binding.params);

                // Вставляем в словарь "m_signals_by_byte_from_KSON" пару: КЛЮЧ - смещение (в байтах) значения
                // сигнала относительно начала информационного блока, передаваемого из КСОН в АПАК;
                // ЗНАЧЕНИЕ - указатель на сигнал.
                m_signals_by_byte_from_KSON.insert(signal_params.byte, signal);

                // Вставляем в словарь "m_params_by_signal_from_KSON" пару: КЛЮЧ - указатель на сигнал; ЗНАЧЕНИЕ -
                // структуру "signal_params", содержащую параметры этого сигнала.
                m_params_by_signal_from_KSON.insert (signal, signal_params);
            }
      } // if(binding.mode == modus::Master)
      else
      { // Сигналы, поступающие от АПАК к КСОН, которые имеют привязку "bindings".
            // Заполняем структуру "SignalParams" параметрами сигналов идущих от АПАК в сеть КСОН:
            // Используемые параметры сигналов от системы АПАК в сеть КСОН это:
            // смещение байта, в котором хранится значение сигнала, от начала информацинного
            // блока информационного кадра от АПАК к КСОН (параметр "byte"), смещением области битов,
            // в которой хранится значение сигнала, от начала байта (количество бит от начала байта)
            // (параметр "оffset"), размером области бит, которая хранит значение сигнала (количество бит),
            // в информационном блоке информационного кадра от AПАК к КСОН (параметр "len"), типом
            // данных сигнала (параметр "data_type"), номером группы (параметр "group").

            // Параметры сигналов описываются в подразделе "params", следующем за номером устройства,
            // к которому они привязаны.
            apak::SignalParams signal_params = apak::SignalParams::fromJson(binding.params);

            // Заполняем соответствующие сигналу, на который указывает аргумент "signal"
            // функции  "bindSignal", поля в структурах данных "m_signal_by_bitNumberInInformBlock_to_KSON"
            // и "m_bitNumberInSignal_by_bitNumberInInformBlock_to_KSON".

            // В цикле for () будем проходить по всем номерам битов сигнала (от 0 до "len").
            // В переменной цикла "bitNumberInSignal" будем хранить текущий номер бита сигнала.
            for (uint8_t bitNumberInSignal = 0; bitNumberInSignal < signal_params.len; bitNumberInSignal++)
            {
                m_signal_by_bitNumberInInformBlock_to_KSON [(uint16_t)signal_params.byte * 8 + signal_params.offset +
                        bitNumberInSignal] = signal;

                m_bitNumberInSignal_by_bitNumberInInformBlock_to_KSON [(uint16_t)signal_params.byte * 8 +
                        signal_params.offset + bitNumberInSignal] = bitNumberInSignal;
            } // for(...
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
    emit message(QString("АПАК-КСОН: Исключение в функции \"bindSignal\" на сигнале: %1").arg(signalName), sv::log::llError, sv::log::mtError);
    qDebug() << QString ("АПАК-КСОН: Исключение в функции \"bindSignal\" на сигнале %1").arg(signalName);

    return false;
  }
}

void apak::SvKsonPacket::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvKsonPacket::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvKsonPacket::start(void)
// В этой функции мы осуществляем:
// 1. Cформируем список "m_signals_from_KSON", хранящий указатели на сигналы приходящие от КСОН.
// Этот список отсортирован по возрастанию смещений значений сигналов (в байтах) относительно
// начала информационного блока, передаваемого из КСОН в АПАК.
// 2. Привязываем вызов функции "noReceivePackage" к наступлению таймаута
// таймера приёма "m_receiveTimer". В этой функции мы будем фиксировать отсутствие
// информационного кадра от КСОН.
// 3. Запускаем таймер приёма "m_receiveTimer".
// 4. Привязываем вызов функции "noConfirmationPackage" к наступлению таймаута
// таймера подтверждения "m_conformTimer". В этой функции мы будем фиксировать
 // ошибку взаимодействия КСОН и АПАК.
// 5. Привязку вызова функции "sendInformFrame" , в которой мы
// формируем информационный кадр от системы АПАК к сети КСОН, к наступлению таймаута
// таймера посылки "m_sendTimer".
// 6. Запускаем на единственное срабатывание таймер посылки "m_sendTimer" с периодом,
// равным интервалу посылки информационных кадров,
// от АПАК к КСОН, заданному  в конфигурационном файле "config_apak.json", как параметр
// протокола устройства КСОН. Далее будем запускать
// этот таймер каждый раз после получения пакета подтверждения от сети КСОН.
// 7. Привязку вызова функции "packageFrom_KSON", к сигналу "modus::IOBuffer::dataReaded". Этот сигнал
// испускается интерфейсной частью по приходу данных от интерефейса (в нашем случае - от сети КСОН).
// 8. Сброс счётчика идущих подряд ошибок взаимодействия КСОН и АПАК
{
    // 1. Формируем список "m_signals_from_KSON":
    m_signals_from_KSON = m_signals_by_byte_from_KSON.values();

    // 2. Привязываем вызов функции "noReceivePackage" к наступлению таймаута
    // таймера приёма "m_receiveTimer". В этой функции мы будем фиксировать отсутствие
    // информационного кадра от КСОН:
    m_receiveTimer = new QTimer;
    m_receiveTimer->setSingleShot(true);
    connect(m_receiveTimer, &QTimer::timeout, this, &SvKsonPacket::noReceivePackage);

    // 3. Запускаем таймер приёма "m_receiveTimer" с периодом, равным предельно допустимому времени
    // между получением информационных кадров от КСОН,
    // заданному  в конфигурационном файле "config_apak.json", как параметр протокола устройства КСОН:
    m_receiveTimer->start(m_params.receive_interval);

    // 4. Привязываем вызов функции "noConfirmationPackage" к наступлению таймаута
    // таймера подтверждения "m_conformTimer". В этой функции мы будем фиксировать
    // ошибку взаимодействия КСОН и АПАК:
    m_conformTimer = new QTimer;
    m_conformTimer->setSingleShot(true);
    connect(m_conformTimer, &QTimer::timeout, this, &SvKsonPacket::noConfirmationPackage);

    // 5. Привязку вызова функции "sendInformFrame" , в которой мы
    // формируем информационный кадр от системы АПАК к сети КСОН, к наступлению таймаута
    // таймера посылки "m_sendTimer":
    m_sendTimer = new QTimer;
    m_sendTimer->setSingleShot(true);
    connect(m_sendTimer, &QTimer::timeout, this, &SvKsonPacket::sendInformFrame);

    // 6. Запускаем на единственное срабатывание таймер посылки "m_sendTimer" с периодом,
    // равным интервалу посылки информационных кадров,
    // от АПАК к КСОН, заданному  в конфигурационном файле "config_apak.json", как параметр
    // протокола устройства КСОН:
    m_sendTimer->start(m_params.send_interval);

    // 7. Привязываем вызов функции "packageFrom_KSON", к сигналу "modus::IOBuffer::dataReaded". Этот сигнал
    // испускается интерфейсной частью по приходу данных от интерефейса (в нашем случае - от сети КСОН).
    // В этой функции мы обрабатываем принятый от сети КСОН информационный кадр или пакет подтверждения.
    connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &SvKsonPacket::messageFrom_KSON);

    // 8. Сброс счётчика ошибок взаимодействия КСОН и АПАК:
    m_interactionErrorCounter = 0;

    p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvKsonPacket::sendInformFrame(void)
// В этой функции мы:
// 1. Формируем и помещаем в массив байт "m_send_data" информационный кадр для передачи от
// системы АПАК к сети КСОН  (в соответствии с протоколом обмена).
// 2. Инициируем передачу этого кадра от протокольной к интерфейcной части
// (для передачи по линии связи).
// 3. Запускаем таймер подтверждения "m_conformTimer", который отсчитывает
// предельно допустимое время от посылки нами информационного кадра к сети КСОН,
// до получения нами пакета подтверждения от сети КСОН.

// Поле данных пакета - DATA_SIZE_FIELD_LENGTH байта
// Поле времени - TIME_FIELD_LENGTH байта
// Поле доступности групп (группы сигналов от системы АПАК к КСОН имеют номера с 0 по 9) -
//                                       GROUP_AVAILABILITY_FIELD_LENGTH байта
// Поле данных кадра (блок параметрической информации) - "params.send_data_len" байтов.
// Блок параметрической информации от системы АПАК представляем собой набор бит, которые
// говорят о состоянии ПУ1 и ПУ2 системы АПАК, о состоянии
// сопряжения АПАК с другими устройствами (СКМ, ИВ-1, КРАБ...).
// Значение бита "1" означает: "Работает / Подключено / Данные поступают", значение "0" -
// противоположное состояние.

// Порядок байт во всех числовых полях кадра - от старшего к младшему (bigEndian).
{
    // qDebug() << "АПАК-КСОН: Информационный пакет APAK к КСОН: Вызов send()";

    if (p_is_active == false)
        return;

    // 1.1. В переменной "m_bitsInformBlock_to_KSON" будем формировать информационный блок информационного кадра
    // от АПАК к КСОН. Для начала заполняем его нулями:
    m_bitsInformBlock_to_KSON.fill (false, m_params.send_data_len*8);

    // 1.2. В цикле for () будем проходить по всем номерам битов (от 0 до m_params.send_data_len*8 - 1)
    // из битового массива "m_bitsInformBlock_to_KSON". Данный массив битов представляет собой информационный
    // блок информационного кадра от АПАК к КСОН. В переменной цикла "bitNumberInInformBlock" будем
    // хранить текущий номер бита массива "m_bitsInformBlock_to_KSON".
    for (quint16 bitNumberInInformBlock = 0; bitNumberInInformBlock <= m_params.send_data_len*8 - 1;
         bitNumberInInformBlock++)
    {
        // 1.2.1. Проверим, должен ли по индексу "bitNumberInInformBlock" в массиве "m_bitsInformBlock_to_KSON" хранится
        // бит какого-либо сигнала:
        if (m_signal_by_bitNumberInInformBlock_to_KSON.contains(bitNumberInInformBlock) == false)
        { // Если не должен, то оставляем соответствующий бит массива "m_bitsInformBlock_to_KSON" нулевым
            //(переходим на следующую итерацию цикла):

            continue;
        }

        // 1.2.2. Если по индексу "bitNumberInInformBlock" в массиве "m_bitsInformBlock_to_KSON" должен храниться
        // бит какого-либо сигнала, то:
        // Получаем указатель на сигнал, один из битов которого должен храниться в массиве
        // "m_bitsInformBlock_to_KSON" по индексу "bitNumberInInformBlock":
        modus::SvSignal* signal = m_signal_by_bitNumberInInformBlock_to_KSON.value( bitNumberInInformBlock);

        // 1.2.3. Получаем имя сигнала:
        QString signalName = signal ->config() ->name;

        // 1.2.4. Проверяем актуальна ли информация, содержащаяся в сигнале:
        if(!signal->value().isValid() || signal->value().isNull())
        {
            // Информация в сигнале не актуальна -> оставляем соответствующий бит массива
            // "m_bitsInformBlock_to_KSON" нулевым (переходим на следующую итерацию цикла):

            emit message(QString("АПАК-КСОН: В сигнале, передаваемом на КСОН: %1 типа boolean информация не актуальна").arg(signalName), sv::log::llInfo, sv::log::mtInfo);
            //qDebug() << QString("АПАК-КСОН: В сигнале, передаваемом на КСОН: %1 типа boolean информация не актуальна").arg(signalName);

            continue;
        }

        bool ok;

        // 1.2.5. Теперь проверим представима ли информация в сигнале беззнаковым целым числом:
        quint8 intValueSignal = signal->value().toUInt(&ok);
        if(ok == false )
        {
            // Информация в сигнале не представима беззнаковым целым числом ->
            // оставляем соответствующий бит массива "m_bitsInformBlock_to_KSON" нулевым
            // (переходим на следующую итерацию цикла):

            // Отображаем оператору значение сигнала:
            emit message(QString("АПАК-КСОН: Значение сигнала %1 типа boolean, передаваемого на КСОН, не представимо типом unsigned").arg(signalName), sv::log::llInfo, sv::log::mtInfo);
            //qDebug() << QString("АПАК-КСОН: Значение сигнала %1 типа boolean, передаваемого на КСОН, не представимо типом unsigned").arg(signalName);

            continue;
        }

        // 1.2.6. Получаем номер бита сигнала, который должен храниться в массиве бит
        // "m_bitsInformBlock_to_KSON" по индексу "bitNumberInInformBlock_to_KSON":
        quint8 bitNumberInSignal =
                m_bitNumberInSignal_by_bitNumberInInformBlock_to_KSON.value (bitNumberInInformBlock);

        // 1.2.7. Получаем и сохраняем бит сигнала в массиве битов "m_bitsInformBlock_to_KSON":
        bool bitValue = (intValueSignal>>bitNumberInSignal) & 0x01 ? true: false;
        m_bitsInformBlock_to_KSON.setBit(bitNumberInInformBlock, bitValue);

        // Отображаем оператору значение сигнала:
        emit message(QString("АПАК-КСОН: Сигнал, передаваемый на КСОН: %1 типа boolean имеет значение: %2").arg(signalName).arg(bitValue), sv::log::llInfo, sv::log::mtInfo);
        //qDebug() << QString("АПАК-КСОН: Сигнал, передаваемый на КСОН: %1 типа boolean имеет значение: %2").arg(signalName).arg(bitValue);
    }
    // qDebug() << "АПАК-КСОН: Информационный блок информационного кадра от АПАК к КСОН: "<<m_bitsInformBlock_to_KSON;

     // 1.3. Теперь преобразуем массив бит "m_bitsInformBlock_to_KSON" в массив байт:
     // В этом массиве байт будет содержаться информационный блок информационного кадра от АПАК к КСОН.
     // 1.3.1. Для начала заполняем этот массив байт нулями:
     QByteArray informBlock (m_params.send_data_len, 0);

     // 1.3.2. Конвертируем данные из массива бит "m_bitsInformBlock_to_KSON" в массив байт "informBlock".
     // В переменной цикла "bitNumberInInformBlock" будем хранить текущий номер бита в
     // массиве "m_bitsInformBlock".
     for(quint16  bitNumberInInformBlock = 0; bitNumberInInformBlock <m_params.send_data_len * 8;
                bitNumberInInformBlock++)
            informBlock[bitNumberInInformBlock / 8] = informBlock.at(bitNumberInInformBlock / 8) |
                  ((m_bitsInformBlock_to_KSON [bitNumberInInformBlock] ? 1 : 0) << (bitNumberInInformBlock % 8));

     // 1.4. В переменной "m_send_data" формируем информационный кадр для передачи от АПАК к КСОН
     // в соответствии с протоколом обмена.
     m_send_data.clear();

     // 3. В массиве "dataSizeField" формируем первое поле информационного кадра - "размер данных",
     // то есть размер всех полей информационного кадра, кроме поля размера данных.
     // Сначала запишем значение этого поля в переменную "sizeField":
     uint64_t sizeField = TIME_FIELD_LENGTH + GROUP_AVAILABILITY_FIELD_LENGTH + m_params.send_data_len;

     // Переведём переменную "sizeField" в массив "dataSizeField":
     QByteArray dataSizeField (DATA_SIZE_FIELD_LENGTH, 0);
     for (int i = DATA_SIZE_FIELD_LENGTH - 1; i >= 0 ; i--)
     {
         dataSizeField [i] = sizeField & 0xFF;
         sizeField = sizeField >> 8;
     }

     // Добавляем поле "размера данных"  к формируемому информационному кадру:
     m_send_data.append(dataSizeField);


     // 1.5. В массиве "timeField" формируем второе поле информационного кадра - поле "времени",
     // которое должно содержать время по данным внешней системы - целое количество секунд
     // с 1 янв. 1970 г.
     // Время формирования информационного кадра записываем в переменную "m_packetTimeTo_KSON",
     // чтобы при получении пакета подтверждения проверить, что время в пакете подтверждения совпадает
     // c этим временем:
     quint64 timeSince_1970 = QDateTime::currentMSecsSinceEpoch() / 1000;
     m_packetTimeTo_KSON = timeSince_1970;

     // Переведём переменную "timeSince_1970" в массив "timeField":
     QByteArray timeField (TIME_FIELD_LENGTH, 0);
     for (int i = TIME_FIELD_LENGTH - 1; i >= 0 ; i--)
     {
         timeField [i] = timeSince_1970 & 0xFF;
         timeSince_1970 = timeSince_1970 >> 8;
     }

     // Добавляем поле "времени"  к формируемому информационному кадру:
     m_send_data.append(timeField);

     // 1.6. Добавляем поле доступности групп "relevanceGroup_Byte_to_KSON"
     // в формируемый информационный кадр:
     m_send_data.append(m_relevanceGroup_Byte_to_KSON);

     // 1.7. Добавим в формируемый информационный кадр блок параметрической информации,
     // сформированный нами в массиве "informBlock":
     m_send_data.append (informBlock);

     qDebug() <<"АПАК-КСОН: Информационный кадр от АПАК к КСОН:";
     qDebug() <<"АПАК-КСОН: Размер: " << m_send_data.length();
     qDebug() <<"АПАК-КСОН: Содержание: " << m_send_data.toHex();

     // 2. Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
     transferToInterface (m_send_data);

     // 3. Запускаем таймер подтверждения "m_conformTimer" с периодом,
     // равным предельно допустимому времени от посылки нами информационного кадра к сети КСОН,
     // до получения нами пакета подтверждения от сети КСОН. Это время
     // задаётся  в конфигурационном файле "config_apak.json", как параметр протокола устройства КСОН:
     m_conformTimer->start(m_params.conform_interval);
}


void apak::SvKsonPacket::messageFrom_KSON(modus::BUFF* buffer)
// Мы последовательно обрабатываем пакеты, из которых состоит принятое сообщение.
// Тип пакета (информационный кадр или пакет подтверждения) мы определем по полю
// размера данных.
{
    //qDebug() << "AПАК: messageFrom_KSON";

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

    // Скопируем пришедшее от КСОН сообщение, которое может состоять:
    // - из информационного кадра
    // - из пакета подтверждения
    // - из информационного кадра и пакета подтверждения в любом порядке
    // в массив "messageFrom_KSON":
    QByteArray messageFrom_KSON = QByteArray(buffer->data, buffer->offset);
    // qDebug() << "АПАК: messageFrom_KSON - длина принятого от КСОН сообщения: " << messageFrom_KSON.length();

    buffer->reset();


    // Разбираемся с принятым от сети КСОН сообщением:

    if(m_data_signal)
    { // Если в файле конфигурации сигналов КСОН имеется "сигнал данных", то присваиваем ему
      // значение, пришедшего от сети КСОН сообщения:

        m_data_signal->setValue(QVariant(messageFrom_KSON));
        emit message(QString("signal %1 updated").arg(m_data_signal->config()->name), sv::log::llDebug, sv::log::mtParse);
    }

    if(m_state_signal)
    {// Если в файле конфигурации сигналов КСОН имеется "сигнал состояния", то, по приходу ЛЮБОГО
     // (правильного или неправильного) информационного кадра или пакета подтверждения от
      // сети КСОН устанавливаем его значение в 1:

       m_state_signal->setValue(int(1));
    }

    if (messageFrom_KSON.length() < DATA_SIZE_FIELD_LENGTH)
    {
        // Обрабатываем ошибку:
        protocolErrorHandling (QString("АПАК-КСОН: Длина принятого от КСОН сообщения равна %1").arg(messageFrom_KSON.length()));

        return;
    }

    // Выделяем из сообщения от КСОН пакет и разбираемся с ним:
    analysisMessageFrom_KSON(messageFrom_KSON);

    if (messageFrom_KSON.length() == 0)
    { // Всё сообщение, которое было принято от КСОН - мы разобрали:

        return;
    }

    if (messageFrom_KSON.length() < DATA_SIZE_FIELD_LENGTH)
    {
        // Обрабатываем ошибку:
        protocolErrorHandling (QString("АПАК-КСОН: Длина остатка принятого от КСОН сообщения равна %1").arg(messageFrom_KSON.length()));

        return;
    }

    // Выделяем из остатка сообщения от КСОН пакет и разбираемся с ним:
    analysisMessageFrom_KSON(messageFrom_KSON);
    return;
}


void apak::SvKsonPacket::analysisMessageFrom_KSON(QByteArray& messageFrom_KSON)
// Эта функция осуществляет выделение из принятого от КСОН сообщения одного пакета и,
// в зависимости от вида пакета (информационный кадр или пакет подтверждения), вызов для
// его разбора сооответствующей функции.
{
    // Перепишем поле "размера данных" в массив "dataSizeField":
    QByteArray dataSizeField = messageFrom_KSON.mid (0, DATA_SIZE_FIELD_LENGTH);

    // Переведём поле "размера данных" в численный вид и запишем его в переменную "sizeField":
    uint64_t sizeField = 0;

    for (int i = 0; i < DATA_SIZE_FIELD_LENGTH; i++)
    {
        sizeField = sizeField << 8;
        sizeField |= (uchar)dataSizeField [i];
    }

    // Разбираемся с полем "размера данных":
    if (sizeField == (unsigned)(TIME_FIELD_LENGTH + GROUP_AVAILABILITY_FIELD_LENGTH + m_params.receive_data_len))
    { // Поле "размера данных" соответствует информационному кадру:

        // Копируем этот информационный кадр в массив  "packageFrom_KSON":
        QByteArray packageFrom_KSON = messageFrom_KSON.mid (0, DATA_SIZE_FIELD_LENGTH +
                         TIME_FIELD_LENGTH + GROUP_AVAILABILITY_FIELD_LENGTH + m_params.receive_data_len);

        // Удалим из массива "messageFrom_KSON" принятый информационный кадр:
        messageFrom_KSON.remove (0, DATA_SIZE_FIELD_LENGTH +
                                 TIME_FIELD_LENGTH + GROUP_AVAILABILITY_FIELD_LENGTH + m_params.receive_data_len);

        // Вызовем функцию обработки пришедшего информационного кадра:
        informFrameFrom_KSON (packageFrom_KSON);

        return;
    }

    if ( sizeField == TIME_FIELD_LENGTH + STATUS_FIELD_LENGTH)
    { // Поле "размера данных" соответствует пакету подтверждения:

        // Копируем этот пакет подтверждения в массив "packageFrom_KSON":
        QByteArray packageFrom_KSON = messageFrom_KSON.mid (0, DATA_SIZE_FIELD_LENGTH +
                         TIME_FIELD_LENGTH + STATUS_FIELD_LENGTH);

        // Удалим из массива "messageFrom_KSON" принятый пакет подтверждения:
        messageFrom_KSON.remove (0, DATA_SIZE_FIELD_LENGTH +
                                 TIME_FIELD_LENGTH + STATUS_FIELD_LENGTH);

        // Вызовем функцию обработки пришедшего пакета подтверждения:
        confirmationPackageFrom_KSON (packageFrom_KSON);

        return;
    }

    // Поле "размера данных" не соответствует ни информационному кадру, ни пакету подтверждения
    // - это ошибка, обрабатываем её:
    protocolErrorHandling (QString("АПАК-КСОН: Поле размера данных в сообщении от КСОН содержит неверную информацию: %1").arg(sizeField));

    // Разобрать что принято, если поле "размер данных" содержит значение не соответствующее ни
    // информационному кадру, ни пакету подтверждения - мы не сможем, поэтому очищаем массив
    // messageFrom_KSON:
    messageFrom_KSON.clear();
    return;
}


void apak::SvKsonPacket::informFrameFrom_KSON (QByteArray packageFrom_KSON)
// Функция обработки пришедшего от КСОН информационного кадра:
// Аргумент функции: "packageFrom_KSON" - содержит информационный кадр от КСОН.

{
    // Останавливаем таймер приёма:
    m_receiveTimer->stop();

    // Аргумент функции "packageFrom_KSON", содержит информационный кадр от КСОН.
    // Правильность информационного кадра в данной версии протокола можно проверить
    // по правильности содержимого поля "Размер данных", соответствию значений в
    // поле времени информационного пакета и в поле времени блока параметрических данных (в
    // случае, если поле времени в блоке параметрических данных - актуально),
    // и по допустимости значений параметрических данных (в случае их актуальности).
    // Правильнсть информационного кадра нам необходимо знать для формирования
    // поля статуса пакета подтверждения на этот информационный кадр. Поле статуса
    // будем формировать в переменной "m_status".
    // Для начала - очистим эту переменную:
    m_status = 0;

    // Перепишем поле "времени" в массив "timeField":
    QByteArray timeField = packageFrom_KSON.mid (DATA_SIZE_FIELD_LENGTH, TIME_FIELD_LENGTH);

    // Переведём поле "времени" в численный вид и запишем его в переменную "m_packetTimeFrom_KSON",
    // чтобы использовать его в пакете подтверждения на этот кадр:
    m_packetTimeFrom_KSON = 0;
    for (int i = 0; i < TIME_FIELD_LENGTH; i++)
    {
        m_packetTimeFrom_KSON = m_packetTimeFrom_KSON << 8;
        m_packetTimeFrom_KSON |= (uchar)timeField [i];
    }

    // Скопируем из информационного кадра поле "доступности групп" в массив "groupAvailability_from_KSON"
    QByteArray groupAvailability_from_KSON = packageFrom_KSON.mid (DATA_SIZE_FIELD_LENGTH + TIME_FIELD_LENGTH,
                                                                   GROUP_AVAILABILITY_FIELD_LENGTH);

    // Переведём массив байт "groupAvailability_from_KSON" в битовый массив "m_relevance_by_group_from_KSON"
    // (пока считаем, что группы нумеруются с нуля):
    m_relevance_by_group_from_KSON.clear();
    m_relevance_by_group_from_KSON.resize(GROUP_AVAILABILITY_FIELD_LENGTH * 8);

    for(quint16  bitNumber = 0; bitNumber < GROUP_AVAILABILITY_FIELD_LENGTH * 8; bitNumber++)
    {
           m_relevance_by_group_from_KSON [bitNumber] = (groupAvailability_from_KSON [bitNumber / 8] >> (bitNumber % 8)) & 0x01;
    }

    // Скопируем "блок параметрической информации" (информационный блок) из информационного кадра в массив "informBlock":
    QByteArray informBlock = packageFrom_KSON.mid (DATA_SIZE_FIELD_LENGTH + TIME_FIELD_LENGTH +
                                        GROUP_AVAILABILITY_FIELD_LENGTH, m_params.receive_data_len);

    // Получим значения полей информационного блока с помощью переменной  "informBlock_Stream":
    QDataStream informBlock_Stream (&informBlock, QIODevice::ReadOnly);

    // Согласно протоколу числа типа "float" занимают 4 байта:
    informBlock_Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);


    // Разбираем на отдельные поля информационный блок кадра, принятого нами:
    for (quint8 signalNumberInInformBlock = 0; signalNumberInInformBlock < m_signals_from_KSON.size();
             signalNumberInInformBlock++)
    { // Проходим по всем номерам параметрических данных (сигналов) в информационном блоке:

        // Получаем указатель на сигнал:
        modus::SvSignal* signal = m_signals_from_KSON [signalNumberInInformBlock];

        // Получаем параметры сигнала:
        apak::SignalParams signal_params = m_params_by_signal_from_KSON.value (signal);

        // Выясним тип сигнала ("boolean", "unsigned", "float") и считаем из потока
        // значение cигнала.

        // Переменная для получения значения сигнала булевского типа:
        bool booleanSignal;

        // Переменная для получения значения сигнала типа "unsigned":
        quint32 unsignedSignal;

        // Переменная для получения значения сигнала типа "float":
        float floatSignal;

        // Получаем имя сигнала:
        QString signalName = signal ->config() ->name;

        // Выясним тип сигнала ("boolean", "unsigned", "float") и считаем из потока значение
        // этого типа:
        switch (signal_params.data_type)
        {
            case boolType:
                // Этот сигнал представляет тип "boolean":
                informBlock_Stream >> booleanSignal;
                break;

            case unsignedType:
                // Этот сигнал представляет тип "unsigned" и занимает 4 байта, порядок
                // которых: big-endian
                informBlock_Stream >> unsignedSignal;
                break;

            case floatType:
            default:
                // Этот сигнал представляет тип "float" и занимает 4 байта, порядок
                //которых: big-endian
                informBlock_Stream >> floatSignal;
                break;
        } // switch

        // Определим к какой группе принадлежит сигнал:
        quint8 group = signal_params.group;

        if (m_relevance_by_group_from_KSON [group] == false)
        { // Если сигналы группы не актуальны -> незачем проверять допустимость их значений:

            emit message(QString("АПАК-КСОН: В информационном кадре от КСОН сигнал: %1 не актуален").arg(signalName), sv::log::llInfo, sv::log::mtInfo);
            qDebug() << QString("АПАК-КСОН: В информационном кадре от КСОН сигнал: %1 не актуален").arg(signalName);

            continue;
        }

        // Если значение сигнала актуально -> проверяем допустимость его значения.

        // Сначала из параметров сигнала получим его минимальное и максимальное значения:
        float min = signal_params.min;
        float max = signal_params.max;

        // Проверки будут различаться В зависимости от типа сигнала:
        switch (signal_params.data_type)
        {
            case boolType:
                // Этот сигнал представляет тип "boolean":
                if( booleanSignal != 0 && booleanSignal != 1)
                { // Если значение булевского сигнала отлично от "0" и от "1" ->
                    //фиксируем ошибку блока параметрической информации:

                    m_status |= PARAMETRIC_INFOBLOCK_ERROR;

                    protocolErrorHandling (QString("АПАК: В информационном кадре от КСОН сигнал: %1, типа boolean, cо значением %2 выходит за диапазон").arg(signalName).arg (booleanSignal));

                    continue;
                }

                // Отображаем оператору значение сигнала:
                emit message(QString("АПАК-КСОН: В информационном кадре от КСОН сигнал: %1 типа boolean имеет значение: %2").arg(signalName).arg(booleanSignal), sv::log::llInfo, sv::log::mtInfo);
                qDebug() << QString("АПАК-КСОН: В информационном кадре от КСОН сигнал: %1 типа boolean имеет значение: %2").arg(signalName).arg(booleanSignal);
                break;

            case unsignedType:
                // Этот сигнал представляет тип "unsigned" и занимает 4 байта, порядок
                // которых: big-endian
                if (unsignedSignal < min || unsignedSignal > max)
                {// Если значение сигнала типа "unsigned" не укладывается в указанные
                    // в протоколе пределы -> фиксируем ошибку блока параметрической информации:

                    m_status |= PARAMETRIC_INFOBLOCK_ERROR;

                    protocolErrorHandling (QString("АПАК: В информационном кадре от КСОН сигнал: %1, типа unsigned, cо значением %2 выходит за диапазон c минимальным значением: %3 и максимальным значением: %4").arg(signalName).arg (unsignedSignal).arg(min).arg(max));

                    continue;
                }

                // Отображаем оператору значение сигнала:
                emit message(QString("АПАК-КСОН: В информационном кадре от КСОН сигнал: %1 типа unsigned имеет значение: %2").arg(signalName).arg(unsignedSignal), sv::log::llInfo, sv::log::mtInfo);
                qDebug() << QString("АПАК-КСОН: В информационном кадре от КСОН сигнал: %1 типа unsigned имеет значение: %2").arg(signalName).arg(unsignedSignal);
                break;

            case floatType:
            default:
                // Этот сигнал представляет тип "float" и занимает 4 байта, порядок
                //которых: big-endian
                if (floatSignal < min || floatSignal > max)
                {// Если значение сигнала типа "float" не укладывается в указанные
                    // в протоколе пределы -> фиксируем ошибку блока параметрической информации:

                    m_status |= PARAMETRIC_INFOBLOCK_ERROR;

                    protocolErrorHandling (QString("АПАК: В информационном кадре от КСОН сигнал: %1, типа float, cо значением %2 выходит за диапазон c минимальным значением: %3 и максимальным значением: %4").arg(signalName).arg (floatSignal).arg(min).arg(max));

                    continue;
                }

                // Отображаем оператору значение сигнала:
                emit message(QString("АПАК-КСОН: В информационном кадре от КСОН сигнал: %1 типа float имеет значение: %2").arg(signalName).arg(floatSignal), sv::log::llInfo, sv::log::mtInfo);
                qDebug() << QString("АПАК-КСОН: В информационном кадре от КСОН сигнал: %1 типа float имеет значение: %2").arg(signalName).arg(floatSignal);
                break;
        } // switch


        if (signal == m_time_signal)
        { // "Это сигнал времени". Если мы оказались в этой точке программы, то это
            // значит, что он актуальный и его значение находится в допустимых пределах ->
            // Проводим проверку на соответствие значения времени в поле времени информационного
            // кадра (хранящейся в переменной "m_packetTimeFrom_KSON") и в информационном блоке
            // информационного кадра (значение сигнала: "unsignedSignal") :
            if (unsignedSignal == m_packetTimeFrom_KSON)
            { // Если значения совпадают -> устанавливаем значение "сигнала времени":

                 m_time_signal->setValue(m_packetTimeFrom_KSON);
            }
            else
            {   // Если значения НЕ совпадают:
                m_status |= PARAMETRIC_INFOBLOCK_ERROR;

                // Обрабатываем ошибку:
                protocolErrorHandling (QString("АПАК-КСОН: Время в поле времени информационного кадра: %1 не совпадает с временем в поле параметрических данных: %2").arg(m_packetTimeFrom_KSON).arg(unsignedSignal));
            }
        } // if (signal == m_time_signal)

    } // for

    if (m_status == 0)
     { // Если при разборе информационного кадра от КСОН мы НЕ обнаружили ошибкок -> сбрасываем
      // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН.

        m_interactionErrorCounter = 0;

        // Выводим сообщение оператору:
        qDebug() << QString("АПАК-КСОН: Принят информационный кадр от КСОН без ошибок");
        emit message(QString("АПАК-КСОН: Принят информационный кадр от КСОН без ошибок"), sv::log::llInfo, sv::log::mtSuccess);
    }

    // p_last_parsed_time = QDateTime::currentDateTime();

    // Запускаем таймер приёма "m_receiveTimer" с периодом, равным предельно допустимому времени
    // между получением информационных кадров от КСОН,
    // заданному  в конфигурационном файле "config_apak.json", как параметр протокола устройства КСОН:
    m_receiveTimer->start(m_params.receive_interval);

    // Сформируем и пошлём пакет подтверждения:
    sendConfirmationPackage();
}


void  apak::SvKsonPacket::confirmationPackageFrom_KSON (QByteArray packageFrom_KSON)
// Функция обработки пришедшего от КСОН пакета подтверждения:
// Аргумент функции: "packageFrom_KSON" - содержит пакет подтверждения от КСОН.
{
    // Останавливаем таймер подтверждения:
    m_conformTimer->stop();

    // Аргумент функции: "packageFrom_KSON" - содержит пакет подтверждения от КСОН.
    // Правильность пакета подтверждения в данной версии протокола можно проверить
    // по правильности значения поля времени
    // (оно должно повторять значение времени в информационном кадре от АПАК
    // к КСОН) и допустимости значений в поле статуса.

    // Переменная "confirmationPackageErrorFlag" - это флаг ошибки пакета подтверждения.
    // Устанавливается, если поле статуса пакета подтверждения
    // содержит значение, говорящее об ошибке в информационном кадре от АПАК к
    // КСОН или имеются любые ошибки в самом пакете подтверждения.

    //  Перед началом анализа пакета подтверждения сбросим флаг ошибки:
    bool confirmationPackageErrorFlag = false;

    // Перепишем поле "времени" в массив "timeField":
    QByteArray timeField = packageFrom_KSON.mid (DATA_SIZE_FIELD_LENGTH, TIME_FIELD_LENGTH);

    // Переведём поле "времени" в численный вид - переменную "timeSince_1970":
    quint64  timeSince_1970 = 0;
    for (int i = 0; i < TIME_FIELD_LENGTH; i++)
    {
        timeSince_1970 = timeSince_1970 << 8;
        timeSince_1970 |= (uchar)timeField [i];
    }

    if (timeSince_1970 !=  m_packetTimeTo_KSON)
    { // Если поля времени в информационном кадре от АПАК к КСОН и в пакете подтверждения
      // от КСОН к АПАК не совпадают, то выставляем флаг ошибки пакета подтвердения:

        confirmationPackageErrorFlag = true;

        protocolErrorHandling(QString("АПАК-КСОН: Время в пакете подтверждения от КСОН: %1 не совпадает с временем в информационном кадре: %2").arg(timeSince_1970).arg(m_packetTimeTo_KSON));
    }

    // Перепишем поле "статуса" в массив "statusField":
    QByteArray statusField = packageFrom_KSON.mid (DATA_SIZE_FIELD_LENGTH + TIME_FIELD_LENGTH,
                                                   STATUS_FIELD_LENGTH);

    // Переведём поле "статуса" в численный вид и запишем его в переменную "statusFromPacket":
    quint64 statusFromPacket = 0;
    for (int i = 0; i < STATUS_FIELD_LENGTH; i++)
    {
        statusFromPacket = statusFromPacket << 8;
        statusFromPacket |= (uchar)statusField [i];
    }

    if (statusFromPacket != 0)
    { // Если значение в поле статуса отлично от 0, то выставляем флаг ошибки пакета подтвердения:

        confirmationPackageErrorFlag = true;

        protocolErrorHandling(QString("АПАК-КСОН: Значение в поле статуса: %1 отлично от нуля").arg(statusFromPacket));
    }

    if ( confirmationPackageErrorFlag == false)
    { //Если при разборе пакета подтверждения мы НЕ обнаружили ошибок -> сбрасываем
        // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:

        m_interactionErrorCounter = 0;

        // Выводим сообщение оператору:
        qDebug() << QString("АПАК-КСОН: Принят пакет подтверждения от КСОН без ошибок");
        emit message(QString("АПАК-КСОН: Принят пакет подтверждения от КСОН без ошибок"), sv::log::llInfo, sv::log::mtSuccess);
    }

    // p_last_parsed_time = QDateTime::currentDateTime();

    // Запускаем на единственное срабатывание "таймер посылки m_sendTimer" отсчитывающий
    // период посылки информационных кадров от АПАК к КСОН с периодом,
    // равным интервалу между информационными кадрами,
    // заданному  в конфигурационном файле "config_apak.json", как параметр протокола
    // имитатора устройства КСОН.
    // По срабатыванию этого таймера будет вызвана функция "send", для посылки информационного кадра к
    // КСОН.
    m_sendTimer->start(m_params.send_interval);
}


void apak::SvKsonPacket::noConfirmationPackage(void)
// Эта функция вызывается в том случае, если в течении предельно допустимого времени от
// посылки нами информационного кадра к КСОН, до получения нами пакета подтверждения от КСОН
// пакет подтверждения так и не пришёл.
{
    // 1. Отрабатываем ошибку протокола:
    protocolErrorHandling(QString("АПАК-КСОН: Пакет подтверждения от КСОН за время: %1 не получен").arg(m_params.conform_interval));

    // 2. Даже если пакета подтверждения не пришло в течении оговоренного в протоколе времени
    // (m_params.conform_interval), то мы всё равно запускаем на единственное срабатывание
    // "таймер посылки m_sendTimer", чтобы продолжать посылать информационные кадры к КСОН:
    m_sendTimer->start(m_params.send_interval);
}


void apak::SvKsonPacket::noReceivePackage(void)
// Эта функция вызывается в том случае, если в течении предельно допустимого времени от
// сети КСОН не пришёл инфомационный кадр.
{
    // 1. Отрабатываем ошибку протокола:
    protocolErrorHandling (QString("АПАК-КСОН: Информационный кадр от КСОН за время: %1 не получен").arg(m_params.receive_interval));

    // 2. Даже если инфомационного кадра не пришло в течении оговоренного в протоколе времени
    // (m_params.receive_interval), то мы всё равно запускаем на единственное срабатывание
    // "таймер приёма m_receiveTimer", чтобы продолжать отображать в утилите "logview" и
    // на консоли факт отсутствия информационных кадров от КСОН:
    m_receiveTimer->start(m_params.receive_interval);
}


void apak::SvKsonPacket::sendConfirmationPackage(void)
// Формирование и посылка пакета подтверждения.
// На данный момент - формат пакета подтверждения не ясен, поэтому будем реализовывать следующий
// формат:
// Поле данных пакета - DATA_SIZE_FIELD_LENGTH байта
// Поле времени - TIME_FIELD_LENGTH байта
// Поле статуса - STATUS_FIELD_LENGTH байт
{
    // В массиве "m_send_data" формируем пакет подтверждения для передачи от АПАК к КСОН
    // в соответствии с протоколом обмена.

    // 1. Вначале, очистим массив:
    m_send_data.clear();

    // 2. В массиве "dataSizeField" формируем первое поле пакета подтверждения - "размер данных",
    // то есть размер всех полей пакета подтверждения, кроме поля размера данных.
    // Сначала запишем значение этого поля в переменную "sizeField":
    uint64_t sizeField = TIME_FIELD_LENGTH + STATUS_FIELD_LENGTH;

    // Переведём переменную "sizeField" в массив "dataSizeField":
    QByteArray dataSizeField (DATA_SIZE_FIELD_LENGTH, 0);
    for (int i = DATA_SIZE_FIELD_LENGTH - 1; i >= 0 ; i--)
    {
        dataSizeField [i] = sizeField & 0xFF;
        sizeField = sizeField >> 8;
    }

    // Добавляем поле "размера данных" к формируемому пакету подтверждения:
    m_send_data.append(dataSizeField);

    // 3. В массиве "timeField" формируем второе поле пакета подтверждения - поле "времени",
    // оно должно повторять время в информационном кадре от КСОН, сохранённое нами в переменной
    // "m_packetTimeFrom_KSON":

    // Переведём переменную "m_packetTimeFrom_KSON" в массив "timeField":
    QByteArray timeField (TIME_FIELD_LENGTH, 0);
    for (int i = TIME_FIELD_LENGTH - 1; i >= 0 ; i--)
    {
        timeField [i] = m_packetTimeFrom_KSON & 0xFF;
        m_packetTimeFrom_KSON = m_packetTimeFrom_KSON >> 8;
    }

    // Добавляем поле "времени"  к формируемому пакету подтверждения:
    m_send_data.append(timeField);


    // 4. В массиве "statusField" формируем третье поле пакета подтверждения - поле "статуса".
    // Значение статуса сформировано нами в переменной "m_status" в процессе
    // разбора информационного кадра от КСОН:

    // Переведём переменную "m_status" в массив "statusField":
    QByteArray statusField (STATUS_FIELD_LENGTH, 0);
    for (int i = STATUS_FIELD_LENGTH - 1; i >= 0 ; i--)
    {
        statusField [i] = m_status & 0xFF;
        m_status = m_status >> 8;
    }

    // Добавляем поле "статуса"  к формируемому пакету подтверждения:
    m_send_data.append(statusField);

    //qDebug() <<"АПАК-КСОН: Пакет подтверждения от АПАК к КСОН:";
    //qDebug() <<"АПАК-КСОН: Размер: " << m_send_data.length();
    //qDebug() <<"АПАК-КСОН: Содержание: " << m_send_data.toHex();

    // 5. Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
     transferToInterface (m_send_data);
}


void apak::SvKsonPacket::transferToInterface (QByteArray data)
// Функция передаёт данные от протокольной к интерфейcной части (для передачи по линии связи).
// Аргумент: "data" - массив байт для передачи
{
    p_io_buffer->output->mutex.lock();

    if (p_io_buffer->output->isReady())
    {   // Если нам надо записать в буфер сообщение (информационный кадр или пакет подтверждения),
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


void apak::SvKsonPacket::protocolErrorHandling (QString str)
// Обработка ошибки протокола. В чем она состоит:
// - Увеличение счётчика подряд идущих ошибок взаимодействия АПАК с КСОН
// - Выдача оператору в утилите "logview" и на консоль сообщения "str" об ошибке
// - В случае трёх ошибок взаимодействия подряд, выдать интерфейсной части
//   команду на разъединение соединения и сбросить счётчик подряд идущих ошибок
//   взаимодействия КСОН и АПАК.
{
    // Увеличиваем счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:
    m_interactionErrorCounter++;

    // Выдаём сообщение об ошибке:
    emit message(str + QString (". Количество идущих подряд ошибок взаимодействия : %1").arg(m_interactionErrorCounter), sv::log::llError, sv::log::mtError);
    qDebug() << str + QString (". Количество идущих подряд ошибок взаимодействия : %1").arg(m_interactionErrorCounter);

    // Если счётчик подряд идущих ошибок взаимодействия АПАК с КСОН больше значения
    // указанного в протоколе:
    if (m_interactionErrorCounter >= m_params.numberOfErrors)
    {
        qDebug() << QString ("АПАК-КСОН: Выдаём команду интерфейсной части на разрыв соединения с TCP-клиентом");
        emit message(QString("АПАК-КСОН: Выдаём команду интерфейсной части на разрыв соединения с TCP-клиентом"), sv::log::llError, sv::log::mtError);

        // Испускаем для интерфейсной части сигнал "say" с командой "breakConnection",
        // который приказывает интерфейсной части разорвать соединение с КСОН:
        emit p_io_buffer->say("breakConnection");

        // Сбрасываем счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:
        m_interactionErrorCounter = 0;
    }
}


/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvKsonPacket();
  return protocol;
}

