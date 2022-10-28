#include "kson_imitator.h"

// Длина (в байтах) поля "размера данных" в информационном кадре и в пакете подтверждения:
#define DATA_SIZE_FIELD_LENGTH              4

// Длина (в байтах) поля "времени" в информационном кадре и в пакете подтверждения:
#define TIME_FIELD_LENGTH                   4

// Длина (в байтах) поля "доступности групп" в информационном кадре:
#define GROUP_AVAILABILITY_FIELD_LENGTH     2

// Длина (в байтах) поля "статуса" в пакете подтверждения:
#define STATUS_FIELD_LENGTH                 1

// Флаг ошибки структуры информационного кадра:
#define INFO_FRAME_ERROR            1

// Флаг ошибки структуры блока параметрической информации:
#define PARAMETRIC_INFOBLOCK_ERROR  2


apak::SvKsonImitator::SvKsonImitator():
  modus::SvAbstractProtocol()
{
    // === ИНИЦИАЛИЗИРУЕМ СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ ФОРМИРОВАНИЯ ИНФОРМАЦИОННОГО КАДРА ОТ имитатора КСОН К АПАК ====:
    // Очистим cловарь, который каждому смещению значения сигнала относительно начала информационного
    // блока, передаваемого из КСОН в АПАК, ставит в соответствие указатель на этот сигнал:
    m_signals_by_byte_to_APAK.clear();

    // Очистим список указателей на сигналы, отсортированный по возрастанию смещений
    // значений сигналов (в байтах) относительно начала информационного блока,
    // передаваемого из КСОН в АПАК.
    m_signals_to_APAK.clear();

    // Очистим словарь, который каждому сигналу ставит в соответствие структуру SignalParams,
    // хранящую параметры для этого сигнала:
    m_params_by_signal_to_APAK.clear();

    // Очистим битовый массив, который для каждого номера группы сигналов (номер - это индекс в этом массиве),
    // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны):
    m_relevance_by_group_to_APAK.clear();
    m_relevance_by_group_to_APAK.resize(GROUP_AVAILABILITY_FIELD_LENGTH * 8);

    // === ИНИЦИАЛИЗИРУЕМ СТРУКТУРЫ ДАННЫХ, НЕОБХОДИМЫЕ ДЛЯ РАЗБОРА ИНФОРМАЦИОННОГО КАДРА ОТ АПАК К имитатору КСОН ====:
    // Очистим словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsInformBlock_from_APAK" ставит в соответствие сигнал, бит которого записан
    // в массив "m_bitsInformBlock_from_APAK":
    m_signal_by_bitNumberInInformBlock_from_APAK.clear();

    // Очистим словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsInformBlock" ставит в соответствие номер бита сигнала, который
    // должен быть записан в массив "m_bitsInformBlock_from_APAK":
    m_bitNumberInSignal_by_bitNumberInInformBlock_from_APAK.clear();

    // Очистим битовый массив, который для каждого номера группы сигналов от АПАК к КСОН
    // (номер - это индекс в этом массиве),
    // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны):
    m_relevance_by_group_from_APAK.clear();
    m_relevance_by_group_from_APAK.resize(GROUP_AVAILABILITY_FIELD_LENGTH * 8);
}

apak::SvKsonImitator::~SvKsonImitator()
{
  deleteLater();
}

bool apak::SvKsonImitator::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
// Эта функция вызывается серевером "mdserver" для всех имитаторов устройств.
// Её цель - инициализировать все структуры, необходимые нам для конкретного
// имитатора (в данном случае, имитатора сети КСОН).
{
    try
    {

        p_config = config;
        p_io_buffer = iobuffer;

        // Заполняем структуру m_params параметром протокола обмена сети КСОН с системой АПАК
        // (периодом поступления данных в мс от КСОН в систему АПАК и размером поля данных
        // в информационном кадре от КCОН к АПАК):
        m_params = apak::ProtocolParams::fromJson(p_config->protocol.params);

        return true;
    }
    catch (SvException& e)
    {
        p_last_error = e.error;
        return false;
    }
}

bool apak::SvKsonImitator::bindSignal(modus::SvSignal *signal, modus::SignalBinding binding)
// Эта функция вызывается сервером "mdserver" для всех сигналов имитатора, связанных с сетью КСОН. Эти
// сигналы описываются в файле "kson.json".
// Перечислим эти сигналы:
// 1. Сигналы параметрической информации, отправляемые в информационном кадре от имитатора КСОН к АПАК.
//    Эти сигналы имеют привязку "binding".
// 2. Сигналы от системы АПАК к сети КСОН. Это сигналы о состоянии ПУ1 и ПУ2 системы АПАК и о состоянии
//    сопряжения АПАК с другими устройствами (СКМ, ИВ-1, КРАБ...)
//    Эти сигналы имеют привязку "master".
//    Эти сигналы нужны нам для возможности разбора значений этих сигналов с целью дальнейшего отображения
//     их значений на консоль и в утилите "logview".

// Цель этой функции состоит в получении всех параметров для каждого сигнала и заполнении в соответствии
// с ними структур данных (перечисленных в файле "kson_imitator.h"), необходимых для формирования информационного кадра от
// имитатора сети КСОН в систему АПАК и отображения на консоли и в утилите "logview" значений сигналов
// информационного кадра от системы АПАК к сети КСОН.

