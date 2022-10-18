#include "kson_imitator.h"

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
    m_relevance_by_group_to_APAK.resize(16);

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
    m_relevance_by_group_from_APAK.resize(16);
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
// с ними структур данных (словари  m_signal_by_bitNumberInInformBlock_from_APAK,
// m_bitNumberInSignal_by_bitNumberInInformBlock_from_APAK, m_signals_by_byte_to_APAK,
// m_params_by_signal_to_APAK), необходимых для формирования информационного кадра от
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
// 7. Привязку вызова функции "packageFrom_APAK", к сигналу "modus::IOBuffer::dataReaded". Этот сигнал
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

    // 7. Привязываем вызов функции "packageFrom_АPAK", к сигналу "modus::IOBuffer::dataReaded". Этот сигнал
    // испускается интерфейсной частью по приходу данных от интерефейса (в нашем случае - от системы АПАК).
    // В этой функции мы обрабатываем принятый от системы АПАК информационный кадр или пакет подтверждения.
    connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &SvKsonImitator::packageFrom_APAK);

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
// Размер данных - 4 байта
// Время - 4 байта
// Доступность групп (группы сигналов от сети КСОН имеют номера с 10 по 12) - 2 байта
// Данные кадра (блок параметрической информации) - 33 байта ( задаётся в "params.send_data_len")
// Порядок байт во всех полях кадра - от старшего к младшему (bigEndian).
{
    qDebug () << "\n" << QString("Имитатор КСОН: Информационный пакет КСОН к APAK: Вызов send()");

    // Останавливаем "таймер посылки" m_sendTimer:
    m_sendTimer->stop();

    if(p_is_active == false)
        return;

    // В переменной "m_relevanceСoncrete_by_group_to_APAK" формируем битовый массив,
    // который для каждого номера группы сигналов (номер - это индекс в этом массиве),
    // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны).

    // Изначально уставновим актуальность групп сигналов, исходя из наличия сигналов
    // данной группы в файле сигналов для устройства КСОН.
    m_relevanceConcrete_by_group_to_APAK = m_relevance_by_group_to_APAK;

    // В переменной "m_send_data" формируем информационный кадр от имитатора сети КСОН
    // в систему АПАК в соответствии с протоколом обмена.
    // Вначале, выделим память и заполним первые два поля:
    m_send_data.fill( 0, 4 + 4);

    // 1.1. Формируем первое поле информационного кадра:
    // Первое поле (4 байта) - размер данных (43 байта): 0x0, 0x0, 0x0, 0х2b
    m_send_data [0] = 0;
    m_send_data [1] = 0;
    m_send_data [2] = 0;
    m_send_data [3] = 0x2b;

    // 1.2. Формируем второе поле информационного кадра:
    // Второе поле (4 байта) - время по данным внешней системы -
    // целое количество секунд с 1 янв. 1970 г.
    quint64  timeSince_1970 = QDateTime::currentMSecsSinceEpoch() / 1000;

    m_send_data [4] = (uint8_t) ((timeSince_1970 >> 24) & 0xFF);
    m_send_data [5] = (uint8_t) ((timeSince_1970 >> 16) & 0xFF);
    m_send_data [6] = (uint8_t) ((timeSince_1970 >>  8) & 0xFF);
    m_send_data [7] = (uint8_t) (timeSince_1970 & 0xFF);

    // 1.3. Формируем поле "данные" информационного кадра. Это поле представляет собой
    // блок параметрической информации. Будем формировать его в переменной "dataFrame":
    QByteArray dataFrame;// ОТЛАДКА(m_params.send_data_len, 0);

    // Переменную "dataFrame" будем формировать с помощью переменной "dataFrameStream":
    QDataStream dataFrameStream(&dataFrame, QIODevice::WriteOnly);

    // Согласно протоколу числа типа "float" должны занимать 4 байта:
    dataFrameStream.setFloatingPointPrecision(QDataStream::SinglePrecision);


    // Формируем информационный блок кадра, передаваемого от КСОН к АПАК:
    for (quint8 signalNumberInDataFrame = 0; signalNumberInDataFrame < m_signals_to_APAK.size();
             signalNumberInDataFrame++)
    { // Проходим по всем номерам параметров (сигналов) в блоке:

        // 1.3.1. Получаем указатель на сигнал:
        modus::SvSignal* signal = m_signals_to_APAK [signalNumberInDataFrame];

        // 1.3.2. Получаем параметры сигнала:
        apak::SignalParams signal_params = m_params_by_signal_to_APAK.value (signal);

        // 1.3.3. Проверим актуальна ли информация, содержащаяся в сигнале:
        if(!signal->value().isValid() || signal->value().isNull())
        {
            // 1.3.3.1. Информация в сигнале не актуальна ->

            // 1.3.3.1.1.Сбросим в "false" соответствующий группе этого сигнала бит в
            // массиве "m_relevanceСoncrete_by_group_to_APAK", то есть объявим сигналы
            // этой группы неактуальными:
            m_relevanceConcrete_by_group_to_APAK [signal_params.group] = false;

            // 1.3.3.1.2. Выясним тип сигнала ("boolean", "unsigned", "false") и запишем в
            // поток НУЛЕВОЕ значение этого типа.

            // Получаем тип сигнала:
            QString type = signal_params.data_type;

            if ( type.compare ("boolean", Qt::CaseInsensitive) == 0)
            { // Этот сигнал представляет тип "boolean" -> для КСОН запишем в поток целый НУЛЕВОЙ байт.

                dataFrameStream << (quint8) 0x00;
            }

            if ( type.compare ("float", Qt::CaseInsensitive) == 0)
            { // Этот сигнал представляет тип "float" и занимает 4 байта, порядок которых: big-endian

                dataFrameStream << (float) 0;
            }

            if ( type.compare ("unsigned", Qt::CaseInsensitive) == 0)
            { // Этот сигнал представляет тип "unsigned" и занимает 4 байта, порядок которых: big-endian.

                dataFrameStream << (unsigned) 0;
            }

            continue;
        } // if(!signal->value().isValid()

        // 1.3.4. Получаем тип сигнала и попытаемся преобразовать содержимое
        // переменной "signalValue" к этому типу:
        QString type = signal_params.data_type;

        if ( type.compare ("boolean", Qt::CaseInsensitive) == 0)
        {  // 1.3.4.1. Этот сигнал представляет тип "boolean" и, в случае КСОН,
           // занимает младший бит в байте (остальные биты в этом
           // байте не важны). Поэтому мы будем записывать в поток целый байт.

            bool booleanSignal;

            // 1.3.4.1.1. Теперь переведём информацию в сигнале в тип "boolean":
            booleanSignal = signal->value().toBool();

            // 1.3.4.1.2. Выводим значение сигнала в поток:
            if (booleanSignal == true)
                dataFrameStream << (quint8) 0x01;
            else
                dataFrameStream << (quint8) 0x00;
        } // Этот сигнал представляет тип "boolean"

        if ( type.compare ("float", Qt::CaseInsensitive) == 0)
        { // 1.3.4.2. Этот сигнал представляет тип "float" и занимает 4 байта, порядок которых: big-endian.

            bool ok;
            float floatSignal;

            // 1.3.4.2.1. Теперь проверим представима ли информация в сигнале числом типа "float":
            floatSignal = signal->value().toFloat(&ok);

            if(ok == false )
            {
                // 1.3.4.2.2. Информация в сигнале не представима числом типа "float" ->
                // 1.3.4.2.2.1. Cбросим в "false" соответствующий группе этого сигнала бит в массиве
                // "m_relevanceСoncrete_by_group_to_APAK", то есть объявим сигналы этой группы неактуальными:
                m_relevanceConcrete_by_group_to_APAK [signal_params.group] = false;

                // 1.3.4.2.2.2. Запишем в поток НУЛЕВОЕ значение типа "float":
                dataFrameStream << (float) 0;
            }
            else
            {
                // 1.3.4.2.3. Выводим значение сигнала в поток:
                dataFrameStream << floatSignal;
            }
        } // Этот сигнал представляет тип "float" и занимает 4 байта

        if ( type.compare ("unsigned", Qt::CaseInsensitive) == 0)
        { // 1.3.4.3. Этот сигнал представляет тип "unsigned" и занимает 4 байта, порядок которых: big-endian.

            bool ok;
            unsigned unsignedSignal;

            // 1.3.4.3.1 Теперь проверим представима ли информация в сигнале числом типа unsigned:
            unsignedSignal = signal->value().toUInt (&ok);

            if(ok == false )
            {
                // 1.3.4.3.2.Информация в сигнале не представима числом типа "unsigned" ->
                // 1.3.4.3.2.1. Cбросим в "false" соответствующий группе этого сигнала бит в массиве
                // "m_relevanceСoncrete_by_group_to_APAK", то есть объявим сигналы этой группы неактуальными:
                m_relevanceConcrete_by_group_to_APAK [signal_params.group] = false;

                // 1.3.4.3.2.2. Запишем в поток НУЛЕВОЕ значение типа "unsigned":
                dataFrameStream << (unsigned) 0;
            }
            else
            {
                // 1.3.4.3.3. Выводим значение сигнала в поток:
                dataFrameStream << unsignedSignal;
            }
       } // Этот сигнал представляет тип "unsigned" и занимает 4 байта
    } // for

    // 1.4. Формируем поле "доступность групп". Это поле представляет собой два байта (сначала -
    // старший, затем - младший). При этом: младший бит младшего байта - соответветствует
    // нулевой группе. Данные для поля "доступность групп" формируем в  массиве бит
    // "m_relevanceConcrete_by_group_to_APAK".

    // 1.4.1. Конвертируем данные из массива бит "m_relevanceConcrete_by_group_to_APAK" в массив байт
    // "relevanceGroup_Byte_to_APAK".
    // Для начала заполняем массив байт "relevanceGroup_Byte_to_APAK" нулями:
    QByteArray relevanceGroup_Byte_to_APAK(2, 0);

    // В переменной цикла "bitNumberIn_relevance_by_group_to_APAK" будем хранить текущий номер бита в
    // массиве "relevanceGroup_Byte_to_APAK".
    for(quint16  bitNumberIn_relevance_by_group_to_APAK = 0; bitNumberIn_relevance_by_group_to_APAK <16;
                   bitNumberIn_relevance_by_group_to_APAK++)
    {
        relevanceGroup_Byte_to_APAK [bitNumberIn_relevance_by_group_to_APAK / 8] = relevanceGroup_Byte_to_APAK.at(bitNumberIn_relevance_by_group_to_APAK / 8) |
                     ((m_relevanceConcrete_by_group_to_APAK [bitNumberIn_relevance_by_group_to_APAK] ? 1 : 0) << (bitNumberIn_relevance_by_group_to_APAK % 8));
    }

    // 1.4.2. Перепишем данные из массива байт "relevanceGroup_Byte_to_APAK" в 8-ой и 9-ый байты массива
    // "m_send_data". При этом 0-ой байт массива байт "relevanceGroup_Byte_to_APAK" перепишем в 9-ый байт массива
    // "m_send_data", a 1-ый байт массива "relevanceGroup_Byte_to_APAK" перепишем в 8-ой байт массива
    // "m_send_data", так как во всех полях кадра "m_send_data" порядок байт от
    // старшего к младшему (bigEndian).
    m_send_data [8] = relevanceGroup_Byte_to_APAK[1];
    m_send_data [9] = relevanceGroup_Byte_to_APAK[0];

    // 1.5. Добавим к трём полям информационного кадра, уже содержащимся в массиве "m_send_data",
    // четвёртое - блок параметрической информации, сформированный нами в массиве "dataFrame".
    m_send_data.append (dataFrame);

    //qDebug() << "m_relevanceConcrete_by_group_to_APAK" << m_relevanceConcrete_by_group_to_APAK;

    //qDebug() << "Имитатор КСОН: Блок параметрической информации от сети КСОН:";
    //qDebug() << "Размер: " << dataFrame.length();
    //qDebug() << "Содержание: " << dataFrame.toHex();

    qDebug() << "Имитатор КСОН: Информационный кадр от сети КСОН:";
    qDebug() << "Имитатор КСОН: Размер: " << m_send_data.length();
    qDebug() << "Имитатор КСОН: Содержание: " << m_send_data.toHex();

    // 2. Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
   p_io_buffer->output->mutex.lock();
   p_io_buffer->output->setData(m_send_data);

   emit message(QString(m_send_data.toHex()), lldbg, sv::log::mtNew);

   p_io_buffer->output->setReady(true);
   emit p_io_buffer->readyWrite(p_io_buffer->output);

   p_io_buffer->output->mutex.unlock();

   // 3. Запускаем таймер подтверждения "m_conformTimer" с периодом,
   // равным предельно допустимому времени от посылки нами информационного кадра к системе АПАК,
   // до получения нами пакета подтверждения от системы АПАК. Это время
   // задаётся  в конфигурационном файле "config_apak_imitator.json", как параметр протокола имитатора
   // устройства КСОН:
   m_conformTimer->start(m_params.conform_interval);
}


