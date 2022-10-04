#include "kson_packet.h"

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
    // === ИНИЦИАЛИЗИРУЕМ СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ ФОРМИРОВАНИЯ ИНФОРМАЦИОННОГО ПАКЕТА ОТ АПАК К КСОН ====:
    // Очистим словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsInformBlock" ставит в соответствие сигнал, бит которого должен быть записан
    // в массив "m_bitsInformBlock_to_KSON":
    m_signal_to_KSON_by_bitNumberInInformBlock.clear();

    // Очистим словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsInformBlock" ставит в соответствие номер бита сигнала, который
    // должен быть записан в массив "m_bitsInformBlock":
    m_bitNumberInSignal_to_KSON_by_bitNumberInInformBlock.clear();

    // Очистим битовый массив, который для каждого номера группы сигналов от АПАК к КСОН
    // (номер - это индекс в этом массиве),
    // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны):
    m_relevance_by_group_to_KSON.clear();
    m_relevance_by_group_to_KSON.resize(16);

    // Установим актуальность всех сигналов от АПАК к КСОН (группы от 0 до 9):
    for (int numberGr = 0; numberGr <= 9; numberGr++ )
        m_relevance_by_group_to_KSON[numberGr] = true;
    for (int numberGr = 10; numberGr <= 12; numberGr++ )
        m_relevance_by_group_to_KSON[numberGr] = false;

    // Конвертируем данные из массива бит "m_relevance_by_group_to_KSON" в массив байт
    // "relevanceGroup_Byte_to_KSON".
    // Для начала заполняем массив байт "relevanceGroup_Byte_to_KSON" нулями:
    m_relevanceGroup_Byte_to_KSON.clear();
    m_relevanceGroup_Byte_to_KSON.resize (2);

    // В переменной цикла "bitNumberIn_relevance_by_group_to_KSON" будем хранить текущий номер бита в
    // массиве "relevanceGroup_Byte".
    for(quint16  bitNumberIn_relevance_by_group_to_KSON = 0; bitNumberIn_relevance_by_group_to_KSON <16;
                           bitNumberIn_relevance_by_group_to_KSON++)
    {
        m_relevanceGroup_Byte_to_KSON [bitNumberIn_relevance_by_group_to_KSON / 8] = m_relevanceGroup_Byte_to_KSON.at(bitNumberIn_relevance_by_group_to_KSON / 8) |
                             ((m_relevance_by_group_to_KSON [bitNumberIn_relevance_by_group_to_KSON] ? 1 : 0) << (bitNumberIn_relevance_by_group_to_KSON % 8));
    }

    // === ИНИЦИАЛИЗИРУЕМ СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ ПРОВЕРКИ ИНФОРМАЦИОННОГО ПАКЕТА ОТ КСОН К АПАК ====:
    // Очистим словарь, который каждому смещению (в байтах) значения сигнала относительно начала информационного
    //     блока, передаваемого из КСОН в АПАК, ставит в соответствие указатель на этот сигнал.
    m_signals_from_KSON_by_byte.clear();

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
    m_relevance_by_group_from_KSON.resize(16);
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

        // Заполняем структуру "m_params" параметрами протокола обмена системы АПАК с сетью КСОН:
        m_params = apak::ProtocolParams::fromJson(p_config->protocol.params);

        return true;
    }
    catch (SvException& e)
    {
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
// с ними структур данных (словари  m_signal_to_KSON_by_bitNumberInInformBlock,
// m_bitNumberInSignal_to_КSON_by_bitNumberInInformBlock, m_signals_from_KSON_by_byte,
// m_params_by_signal_from_KSON), необходимых для формирования
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
          // библиотекой "libapak_kson_imitator" не используются.

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

                // В ранних версиях конфигурационных файлов сигналов параметры сигналов описывались в подразделе "params"
                // раздела "bindings". Однако позднее подраздел "params" был перенесён из раздела "bindings"
                // на уровень выше - прямо в общий раздел сигнала (туда же где находятся: идентификатор сигнала,
                // имя сигнала, описание сигнала...). Поэтому аргументом функции SignalParams::fromJson()
                // является "signal->config()->params".
                apak::SignalParams signal_params = apak::SignalParams::fromJson(signal->config()->params);

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

                // В ранних версиях конфигурационных файлов сигналов параметры сигналов описывались в подразделе "params"
                // раздела "bindings". Однако позднее подраздел "params" был перенесён из раздела "bindings"
                // на уровень выше - прямо в общий раздел сигнала (туда же где находятся: идентификатор сигнала,
                // имя сигнала, описание сигнала...). Поэтому аргументом функции SignalParams::fromJson()
                // является "signal->config()->params".
                apak::SignalParams signal_params = apak::SignalParams::fromJson(signal->config()->params);

                // Вставляем в словарь "m_signals_from_KSON_by_byte" пару: КЛЮЧ - смещение (в байтах) значения
                // сигнала относительно начала информационного блока, передаваемого из КСОН в АПАК;
                // ЗНАЧЕНИЕ - указатель на сигнал.
                m_signals_from_KSON_by_byte.insert(signal_params.byte, signal);

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

            // В ранних версиях конфигурационных файлов сигналов параметры сигналов описывались в подразделе "params"
            // раздела "bindings". Однако позднее подраздел "params" был перенесён из раздела "bindings"
            // на уровень выше - прямо в общий раздел сигнала (туда же где находятся: идентификатор сигнала,
            // имя сигнала, описание сигнала...). Поэтому аргументом функции SignalParams::fromJson()
            // является "signal->config()->params".
            apak::SignalParams signal_params = apak::SignalParams::fromJson(signal->config()->params);


            // Заполняем соответствующие сигналу, на который указывает аргумент "signal"
            // функции  "bindSignal", поля в структурах данных "m_signal_to_KSON_by_bitNumberInInformBlock"
            // и "m_bitNumberInSignal_to_KSON_by_bitNumberInInformBlock".

            // В цикле for () будем проходить по всем номерам битов сигнала (от 0 до "len").
            // В переменной цикла "bitNumberInSignal" будем хранить текущий номер бита сигнала.
            for (uint8_t bitNumberInSignal = 0; bitNumberInSignal < signal_params.len; bitNumberInSignal++)
            {
                m_signal_to_KSON_by_bitNumberInInformBlock [(uint16_t)signal_params.byte * 8 + signal_params.offset +
                        bitNumberInSignal] = signal;

                m_bitNumberInSignal_to_KSON_by_bitNumberInInformBlock [(uint16_t)signal_params.byte * 8 +
                        signal_params.offset + bitNumberInSignal] = bitNumberInSignal;
            } // for(...
      }
    } // if(r)

    return r;
  }

  catch(SvException& e) {
    p_last_error = e.error;
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
// 2. Привязку вызова функции "sendInformFrame", в которой мы
// формируем информационный кадр от системы АПАК к сети КСОН, к наступлению таймаута
// "таймера посылки m_sendTimer". Таймер запускаем на ЕДИНСТВЕННОЕ срабатывание. Далее будем запускать
// этот таймер каждый раз после получения пакета подтверждения от сети КСОН.
// 3. Привязку вызова функции "packageFrom_KSON", к сигналу "modus::IOBuffer::dataReaded". Этот сигнал
// испускается интерфейсной частью по приходу данных от интерефейса (в нашем случае - от сети КСОН).
// 4. Сброс счётчика идущих подряд ошибок взаимодействия КСОН и АПАК
{
    // Формируем список "m_signals_from_KSON":
    m_signals_from_KSON = m_signals_from_KSON_by_byte.values();

    // Привязываем вызов функции "noReceivePackage" к наступлению таймаута
    // таймера приёма "m_receiveTimer". В этой функции мы будем фиксировать отсутствие
    // информационного кадра от КСОН:
    m_receiveTimer = new QTimer;
    m_receiveTimer->setInterval(true);
    connect(m_receiveTimer, &QTimer::timeout, this, &SvKsonPacket::noReceivePackage);

    // Запускаем таймер приёма "m_receiveTimer" с периодом, равным предельно допустимому времени
    // между получением информационных кадров от КСОН,
    // заданному  в конфигурационном файле "config_apak.json", как параметр протокола устройства КСОН:
    m_receiveTimer->start(m_params.receive_interval);

    // Привязываем вызов функции "noConfirmationPackage" к наступлению таймаута
    // таймера подтверждения "m_conformTimer". В этой функции мы будем фиксировать
    // ошибку взаимодействия КСОН и АПАК:
    m_conformTimer = new QTimer;
    m_conformTimer->setInterval(true);
    connect(m_conformTimer, &QTimer::timeout, this, &SvKsonPacket::noConfirmationPackage);

    // Привязываем вызов функции "sendInformFrame" к наступлению таймаута
    // "таймера посылки m_sendTimer":
    m_sendTimer = new QTimer;
    m_sendTimer->setSingleShot(true);
    connect(m_sendTimer, &QTimer::timeout, this, &SvKsonPacket::sendInformFrame);

    // Запускаем таймер посылки "m_sendTimer" с периодом, равным интервалу посылки информационных кадров,
    // от АПАК к КСОН, заданному  в конфигурационном файле "config_apak.json", как параметр
    // протокола устройства КСОН:
    m_sendTimer->start(m_params.send_interval);

    // Привязываем вызов функции "packageFrom_KSON", к сигналу "modus::IOBuffer::dataReaded". Этот сигнал
    // испускается интерфейсной частью по приходу данных от интерефейса (в нашем случае - от сети КСОН).
    // В этой функции мы обрабатываем принятый от сети КСОН информационный кадр или пакет подтверждения.
    connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &SvKsonPacket::packageFrom_KSON);

    // Сброс счётчика ошибок взаимодействия КСОН и АПАК:
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

// На данный момент - формат информационного кадра не ясен, поэтому будем реализовывать следующий
// формат:
// Размер данных - 4 байта
// Время - 4 байта
// Доступность групп (группы сигналов от системы АПАК к КСОН имеют номера с 0 по 9) - 2 байта
// Данные кадра (блок параметрической информации) - 15 бит (2 байта).
// Блок параметрической информации от системы АПАК представляем собой набор бит, которые
// говорят о состоянии ПУ1 и ПУ2 системы АПАК, о состоянии
// сопряжения АПАК с другими устройствами (СКМ, ИВ-1, КРАБ...).
// Значение бита "1" означает: "Работает / Подключено / Данные поступают", значение "0" -
// противоположное состояние.
// Порядок байт во всех полях кадра - от старшего к младшему (bigEndian).
{
    qDebug() << "Информационный пакет APAK к КСОН: Вызов send()";

    if (p_is_active == false)
        return;

    // 1. В переменной "m_bitsInformBlock_to_KSON" будем формировать информационный блок информационного кадра
    // от АПАК к КСОН. Для начала заполняем его нулями:
    m_bitsInformBlock_to_KSON.fill (false, m_params.data_len*8);

    // 2. В цикле for () будем проходить по всем номерам битов (от 0 до m_params.data_len*8 - 1)
    // из битового массива "m_bitsInformBlock_to_KSON". Данный массив битов представляет собой информационный
    // блок информационного кадра от АПАК к КСОН. В переменной цикла "bitNumberInInformBlock" будем
    // хранить текущий номер бита массива "m_bitsInformBlock_to_KSON".
    for (quint16 bitNumberInInformBlock = 0; bitNumberInInformBlock <= m_params.data_len*8 - 1;
         bitNumberInInformBlock++)
    {
        // 2.1. Проверим, должен ли по индексу "bitNumberInInformBlock" в массиве "m_bitsInformBlock_to_KSON" хранится
        // бит какого-либо сигнала:
        if (m_signal_to_KSON_by_bitNumberInInformBlock.contains(bitNumberInInformBlock) == false)
        { // Если не должен, то оставляем соответствующий бит массива "m_bitsInformBlock_to_KSON" нулевым
            //(переходим на следующую итерацию цикла):

            continue;
        }

        // 2.2. Если по индексу "bitNumberInInformBlock" в массиве "m_bitsInformBlock_to_KSON" должен храниться
        // бит какого-либо сигнала, то:
        // Получаем указатель на сигнал, один из битов которого должен храниться в массиве
        // "m_bitsInformBlock_to_KSON" по индексу "bitNumberInInformBlock":
        modus::SvSignal* signal = m_signal_to_KSON_by_bitNumberInInformBlock.value( bitNumberInInformBlock);

        // 2.3. Проверяем актуальна ли информация, содержащаяся в сигнале:
        if(!signal->value().isValid() || signal->value().isNull())
        {
            // Информация в сигнале не актуальна -> оставляем соответствующий бит массива
            // "m_bitsInformBlock_to_KSON" нулевым (переходим на следующую итерацию цикла):

            continue;
        }

        bool ok;

        // 2.4. Теперь проверим представима ли информация в сигнале беззнаковым целым числом:
        quint8 intValueSignal = signal->value().toUInt(&ok);
        if(ok == false )
        {
            // Информация в сигнале не представима беззнаковым целым числом ->
            // оставляем соответствующий бит массива "m_bitsInformBlock_to_KSON" нулевым
            // (переходим на следующую итерацию цикла):

            continue;
        }

        // 2.5. Получаем номер бита сигнала, который должен храниться в массиве бит
        // "m_bitsInformBlock_to_KSON" по индексу "bitNumberInInformBlock_to_KSON":
        quint8 m_bitNumberInSignal =
                m_bitNumberInSignal_to_KSON_by_bitNumberInInformBlock.value (bitNumberInInformBlock);

        // 2.6. Получаем и сохраняем бит сигнала в массиве битов "m_bitsInformBlock_to_KSON":
        m_bitsInformBlock_to_KSON.setBit(bitNumberInInformBlock,
                                  (intValueSignal>>m_bitNumberInSignal) & 0x01 ? true: false);
    }
     qDebug() << "Инфорационный блок информационного кадра от АПАК к КСОН: "<<m_bitsInformBlock_to_KSON;

     // 3. Теперь преобразуем массив бит "m_bitsInformBlock_to_KSON" в массив байт:
     // В этом массиве байт будет содержаться информационный блок информационного кадра от АПАК к КСОН.
     // 3.1. Для начала заполняем этот массив байт нулями:
     QByteArray informBlock (m_params.data_len, 0);

     // 3.2. Конвертируем данные из массива бит "m_bitsInformBlock_to_KSON" в массив байт "informBlock".
     // В переменной цикла "bitNumberInInformBlock" будем хранить текущий номер бита в
     // массиве "m_bitsInformBlock".
     for(quint16  bitNumberInInformBlock = 0; bitNumberInInformBlock <m_params.data_len * 8;
                bitNumberInInformBlock++)
            informBlock[bitNumberInInformBlock / 8] = informBlock.at(bitNumberInInformBlock / 8) |
                  ((m_bitsInformBlock_to_KSON [bitNumberInInformBlock] ? 1 : 0) << (bitNumberInInformBlock % 8));

     // 4. В переменной "m_send_data" формируем информационный кадр для передачи от АПАК к КСОН
     // в соответствии с протоколом обмена.

     // 4.1. Вначале, выделим память:
     m_send_data.fill( 0, 12);

     // 4.2. Формируем первое поле информационного кадра:
     // Первое поле (4 байта) - размер данных (4 + 4 + 2 + 2 = 12 байт): 0x0, 0x0, 0x0, 0х0c
     m_send_data [0] = 0;
     m_send_data [1] = 0;
     m_send_data [2] = 0;
     m_send_data [3] = 0x0c;

     // 4.3. Формируем второе поле информационного кадра:
     // Второе поле (4 байта) - время по данным внешней системы -
     // целое количество секунд с 1 янв. 1970 г.
     quint64  timeSince_1970 = QDateTime::currentMSecsSinceEpoch() / 1000;

     m_send_data [4] = (uint8_t) ((timeSince_1970 >> 24) & 0xFF);
     m_send_data [5] = (uint8_t) ((timeSince_1970 >> 16) & 0xFF);
     m_send_data [6] = (uint8_t) ((timeSince_1970 >>  8) & 0xFF);
     m_send_data [7] = (uint8_t) (timeSince_1970 & 0xFF);

     // 4.4. Занесём время формирования информационного кадра в переменную "m_packetTimeTo_KSON",
     // чтобы при получении пакета подтверждения проверить, что время в пакете подтверждения совпадает
     // c этим временем:
     m_packetTimeTo_KSON = timeSince_1970;

     // 4.5. Копируем поле доступности групп "relevanceGroup_Byte_to_KSON" длиной 2 байта в массив "m_send_data":
     m_send_data [8] = m_relevanceGroup_Byte_to_KSON [0];
     m_send_data [9] = m_relevanceGroup_Byte_to_KSON [1];

     // 4.6. Копируем массив байт "informBlock", длиной в 2 байта в пакет "m_send_data":
     m_send_data [10] = informBlock [0];
     m_send_data [11] = informBlock [1];

     qDebug() <<"Информационный кадр (в окончательном виде) от АПАК к КСОН:";
     qDebug() <<"Размер: " << m_send_data.length();
     qDebug() <<"Содержание: " << m_send_data.toHex();

     // 5. Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
     p_io_buffer->output->mutex.lock();
     p_io_buffer->output->setData(m_send_data);

     emit message(QString(m_send_data.toHex()), lldbg, sv::log::mtNew);

     p_io_buffer->output->setReady(true);
     emit p_io_buffer->readyWrite(p_io_buffer->output);

     p_io_buffer->output->mutex.unlock();

     // 6. Запускаем таймер подтверждения "m_conformTimer" с периодом,
     // равным предельно допустимому времени от посылки нами информационного кадра к сети КСОН,
     // до получения нами пакета подтверждения от сети КСОН. Это время
     // задаётся  в конфигурационном файле "config_apak.json", как параметр протокола устройства КСОН:
     m_conformTimer->start(m_params.conform_interval);
}


void apak::SvKsonPacket::packageFrom_KSON(modus::BUFF* buffer)
// В этой функции мы обрабатываем принятый от сети КСОН кадр. Этот кадр может быть двух типов -
// информационный кадр или пакет подтверждения. По значению поля "размер данных", принятого кадра, мы
// определяем тип принятого кадра (СЕЙЧАС определяем тип кадра по его ДЛИНЕ).
// Если принят информационный кадр -> мы копируем его в значение сигнала,
// на который указывает "m_data_signal" (сигнал данных от сети КСОН).
// Если принят пакет подтверждения, то останавливаем "таймер подтверждения m_confirmTimer",
// отсчитывающий время от посылки нами информиционного кадра КСОНу, до получения нами
// от сети КСОН пакета подтверждения. Также при этом мы запускаем "таймер посылки m_sendTimer".
{
  if(p_is_active) {

    buffer->mutex.lock();

    if(!buffer->isReady())
    { // Если принятых данных нет, то не в чем и разбираться (выходим из функции):
        return;
    }

    // Скопируем пришедший от КСОН пакет в массив "packageFrom_KSON":
    QByteArray packageFrom_KSON = QByteArray(buffer->data, buffer->offset);

    buffer->reset();
    buffer->mutex.unlock();

    // Разбираемся с принятым от сети КСОН пакетом:

    // Определим тип принятого кадра:
    unsigned length = packageFrom_KSON.length();

    if ( length == 43)
    {// Принят информационный кадр:

        // Останавливаем таймер приёма:
        m_receiveTimer->stop();

        // Очистим переменную "m_status", в которой мы формируем поле статуса пакета подтверждения от АПАК к КСОН:
        m_status = 0;

        if(m_data_signal)
        { // Если в файле конфигурации сигналов КСОН имеется "сигнал данных", то присваиваем ему
          // значение, пришедшего от сети КСОН информационного кадра:

            m_data_signal->setValue(QVariant(packageFrom_KSON));
            emit message(QString("signal %1 updated").arg(m_data_signal->config()->name), sv::log::llDebug, sv::log::mtParse);
        }

        // Переменная "packageFrom_KSON", содержит информационный кадр от КСОН.
        // Правильность информационного кадра в данной версии протокола можно проверить
        // по правильности содержимого поля "Размер данных", соответствию значений в
        // поле времени информационного пакета и в поле времени блока параметрических данных (в
        // случае, если поле времени в блоке параметрических данных - актуально),
        // и по допустимости значений параметрических данных (в случае их актуальности).

        // Получим значения полей пакета с помощью переменной  "packageFrom_KSON_Stream":
        QDataStream packageFrom_KSON_Stream (&packageFrom_KSON, QIODevice::ReadOnly);

        // Согласно протоколу числа типа "float" занимают 4 байта:
        packageFrom_KSON_Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        // Первое поле (4 байта) - размер данных. Он должен быть 4+4+2+33 = 43 байт
        quint64 dataSize;
        packageFrom_KSON_Stream >> dataSize;

        if (dataSize != 43)
        { // Если поле даннных содержит неверную информацию, то устанавливаем флаг ошибки структуры
          // информационного кадра в переменной "status":

            m_status |= INFO_FRAME_ERROR;

            qDebug () << QString("Поле данных в информационном кадре от КСОН содержит неверную информацию: %1").arg(dataSize);
            emit message(QString("Поле данных в информационном кадре от КСОН содержит неверную информацию: %1").arg(dataSize), sv::log::llError, sv::log::mtError);
        }

        // Второе поле (4 байта) - время по данным внешней системы -
        // целое количество секунд с 1 янв. 1970 г.
        quint32  timeSince_1970;
        packageFrom_KSON_Stream >> timeSince_1970;

        // Сохраним это время в переменной "m_packetTimeFrom_KSON", чтобы использовать его
        // в пакете подтверждения на этот кадр:
        m_packetTimeFrom_KSON = timeSince_1970;

        // Третье поле (2 байта) - доступность групп.
        quint16 groupAvailability;
        packageFrom_KSON_Stream >> groupAvailability;

        // Переведём поле "groupAvailability" в битовый массив "m_relevance_by_group_from_KSON"
        // (пока считаем, что группы нумеруются с нуля):
        for (quint8 bitNumber = 0; bitNumber <16; bitNumber++)
        {
            if (((groupAvailability >> bitNumber) & 0x0001) == 0x0001)
                m_relevance_by_group_from_KSON [bitNumber] = true;
            else
                m_relevance_by_group_from_KSON [bitNumber] = false;
        }

        // Дальнейшие поля - это поля параметрических данных.
        // Разбираем на отдельные поля информационный блок кадра, принятого нами:
        for (quint8 signalNumberInInformBlock = 0; signalNumberInInformBlock < m_signals_from_KSON.size();
                 signalNumberInInformBlock++)
        { // Проходим по всем номерам параметров (сигналов) в информационном блоке:

            // Получаем указатель на сигнал:
            modus::SvSignal* signal = m_signals_from_KSON [signalNumberInInformBlock];

            // Получаем параметры сигнала:
            apak::SignalParams signal_params = m_params_by_signal_from_KSON.value (signal);

            // Выясним тип сигнала ("boolean", "unsigned", "false") и считаем из потока
            // значение cигнала.

            // Переменная для получения значения сигнала булевского типа:
            bool booleanSignal;

            // Переменная для получения значения сигнала типа "unsigned":
            quint32 unsignedSignal;

            // Переменная для получения значения сигнала типа "float":
            float floatSignal;

            // Получаем тип сигнала:
            QString type = signal_params.data_type;

            // Считываем из потока значение сигнала:
            if ( type.compare ("boolean", Qt::CaseInsensitive) == 0)
            { // Этот сигнал представляет тип "boolean" -> сигнал занимает 1 бит.

                packageFrom_KSON_Stream >> booleanSignal;
            }

            if ( type.compare ("float", Qt::CaseInsensitive) == 0)
            { // Этот сигнал представляет тип "float" и занимает 4 байта, порядок которых: big-endian

                packageFrom_KSON_Stream >> floatSignal;
            }

            if ( type.compare ("unsigned", Qt::CaseInsensitive) == 0)
            { // Этот сигнал представляет тип "unsigned" и занимает 4 байта, порядок которых: big-endian.

                packageFrom_KSON_Stream >> unsignedSignal;
            }

            // 3.3. Определим к какой группе принадлежит сигнал:
            quint8 group = signal_params.group;

            if (m_relevance_by_group_from_KSON [group] == false)
            { // Если сигналы группы не актуальны -> незачем проверять допустимость их значений:

                continue;
            }

            // Если значение сигнала актуально -> проверяем допустимость его значения:

            // Сначала из параметров сигнала получим его минимальное и максимальное значения:
            float min = signal_params.min;
            float max = signal_params.max;

            if ( type.compare ("boolean", Qt::CaseInsensitive) == 0)
            { // Тип сигнала "boolean"

                if( booleanSignal != 0 && booleanSignal != 1)
                { // Если значение булевского сигнала отлично от "0" и от "1" ->
                    //фиксируем ошибку блока параметрической информации:

                    m_status |= PARAMETRIC_INFOBLOCK_ERROR;

                    qDebug() << QString("Ошибка диапазона параметрического данного: значение %1, тип %2, минимум %3, максимум %4").arg (booleanSignal).arg(type).arg(min).arg(max);
                    emit message(QString("Ошибка диапазона параметрического данного: значение %1, тип %2, минимум %3, максимум %4").arg (booleanSignal).arg(type).arg(min).arg(max), sv::log::llError, sv::log::mtError);
                }
            }

            if ( type.compare ("float", Qt::CaseInsensitive) == 0)
            { // Этот сигнал представляет тип "float"

                if ( floatSignal < min || floatSignal > max)
                { // Если значение сигнала типа "float" не укладывается в указанные
                    // в протоколе пределы -> фиксируем ошибку блока параметрической информации:

                    m_status |= PARAMETRIC_INFOBLOCK_ERROR;

                    qDebug() << QString("Ошибка диапазона параметрического данного: значение %1, тип %2, минимум %3, максимум %4").arg (floatSignal).arg(type).arg(min).arg(max);
                    emit message(QString("Ошибка диапазона параметрического данного: значение %1, тип %2, минимум %3, максимум %4").arg (floatSignal).arg(type).arg(min).arg(max), sv::log::llError, sv::log::mtError);

                }
            }

            if ( type.compare ("unsigned", Qt::CaseInsensitive) == 0)
            { // Этот сигнал представляет тип "unsigned"

                if (unsignedSignal < min || unsignedSignal > max)
                {// Если значение сигнала типа "unsigned" не укладывается в указанные
                    // в протоколе пределы -> фиксируем ошибку блока параметрической информации:

                    m_status |= PARAMETRIC_INFOBLOCK_ERROR;

                    qDebug() << QString("Ошибка диапазона параметрического данного: значение %1, тип %2, минимум %3, максимум %4").arg (unsignedSignal).arg(type).arg(min).arg(max);
                    emit message(QString("Ошибка диапазона параметрического данного: значение %1, тип %2, минимум %3, максимум %4").arg (unsignedSignal).arg(type).arg(min).arg(max), sv::log::llError, sv::log::mtError);
                }
            }

            if (signal == m_time_signal)
            { // "Это сигнал времени". Если мы оказались в этой точке программы, то это
                // значит, что он актуальный и его значение находится в допустимых пределах ->
                // Проводим проверку на соответствие значения времени в поле времени
                // в информационном пакете (значение сигнала) и значения в поле времени в
                // блоке параметрической информации (переменная "timeSince_1970"):
                if (unsignedSignal == timeSince_1970)
                { // Если значения совпадают -> устанавливаем значение "сигнала времени":

                     m_time_signal->setValue(timeSince_1970);
                }
                else
                {
                    qDebug () << QString("Время в поле времени информационного кадра: %1 не совпадает с временем в поле параметрических данных: %2").arg(timeSince_1970).arg(unsignedSignal);
                    emit message(QString("Время в поле времени информационного кадра: %1 не совпадает с временем в поле параметрических данных: %2").arg(timeSince_1970).arg(unsignedSignal), sv::log::llError, sv::log::mtError);

                }
            } // if (signal == m_time_signal)
        } // Проходим по всем номерам параметров (сигналов) в информационном блоке:


        if (m_status != 0)
        { // Если при разборе информационного кадра от КСОН мы обнаружили ошибку -> увеличиваем
            // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:

            m_interactionErrorCounter++;

            qDebug() << QString("При разборе информационного кадра от КСОН обнаружили ошибку. Количество идущих подряд ошибок взаимодействия : %1").arg(m_interactionErrorCounter);
            emit message(QString("При разборе информационного кадра от КСОН обнаружили ошибку. Количество идущих подряд ошибок взаимодействия : %1").arg(m_interactionErrorCounter), sv::log::llError, sv::log::mtError);

        }
        else
        { // Если при разборе информационного кадра от КСОН мы НЕ обнаружили ошибкок -> сбрасываем
          // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН.

            m_interactionErrorCounter = 0;
        }

        // Сформируем и пошлём пакет подтверждения:
        sendConfirmationPackage();
    } // if ( lenght == 43)

    if ( length == 9)
    { // Принят пакет подтверждения:

        // Останавливаем таймер подтверждения:
        m_conformTimer->stop();

        // Переменная "packageFrom_KSON", содержит пакет подтверждения от КСОН.
        // Правильность пакета подтверждения в данной версии протокола можно проверить
        // по правильности содержимого поля "Размер данных", правильности значения поля времени
        // (оно должно повторять значение времени в информационном кадре от системы АПАК к
        // сети КСОН) и допустимости значений в поле статуса.

        // Переменная "confirmationPackageErrorFlag" - это флаг ошибки пакета подтверждения.
        // Устанавливается, если поле статуса пакета подтверждения
        // содержит значение, говорящее об ошибке в информационном пакете от АПАК к КСОН
        // или имеются любые ошибки в самом пакете подтверждения.

        //  Перед началом анализа пакета подтверждения сбросим флаг ошибки:
        bool confirmationPackageErrorFlag = false;

        // Получим значения полей пакета с помощью переменной  "packageFrom_KSON_Stream":
        QDataStream packageFrom_KSON_Stream (&packageFrom_KSON, QIODevice::ReadOnly);

        // Первое поле (4 байта) - размер данных. Он должен быть 4+4+1 = 9 байт
        quint64 dataSize;
        packageFrom_KSON_Stream >> dataSize;

        if (dataSize != 9)
        { // Если поле данных содержит неверную информацию, то
          // выставляем флаг ошибки пакета подтвердения:

            qDebug () << QString("Поле данных в пакете подтверждения от КСОН содержит неверную информацию: %1").arg(dataSize);
            emit message(QString("Поле данных в пакете подтверждения от КСОН содержит неверную информацию: %1").arg(dataSize), sv::log::llError, sv::log::mtError);

            confirmationPackageErrorFlag = true;
        }

        // Второе поле (4 байта) - время, информационного кадра от АПАК к КСОН.
        quint32  timeSince_1970;
        packageFrom_KSON_Stream >> timeSince_1970;

        if (timeSince_1970 !=  m_packetTimeTo_KSON)
        { // Если поля времени в информационном кадре от АПАК к КСОН и в пакете подтверждения
          // от КСОН к АПАК не совпадают, то выставляем флаг ошибки пакета подтвердения:

            qDebug () << QString("Время в пакете подтверждения от КСОН: %1 не совпадает с временем в информационном кадре: %2").arg(timeSince_1970).arg(m_packetTimeTo_KSON);
            emit message(QString("Время в пакете подтверждения от КСОН: %1 не совпадает с временем в информационном кадре: %2").arg(timeSince_1970).arg(m_packetTimeTo_KSON), sv::log::llError, sv::log::mtError);

            confirmationPackageErrorFlag = true;
        }

        // Третье поле (1 байт) - статус.
        quint8 statusFromPacket;
        packageFrom_KSON_Stream >> statusFromPacket;
        if (statusFromPacket != 0)
        { // Если значение в поле статуса отлично от 0, то выставляем флаг ошибки пакета подтвердения:

            confirmationPackageErrorFlag = true;

            qDebug () << QString("Значение в поле статуса: %1 отлично от нуля").arg(statusFromPacket);
            emit message(QString("Значение в поле статуса: %1 отлично от нуля").arg(statusFromPacket), sv::log::llError, sv::log::mtError);

        }

        if ( confirmationPackageErrorFlag == true)
        { // Если при разборе пакета подтверждения мы обнаружили ошибку -> увеличиваем
            // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:

            m_interactionErrorCounter++;

            qDebug() << QString("При разборе пакета подтверждения обнаружили ошибку. Количество идущих подряд ошибок взаимодействия : %1").arg(m_interactionErrorCounter);
            emit message(QString("При разборе пакета подтверждения обнаружили ошибку. Количество идущих подряд ошибок взаимодействия : %1").arg(m_interactionErrorCounter), sv::log::llError, sv::log::mtError);
        }
        else
        { //Если при разборе пакета подтверждения мы НЕ обнаружили ошибок -> сбрасываем
            // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:

            m_interactionErrorCounter = 0;
        }

        // Запускаем на единственное срабатывание "таймер посылки m_sendTimer" отсчитывающий
        // период посылки информационных кадров от системы АПАК к сети КСОНс периодом,
        // равным интервалу между информационными кадрами,
        // заданному  в конфигурационном файле "config_apak.json", как параметр протокола устройства КСОН.
        // По срабатыванию этого таймера будет вызвана функция "send", для посылки информационного кадра к
        // сети КСОН.
        m_sendTimer->start(m_params.send_interval);
    } // if ( length == 9)

    if ( length != 9 || length != 43)
    { // Если размер принятого кадра != 9 (размер пакета подтверждения) и
        // размер принятого кадра != 43 (размер информационного кадра)

        // Выдаём сообщение об ошибке:
        emit message(QString("Длина принятого от КСОН сообщения равна %1").arg(buffer->offset), sv::log::llError, sv::log::mtError);
        qDebug() << QString("Длина принятого от КСОН сообщения равна %1").arg(buffer->offset);

        // Увеличиваем счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:
        m_interactionErrorCounter++;
    } // if ( length != 9 || lenght != 43)

    if (m_interactionErrorCounter >= m_params.numberOfErrors)
    { // Если счётчик подряд идущих ошибок взаимодействия АПАК с КСОН больше значения
      // указанного в протоколе, то испускаем для интерфейсной части сигнал "say"
      // с командой "breakConnection", который приказывает интерфейсной части разорвать
      // соединение с сервером КСОН:
        qDebug() << QString ("Выдаём сигнал на разрыв соединения с TCP-сервером");
        emit message(QString("Выдаём сигнал на разрыв соединения с TCP-сервером"), sv::log::llError, sv::log::mtError);

        emit p_io_buffer->say("breakConnection");
    }

    if(m_state_signal)
    {// Если в файле конфигурации сигналов КСОН имеется "сигнал состояния", то, по приходу ЛЮБОГО
     // (правильного или неправильного) информационного кадра или пакета подтверждения от
      // сети КСОН устанавливаем его значение в 1:

       m_state_signal->setValue(int(1));
    }
    // p_last_parsed_time = QDateTime::currentDateTime();
  }
}


void apak::SvKsonPacket::noConfirmationPackage(void)
// Эта функция вызывается в том случае, если в течении предельно допустимого времени от
// посылки нами информационного кадра к сети КСОН, до получения нами пакета подтверждения от сети КСОН
// пакет подтверждения так и не пришёл.
{
    // 1. Выводим в утилиту "logview" и на консоль информацию о том, что пакет
    // подтверждения не получен:
    emit message(QString("Пакет подтверждения от КСОН за время: %1 не получен").arg(m_params.conform_interval), sv::log::llError, sv::log::mtError);
    qDebug() << "Пакет подтверждения от КСОН не получен";

    // 2. Увеличиваем счётчик подряд идущих ошибок взаимодействия АПАК с КСОН.
    m_interactionErrorCounter++;

    // 3. Если счётчик подряд идущих ошибок взаимодействия АПАК с КСОН больше значения
    // указанного в протоколе, то испускаем для интерфейсной части сигнал "say"
    // с командой "breakConnection", который приказывает интерфейсной части разорвать
    // соединение с сервером КСОН:
    if (m_interactionErrorCounter >= m_params.numberOfErrors)
    {
        emit p_io_buffer->say("breakConnection");
    }

    // 4. Даже если пакета подтверждения не пришло в течении оговоренного в протоколе времени
    // (m_params.conform_interval), то мы всё равно запускаем на единственное срабатывание
    // "таймер посылки m_sendTimer", чтобы продолжать посылать информационные кадры к КСОН:
    m_sendTimer->start(m_params.send_interval);
}


void apak::SvKsonPacket::noReceivePackage(void)
// Эта функция вызывается в том случае, если в течении предельно допустимого времени от
// сети КСОН не пришёл инфомационный кадр.
{
    // 1. Выводим в утилиту "logview" и на консоль информацию о том, что информационный кадр от КСОН
    // не получен:
    emit message(QString("Информационный кадр от КСОН за время: %1 не получен").arg(m_params.receive_interval), sv::log::llError, sv::log::mtError);
    qDebug() << "Информационный кадр от КСОН не получен";

    // 2. Увеличиваем счётчик подряд идущих ошибок взаимодействия АПАК с КСОН.
    m_interactionErrorCounter++;

    // 3. Если счётчик подряд идущих ошибок взаимодействия АПАК с КСОН больше значения
    // указанного в протоколе, то испускаем для интерфейсной части сигнал "say"
    // с командой "breakConnection", который приказывает интерфейсной части разорвать
    // соединение с сервером КСОН:
    if (m_interactionErrorCounter >= m_params.numberOfErrors)
    {
        emit p_io_buffer->say("breakConnection");
    }

    // 4. Даже если инфомационного кадра не пришло в течении оговоренного в протоколе времени
    // (m_params.receive_interval), то мы всё равно запускаем на единственное срабатывание
    // "таймер приёма m_receiveTimer", чтобы продолжать отображать в утилите "logview" и
    // на консоли факт отсутствия информационных кадров от КСОН:
    m_receiveTimer->start(m_params.receive_interval);
}

void apak::SvKsonPacket::sendConfirmationPackage(void)
// Формирование и посылка пакета подтверждения.
// На данный момент - формат пакета подтверждения не ясен, поэтому будем реализовывать следующий
// формат:
// Размер данных - 4 байта
// Время - 4 байта
// Статус - 1 байт
{
    // В переменной "m_send_data" формируем пакет подтверждения для передачи от АПАК к КСОН
    // в соответствии с протоколом обмена.

    // 1. Вначале, выделим память:
    m_send_data.fill( 0, 9);

    // 2. Формируем первое поле информационного кадра:
    // Первое поле (4 байта) - размер данных (4 + 4 + 1 = 9 байт): 0x0, 0x0, 0x0, 0х09
    m_send_data [0] = 0;
    m_send_data [1] = 0;
    m_send_data [2] = 0;
    m_send_data [3] = 0x09;

    // 3. Формируем второе поле информационного кадра - время, которое должно повторять
    // время в информационном кадре от КСОН, сохранённое нами в переменной "m_packetTimeFrom_KSON":

    m_send_data [4] = (uint8_t) ((m_packetTimeFrom_KSON >> 24) & 0xFF);
    m_send_data [5] = (uint8_t) ((m_packetTimeFrom_KSON >> 16) & 0xFF);
    m_send_data [6] = (uint8_t) ((m_packetTimeFrom_KSON >>  8) & 0xFF);
    m_send_data [7] = (uint8_t) (m_packetTimeFrom_KSON & 0xFF);

    // 4. Записываем третье поле пакета подтверждения - статус. Оно сформировано нами в процессе
    // разбора информационного кадра от КСОН:
    m_send_data [8] = m_status;


    qDebug() <<"Пакет подтверждения (в окончательном виде) от АПАК к КСОН:";
    qDebug() <<"Размер: " << m_send_data.length();
    qDebug() <<"Содержание: " << m_send_data.toHex();

    // 5. Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
    p_io_buffer->output->mutex.lock();
    p_io_buffer->output->setData(m_send_data);

    emit message(QString(m_send_data.toHex()), lldbg, sv::log::mtNew);

    p_io_buffer->output->setReady(true);
    emit p_io_buffer->readyWrite(p_io_buffer->output);

    p_io_buffer->output->mutex.unlock();
}




/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvKsonPacket();
  return protocol;
}