{
  try
    {

        bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

        // Заполняем структуру SignalParams параметрами конкретного сигнала сети КСОН:
        // - количеством байт от начала информационного блока (параметр "byte") кадра от КСОН к АПАК.
        // - количеством бит (параметр "оffset") от начала байта на который указывает параметр "byte".
        // - размером области бит, которая хранит значение сигнала (параметр "len").
        // - тип данных сигнала ("unsigned", "float", "boolean").
        // - максимальным значением сигнала ("max").
        // - минимальным значением сигнала ("min").
        // - номер группы, к которой относится данный сигнал (с 10-ой по 12-ую).

        // В имитаторе сети КСОН сигналы, идущие от КСОН к АПАК, имеют привязку - "bindings", а
        // сигналы, идущие от АПАК к КСОН, привязку "master".

        // Параметры сигналов устройства описываются в файле сигналов имитатора конкретного устройства.
        // В ранних версиях имитаторов устройств параметры сигналов описывались в подразделе "params"
        // раздела "bindings" или "master". Однако, поскольку файл сигналов имитатора конкретного устройства
        // используется, помимо имитатора, еще программой для отображения информации, записанной от
        // этого устройства в чёрный ящик, то подраздел "params" был перенесён из разделов "bindings"
        // или "master" на уровень выше - прямо в общий раздел сигнала (туда же где находятся:
        // идентификатор сигнала, имя сигнала, описание сигнала...). Поэтому аргументом функции
        // SignalParams::fromJson() является "signal->config()->params".
        apak::SignalParams signal_params = apak::SignalParams::fromJson(signal->config()->params);

        if(r)
        {

            if(binding.mode == modus::Master)
            {
                // Заполняем соответствующие сигналу, на который указывает аргумент "signal"
                // функции  "bindSignal", поля в структурах данных "m_signal_by_bitNumberInInformBlock_from_APAK"
                // и "m_bitNumberInSignal_by_bitNumberInInformBlock_from_APAK".

                // В цикле for () будем проходить по всем номерам битов сигнала (от 0 до "len").
                // В переменной цикла "bitNumberInSignal" будем хранить текущий номер бита сигнала.
                for (uint8_t bitNumberInSignal = 0; bitNumberInSignal < signal_params.len; bitNumberInSignal++)
                {
                    m_signal_by_bitNumberInInformBlock_from_APAK [(uint16_t)signal_params.byte * 8 + signal_params.offset +
                              bitNumberInSignal] = signal;

                    m_bitNumberInSignal_by_bitNumberInInformBlock_from_APAK [(uint16_t)signal_params.byte * 8 +
                              signal_params.offset + bitNumberInSignal] = bitNumberInSignal;
                } // for(...
            }
            else
            {
                // Вставляем в словарь "m_signals_by_byte_to_APAK" пару: КЛЮЧ - смещение (в байтах) значения
                // сигнала относительно начала информационного блока, передаваемого из КСОН в АПАК;
                // ЗНАЧЕНИЕ - указатель на сигнал.
                m_signals_by_byte_to_APAK.insert(signal_params.byte, signal);

                // Вставляем в словарь "m_params_by_signal_to_APAK" пару: КЛЮЧ - указатель на сигнал; ЗНАЧЕНИЕ -
                // структуру "signal_params", содержащую параметры этого сигнала.
                m_params_by_signal_to_APAK.insert (signal, signal_params);

                // В битовом массиве "m_relevance_by_group_to_APAK" устанавливаем в "true" бит, соответствующий
                // номеру группы, к которой принадлежит сигнал. Этим мы обозначаем актуальность группы.
                m_relevance_by_group_to_APAK [signal_params.group] = true;
            }
        } // if (r)

        return r;
    } // try

    catch(SvException& e)
    {
        p_last_error = e.error;
        return false;
    }
}

void apak::SvKsonImitator::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvKsonImitator::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvKsonImitator::start(void)
// В этой функции мы осуществляем:
// 1. Cформируем список "m_signals_to_APAK", хранящий указатели на сигналы исходящие от КСОН.
//    Этот список отсортирован по возрастанию смещений значений сигналов (в байтах) относительно
//    начала информационного блока, передаваемого из КСОН в АПАК.
// 2. Привязываем вызов функции "noReceivePackage" к наступлению таймаута
//    таймера приёма "m_receiveTimer". В этой функции мы будем фиксировать отсутствие
//    информационного кадра от АПАК.
// 3. Запускаем таймер приёма "m_receiveTimer" с периодом, равным предельно допустимому времени
//    между получением информационных кадров от АПАК, заданному  в конфигурационном файле
//   "config_apak_imitator.json", как параметр протокола устройства КСОН
// 4. Привязываем вызов функции "noConfirmationPackage" к наступлению таймаута
//    таймера подтверждения "m_conformTimer". В этой функции мы будем фиксировать
 //   ошибку взаимодействия КСОН и АПАК.