void apak::SvKsonImitator::packageFrom_APAK(modus::BUFF* buffer)
// В этой функции мы обрабатываем принятый от системы АПАК кадр. Этот кадр может быть двух типов -
// информационный кадр или пакет подтверждения. По длине принятого кадра, мы
// определяем его тип (СЕЙЧАС определяем тип кадра по его ДЛИНЕ).
// Если принят информационный кадр -> мы разбираем его на отдельные поля и отображаем информацию
// о них в утилите "logview" и на консоли.
// Если принят пакет подтверждения, то останавливаем "таймер подтверждения m_confirmTimer",
// отсчитывающий время от посылки нами информиционного кадра АПАКу, до получения нами
// от АПАКа пакета подтверждения. Также при этом мы запускаем "таймер посылки m_sendTimer".
{
    qDebug() << "Имитатор КСОН: packageFrom_APAK";

    if(p_is_active == false)
        return;

    buffer->mutex.lock();

    if(!buffer->isReady())
    { // Если принятых данных нет, то не в чем и разбираться (выходим из функции):
        return;
    }

    // Скопируем пришедший от АПАК пакет в массив "packageFrom_APAK":
    QByteArray packageFrom_APAK = QByteArray(buffer->data, buffer->offset);

    buffer->reset();
    buffer->mutex.unlock();

    // Разбираемся с принятым от системы АПАК пакетом:

    // Определим тип принятого кадра:
    quint16 length = packageFrom_APAK.length();

    qDebug() << "Имитатор КСОН: packageFrom_APAK - длина принятого от АПАК пакета: " << length;

    if ( length == 4+4+2+m_params.receive_data_len)
    {// Принят информационный кадр (длина 12 байт):

        // Останавливаем таймер приёма:
        m_receiveTimer->stop();

        // Очистим переменную "m_status", в которой мы формируем поле статуса пакета подтверждения от имитатора КСОН к АПАК:
        m_status = 0;

        // Переменная "packageFrom_APAK", содержит информационный кадр от АПАК.
        // Правильность информационного кадра в данной версии протокола можно проверить
        // по правильности содержимого поля "Размер данных", соответствию значений в
        // поле времени информационного пакета и в поле времени блока параметрических данных (в
        // случае, если поле времени в блоке параметрических данных - актуально),
        // и по допустимости значений параметрических данных (в случае их актуальности).

        // Получим значения полей пакета с помощью переменной  "packageFrom_APAK_Stream":
        QDataStream packageFrom_APAK_Stream (&packageFrom_APAK, QIODevice::ReadOnly);

        // Согласно протоколу числа типа "float" занимают 4 байта:
        packageFrom_APAK_Stream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        // Первое поле (4 байта) - размер данных. Он должен быть 4+4+2+m_params.data_len = 43 байт
        unsigned dataSize;
        packageFrom_APAK_Stream >> dataSize;

        if (dataSize != unsigned(4+4+2+m_params.receive_data_len))
        { // Если поле даннных содержит неверную информацию, то устанавливаем флаг ошибки структуры
          // информационного кадра в переменной "status":

            m_status |= INFO_FRAME_ERROR;

            qDebug () << QString("Имитатор КСОН: Поле данных в информационном кадре от АПАК содержит неверную информацию: %1").arg(dataSize);
            emit message(QString("Имитатор КСОН: Поле данных в информационном кадре от АПАК содержит неверную информацию: %1").arg(dataSize), sv::log::llError, sv::log::mtError);
        }

        // Второе поле (4 байта) - время по данным внешней системы -
        // целое количество секунд с 1 янв. 1970 г.
        quint32  timeSince_1970;
        packageFrom_APAK_Stream >> timeSince_1970;

        // Сохраним это время в переменной "m_packetTimeFrom_APAK", чтобы использовать его
        // в пакете подтверждения на этот кадр:
        m_packetTimeFrom_APAK = timeSince_1970;

        // Третье поле (2 байта) - доступность групп.
        quint16 groupAvailability_from_APAK;
        packageFrom_APAK_Stream >> groupAvailability_from_APAK;

        // Переведём поле "groupAvailability_from_APAK" в битовый массив "m_relevance_by_group_from_APAK"
        // (пока считаем, что группы нумеруются с нуля):
        for (quint8 bitNumber = 0; bitNumber <16; bitNumber++)
        {
            if (((groupAvailability_from_APAK >> bitNumber) & 0x0001) == 0x0001)
                m_relevance_by_group_from_APAK [bitNumber] = true;
            else
                m_relevance_by_group_from_APAK [bitNumber] = false;
        }

        // Дальнейшие поля - это поля параметрических данных.
        // Скопируем параметричекие данные в массив байт "informBlock":
        QByteArray informBlock (m_params.receive_data_len, 0);

        informBlock = packageFrom_APAK.mid (4+4+2, m_params.receive_data_len);
        m_bitsInformBlock_from_APAK.clear();
        m_bitsInformBlock_from_APAK.resize(m_params.receive_data_len * 8);

        // Конвертируем данные из массива байт "informBlock" в массив бит "m_bitsInformBlock_from_APAK".
        // В переменной цикла "bitNumberInInformBlock" будем хранить текущий номер бита в
        // массиве "m_bitsInformBlock":
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

            // Получаем бит сигнала из массива битов "m_bitsInformBlock_to_KSON" и отображаем его для
            // проверки оператором в утилите "logview" и на консоль:
            int bitValue = m_bitsInformBlock_from_APAK [bitNumberInInformBlock];

            // Получаем имя сигнала:
            QString signalName = signal ->config() ->name;

            emit message(QString("Имитатор КСОН: В информационном кадре от АПАК сигнал: %1 имеет значение: %2").arg(signalName).arg(bitValue), sv::log::llInfo, sv::log::mtInfo);
            qDebug() << QString("Имитатор КСОН: В информационном кадре от АПАК сигнал: %1 имеет значение: %2").arg(signalName).arg(bitValue);
        }

        if (m_status != 0)
        { // Если при разборе информационного кадра от КСОН мы обнаружили ошибку -> увеличиваем
            // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:

            m_interactionErrorCounter++;

            qDebug() << QString("Имитатор КСОН: При разборе информационного кадра от КСОН обнаружили ошибку. Количество идущих подряд ошибок взаимодействия : %1").arg(m_interactionErrorCounter);
            emit message(QString("Имитатор КСОН: При разборе информационного кадра от КСОН обнаружили ошибку. Количество идущих подряд ошибок взаимодействия : %1").arg(m_interactionErrorCounter), sv::log::llError, sv::log::mtError);
        }
        else
        { // Если при разборе информационного кадра от КСОН мы НЕ обнаружили ошибкок -> сбрасываем
          // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН.

            m_interactionErrorCounter = 0;
        }

        // Сформируем и пошлём пакет подтверждения:
        sendConfirmationPackage();
    } // Если принят информационный кадр (длина 12 байт)

    if ( length == 9)
    { // Принят пакет подтверждения:

        // Останавливаем таймер подтверждения:
        m_conformTimer->stop();

        // Переменная "packageFrom_APAK", содержит пакет подтверждения от КСОН.
        // Правильность пакета подтверждения в данной версии протокола можно проверить
        // по правильности содержимого поля "Размер данных", правильности значения поля времени
        // (оно должно повторять значение времени в информационном кадре от имитатора КСОН
        // к АПАК) и допустимости значений в поле статуса.

        // Переменная "confirmationPackageErrorFlag" - это флаг ошибки пакета подтверждения.
        // Устанавливается, если поле статуса пакета подтверждения
        // содержит значение, говорящее об ошибке в информационном пакете от имитатора КСОН к
        // АПАК или имеются любые ошибки в самом пакете подтверждения.

        //  Перед началом анализа пакета подтверждения сбросим флаг ошибки:
        bool confirmationPackageErrorFlag = false;

        // Получим значения полей пакета с помощью переменной  "packageFrom_APAK_Stream":
        QDataStream packageFrom_APAK_Stream (&packageFrom_APAK, QIODevice::ReadOnly);

        // Первое поле (4 байта) - размер данных. Он должен быть 4+4+1 = 9 байт
        quint64 dataSize;
        packageFrom_APAK_Stream >> dataSize;

        if (dataSize != 9)
        { // Если поле данных содержит неверную информацию, то
          // выставляем флаг ошибки пакета подтвердения:

            qDebug () << QString("Имитатор КСОН: Поле данных в пакете подтверждения от КСОН содержит неверную информацию: %1").arg(dataSize);
            emit message(QString("Имитатор КСОН: Поле данных в пакете подтверждения от КСОН содержит неверную информацию: %1").arg(dataSize), sv::log::llError, sv::log::mtError);

            confirmationPackageErrorFlag = true;
        }

        // Второе поле (4 байта) - время, информационного кадра от имитатора КСОН к АПАК.
        quint32  timeSince_1970;
        packageFrom_APAK_Stream >> timeSince_1970;

        if (timeSince_1970 !=  m_packetTimeTo_APAK)
        { // Если поля времени в информационном кадре от имитатора КСОН к АПАК и в пакете подтверждения
          // от АПАК к имитатору КСОН не совпадают, то выставляем флаг ошибки пакета подтвердения:

            qDebug () << QString("Имитатор КСОН: Время в пакете подтверждения от АПАК: %1 не совпадает с временем в информационном кадре: %2").arg(timeSince_1970).arg(m_packetTimeTo_APAK);
            emit message(QString("Имитатор КСОН: Время в пакете подтверждения от АПАК: %1 не совпадает с временем в информационном кадре: %2").arg(timeSince_1970).arg(m_packetTimeTo_APAK), sv::log::llError, sv::log::mtError);

            confirmationPackageErrorFlag = true;
        }

        // Третье поле (1 байт) - статус.
        quint8 statusFromPacket;
        packageFrom_APAK_Stream >> statusFromPacket;
        if (statusFromPacket != 0)
        { // Если значение в поле статуса отлично от 0, то выставляем флаг ошибки пакета подтвердения:

            confirmationPackageErrorFlag = true;

            qDebug () << QString("Имитатор КСОН: Значение в поле статуса: %1 отлично от нуля").arg(statusFromPacket);
            emit message(QString("Имитатор КСОН: Значение в поле статуса: %1 отлично от нуля").arg(statusFromPacket), sv::log::llError, sv::log::mtError);

        }

        if ( confirmationPackageErrorFlag == true)
        { // Если при разборе пакета подтверждения мы обнаружили ошибку -> увеличиваем
            // счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:

            m_interactionErrorCounter++;

            qDebug() << QString("Имитатор КСОН: При разборе пакета подтверждения обнаружили ошибку. Количество идущих подряд ошибок взаимодействия : %1").arg(m_interactionErrorCounter);
            emit message(QString("Имитатор КСОН: При разборе пакета подтверждения обнаружили ошибку. Количество идущих подряд ошибок взаимодействия : %1").arg(m_interactionErrorCounter), sv::log::llError, sv::log::mtError);
        }
        else
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
    } // Если принят пакет подтверждения (длина 9 байт)

    if ( length != 9 && length != 4+4+2+m_params.receive_data_len)
    { // Если размер принятого кадра != 9 (размер пакета подтверждения) и
        // размер принятого кадра != 4+4+2+m_params.data_len (размер информационного кадра)

        // Выдаём сообщение об ошибке:
        emit message(QString("Имитатор КСОН: Длина принятого от КСОН сообщения равна %1").arg(length), sv::log::llError, sv::log::mtError);
        qDebug() << QString("Имитатор КСОН: Длина принятого от КСОН сообщения равна %1").arg(length);

        // Увеличиваем счётчик подряд идущих ошибок взаимодействия АПАК с КСОН:
        m_interactionErrorCounter++;
    } // Если размер принятого пакета не соответствует ни длине информационного кадра (12 байт)
      // ни длине пакета подтверждения (9 байт).

    if (m_interactionErrorCounter >= m_params.numberOfErrors)
    { // Если счётчик подряд идущих ошибок взаимодействия АПАК с КСОН больше значения
      // указанного в протоколе, то испускаем для интерфейсной части сигнал "say"
      // с командой "breakConnection", который приказывает интерфейсной части разорвать
      // соединение с АПАК:
        qDebug() << QString ("Имитатор КСОН: Выдаём команду интерфейсной части на разрыв соединения с TCP-клиентом");
        emit message(QString("Имитатор КСОН: Выдаём команду интерфейсной части на разрыв соединения с TCP-клиентом"), sv::log::llError, sv::log::mtError);

        emit p_io_buffer->say("breakConnection");
    }
}


