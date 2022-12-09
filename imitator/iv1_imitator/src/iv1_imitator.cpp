#include "iv1_imitator.h"

apak::SvIv1Imitator::SvIv1Imitator():
  modus::SvAbstractProtocol()
{
    // Очистим cловарь, который каждому сигналу ставит в соответствие
    // структуру SignalParams, хранящую параметры для этого сигнала:
    m_params_by_signal.clear();

    // Очистим словарь, который каждому порядковому номеру датчика
    // ставит в соответствие сигнал, содержащий значение
    // температуры от данного датчика:
    m_temperature.clear();

    // Очистим словарь, который каждому порядковому номеру датчика
    // ставит в соответствие сигнал, содержащий значение
    // влажности от данного датчика:
    m_moisture.clear();
}

apak::SvIv1Imitator::~SvIv1Imitator()
{
  deleteLater();
}

bool apak::SvIv1Imitator::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
// Эта функция вызывается серевером "mdserver" для всех имитаторов устройств.
// Её цель - инициализировать все структуры, необходимые нам для конкретного
// имитатора (в данном случае, имитатора ИВ-1).
{
    try
    {
        p_config = config;
        p_io_buffer = iobuffer;

        // Заполняем структуру m_params параметром протокола обмена ИВ-1 с системой АПАК
        // (периодом поступления данных в мс от ИВ-1 в систему АПАК)
        m_params = iv1::ProtocolParams::fromJson(p_config->protocol.params);

        return true;
    }
    catch (SvException& e)
    {
         // Отображаем оператору сообщение о месте ошибке. Сообщение о самой ошибке
         // хранится в исключении (e.error) и будет передано в "mdserver" через "p_last_error":
         emit message(QString("Имитатор ИВ-1: Исключение в функции \"configure\""), sv::log::llError, sv::log::mtError);
         qDebug() << "Имитатор ИВ-1: Исключение в функции \"configure\"";

        p_last_error = e.error;
        return false;
    }
}

bool apak::SvIv1Imitator::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
// Эта функция вызывается сервером "mdserver" для всех сигналов имитатора устройства ИВ-1.
// (Структура SignalBinding описывает привязку ОДНОГО сигнала к ОДНОМУ устройству).
// Информация об этой привязке берётся из json-файлa сигналов для имитатора устройства ИВ-1 -
// (файл: iv-1.json).
// Замечание: вообще, каждый сигнал может быть привязан к одному устройству, которое устанавливает его
// значение (эти устройства в файле сигналов называются "master"), и к нескольким устройствам,
// которые "читают" его значение (такие устройства называют "bindings").
// В данном случае функция функция bindSignal() будет вызываться по одному разу для каждого из 42-ух
// сигналов имитатора ИВ-1 (первый аргумент функции) и binding - привязке этого сигнала к
// устройству ИВ-1 (второй аргумент).
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    // Заполняем структуру SignalParams параметрами конкретного сигнала устройства ИВ-1:
    // (порядковым номером датчика, старшим байтом заводского номера датчика, младшим
    // байтом заводского номера датчика, типом сигнала(температура или влажность)).

    // Параметры сигналов устройства описываются в файле сигналов имитатора конкретного устройства.
    // В ранних версиях имитаторов устройств параметры сигналов описывались в подразделе "params"
    // раздела "bindings" или "master". Однако, поскольку файл сигналов имитатора конкретного устройства
    // используется, помимо имитатора, еще программой для отображения информации, записанной от
    // этого устройства в чёрный ящик, то подраздел "params" был перенесён из разделов "bindings"
    // или "master" на уровень выше - прямо в общий раздел сигнала (туда же где находятся:
    // идентификатор сигнала, имя сигнала, описание сигнала...). Поэтому аргументом функции
    // SignalParams::fromJson() является "signal->config()->params".
    iv1::SignalParams signal_params = iv1::SignalParams::fromJson(signal->config()->params);

    if(r)
    {

      if(binding.mode == modus::Master)
      {
      }
      else
      {
        m_params_by_signal [signal] = signal_params;

        if (signal_params.signal_type == 0)
        { // Сигнал содержит значение температуры:
            m_temperature [signal_params.serial_number] = signal;
        }
        else
        { // Сигнал содержит значение влажности:
            m_moisture [signal_params.serial_number] = signal;
        }
      }
    } // if (r)

    return r;
  } // try

  catch(SvException& e)
  {
    p_last_error = e.error;

    //Получаем имя сигнала:
    QString signalName = signal ->config() ->name;

    // Отображаем оператору сообщение о месте ошибке. Сообщение о самой ошибке
    // хранится в исключении (e.error) и будет передано в "mdserver" через "p_last_error":
    emit message(QString("Имитатор ИВ-1: Исключение в функции \"bindSignal\" на сигнале: %1").arg(signalName), sv::log::llError, sv::log::mtError);
    qDebug() << QString ("Имитатор ИВ-1: Исключение в функции \"bindSignal\" на сигнале %1").arg(signalName);

    return false;
  }
}