// 5. Привязку вызова функции "sendInformFrame" , в которой мы
//    формируем информационный кадр от имитатора сети КСОН к системе АПАК, к наступлению таймаута
//    таймера посылки "m_sendTimer".
// 6. Запускаем на единственное срабатывание таймер посылки "m_sendTimer" с периодом,
//    равным интервалу посылки информационных кадров,
//    от КСОН к АПАК, заданному  в конфигурационном файле "config_apak_imitator.json", как параметр
//    протокола устройства КСОН. Далее будем запускать
//    этот таймер каждый раз после получения пакета подтверждения от системы АПАК.
// 7. Привязку вызова функции "messageFrom_APAK", к сигналу "modus::IOBuffer::dataReaded". Этот сигнал
//    испускается интерфейсной частью по приходу данных от интерефейса (в нашем случае - от системы АПАК).
//    В этой функции мы обрабатываем принятый от системы АПАК информационный кадр или пакет подтверждения.
// 8. Сброс счётчика идущих подряд ошибок взаимодействия КСОН и АПАК
{
    // 1. Формируем список "m_signals_to_APAK":
    m_signals_to_APAK = m_signals_by_byte_to_APAK.values();

    // 2. Привязываем вызов функции "noReceivePackage" к наступлению таймаута
    // таймера приёма "m_receiveTimer". В этой функции мы будем фиксировать отсутствие
    // информационного кадра от АПАК:
    m_receiveTimer = new QTimer;
    m_receiveTimer->setSingleShot(true);
    connect(m_receiveTimer, &QTimer::timeout, this, &SvKsonImitator::noReceivePackage);

    // 3. Запускаем таймер приёма "m_receiveTimer" с периодом, равным предельно допустимому времени
    // между получением информационных кадров от АПАК,
    // заданному  в конфигурационном файле "config_apak_imitator.json", как параметр протокола устройства КСОН:
    m_receiveTimer->start(m_params.receive_interval);

    // 4. Привязываем вызов функции "noConfirmationPackage" к наступлению таймаута
    // таймера подтверждения "m_conformTimer". В этой функции мы будем фиксировать
    // ошибку взаимодействия КСОН и АПАК:
    m_conformTimer = new QTimer;
    m_conformTimer ->setSingleShot(true);
    connect(m_conformTimer, &QTimer::timeout, this, &SvKsonImitator::noConfirmationPackage);

    // 5. Привязку вызова функции "sendInformFrame", в которой мы
    // формируем информационный кадр от имитатора сети КСОН к системе АПАК, к наступлению таймаута
    // таймера посылки "m_sendTimer".
    m_sendTimer = new QTimer;
    m_sendTimer->setSingleShot(true);
    connect(m_sendTimer, &QTimer::timeout, this, &SvKsonImitator::sendInformFrame);

    // 6. Запускаем на единственное срабатывание таймер посылки "m_sendTimer" с периодом,
    // равным интервалу посылки информационных кадров,
    // от КСОН к АПАК, заданному  в конфигурационном файле "config_apak_imitator.json", как параметр
    // протокола устройства КСОН:
    m_sendTimer->start(m_params.send_interval);

    // 7. Привязываем вызов функции "messageFrom_АPAK", к сигналу "modus::IOBuffer::dataReaded". Этот сигнал
    // испускается интерфейсной частью по приходу данных от интерефейса (в нашем случае - от системы АПАК).
    // В этой функции мы обрабатываем принятый от системы АПАК информационный кадр или пакет подтверждения.
    connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &SvKsonImitator::messageFrom_APAK);

    // 8. Сброс счётчика ошибок взаимодействия КСОН и АПАК:
    m_interactionErrorCounter = 0;

    p_is_active = bool(p_config) && bool(p_io_buffer);
}


void apak::SvKsonImitator::sendInformFrame(void)
// В этой функции мы:
// 1. Формируем и помещаем в массив байт "m_send_data" информационный кадр для передачи от имитатора
// сети КСОН в систему АПАК (в соответствии с протоколом обмена)
// 2. Инициируем передачу этого кадра от протокольной к интерфейcной части имитатора (для передачи по линии связи).
// 3. Запускаем таймер подтверждения "m_conformTimer", который отсчитывает
// предельно допустимое время от посылки нами информационного кадра в систему АПАК,
// до получения нами пакета подтверждения от системы АПАК.

// На данный момент - формат информационного кадра не ясен, поэтому будем реализовывать следующий
// формат:
// Поле данных пакета - DATA_SIZE_FIELD_LENGTH байта
// Поле времени - TIME_FIELD_LENGTH байта
// Поле доступности групп (группы сигналов от сети КСОН имеют номера с 10 по 12) -
//                                       GROUP_AVAILABILITY_FIELD_LENGTH байта
// Поле данных кадра (блок параметрической информации) - "params.send_data_len" байтов.

