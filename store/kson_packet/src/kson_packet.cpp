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
    // Очистим словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsInformBlock" ставит в соответствие сигнал, бит которого должен быть записан
    // в массив "m_bitsInformBlock":
    m_signal_by_bitNumberInInformBlock.clear();

    // Очистим словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsInformBlock" ставит в соответствие номер бита сигнала, который
    // должен быть записан в массив "m_bitsInformBlock":
    m_bitNumberInSignal_by_bitNumberInInformBlock.clear();

    // Очистим битовый массив, который для каждого номера группы сигналов от АПАК к КСОН
    // (номер - это индекс в этом массиве),
    // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны):
    m_relevance_by_group.clear();
    m_relevance_by_group.resize(16);

    // Установим актуальность всех сигналов от АПАК к КСОН (группы от 0 до 9):
    for (int numberGr = 0; numberGr <= 9; numberGr++ )
        m_relevance_by_group[numberGr] = true;
    for (int numberGr = 10; numberGr <= 12; numberGr++ )
        m_relevance_by_group[numberGr] = false;)

    // Конвертируем данные из массива бит "m_relevance_by_group" в массив байт
    // "relevanceGroup_Byte".
    // Для начала заполняем массив байт "relevanceGroup_Byte" нулями:
    m_relevanceGroup_Byte.clear();
    m_relevanceGroup_Byte.resize (2);

    // В переменной цикла "bitNumberIn_relevance_by_group" будем хранить текущий номер бита в
    // массиве "relevanceGroup_Byte".
    for(quint16  bitNumberIn_relevance_by_group = 0; bitNumberIn_relevance_by_group <16;
                           bitNumberIn_relevance_by_group++)
    {
        m_relevanceGroup_Byte [bitNumberIn_relevance_by_group / 8] = relevanceGroup_Byte.at(bitNumberIn_relevance_by_group / 8) |
                             ((m_relevanceConcrete_by_group [bitNumberIn_relevance_by_group] ? 1 : 0) << (bitNumberIn_relevance_by_group % 8));
    }
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
// кадр от КСОН, а не только блок параметрической информации из этого кадра. Сигнал описывается
// в файле: "data_KSON.json".
// 2. "Сигнал состояния cопряжения сети КСОН с системой АПАК" (0 - данные от КСОН к АПАК НЕ поступают/
// 1 - данные от КСОН к АПАК ПОСТУПАЮТ). Мы устанавливаем этот сигнал в "1" по приходу пакета от КСОН.
// Сигнал описывается в файле "state_KSON.json".
// 3. "Сигнал времени" - сигнал содержит время в формате UTC (число секунд с 01.01.1970).
// Его значение устанавливается нами из первого поля блока параметрической информации,
// информационного кадра от КСОН к АПАК.
// Сигнал описывается в файле: "time_KSON.json".
// 4. Сигналы от системы АПАК к сети КСОН. Это сигналы о состоянии ПУ1 и ПУ2 системы АПАК и о состоянии
// сопряжения АПАК с другими устройствами (СКМ, ИВ-1, КРАБ...)
// Сигналы описываются в файле: "to_KSON.json".