void apak::SvIv1Imitator::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvIv1Imitator::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvIv1Imitator::start()
// В этой функции мы осуществляем привязку вызова функции send() к наступлению таймаута таймера m_timer.
{
  m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &SvIv1Imitator::send);
  m_timer->start(m_params.send_interval);

  p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvIv1Imitator::send()
// В этой функции мы формируем и помещаем в массив байт m_send_data пакет для передачи от имитатора
// устройства ИВ-1 в систему АПАК (в соответствии с протоколом обмена) и инициируем передачу
// этого пакета от протокольной к интерфейcной части имитатора (для передачи по линии связи).
{
    qDebug () << "Имитатор ИВ-1: Вызов send()";

    if(p_is_active)
    {
        // В переменной m_send_data формируем пакет данных от имитатора устройства ИВ-1
        // в систему АПАК в соответствии с протоколом обмена:
        m_send_data.resize(5);

        // 1. Формируем заголовок пакета (5 байт):
        // Первый байт - признак начала пакета - 0x1F:
        m_send_data [0] = 0x1F;

        // Второй байт - адрес системы АПАК РКИ - 0x09:
        m_send_data [1] = 0x09;

        // Третий байт - адрес ведущего блока ИВ-1 на линии - 0x01:
        m_send_data [2] = 0x01;

        // Четвёртый байт - признак протокола - 0x24:
        m_send_data [3] = 0x24;

        // Пятый байт - код команды 0x33:
        m_send_data [4] = 0x33;

        // 2. Формируем тело пакета - 21 поле (по 5 байт на датчик) о состоянии каждого из датчиков.

        // В этой переменной будем формировать тело пакета:
        QByteArray packageBody;

        foreach (uint8_t serialNumberOfTheSensor, m_temperature.keys())
        {
           // Формируем поле infoFromTheSensor (длиной 5 байт) о состоянии одного датчика.
           // Всего в данной версии протокола обмена устротйства ИВ-1 и системы АПАК имеется 21 датчик.
            QByteArray infoFromTheSensor;

            infoFromTheSensor.resize(5);

            // 2.1. Первый байт - порядковый номер датчика:
            infoFromTheSensor[0] = serialNumberOfTheSensor;

            // 2.2. Второй байт - старший байт заводского номера датчика:
            modus::SvSignal* signal = m_temperature.value(serialNumberOfTheSensor);
            iv1::SignalParams signal_params = m_params_by_signal.value (signal);

            infoFromTheSensor[1] = signal_params.high_factory_number;

            // 2.3. Третий байт - младший байт заводского номера датчика:
            infoFromTheSensor[2] = signal_params.low_factory_number;

            // 2.4. Четвёртый байт - температура:
            uint8_t temperature;

            // Сначала проверим актуальна ли информация, содержащаяся в сигнале:
            if(!signal->value().isValid() || signal->value().isNull())
            {
                // Информация в сигнале не актуальна -> выставляем в поле температуры значение 0xF0.
                infoFromTheSensor[3] = 0xF0;
            }
            else
            {
                bool ok;

                // Теперь проверим представима ли информация в сигнале беззнаковым целым числом:
                temperature = signal->value().toUInt(&ok);

                if(ok == false )
                {
                    // Информация в сигнале не представима беззнаковым целым числом ->
                    // выставляем в поле температуры значение 0xF0:
                    infoFromTheSensor[3] = 0xF0;
                }
                else
                {
                    // Устанавливаем значение температуры из сигнала:
                    infoFromTheSensor[3] = temperature;
                }
            }

           // 2.5. Пятый байт - влажность:
            uint8_t moisture;

            // По порядковому номеру датчика получим сигнал, содержащий значение влажности:
            signal = m_moisture.value(serialNumberOfTheSensor);

            // Сначала проверим актуальна ли информация, содержащаяся в сигнале:
            if(!signal->value().isValid() || signal->value().isNull())
            {
                // Информация в сигнале не актуальна -> выставляем в поле влажности значение 0xF0:
                infoFromTheSensor[4] = 0xF0;
            }
            else
            {
                bool ok;

                // Теперь проверим представима ли информация в сигнале беззнаковым целым числом:
                moisture = signal->value().toUInt(&ok);
                if(ok == false )
                {
                    // Информация в сигнале не представима беззнаковым целым числом ->
                    // выставляем в поле влажности значение 0xF0:
                    infoFromTheSensor[4] = 0xF0;
                }
                else
                {
                    // Устанавливаем значение влажности из сигнала:
                    infoFromTheSensor[4] = moisture;
                }
            }

            // Добавляем информацию о датчике в тело пакета packageBody:
            packageBody.append(infoFromTheSensor);
        } // foreach

       qDebug() << "Имитатор ИВ-1: Тело пакета от имитатора устройства ИВ-1 "
                   "(без заголовка и контрольной суммы, до удвоения байтов 0x1F, 0x2F, 0x55): ";
       qDebug() << "Имитатор ИВ-1: Размер: " << packageBody.length();
       qDebug() << "Имитатор ИВ-1: Содержание: " << packageBody.toHex();

       // 3. Считаем контрольную сумму тела пакета:
       quint16 crc = crc::crc16ccitt(packageBody);

       // 4. Добавляем контрольную сумму к телу пакета - сначала младший байт, затем старший:
       packageBody.append(quint8(crc & 0xFF));
       packageBody.append(quint8(crc >> 8));

       // 5. Удваиваем символы 0x1F, 0x2F, 0x55 в теле пакета и контрольной сумме:
       for (int i = 0; i < packageBody.size(); i++)
       {
           char ch = packageBody.at(i);

           if (ch == 0x1F || ch == 0x2F || ch == 0x55)
           {
               // Если символ равен 0x1F, 0x2F, 0x55 - вставляем такой же:
               packageBody.insert(i, ch);

               // Увеличиваем индекс, чтобы на следующем цикле не оказаться на вставленном сейчас элементе:
               i++;
           }          
       } // for (

       qDebug() << "Имитатор ИВ-1: Тело пакета от имитатора устройства ИВ-1 "
                   "(без заголовка, но с контрольной суммой, после удвоения байтов 0x1F, 0x2F, 0x55): ";
       qDebug() << "Имитатор ИВ-1: Размер: " << packageBody.length();
       qDebug() << "Имитатор ИВ-1: Содержание: " << packageBody.toHex();

       // 6. Добавляем к заголовку пакета (находящемуся в массиве байт m_send_data) тело пакета и
       // контрольную сумму (после увдоения символов 0x1F, 0x2F, 0x55):
       m_send_data.append(packageBody);

       // 7. Помещаем в пакет признак конца пакета: байты 0x2F и 0х55.
       m_send_data.append(0x2F);
       m_send_data.append(0x55);

       qDebug() << "Имитатор ИВ-1: Пакет (в окончательном виде) от имитатора устройства ИВ-1";
       qDebug() << "Имитатор ИВ-1: Размер: " << m_send_data.length();
       qDebug() << "Имитатор ИВ-1: Содержание: " << m_send_data.toHex();


       // 8.Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
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
  modus::SvAbstractProtocol* protocol = new apak::SvIv1Imitator();
  return protocol;
}