void apak::SvKsonImitator::noConfirmationPackage(void)
// Эта функция вызывается в том случае, если в течении предельно допустимого времени от
// посылки нами информационного кадра к АПАК, до получения нами пакета подтверждения от АПАК
// пакет подтверждения так и не пришёл.
{
    // 1. Выводим в утилиту "logview" и на консоль информацию о том, что пакет
    // подтверждения не получен:
    emit message(QString("Имитатор КСОН: Пакет подтверждения от АПАК за время: %1 не получен").arg(m_params.conform_interval), sv::log::llError, sv::log::mtError);
    qDebug() << QString("Имитатор КСОН: Пакет подтверждения от АПАК за время: %1 не получен").arg(m_params.conform_interval);

    // 2. Увеличиваем счётчик подряд идущих ошибок взаимодействия АПАК с КСОН.
    m_interactionErrorCounter++;

    // 3. Если счётчик подряд идущих ошибок взаимодействия АПАК с КСОН больше значения
    // указанного в протоколе, то испускаем для интерфейсной части сигнал "say"
    // с командой "breakConnection", который приказывает интерфейсной части разорвать
    // соединение с TCP-клиентом:
    if (m_interactionErrorCounter >= m_params.numberOfErrors)
    {
        qDebug() << QString ("Имитатор КСОН: Выдаём команду интерфейсной части на разрыв соединения с TCP-клиентом");
        emit message(QString("Имитатор КСОН: Выдаём команду интерфейсной части на разрыв соединения с TCP-клиентом"), sv::log::llError, sv::log::mtError);

        emit p_io_buffer->say("breakConnection");
    }

    // 4. Даже если пакета подтверждения не пришло в течении оговоренного в протоколе времени
    // (m_params.conform_interval), то мы всё равно запускаем на единственное срабатывание
    // "таймер посылки m_sendTimer", чтобы продолжать посылать информационные кадры к АПАК:
    m_sendTimer->start(m_params.send_interval);
}