// Порядок байт во всех числовых полях кадра - от старшего к младшему (bigEndian).
{
    // qDebug () << "\n" << QString("Имитатор КСОН: Информационный пакет КСОН к APAK: Вызов send()");

    // Останавливаем "таймер посылки" m_sendTimer:
    m_sendTimer->stop();

    if(p_is_active == false)
        return;

    // 1. В переменной "m_relevanceСoncrete_by_group_to_APAK" формируем битовый массив,
    // который для каждого номера группы сигналов (номер - это индекс в этом массиве),
    // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны).

    // Изначально уставновим актуальность групп сигналов, исходя из наличия сигналов
    // данной группы в файле сигналов для устройства КСОН.
    m_relevanceConcrete_by_group_to_APAK = m_relevance_by_group_to_APAK;

    // 2. В переменной "m_send_data" формируем информационный кадр от имитатора сети КСОН
    // в систему АПАК в соответствии с протоколом обмена.
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

    // 4. В массиве "timeField" формируем второе поле информационного кадра - поле "времени",
    // которое должно содержать время по данным внешней системы - целое количество секунд
    // с 1 янв. 1970 г.
    // Время формирования информационного кадра записываем в переменную "m_packetTimeTo_APAK",
    // чтобы при получении пакета подтверждения проверить, что время в пакете подтверждения совпадает
    // c этим временем:
    quint64 timeSince_1970 = QDateTime::currentMSecsSinceEpoch() / 1000;
    m_packetTimeTo_APAK = timeSince_1970;

    // Переведём переменную "timeSince_1970" в массив "timeField":
    QByteArray timeField (TIME_FIELD_LENGTH, 0);
    for (int i = TIME_FIELD_LENGTH - 1; i >= 0 ; i--)
    {
        timeField [i] = timeSince_1970 & 0xFF;
        timeSince_1970 = timeSince_1970 >> 8;
    }

    // Добавляем поле "времени"  к формируемому информационному кадру:
    m_send_data.append(timeField);

    // 5. Формируем поле "данные" информационного кадра. Это поле представляет собой
    // блок параметрической информации. Будем формировать его в переменной "dataFrame":
    QByteArray dataFrame (m_params.send_data_len, 0);

    // 5.1. Переменную "dataFrame" будем формировать с помощью переменной "dataFrameStream":
    QDataStream dataFrameStream(&dataFrame, QIODevice::WriteOnly);

    // 5.2.Согласно протоколу числа типа "float" должны занимать 4 байта:
    dataFrameStream.setFloatingPointPrecision(QDataStream::SinglePrecision);


    // 5.3. Формируем информационный блок кадра, передаваемого от КСОН к АПАК:
    for (quint8 signalNumberInDataFrame = 0; signalNumberInDataFrame < m_signals_to_APAK.size();
             signalNumberInDataFrame++)
    { // Проходим по всем номерам параметров (сигналов) в блоке:

        // 5.3.1. Получаем указатель на сигнал:
        modus::SvSignal* signal = m_signals_to_APAK [signalNumberInDataFrame];

        // 5.3.2. Получаем параметры сигнала:
        apak::SignalParams signal_params = m_params_by_signal_to_APAK.value (signal);

        // 5.3.3. Проверим актуальна ли информация, содержащаяся в сигнале:
        if(!signal->value().isValid() || signal->value().isNull())
        {
            // 5.3.3.1. Информация в сигнале не актуальна ->

            // 5.3.3.1.1.Сбросим в "false" соответствующий группе этого сигнала бит в
            // массиве "m_relevanceСoncrete_by_group_to_APAK", то есть объявим сигналы
            // этой группы неактуальными:
            m_relevanceConcrete_by_group_to_APAK [signal_params.group] = false;

            // 5.3.3.1.2. Выясним тип сигнала ("boolean", "unsigned", "false") и запишем в
            // поток НУЛЕВОЕ значение этого типа.

            switch (signal_params.data_type)
            {
                case boolType:
                    // Этот сигнал представляет тип "boolean" -> для КСОН запишем
                    // в поток целый НУЛЕВОЙ байт
                    dataFrameStream << (quint8) 0x00;
                    break;

                case unsignedType:
                    // Этот сигнал представляет тип "unsigned" и занимает 4 байта, порядок
                    // которых: big-endian
                    dataFrameStream << (unsigned) 0;
                    break;

                case floatType:
                default:
                    // Этот сигнал представляет тип "float" и занимает 4 байта, порядок
                    //которых: big-endian
                    dataFrameStream << (float) 0;
                    break;
            } // switch

            continue;

        } // if(!signal->value().isValid() ...

        // 5.3.4. Выясним тип сигнала ("boolean", "unsigned", "false") и попытаемся
        // преобразовать содержимое переменной "signalValue" к этому типу:
        bool ok;
        switch (signal_params.data_type)
        {
            case boolType:
                // 5.3.4.1. Этот сигнал представляет тип "boolean" -> для КСОН будем записывать
                // в поток целый байт
                bool booleanSignal;

                // 5.3.4.1.1. Теперь переведём информацию в сигнале в тип "boolean":
                booleanSignal = signal->value().toBool();

                // 5.3.4.1.2. Выводим значение сигнала в поток:
                if (booleanSignal == true)
                    dataFrameStream << (quint8) 0x01;
                else
                    dataFrameStream << (quint8) 0x00;
                dataFrameStream << (quint8) 0x00;
                break;

            case unsignedType:
                // 5.3.4.2. Этот сигнал представляет тип "unsigned" и занимает 4 байта, порядок
                // которых: big-endian

                unsigned unsignedSignal;

                // 5.3.4.2.1 Проверим представима ли информация в сигнале числом типа unsigned:
                unsignedSignal = signal->value().toUInt (&ok);

                if(ok == false )
                {
                    // 5.3.4.2.2.Информация в сигнале не представима числом типа "unsigned" ->
                    // 5.3.4.2.2.1. Cбросим в "false" соответствующий группе этого сигнала бит в массиве
                    // "m_relevanceСoncrete_by_group_to_APAK", то есть объявим сигналы этой группы неактуальными:
                    m_relevanceConcrete_by_group_to_APAK [signal_params.group] = false;

                    // 5.3.4.2.2.2. Запишем в поток НУЛЕВОЕ значение типа "unsigned":
                    dataFrameStream << (unsigned) 0;
                }
                else
                {
                    // 5.3.4.2.3. Выводим значение сигнала в поток:
                    dataFrameStream << unsignedSignal;
                }
                break;

            case floatType:
            default:
                // 5.3.4.3. Этот сигнал представляет тип "float" и занимает 4 байта, порядок
                // которых: big-endian.

                float floatSignal;

                // 5.3.4.3.1. Теперь проверим представима ли информация в сигнале числом
                // типа "float":
                floatSignal = signal->value().toFloat(&ok);

                if(ok == false )
                {
                    // 5.3.4.3.2. Информация в сигнале не представима числом типа "float" ->
                    // 5.3.4.3.2.1. Cбросим в "false" соответствующий группе этого сигнала бит в массиве
                    // "m_relevanceСoncrete_by_group_to_APAK", то есть объявим сигналы этой группы неактуальными:
                    m_relevanceConcrete_by_group_to_APAK [signal_params.group] = false;

                    // 5.3.4.3.2.2. Запишем в поток НУЛЕВОЕ значение типа "float":
                    dataFrameStream << (float) 0;
                }
                else
                {
                 // 5.3.4.3.3. Выводим значение сигнала в поток:
                 dataFrameStream << floatSignal;
                }
        } // switch
    } // for

    // 6. Формируем поле "доступность групп". Это поле представляет собой два байта (сначала -
    // младший, затем - старший). При этом: младший бит младшего байта - соответветствует
    // нулевой группе. Данные для поля "доступность групп" формируем в  массиве бит
    // "m_relevanceConcrete_by_group_to_APAK".

    // 6.1. Конвертируем данные из массива бит "m_relevanceConcrete_by_group_to_APAK" в массив байт
    // "relevanceGroup_Byte_to_APAK".
    // Для начала заполняем массив байт "relevanceGroup_Byte_to_APAK" нулями:
    QByteArray relevanceGroup_Byte_to_APAK(GROUP_AVAILABILITY_FIELD_LENGTH, 0);

    // В переменной цикла "bitNumber" будем хранить текущий номер бита в
    // массиве "relevanceGroup_Byte_to_APAK".
    for(quint16  bitNumber = 0; bitNumber < 8* GROUP_AVAILABILITY_FIELD_LENGTH; bitNumber++)
    {
        relevanceGroup_Byte_to_APAK [bitNumber / 8] = relevanceGroup_Byte_to_APAK.at(bitNumber / 8) |
                     ((m_relevanceConcrete_by_group_to_APAK [bitNumber] ? 1 : 0) << (bitNumber % 8));
    }

    // 6.2. Добавим сформированное нами в массиве "relevanceGroup_Byte_to_APAK" поле
    // "доступность групп" к информационному кадру:
    m_send_data.append(relevanceGroup_Byte_to_APAK);

    // 7.. Добавим к информационному кадру блок параметрической информации,
    // сформированный нами в массиве "dataFrame".
    m_send_data.append (dataFrame);

    //qDebug() << "m_relevanceConcrete_by_group_to_APAK" << m_relevanceConcrete_by_group_to_APAK;

    //qDebug() << "Имитатор КСОН: Блок параметрической информации от сети КСОН:";
    //qDebug() << "Размер: " << dataFrame.length();
    //qDebug() << "Содержание: " << dataFrame.toHex();

    //qDebug() << "Имитатор КСОН: Информационный кадр от сети КСОН:";
    //qDebug() << "Имитатор КСОН: Размер: " << m_send_data.length();
    //qDebug() << "Имитатор КСОН: Содержание: " << m_send_data.toHex();

    // 8. Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
   p_io_buffer->output->mutex.lock();
   p_io_buffer->output->setData(m_send_data);

   emit message(QString(m_send_data.toHex()), lldbg, sv::log::mtNew);

   p_io_buffer->output->setReady(true);
   emit p_io_buffer->readyWrite(p_io_buffer->output);

   p_io_buffer->output->mutex.unlock();

   // 9. Запускаем таймер подтверждения "m_conformTimer" с периодом,
   // равным предельно допустимому времени от посылки нами информационного кадра к системе АПАК,
   // до получения нами пакета подтверждения от системы АПАК. Это время
   // задаётся  в конфигурационном файле "config_apak_imitator.json", как параметр протокола имитатора
   // устройства КСОН:
   m_conformTimer->start(m_params.conform_interval);
}


