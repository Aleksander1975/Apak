#include "kson_imitator.h"


apak::SvKsonImitator::SvKsonImitator():
  modus::SvAbstractProtocol()
{
    // Очистим cловарь, который каждому смещению значения сигнала относительно начала информационного
    // блока, передаваемого из КСОН в АПАК, ставит в соответствие указатель на этот сигнал:
    m_signals_by_byte.clear();

    // Очистим список указателей на сигналы, отсортированный по возрастанию смещений
    // значений сигналов (в байтах) относительно начала информационного блока,
    // передаваемого из КСОН в АПАК.
    m_signals.clear();

    // Очистим словарь, который каждому сигналу ставит в соответствие структуру SignalParams,
    // хранящую параметры для этого сигнала:
    m_params_by_signal.clear();

    // Очистим битовый массив, который для каждого номера группы сигналов (номер - это индекс в этом массиве),
    // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны):
    m_relevance_by_group.clear();
    m_relevance_by_group.resize(16);
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
        // (периодом поступления данных в мс от КСОН в систему АПАК)
        m_params = kson::ProtocolParams::fromJson(p_config->protocol.params);

        return true;
    }
    catch (SvException& e)
    {
        p_last_error = e.error;
        return false;
    }
}

bool apak::SvKsonImitator::bindSignal(modus::SvSignal *signal, modus::SignalBinding binding)
// Эта функция вызывается сервером "mdserver" для всех сигналов имитатора КСОН.
// Её цель состоит в получении всех параметров для каждого сигнала и заполнении в соответствии
// с ними структур данных (словари "m_signal_by_byte", "m_params_by_signal", "m_relevance_by_group"),
// необходимых для формирования информационного кадра от сети КСОН к системе АПАК.
{
  try
    {

        bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

        // Заполняем структуру SignalParams параметрами конкретного сигнала сети КСОН:
        // - количеством байт от начала информационного блока (параметр "byte") кадра от КСОН к АПАК.
        // - количеством бит (параметр "оffset") от начала байта на который указывает параметр "byte".
        // - размером области бит, которая хранит значение сигнала (параметр "len").
        // - тип данных сигнала ("unsigned", "float", "boolean").
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
        kson::SignalParams signal_params = kson::SignalParams::fromJson(signal->config()->params);

        if(r)
        {

            if(binding.mode == modus::Master)
            {
            }
            else
            {
                // Вставляем в словарь "m_signals_by_byte" пару: КЛЮЧ - смещение (в байтах) значения
                // сигнала относительно начала информационного блока, передаваемого из КСОН в АПАК;
                // ЗНАЧЕНИЕ - указатель на сигнал.
                m_signals_by_byte.insert(signal_params.byte, signal);

                // Вставляем в словарь "m_params_by_signal" пару: КЛЮЧ - указатель на сигнал; ЗНАЧЕНИЕ -
                // структуру "signal_params", содержащую параметры этого сигнала.
                m_params_by_signal.insert (signal, signal_params);

                // В битовом массиве "m_relevance_by_group" устанавливаем в "true" бит, соответствующий
                // номеру группы, к которой принадлежит сигнал. Этим мы обозначаем актуальность группы.
                m_relevance_by_group [signal_params.group] = true;
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

void apak::SvKsonImitator::start()
// В этой функции мы осуществляем привязку вызова функции send() к наступлению таймаута
// таймера m_timer. Кроме того, в ней мы сформируем список "m_signals", хранящий
// указатели на сигналы. Этот список отсортирован по возрастанию смещений
// значений сигналов (в байтах) относительно начала информационного блока,
// передаваемого из КСОН в АПАК.
{
  m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &SvKsonImitator::send);
  m_timer->start(m_params.send_interval);

  // Формируем список "m_signals":
  m_signals = m_signals_by_byte.values();

  p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvKsonImitator::send()
// В этой функции мы формируем и помещаем в массив байт m_send_data кадр для передачи от имитатора
// сети КСОН в систему АПАК (в соответствии с протоколом обмена) и инициируем передачу
// этого кадра от протокольной к интерфейcной части имитатора (для передачи по линии связи).

// На данный момент - формат информационного кадра не ясен, поэтому будем реализовывать следующий
// формат:
// Размер данных - 4 байта
// Время - 4 байта
// Доступность групп (группы сигналов от сети КСОН имеют номера с 10 по 12) - 2 байта
// Данные кадра (блок параметрической информации) - 33 байта
// Порядок байт во всех полях кадра - от старшего к младшему (bigEndian).
{
    qDebug () << "\n" << "КСОН: Вызов send()";

    if(p_is_active)
    {
        // В переменной "m_relevanceСoncrete_by_group" формируем битовый массив,
        // который для каждого номера группы сигналов (номер - это индекс в этом массиве),
        // определяет актуальны ли её сигналы (false - не актуальны, true - актуальны).

        // Изначально уставновим актуальность групп сигналов, исходя из наличия сигналов
        // данной группы в файле сигналов для устройства КСОН.
        m_relevanceConcrete_by_group = m_relevance_by_group;

        // В переменной "m_send_data" формируем информационный кадр от имитатора сети КСОН
        // в систему АПАК в соответствии с протоколом обмена.
        // Вначале, выделим память и заполним первые два поля:
        m_send_data.fill( 0, 4 + 4);

        // 1. Формируем первое поле информационного кадра:
        // Первое поле (4 байта) - размер данных (43 байта): 0x0, 0x0, 0x0, 0х2b
        m_send_data [0] = 0;
        m_send_data [1] = 0;
        m_send_data [2] = 0;
        m_send_data [3] = 0x2b;

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

        // Переменную "dataFrame" будем формировать с помощью переменной "dataFrameStream":
        QDataStream dataFrameStream(&dataFrame, QIODevice::WriteOnly);

        // Согласно протоколу числа типа "float" должны занимать 4 байта:
        dataFrameStream.setFloatingPointPrecision(QDataStream::SinglePrecision);

        // Формируем информационный блок кадра, передаваемого от КСОН к АПАК:
        for (quint8 signalNumberInDataFrame = 0; signalNumberInDataFrame < m_signals.size();
                 signalNumberInDataFrame++)
        { // Проходим по всем номерам параметров (сигналов) в блоке:

            // 3.1. Получаем указатель на сигнал:
            modus::SvSignal* signal = m_signals [signalNumberInDataFrame];

            // 3.2. Получаем параметры сигнала:
            kson::SignalParams signal_params = m_params_by_signal.value (signal);

            // 3.3. Проверим актуальна ли информация, содержащаяся в сигнале:
            if(!signal->value().isValid() || signal->value().isNull())
            {
                // 3.3.1. Информация в сигнале не актуальна ->

                // 3.3.1.1.Сбросим в "false" соответствующий группе этого сигнала бит в
                // массиве "m_relevanceСoncrete_by_group", то есть объявим сигналы
                // этой группы неактуальными:
                m_relevanceConcrete_by_group [signal_params.group] = false;

                // 3.3.1.2. Выясним тип сигнала ("boolean", "unsigned", "false") и запишем в
                // поток НУЛЕВОЕ значение этого типа.

                // Получаем тип сигнала:
                QString type = signal_params.type;

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
            }

            // 3.4. Получаем тип сигнала и попытаемся преобразовать содержимое
            // переменной "signalValue" к этому типу:
            QString type = signal_params.type;

            if ( type.compare ("boolean", Qt::CaseInsensitive) == 0)
            {  // 3.4.1. Этот сигнал представляет тип "boolean" и, в случае КСОН,
               // занимает младший бит в байте (остальные биты в этом
               // байте не важны). Поэтому мы будем записывать в поток целый байт.

                bool booleanSignal;

                // 3.4.1.1. Теперь переведём информацию в сигнале в тип "boolean":
                booleanSignal = signal->value().toBool();

                // 3.4.1.2. Выводим значение сигнала в поток:
                if (booleanSignal == true)
                    dataFrameStream << (quint8) 0x01;
                else
                    dataFrameStream << (quint8) 0x00;
            } // Этот сигнал представляет тип "boolean"

            if ( type.compare ("float", Qt::CaseInsensitive) == 0)
            { // 3.4.2. Этот сигнал представляет тип "float" и занимает 4 байта, порядок которых: big-endian.

                bool ok;
                float floatSignal;

                // 3.4.2.1. Теперь проверим представима ли информация в сигнале числом типа "float":
                floatSignal = signal->value().toFloat(&ok);

                if(ok == false )
                {
                    // 3.4.2.2. Информация в сигнале не представима числом типа "float" ->
                    // 3.4.2.2.1. Cбросим в "false" соответствующий группе этого сигнала бит в массиве
                    // "m_relevanceСoncrete_by_group", то есть объявим сигналы этой группы неактуальными:
                    m_relevanceConcrete_by_group [signal_params.group] = false;

                    // 3.4.2.2.2. Запишем в поток НУЛЕВОЕ значение типа "float":
                    dataFrameStream << (float) 0;
                }
                else
                {
                    // 3.4.2.3. Выводим значение сигнала в поток:
                    dataFrameStream << floatSignal;
                }
            } // Этот сигнал представляет тип "float" и занимает 4 байта

            if ( type.compare ("unsigned", Qt::CaseInsensitive) == 0)
            { // 3.4.3. Этот сигнал представляет тип "unsigned" и занимает 4 байта, порядок которых: big-endian.

                bool ok;
                unsigned unsignedSignal;

                // 3.4.3.1 Теперь проверим представима ли информация в сигнале числом типа unsigned:
                unsignedSignal = signal->value().toUInt (&ok);

                if(ok == false )
                {
                    // 3.4.3.2.Информация в сигнале не представима числом типа "unsigned" ->
                    // 3.4.3.2.1. Cбросим в "false" соответствующий группе этого сигнала бит в массиве
                    // "m_relevanceСoncrete_by_group", то есть объявим сигналы этой группы неактуальными:
                    m_relevanceConcrete_by_group [signal_params.group] = false;

                    // 3.4.3.2.2. Запишем в поток НУЛЕВОЕ значение типа "unsigned":
                    dataFrameStream << (unsigned) 0;
                }
                else
                {
                    // 3.4.3.3. Выводим значение сигнала в поток:
                    dataFrameStream << unsignedSignal;
                }
           } // Этот сигнал представляет тип "unsigned" и занимает 4 байта
        } // for

        // 4. Формируем поле "доступность групп". Это поле представляет собой два байта (сначала -
        // старший, затем - младший). При этом: младший бит младшего байта - соответветствует
        // нулевой группе. Данные для поля "доступность групп" формируем в  массиве бит
        // "m_relevanceConcrete_by_group".

        // 4.1. Конвертируем данные из массива бит "m_relevanceConcrete_by_group" в массив байт
        // "relevanceGroup_Byte".
        // Для начала заполняем массив байт "relevanceGroup_Byte" нулями:
        QByteArray relevanceGroup_Byte(2, 0);

        // В переменной цикла "bitNumberIn_relevance_by_group" будем хранить текущий номер бита в
        // массиве "relevanceGroup_Byte".
        for(quint16  bitNumberIn_relevance_by_group = 0; bitNumberIn_relevance_by_group <16;
                       bitNumberIn_relevance_by_group++)
        {
            relevanceGroup_Byte [bitNumberIn_relevance_by_group / 8] = relevanceGroup_Byte.at(bitNumberIn_relevance_by_group / 8) |
                         ((m_relevanceConcrete_by_group [bitNumberIn_relevance_by_group] ? 1 : 0) << (bitNumberIn_relevance_by_group % 8));
        }

        // 4.2. Перепишем данные из массива байт "relevanceGroup_Byte" в 8-ой и 9-ый байты массива
        // "m_send_data". При этом 0-ой байт массива байт "relevanceGroup_Byte" перепишем в 9-ый байт массива
        // "m_send_data", a 1-ый байт массива "relevanceGroup_Byte" перепишем в 8-ой байт массива
        // "m_send_data", так как во всех полях кадра "m_send_data" порядок байт от
        // старшего к младшему (bigEndian).
        m_send_data [8] = relevanceGroup_Byte[1];
        m_send_data [9] = relevanceGroup_Byte[0];

        // 5. Добавим к трём полям информационного кадра, уже содержащимся в массиве "m_send_data",
        // четвёртое - блок параметрической информации, сформированный нами в массиве "dataFrame".
        m_send_data.append (dataFrame);

        qDebug() << "m_relevanceConcrete_by_group" << m_relevanceConcrete_by_group;

        qDebug() << "Блок параметрической информации от сети КСОН:";
        qDebug() << "Размер: " << dataFrame.length();
        qDebug() << "Содержание: " << dataFrame.toHex();

        qDebug() << "Информационный кадр от сети КСОН:";
        qDebug() << "Размер: " << m_send_data.length();
        qDebug() << "Содержание: " << m_send_data.toHex();

        // 6. Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
       p_io_buffer->output->mutex.lock();
       p_io_buffer->output->setData(m_send_data);

       emit message(QString(m_send_data.toHex()), lldbg, sv::log::mtNew);

       p_io_buffer->output->setReady(true);
       emit p_io_buffer->readyWrite(p_io_buffer->output);

       p_io_buffer->output->mutex.unlock();

    } //if(p_is_active)
}

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvKsonImitator();
  return protocol;
}