void apak::SvKsonImitator::noReceivePackage(void)
// Эта функция вызывается в том случае, если в течении предельно допустимого времени от
// сети КСОН не пришёл инфомационный кадр.
{
    // 1. Выводим в утилиту "logview" и на консоль информацию о том, что информационный кадр от КСОН
    // не получен:
    emit message(QString("Имитатор КСОН: Информационный кадр от АПАК за время: %1 не получен").arg(m_params.receive_interval), sv::log::llError, sv::log::mtError);
    qDebug() << QString("Имитатор КСОН: Информационный кадр от АПАК за время: %1 не получен").arg(m_params.receive_interval);

    // 2. Увеличиваем счётчик подряд идущих ошибок взаимодействия АПАК с КСОН.
    m_interactionErrorCounter++;

    // 3. Если счётчик подряд идущих ошибок взаимодействия АПАК с КСОН больше значения
    // указанного в протоколе, то испускаем для интерфейсной части сигнал "say"
    // с командой "breakConnection", который приказывает интерфейсной части разорвать
    // соединение с TCP-клиентом:
    if (m_interactionErrorCounter >= m_params.numberOfErrors)
    {
        qDebug() << QString ("Имитатор КСОН: Выдаём команду интерфейсной части на разрыв соединения с TCP-клиентом");
        emit message(QString("Имитатор КСОН: Выдаём команду интерфейсной части на разрыв соединения с TCP-клиентом"), sv::log::llError, sv::log::mtError);

        emit p_io_buffer->say("breakConnection");
    }

    // 4. Даже если инфомационного кадра не пришло в течении оговоренного в протоколе времени
    // (m_params.receive_interval), то мы всё равно запускаем на единственное срабатывание
    // "таймер приёма m_receiveTimer", чтобы продолжать отображать в утилите "logview" и
    // на консоли факт отсутствия информационных кадров от АПАК:
    m_receiveTimer->start(m_params.receive_interval);
}