void apak::SvKsonImitator::messageFrom_APAK(modus::BUFF* buffer)
// Мы последовательно обрабатываем пакеты, из которых состоит принятое сообщение.
// Тип пакета (информационный кадр или пакет подтверждения) мы определем по полю
// размера данных.
{
    //qDebug() << "Имитатор КСОН: messageFrom_APAK";

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

    // Скопируем пришедшее от АПАК сообщение, которое может состоять:
    // - из информационного кадра
    // - из пакета подтверждения
    // - из информационного кадра и пакета подтверждения в любом порядке
    // в массив "messageFrom_APAK":
    QByteArray messageFrom_APAK = QByteArray(buffer->data, buffer->offset);

    buffer->reset();


    // Разбираемся с принятым от системы АПАК сообщением:
    qDebug() << "Имитатор КСОН: messageFrom_APAK - длина принятого от АПАК сообщения: " << messageFrom_APAK.length();

    if (messageFrom_APAK.length() < DATA_SIZE_FIELD_LENGTH)
    {
        // Обрабатываем ошибку:
        protocolErrorHandling (QString("Имитатор КСОН: Длина принятого от АПАК сообщения равна %1").arg(messageFrom_APAK.length()));

        return;
    }

    // Выделяем из сообщения от АПАК пакет и разбираемся с ним:
    analysisMessageFrom_APAK(messageFrom_APAK);

    if (messageFrom_APAK.length() == 0)
    { // Всё сообщение, которое было принято от АПАК - мы разобрали:

        return;
    }

    if (messageFrom_APAK.length() < DATA_SIZE_FIELD_LENGTH)
    {
        // Обрабатываем ошибку:
        protocolErrorHandling (QString("Имитатор КСОН: Длина остатка принятого от АПАК сообщения равна %1").arg(messageFrom_APAK.length()));

        return;
    }

    // Выделяем из остатка сообщения от АПАК пакет и разбираемся с ним:
    analysisMessageFrom_APAK(messageFrom_APAK);
    return;
}


void apak::SvKsonImitator::analysisMessageFrom_APAK(QByteArray& messageFrom_APAK)
// Эта функция осуществляет выделение из принятого от АПАК сообщения одного пакета и,
// в зависимости от вида пакета (информационный кадр или пакет подтверждения), вызов для
// его разбора сооответствующей функции.
{
    // Перепишем поле "размера данных" в массив "dataSizeField":
    QByteArray dataSizeField = messageFrom_APAK.mid (0, DATA_SIZE_FIELD_LENGTH);

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

        // Копируем этот информационный кадр в массив  "packageFrom_APAK":
        QByteArray packageFrom_APAK = messageFrom_APAK.mid (0, DATA_SIZE_FIELD_LENGTH +
                         TIME_FIELD_LENGTH + GROUP_AVAILABILITY_FIELD_LENGTH + m_params.receive_data_len);

        // Удалим из массива "messageFrom_APAK" принятый информационный кадр:
        messageFrom_APAK.remove (0, DATA_SIZE_FIELD_LENGTH +
                                 TIME_FIELD_LENGTH + GROUP_AVAILABILITY_FIELD_LENGTH + m_params.receive_data_len);

        // Вызовем функцию обработки пришедшего информационного кадра:
        informFrameFrom_APAK (packageFrom_APAK);

        return;
    }

    if ( sizeField == TIME_FIELD_LENGTH + STATUS_FIELD_LENGTH)
    { // Поле "размера данных" соответствует пакету подтверждения:

        // Копируем этот пакет подтверждения в массив "packageFrom_APAK":
        QByteArray packageFrom_APAK = messageFrom_APAK.mid (0, DATA_SIZE_FIELD_LENGTH +
                         TIME_FIELD_LENGTH + STATUS_FIELD_LENGTH);

        // Удалим из массива "messageFrom_APAK" принятый пакет подтверждения:
        messageFrom_APAK.remove (0, DATA_SIZE_FIELD_LENGTH +
                                 TIME_FIELD_LENGTH + STATUS_FIELD_LENGTH);

        // Вызовем функцию обработки пришедшего пакета подтверждения:
        confirmationPackageFrom_APAK (packageFrom_APAK);

        return;
    }

    // Поле "размера данных" не соответствует ни информационному кадру, ни пакету подтверждения
    // - это ошибка, обрабатываем её:
    protocolErrorHandling (QString("Имитатор КСОН: Поле размера данных в сообщении от АПАК содержит неверную информацию: %1").arg(sizeField));
    return;
}