// Цель этой функции состоит в получении всех параметров для каждого сигнала и заполнении в соответствии
// с ними структур данных (словари m_signal_by_bitNumberInInformBlock и
// m_bitNumberInSignal_by_bitNumberInInformBlock), необходимых для формирования
// информационного кадра от системы АПАК к сети КСОН.
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);


    if(r)
    {
        if(binding.mode == modus::Master)
        { // Три сигнала:
          // 1. данных, поступающих от КСОН
          // 2. состояния cопряжения сети КСОН с системой АПАК
          // 3. сигнал времени от сети КСОН
          // - имеют привязку "master".
          // Параметры сигналов "данные, поступающие от КСОН" и "состояние сопряжения сети КСОН с системой АПАК"
          // библиотекой "libapak_kson_imitator" не используются.

          // В "сигнале времени от сети КСОН" нас будет интересовать только один параметр - номер группы.
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
                // Мы устанавливаем этот сигнал из поля времени блока параметрической информации
                // информации информационного кадра от КСОН.
                // Сигнал описывается в файле "time_KSON.json", но тип сигнала ("type": "TIME") может
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
                // смещением байта, в котором хранится значение сигнала, от начала информацинного
                // блока информационного кадра от КСОН к АПАК (параметр "byte"), смещением области битов,
                // в которой хранится значение сигнала, от начала байта (количество бит от начала байта)
                // (параметр "оffset"), размером области бит, которая хранит значение сигнала (количество бит),
                // в информационном блоке информационного кадра от AПАК к КСОН (параметр "len"), номер  группы
                // сигналов, к которой принадлежит сигнал (параметр "group").

                // В ранних версиях конфигурационных файлов сигналов параметры сигналов описывались в подразделе "params"
                // раздела "bindings". Однако позднее подраздел "params" был перенесён из раздела "bindings"
                // на уровень выше - прямо в общий раздел сигнала (туда же где находятся: идентификатор сигнала,
                // имя сигнала, описание сигнала...). Поэтому аргументом функции SignalParams::fromJson()
                // является "signal->config()->params".
                apak::SignalParams signal_params = apak::SignalParams::fromJson(signal->config()->params);

                // Присваиваем указателю "m_time_signal" указатель на "сигнал времени":
                m_time_signal = signal;
            } // Если тип сигнала - "TIME"

      } // if(binding.mode == modus::Master)
      else
      { // Сигналы, поступающие от АПАК к КСОН, которые имеют привязку "bindings".

            // Заполняем структуру "SignalParams" параметрами конкретного входящего сигнала устройства КСОН:
            // Параметры входящих сигналов это:
            // смещение байта, в котором хранится значение сигнала, от начала информацинного
            // блока информационного кадра от АПАК к КСОН (параметр "byte"), смещением области битов,
            // в которой хранится значение сигнала, от начала байта (количество бит от начала байта)
            // (параметр "оffset"), размером области бит, которая хранит значение сигнала (количество бит),
            // в информационном блоке информационного кадра от AПАК к КСОН (параметр "len").

            // В ранних версиях конфигурационных файлов сигналов параметры сигналов описывались в подразделе "params"
            // раздела "bindings". Однако позднее подраздел "params" был перенесён из раздела "bindings"
            // на уровень выше - прямо в общий раздел сигнала (туда же где находятся: идентификатор сигнала,
            // имя сигнала, описание сигнала...). Поэтому аргументом функции SignalParams::fromJson()
            // является "signal->config()->params".
            apak::SignalParams signal_params = apak::SignalParams::fromJson(signal->config()->params);


            // Заполняем соответствующие сигналу, на который указывает аргумент "signal"
            // функции  "bindSignal", поля в структурах данных "m_signal_by_bitNumberInInformBlock"
            // и "m_bitNumberInSignal_by_bitNumberInInformBlock".

            // В цикле for () будем проходить по всем номерам битов сигнала (от 0 до "len").
            // В переменной цикла "bitNumberInSignal" будем хранить текущий номер бита сигнала.
            for (uint8_t bitNumberInSignal = 0; bitNumberInSignal < signal_params.len; bitNumberInSignal++)
            {
                m_signal_by_bitNumberInInformBlock [(uint16_t)signal_params.byte * 8 + signal_params.offset +
                        bitNumberInSignal] = signal;

                m_bitNumberInSignal_by_bitNumberInInformBlock [(uint16_t)signal_params.byte * 8 +
                        signal_params.offset + bitNumberInSignal] = bitNumberInSignal;
            }
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

void apak::SvKsonPacket::start()
// В этой функции мы осуществляем:
// 1. Привязку вызова функции "send", в которой мы
// формируем информационный кадр от системы АПАК к сети КСОН, к наступлению таймаута
// "таймера посылки m_sendTimer". Таймер запускаем на ЕДИНСТВЕННОЕ срабатывание. Далее будем запускать
// этот таймер каждый раз после получения пакета подтверждения от сети КСОН.
// 2. Привязку вызова функции "packageFrom_KSON", к сигналу "modus::IOBuffer::dataReaded". Этот сигнал
// испускается интерфейсной частью по приходу данных от интерефейса (в нашем случае - от сети КСОН).
// 3. Сброс счётчика ошибок взаимодействия КСОН и АПАК
{
    // Привязываем вызов функции "send" к наступлению таймаута
    // "таймера посылки m_sendTimer":
    m_sendTimer = new QTimer;
    m_sendTimer->setSingleShot(true);
    connect(m_sendTimer, &QTimer::timeout, this, &SvKsonPacket::send);

    // Запускаем "таймер посылки m_sendTimer" с периодом, равным интервалу между информационными кадрами,
    // заданному  в конфигурационном файле "config_apak.json", как параметр протокола устройства КСОН:
    m_sendTimer->start(m_params.send_interval);

    // Привязываем вызов функции "packageFrom_KSON", к сигналу "modus::IOBuffer::dataReaded". Этот сигнал
    // испускается интерфейсной частью по приходу данных от интерефейса (в нашем случае - от сети КСОН).
    // В этой функции мы обрабатываем принятый от сети КСОН информационный кадр или пакет подтверждения.
    connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &SvKsonPacket::packageFrom_KSON);

    // Сброс счётчика ошибок взаимодействия КСОН и АПАК:
    interactionErrorCounter = 0;

    p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvKsonPacket::send()
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

    // 1. В переменной "m_bitsInformBlock" будем формировать информационный блок информационного кадра
    // от АПАК к КСОН. Для начала заполняем его нулями:
    m_bitsInformBlock.fill (false, m_params.data_len*8);

    // 2. В цикле for () будем проходить по всем номерам битов (от 0 до m_params.data_len*8 - 1)
    // из битового массива "m_bitsInformBlock". Данный массив битов представляет собой информационный
    // блок информационного кадра от АПАК к КСОН. В переменной цикла "bitNumberInInformBlock" будем
    // хранить текущий номер бита массива "m_bitsInformBlock".
    for (quint16 bitNumberInInformBlock = 0; bitNumberInInformBlock <= m_params.data_len*8 - 1;
         bitNumberInInformBlock++)
    {
        // 2.1. Проверим, должен ли по индексу "bitNumberInInformBlock" в массиве "m_bitsInformBlock" хранится
        // бит какого-либо сигнала:
        if (m_signal_by_bitNumberInInformBlock.contains(bitNumberInInformBlock) == false)
        { // Если не должен, то оставляем соответствующий бит массива "m_bitsInformBlock" нулевым
            //(переходим на следующую итерацию цикла):

            continue;
        }

        // 2.2. Если по индексу "bitNumberInInformBlock" в массиве "m_bitsInformBlock" должен храниться
        // бит какого-либо сигнала, то:
        // Получаем указатель на сигнал, один из битов которого должен храниться в массиве
        // "m_bitsInformBlock" по индексу "bitNumberInInformBlock":
        modus::SvSignal* signal = m_signal_by_bitNumberInInformBlock.value( bitNumberInInformBlock);

        // 2.3. Проверяем актуальна ли информация, содержащаяся в сигнале:
        if(!signal->value().isValid() || signal->value().isNull())
        {
            // Информация в сигнале не актуальна -> оставляем соответствующий бит массива
            // "m_bitsInformBlock" нулевым (переходим на следующую итерацию цикла):

            continue;
        }

        bool ok;

        // 2.4. Теперь проверим представима ли информация в сигнале беззнаковым целым числом:
        quint8 intValueSignal = signal->value().toUInt(&ok);
        if(ok == false )
        {
            // Информация в сигнале не представима беззнаковым целым числом ->
            // оставляем соответствующий бит массива "m_bitsInformBlock" нулевым
            // (переходим на следующую итерацию цикла):

            continue;
        }

        // 2.5. Получаем номер бита сигнала, который должен храниться в массиве бит
        // "m_bitsInformBlock" по индексу "bitNumberInInformBlock":
        quint8 m_bitNumberInSignal =
                m_bitNumberInSignal_by_bitNumberInInformBlock.value (bitNumberInInformBlock);

        // 2.6. Получаем и сохраняем бит сигнала в массиве битов "m_bitsInformBlock":
        m_bitsInformBlock.setBit(bitNumberInInformBlock,
                                  (intValueSignal>>m_bitNumberInSignal) & 0x01 ? true: false);
    }

    qDebug() << m_bitsInformBlock;
     // 3. Теперь преобразуем массив бит "m_bitsInformBlock" в массив байт:
     // В этом массиве байт будет содержаться информационный блок информационного кадра от АПАК к КСОН.
     // 3.1. Для начала заполняем этот массив байт нулями:
     QByteArray informBlock (m_params.data_len, 0);

     // 3.2. Конвертируем данные из массива бит "m_bitsInformBlock" в массив байт "informBlock".
     // В переменной цикла "bitNumberInInformBlock" будем хранить текущий номер бита в
     // массиве "m_bitsInformBlock".
     for(quint16  bitNumberInInformBlock = 0; bitNumberInInformBlock <m_params.data_len * 8;
                bitNumberInInformBlock++)
            informBlock[bitNumberInInformBlock / 8] = informBlock.at(bitNumberInInformBlock / 8) |
                  ((m_bitsInformBlock [bitNumberInInformBlock] ? 1 : 0) << (bitNumberInInformBlock % 8));

     // 4. В переменной "m_send_data" формируем информационный кадр для передачи от АПАК к КСОН
     // в соответствии с протоколом обмена.

     // В переменной "m_relevance_by_group" формируем битовый массив,
     // который для каждого номера группы сигналов (номер - это индекс в этом массиве),
     // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны).

     // Для АПАК выделены группы с 0 по 9. Мы всегда устанавливаем их АКТУАЛЬНЫМИ.
     m_relevanceConcrete_by_group = m_relevance_by_group;

     // В переменной "m_send_data" формируем информационный кадр от системы АПАК к сети КСОН
     // в  в соответствии с протоколом обмена.
     // Вначале, выделим память и заполним первые два поля:
     m_send_data.fill( 0, 4 + 4);

     // 1. Формируем первое поле информационного кадра:
     // Первое поле (4 байта) - размер данных (4 + 4 + 2 + 2 = 12 байт): 0x0, 0x0, 0x0, 0х0c
     m_send_data [0] = 0;
     m_send_data [1] = 0;
     m_send_data [2] = 0;
     m_send_data [3] = 0x0c;

     // 2. Формируем второе поле информационного кадра:
     // Второе поле (4 байта) - время по данным внешней системы -
     // целое количество секунд с 1 янв. 1970 г.
     quint64  timeSince_1970 = QDateTime::currentMSecsSinceEpoch() / 1000;

     m_send_data [4] = (uint8_t) ((timeSince_1970 >> 24) & 0xFF);
     m_send_data [5] = (uint8_t) ((timeSince_1970 >> 16) & 0xFF);
     m_send_data [6] = (uint8_t) ((timeSince_1970 >>  8) & 0xFF);
     m_send_data [7] = (uint8_t) (timeSince_1970 & 0xFF);

     // 3. Формируем поле "данные" информационного кадра. Это поле представляет собой
     // блок параметрической информации. Будем формировать его в переменной "dataFrame":
     QByteArray dataFrame;

     qDebug() <<"Пакет (в окончательном виде) от КРАБ:";
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

    if ( lenght == 43)
    {// Принят информационный кадр:

        // Очистим переменную "status", в которой мы формируем поле статуса пакета подтверждения:
        status = 0;

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

        // Первое поле (4 байта) - размер данных. Он должен быть 4+4+2+33 = 43 байт
        quint64 dataSize;
        packageFrom_KSON_Stream >> dataSize;

        if (dataSize != 43)
        { // Если поле даннных содержит неверную информацию, то устанавливаем флаг ошибки структуры
          // информационного кадра в переменной "status":

            status |= INFO_FRAME_ERROR;
        }

        // Второе поле (4 байта) - время по данным внешней системы -
        // целое количество секунд с 1 янв. 1970 г.
        quint32  timeSince_1970;
        packageFrom_KSON_Stream >> timeSince_1970;


        // Третье поле (2 байта) - доступность групп.
        quint32 groupAvailability;
        packageFrom_KSON_Stream >> groupAvailability;

        // Дальнейшие поля - это поля параметрических данных.
        // 1. Время в формате UTC, число секунд с 01.01.1970:
        quint32  timeSince_1970_Parametric;
        packageFrom_KSON_Stream >> timeSince_1970_Parametric;

        if (timeSince_1970 != timeSince_1970_Parametric)
        { // Если поля времени в информационном пакете и в блоке параметрической информации
          // не совпадают, то выставляем ошибку структуры информационного кадра и ошибку
          // блока параметрической информации:
            status |= INFO_FRAME_ERROR | PARAMETRIC_INFOBLOCK_ERROR;
        }

        // 2. Широта места:
        float locationLatitude;
        packageFrom_KSON_Stream >>locationLatitude;
        if (locationLatitude < -90 || locationLatitude > 90)
        { // Если широта места не укладываетя в диапазон: от -90 до 90 градусов, то выставляем
          // ошибку блока параметрической информации:
           status |= PARAMETRIC_INFOBLOCK_ERROR;
        }

        // 3. Долгота:
        float longitude;
        packageFrom_KSON_Stream >> longitude;
        if (longitude < -180 || longitude > 180)
        { // Если долгота не укладываетя в диапазон: от -180 до 180 градусов, то выставляем
          // ошибку блока параметрической информации:
           status |= PARAMETRIC_INFOBLOCK_ERROR;
        }

        // 4. Скорость корабля относительно воды:
        float shipSpeed;
        packageFrom_KSON_Stream >> shipSpeed;
        if (shipSpeed < 0 || shipSpeed > 50)
        { // Если скорость корабля относительно воды не укладываетя в диапазон:
          // от 0 до 50 узлов, то выставляем ошибку блока параметрической информации:
           status |= PARAMETRIC_INFOBLOCK_ERROR;
        }

        // 5. Курс корабля истинный:
        float shipCourse;
        packageFrom_KSON_Stream >> shipCourse;
        if (shipCourse < 0 || shipCourse > 359.0)
        { // Если истинный курс корабля не укладываетя в диапазон: от 0.0 до 359.0 градусов, то выставляем
          // ошибку блока параметрической информации:
           status |= PARAMETRIC_INFOBLOCK_ERROR;
        }

        // 6. Глубина под килем корабля:
        float depth;
        packageFrom_KSON_Stream >> depth;
        if (depth < 0 || depth > 999.9)
        { // Если глубина под килем корабля не укладываетя в диапазон: от 0.0 до 999.9 метров, то выставляем
          // ошибку блока параметрической информации:
           status |= PARAMETRIC_INFOBLOCK_ERROR;
        }

        // 7. Средняя скорость кажущегося ветра:
        float аverageWindSpeed;
        packageFrom_KSON_Stream >> аverageWindSpeed;
        if (аverageWindSpeed < 0 || аverageWindSpeed > 50.0)
        { // Если средняя скорость кажущегося ветра не укладывается в диапазон:
          // от 0,0 до 50.0 метров в секунду, то выставляем
          // ошибку блока параметрической информации:
           status |= PARAMETRIC_INFOBLOCK_ERROR;
        }

        // 8. Средний угол кажущегося ветра:
        float averageWindAngle;
        packageFrom_KSON_Stream >> averageWindAngle;
        if (аverageWindAngle < 0 || аverageWindAngle > 359.0)
        { // Если средний угол кажущегося ветра не укладывается в диапазон:
          // от 0,0 до 359.0 градусов, то выставляем
          // ошибку блока параметрической информации:
           status |= PARAMETRIC_INFOBLOCK_ERROR;
        }

        // 9. Состояние КСОН - Работа:
        float statusOf_KSON_Work;
        packageFrom_KSON_Stream >> statusOf_KSON_Work;

        if (status == 0)
        { // Если при разборе информационного кадра мы не обнаружили ошибок:
            if(m_time_signal)
            {// Если в файле конфигурации сигналов КСОН имеется "сигнал времени", то, по приходу
             // ПРАВИЛЬНОГО информационного кадра от сети КСОН, устанавливаем его значение в значение
             // времени, взятое из блока параметрической информации информационного кадра от сети КСОН:

               m_time_signal->setValue(timeSince_1970_Parametric);
            }
        }
        else
        { // Если при разборе информационного кадра мы обнаружили ошибку -> увеличиваем
            // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН.
            interactionErrorCounter++;
        }
    } // if ( lenght == 43)

    if ( length == 9)
    { // Принят пакет подтверждения:
      // Переменная "packageFrom_KSON", содержит пакет подтверждения от КСОН.
      // Правильность пакета подтверждения в данной версии протокола можно проверить
      // по правильности содержимого поля "Размер данных", правильности значения поля времени
      // (оно должно повторять значение времени в информационном кадре от системы АПАК к
      // сети КСОН) и допустимости значений в поле статуса.

        //  Сбросим флаг ошибки пакета подтверждения "confirmationPackageErrorFlag":
        confirmationPackageErrorFlag = false;

        // Получим значения полей пакета с помощью переменной  "packageFrom_KSON_Stream":
        QDataStream packageFrom_KSON_Stream (&packageFrom_KSON, QIODevice::ReadOnly);

        // Первое поле (4 байта) - размер данных. Он должен быть 4+4+1 = 9 байт
        quint64 dataSize;
        packageFrom_KSON_Stream >> dataSize;

        if (dataSize != 9)
        { // Если поле данных содержит неверную информацию, то
          // выставляем флаг ошибки пакета подтвердения:

            confirmationPackageErrorFlag = true;
        }

        // Второе поле (4 байта) - время, информационного кадра от АПАК к КСОН.
        quint32  timeSince_1970;
        packageFrom_KSON_Stream >> timeSince_1970;
        if (timeSince_1970 !=  packetTimeTo_KSON)
        { // Если поля времени в информационном кадре от АПАК к КСОН и в пакете подтверждения
          // от КСОН к АПАК не совпадают, то выставляем флаг ошибки пакета подтвердения:

            confirmationPackageErrorFlag = true;
        }

        // Третье поле (1 байт) - статус.
        quint8 statusFromPacket;
        packageFrom_KSON_Stream >> statusFromPacket;
        if (status != 0)
        { // Если значение в поле статуса отлично от 0, то выставляем флаг ошибки пакета подтвердения:

            confirmationPackageErrorFlag = true;
        }

        if ( confirmationPackageErrorFlag == true)
        { // Если при разборе пакета подтверждения мы обнаружили ошибку -> увеличиваем
            // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН.
            interactionErrorCounter++;
        }
    } // if ( length == 9)

    if ( length != 9 || lenght != 43)
    { // Если размер принятого кадра != 9 (размер пакета подтверждения) и
        // размер принятого кадра != 43 (размер информационного кадра)

        // Выдаём сообщение об ошибке:
        emit message(QString("Длина принятого от КСОН сообщения равна %1").arg(buffer->offset), sv::log::llError, sv::log::mtError);
        qDebug() << QString("Длина принятого от КСОН сообщения равна %1").arg(buffer->offset);

        // Увеличиваем счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:
        interactionErrorCounter++;
    } // if ( length != 9 || lenght != 43)

    if (interactionErrorCounter >= m_params.maxNumberOfCommunicationErrors)
    { // Если счётчик подряд идущих ошибок взаимодействия АПАК с КСОН больше значения
      // указанного в протоколе, то испускаем для интерфейсной части сигнал "say"
      // с командой "breakConnection", который приказывает интерфейсной части разорвать
      // соединение с сервером КСОН:
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


/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvKsonPacket();
  return protocol;
}