void apak::SvKsonImitator::sendConfirmationPackage(void)
// Формирование и посылка пакета подтверждения.
// На данный момент - формат пакета подтверждения не ясен, поэтому будем реализовывать следующий
// формат:
// Размер данных - 4 байта
// Время - 4 байта
// Статус - 1 байт
{
    // В переменной "m_send_data" формируем пакет подтверждения для передачи от имитатора КСОН к АПАК
    // в соответствии с протоколом обмена.

    // 1. Вначале, выделим память:
    m_send_data.fill( 0, 9);

    // 2. Формируем первое поле пакета подтверждения:
    // Первое поле (4 байта) - размер данных (4 + 4 + 1 = 9 байт): 0x0, 0x0, 0x0, 0х09
    m_send_data [0] = 0;
    m_send_data [1] = 0;
    m_send_data [2] = 0;
    m_send_data [3] = 0x09;

    // 3. Формируем второе поле пакета подтверждения - время, которое должно повторять
    // время в информационном кадре от АПАК, сохранённое нами в переменной "m_packetTimeFrom_APAK":

    m_send_data [4] = (uint8_t) ((m_packetTimeFrom_APAK >> 24) & 0xFF);
    m_send_data [5] = (uint8_t) ((m_packetTimeFrom_APAK >> 16) & 0xFF);
    m_send_data [6] = (uint8_t) ((m_packetTimeFrom_APAK >>  8) & 0xFF);
    m_send_data [7] = (uint8_t) (m_packetTimeFrom_APAK & 0xFF);

    // 4. Записываем третье поле пакета подтверждения - статус. Оно сформировано нами в процессе
    // разбора информационного кадра от АПАК:
    m_send_data [8] = m_status;


    qDebug() <<"Имитатор КСОН: Пакет подтверждения (в окончательном виде) от КСОН к АПАК:";
    qDebug() <<"Имитатор КСОН: Размер: " << m_send_data.length();
    qDebug() <<"Имитатор КСОН: Содержание: " << m_send_data.toHex();

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
  modus::SvAbstractProtocol* protocol = new apak::SvKsonImitator();
  return protocol;
}