void apak::SvKsonImitator::informFrameFrom_APAK (QByteArray packageFrom_APAK)
// Функция обработки пришедшего от АПАК информационного кадра:
// Аргумент функции: "packageFrom_APAK" - содержит информационный кадр от АПАК.

{
    // Останавливаем таймер приёма:
    m_receiveTimer->stop();

    // Аргумент функции "packageFrom_APAK", содержит информационный кадр от АПАК.
    // Правильность информационного кадра в данной версии протокола никак проверить нельзя,
    // поэтому очистим переменную "m_status", которая содержит поле статуса пакета
    // подтверждения от имитатора КСОН к АПАК:
    m_status = 0;

    // Перепишем поле "времени" в массив "timeField":
    QByteArray timeField = packageFrom_APAK.mid (DATA_SIZE_FIELD_LENGTH, TIME_FIELD_LENGTH);

    // Переведём поле "времени" в численный вид и запишем его в переменную "m_packetTimeFrom_APAK",
    // чтобы использовать его в пакете подтверждения на этот кадр:

    m_packetTimeFrom_APAK = 0;
    for (int i = 0; i < TIME_FIELD_LENGTH; i++)
    {
        m_packetTimeFrom_APAK = m_packetTimeFrom_APAK << 8;
        m_packetTimeFrom_APAK |= (uchar)timeField [i];
    }

    // Скопируем из информационного кадра поле "доступности групп" в массив "groupAvailability_from_APAK"
    QByteArray groupAvailability_from_APAK = packageFrom_APAK.mid (DATA_SIZE_FIELD_LENGTH + TIME_FIELD_LENGTH,
                                                                   GROUP_AVAILABILITY_FIELD_LENGTH);

    // Переведём массив байт "groupAvailability_from_APAK" в битовый массив "m_relevance_by_group_from_APAK"
    // (пока считаем, что группы нумеруются с нуля):
    m_relevance_by_group_from_APAK.clear();
    m_relevance_by_group_from_APAK.resize(GROUP_AVAILABILITY_FIELD_LENGTH * 8);

    for(quint16  bitNumber = 0; bitNumber < m_params.receive_data_len * 8; bitNumber++)
    {
           m_relevance_by_group_from_APAK [bitNumber] = (groupAvailability_from_APAK[bitNumber / 8] >> (bitNumber % 8)) & 0x01;
    }

    // Скопируем "блок параметрической информации" из информационного кадра в массив "informBlock":
    QByteArray informBlock (m_params.receive_data_len, 0);

    informBlock = packageFrom_APAK.mid (DATA_SIZE_FIELD_LENGTH + TIME_FIELD_LENGTH +
                                        GROUP_AVAILABILITY_FIELD_LENGTH, m_params.receive_data_len);

    // Переведём битовую параметрическую информацию из байтового массива "informBlock" в
    // битовый массив "m_bitsInformBlock_from_APAK":
    m_bitsInformBlock_from_APAK.clear();
    m_bitsInformBlock_from_APAK.resize(m_params.receive_data_len * 8);

    // В переменной цикла "bitNumberInInformBlock" будем хранить текущий номер бита в
    // массиве "m_bitsInformBlock_from_APAK":
    for(quint16  bitNumberInInformBlock = 0; bitNumberInInformBlock <m_params.receive_data_len * 8;
    bitNumberInInformBlock++)
    {
        m_bitsInformBlock_from_APAK[bitNumberInInformBlock] = (informBlock[bitNumberInInformBlock/8] >>
                                                                      (bitNumberInInformBlock%8)) & 0x01;
    }

    // В цикле for () будем проходить по всем номерам битов (от 0 до m_params.receive_data_len*8 - 1)
    // из битового массива "m_bitsInformBlock_from_APAK". Данный массив битов представляет собой информационный
    // блок информационного кадра от АПАК к КСОН. В переменной цикла "bitNumberInInformBlock" будем
    // хранить текущий номер бита массива "m_bitsInformBlock_from_APAK".
    for (quint16 bitNumberInInformBlock = 0; bitNumberInInformBlock <= m_params.receive_data_len*8 - 1;
    bitNumberInInformBlock++)
    {
        // Проверим, должен ли по индексу "bitNumberInInformBlock" в массиве "m_bitsInformBlock_from_APAK" хранится
        // бит какого-либо сигнала:
        if (m_signal_by_bitNumberInInformBlock_from_APAK.contains(bitNumberInInformBlock) == false)
        { // Если не должен, то этот бит массива "m_bitsInformBlock_from_APAK" нас не интересует
            //(переходим на следующую итерацию цикла):

            continue;
        }

        // Если по индексу "bitNumberInInformBlock" в массиве "m_bitsInformBlock_from_APAK" должен храниться
        // бит какого-либо сигнала, то:
        // Получаем указатель на сигнал, один из битов которого должен храниться в массиве
        // "m_bitsInformBlock_from_APAK" по индексу "bitNumberInInformBlock":
        modus::SvSignal* signal = m_signal_by_bitNumberInInformBlock_from_APAK.value( bitNumberInInformBlock);

        // Получаем номер бита сигнала, который должен храниться в массиве бит
        // "m_bitsInformBlock_from_APAK" по индексу "bitNumberInInformBlock_from_APAK":
        quint8 bitNumberInSignal =
                m_bitNumberInSignal_by_bitNumberInInformBlock_from_APAK.value (bitNumberInInformBlock);

        // Все получаемые имитатором КСОН от АПАК сигналы - булевые, поэтому номер бита в сигнале
        // "bitNumberInSignal" всегда должен быть НУЛЕВЫМ. Если это не так -> выводим сообщение об
        // ошибке в утилиту "logview" и на консоль:
        if (bitNumberInSignal != 0)
        {
            emit message(QString("Имитатор КСОН: В булевом сигнале указан ненулевой бит: %1").arg(bitNumberInSignal), sv::log::llError, sv::log::mtError);
            qDebug() << QString("Имитатор КСОН: В булевом сигнале указан ненулевой бит: %1").arg(bitNumberInSignal);
        }

        // Получаем бит сигнала из массива битов "m_bitsInformBlock_from_APAK" и отображаем его для
        // проверки оператором в утилите "logview" и на консоль:
        int bitValue = m_bitsInformBlock_from_APAK [bitNumberInInformBlock];

        // Получаем имя сигнала:
        QString signalName = signal ->config() ->name;

        // Получаем группу, к которой относится сигнал, и проверяем актуальность сигналов
        // этой группы. Для АПАК - все сигналы - ВСЕГДА актуальны. Мы будем использовать
        // "неактуальность" этих сигналов только на этапе разработки - пока библиотеки для
        // соответствующих этим сигналом устойств не реализованы.
        SignalParams signal_params = m_params_by_signal_to_APAK[signal];
        quint8 group = signal_params.group;

        if (m_relevance_by_group_from_APAK[group] == true)
        { // Cигналы группы - актуальны:

            emit message(QString("Имитатор КСОН: В информационном кадре от АПАК сигнал: %1 имеет значение: %2").arg(signalName).arg(bitValue), sv::log::llInfo, sv::log::mtInfo);
            //qDebug() << QString("Имитатор КСОН: В информационном кадре от АПАК сигнал: %1 имеет значение: %2").arg(signalName).arg(bitValue);
        }
        else
        { // Cигналы группы - НЕ актуальны:
            emit message(QString("Имитатор КСОН: В информационном кадре от АПАК сигнал: %1 не актуален").arg(signalName), sv::log::llInfo, sv::log::mtInfo);
            //qDebug() << QString("Имитатор КСОН: В информационном кадре от АПАК сигнал: %1 не актуален").arg(signalName);
        }
    }

    qDebug() << QString("Имитатор КСОН: Принят информационный кадр от АПАК без ошибок");
    emit message(QString("Имитатор КСОН: Принят информационный кадр от АПАК без ошибок"), sv::log::llInfo, sv::log::mtSuccess);

    // Сформируем и пошлём пакет подтверждения:
    sendConfirmationPackage();
}


void  apak::SvKsonImitator::confirmationPackageFrom_APAK (QByteArray packageFrom_APAK)
// Функция обработки пришедшего от АПАК пакета подтверждения:
// Аргумент функции: "packageFrom_APAK" - содержит пакет подтверждения от АПАК.
{
    // Останавливаем таймер подтверждения:
    m_conformTimer->stop();

    // Аргумент функции: "packageFrom_APAK" - содержит пакет подтверждения от АПАК.
    // Правильность пакета подтверждения в данной версии протокола можно проверить
    // по правильности значения поля времени
    // (оно должно повторять значение времени в информационном кадре от имитатора КСОН
    // к АПАК) и допустимости значений в поле статуса.

    // Переменная "confirmationPackageErrorFlag" - это флаг ошибки пакета подтверждения.
    // Устанавливается, если поле статуса пакета подтверждения
    // содержит значение, говорящее об ошибке в информационном кадре от имитатора КСОН к
    // АПАК или имеются любые ошибки в самом пакете подтверждения.

    //  Перед началом анализа пакета подтверждения сбросим флаг ошибки:
    bool confirmationPackageErrorFlag = false;

    // Перепишем поле "времени" в массив "timeField":
    QByteArray timeField = packageFrom_APAK.mid (DATA_SIZE_FIELD_LENGTH, TIME_FIELD_LENGTH);

    // Переведём поле "времени" в численный вид - переменную "timeSince_1970":
    quint64  timeSince_1970 = 0;
    for (int i = 0; i < TIME_FIELD_LENGTH; i++)
    {
        timeSince_1970 = timeSince_1970 << 8;
        timeSince_1970 |= (uchar)timeField [i];
    }

    if (timeSince_1970 !=  m_packetTimeTo_APAK)
    { // Если поля времени в информационном кадре от имитатора КСОН к АПАК и в пакете подтверждения
      // от АПАК к имитатору КСОН не совпадают, то выставляем флаг ошибки пакета подтвердения:

        confirmationPackageErrorFlag = true;

        protocolErrorHandling(QString("Имитатор КСОН: Время в пакете подтверждения от АПАК: %1 не совпадает с временем в информационном кадре: %2").arg(timeSince_1970).arg(m_packetTimeTo_APAK));
    }

    // Третье поле (1 байт) - статус.
    char statusFromPacket = packageFrom_APAK[DATA_SIZE_FIELD_LENGTH + TIME_FIELD_LENGTH];

    if (statusFromPacket != 0)
    { // Если значение в поле статуса отлично от 0, то выставляем флаг ошибки пакета подтвердения:

        confirmationPackageErrorFlag = true;

        protocolErrorHandling(QString("Имитатор КСОН: Значение в поле статуса: %1 отлично от нуля").arg(statusFromPacket));
    }

    if ( confirmationPackageErrorFlag == false)
    { //Если при разборе пакета подтверждения мы НЕ обнаружили ошибок -> сбрасываем
        // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:

        m_interactionErrorCounter = 0;

        // Выводим сообщение оператору:
        qDebug() << QString("Имитатор КСОН: Принят пакет подтверждения от АПАК без ошибок");
        emit message(QString("Имитатор КСОН: Принят пакет подтверждения от АПАК без ошибок"), sv::log::llInfo, sv::log::mtSuccess);
    }

    // Запускаем на единственное срабатывание "таймер посылки m_sendTimer" отсчитывающий
    // период посылки информационных кадров от имитатора КСОН к системе АПАК с периодом,
    // равным интервалу между информационными кадрами,
    // заданному  в конфигурационном файле "config_apak_imitator.json", как параметр протокола
    // имитатора устройства КСОН.
    // По срабатыванию этого таймера будет вызвана функция "send", для посылки информационного кадра к
    // системе АПАК.
    m_sendTimer->start(m_params.send_interval);
}


void apak::SvKsonImitator::noConfirmationPackage(void)
// Эта функция вызывается в том случае, если в течении предельно допустимого времени от
// посылки нами информационного кадра к АПАК, до получения нами пакета подтверждения от АПАК
// пакет подтверждения так и не пришёл.
{
    // 1. Отрабатываем ошибку протокола:
    protocolErrorHandling(QString("Имитатор КСОН: Пакет подтверждения от АПАК за время: %1 не получен").arg(m_params.conform_interval));

    // 2. Даже если пакета подтверждения не пришло в течении оговоренного в протоколе времени
    // (m_params.conform_interval), то мы всё равно запускаем на единственное срабатывание
    // "таймер посылки m_sendTimer", чтобы продолжать посылать информационные кадры к АПАК:
    m_sendTimer->start(m_params.send_interval);
}


void apak::SvKsonImitator::noReceivePackage(void)
// Эта функция вызывается в том случае, если в течении предельно допустимого времени от
// сети КСОН не пришёл инфомационный кадр.
{
    // 1. Отрабатываем ошибку протокола:
    protocolErrorHandling (QString("Имитатор КСОН: Информационный кадр от АПАК за время: %1 не получен").arg(m_params.receive_interval));

    // 2. Даже если инфомационного кадра не пришло в течении оговоренного в протоколе времени
    // (m_params.receive_interval), то мы всё равно запускаем на единственное срабатывание
    // "таймер приёма m_receiveTimer", чтобы продолжать отображать в утилите "logview" и
    // на консоли факт отсутствия информационных кадров от АПАК:
    m_receiveTimer->start(m_params.receive_interval);
}

void apak::SvKsonImitator::sendConfirmationPackage(void)
// Формирование и посылка пакета подтверждения.
// На данный момент - формат пакета подтверждения не ясен, поэтому будем реализовывать следующий
// формат:
// Поле данных пакета - DATA_SIZE_FIELD_LENGTH байта
// Поле времени - TIME_FIELD_LENGTH байта
// Поле статуса - STATUS_FIELD_LENGTH байт
{
    // В массиве "m_send_data" формируем пакет подтверждения для передачи от имитатора КСОН к АПАК
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

    // Добавляем поле "размера данных"  к формируемому информационному кадру:
    m_send_data.append(dataSizeField);

    // 3. В массиве "timeField" формируем второе поле пакета подтверждения - поле "времени",
    // оно должно повторять время в информационном кадре от АПАК, сохранённое нами в переменной
    // "m_packetTimeFrom_APAK":

    // Переведём переменную "m_packetTimeFrom_APAK" в массив "timeField":
    QByteArray timeField (TIME_FIELD_LENGTH, 0);
    for (int i = TIME_FIELD_LENGTH - 1; i >= 0 ; i--)
    {
        timeField [i] = m_packetTimeFrom_APAK & 0xFF;
        m_packetTimeFrom_APAK = m_packetTimeFrom_APAK >> 8;
    }

    // Добавляем поле "времени"  к формируемому информационному кадру:
    m_send_data.append(timeField);


    // 4. Добавляем к пакету подтверждения - поле статуса. Оно сформировано нами в процессе
    // разбора информационного кадра от АПАК:
    m_send_data.append( m_status);


    //qDebug() <<"Имитатор КСОН: Пакет подтверждения от КСОН к АПАК:";
    //qDebug() <<"Имитатор КСОН: Размер: " << m_send_data.length();
    //qDebug() <<"Имитатор КСОН: Содержание: " << m_send_data.toHex();

    // 5. Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
    p_io_buffer->output->mutex.lock();
    p_io_buffer->output->setData(m_send_data);

    emit message(QString(m_send_data.toHex()), lldbg, sv::log::mtNew);

    p_io_buffer->output->setReady(true);
    emit p_io_buffer->readyWrite(p_io_buffer->output);

    p_io_buffer->output->mutex.unlock();
}

void apak::SvKsonImitator::protocolErrorHandling (QString str)
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
        qDebug() << QString ("Имитатор КСОН: Выдаём команду интерфейсной части на разрыв соединения с TCP-клиентом");
        emit message(QString("Имитатор КСОН: Выдаём команду интерфейсной части на разрыв соединения с TCP-клиентом"), sv::log::llError, sv::log::mtError);

        // Испускаем для интерфейсной части сигнал "say" с командой "breakConnection",
        // который приказывает интерфейсной части разорвать соединение АПАК:
        emit p_io_buffer->say("breakConnection");

        // Сбрасываем счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:
        m_interactionErrorCounter = 0;
    }
 }

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvKsonImitator();
  return protocol;
}

