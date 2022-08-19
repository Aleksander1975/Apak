#include "krab_imitator.h"

apak::SvKrabImitator::SvKrabImitator():
  modus::SvAbstractProtocol()
{
    // Очистим словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsDataField" ставит в соответствие сигнал, бит которого должен быть записан
    // в массив "m_bitsDataField":
    m_signal_by_bitNumberInDataField.clear();

    // Очистим словарь, который каждому номеру бита (от 0 до m_params.data_len*8-1) из битового массива
    // "m_bitsDataField" ставит в соответствие номер бита сигнала, который
    // должен быть записан в массив "m_bitsDataField":
    m_bitNumberInSignal_by_bitNumberInDataField.clear();
}

apak::SvKrabImitator::~SvKrabImitator()
{
  deleteLater();
}

bool apak::SvKrabImitator::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
// Эта функция вызывается серевером "mdserver" для всех имитаторов устройств.
// Её цель - инициализировать все структуры, необходимые нам для конкретного
// имитатора (в данном случае, имитатора КРАБ).
{
  try {

    p_config = config;
    p_io_buffer = iobuffer;

    // Заполняем структуру m_params параметром протокола обмена КРАБ'а с системой АПАК
    // - Смещением байта, в котором хранится значение сигнала, от начала пакета
    //                "запроса на запись" (количество байт от начала пакета),
    // - Смещением области битов, в которой хранится значение сигнала, от начала байта
    //                 (количество бит от начала байта),
    // - Размером области бит, которая хранит значение сигнала (количество бит).
    m_params = krab::ProtocolParams::fromJson(p_config->protocol.params);

    return true;

  } catch (SvException& e) {

    p_last_error = e.error;
    return false;
  }
}

bool apak::SvKrabImitator::bindSignal(modus::SvSignal *signal, modus::SignalBinding binding)
// Эта функция вызывается сервером "mdserver" для всех сигналов имитатора КРАБ.
// Её цель состоит в получении всех параметров для каждого сигнала и заполнинии в соответствии
// с ними структур данных (словари m_signal_by_bitNumberInDataField и
// m_bitNumberInSignal_by_bitNumberInDataField), необходимых для формирования пакета
// запроса на запись от КРАБ'а к АПАК.
{
    try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    // Заполняем структуру SignalParams параметрами конкретного сигнала устройства КРАБ:
    // количеством байт от начала пакета (параметр "byte"), количеством бит от начала байта
    // (параметр "оffset"), размером области бит, которая хранит значение сигнала (параметр "len").

    // Параметры сигналов устройства описываются в файле сигналов имитатора конкретного устройства.
    // В ранних версиях имитаторов устройств параметры сигналов описывались в подразделе "params"
    // раздела "bindings". Однако, поскольку файл сигналов имитатора конкретного устройства
    // используется, помимо имитатора, еще программой для отображения информации, записанной от
    // этого устройства в чёрный ящик, то подраздел "params" был перенесён из раздела "bindings"
    // на уровень выше - прямо в общий раздел сигнала (туда же где находятся: идентификатор сигнала,
    // имя сигнала, описание сигнала...). Поэтому аргументом функции SignalParams::fromJson()
    // является "signal->config()->params".
    krab::SignalParams signal_params = krab::SignalParams::fromJson(signal->config()->params);

    if(r)
    {

      if(binding.mode == modus::Master)
      {
      }
      else
      {
          // Заполняем соответствующие сигналу, на который указывает аргумент "signal"
          // функции  "bindSignal", поля в структурах данных "m_signal_by_bitNumberInDataField"
          // и "m_bitNumberInSignal_by_bitNumberInDataField". Вычитание константы KRAB_HEADER_LEN
          // необходимо, так как параметр сигнала "byte" в файле сигналов КРАБ указывается от
          // начала пакета, а не от начала поля данных запроса на запись от КРАБ'а к АПАК.

          // В цикле for () будем проходить по всем номерам битов сигнала (от 0 до "len").
          // В переменной цикла "bitNumberInSignal" будем хранить текущий номер бита сигнала.
          for (uint8_t bitNumberInSignal = 0; bitNumberInSignal < signal_params.len; bitNumberInSignal++)
          {
              m_signal_by_bitNumberInDataField [(uint16_t)signal_params.byte * 8 + signal_params.offset +
                      bitNumberInSignal - KRAB_HEADER_LEN*8] = signal;

              m_bitNumberInSignal_by_bitNumberInDataField [(uint16_t)signal_params.byte * 8 +
                      signal_params.offset + bitNumberInSignal - KRAB_HEADER_LEN*8] = bitNumberInSignal;
          }
      }
    } // if (r)

    return r;
  } // try

  catch(SvException& e) {

    p_last_error = e.error;
    return false;
  }
}

void apak::SvKrabImitator::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvKrabImitator::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvKrabImitator::start()
// В этой функции мы осуществляем привязку вызова функции send() к наступлению таймаута таймера m_timer.
{
  m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &SvKrabImitator::send);
  m_timer->start(m_params.send_interval);

  p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvKrabImitator::send()
{
    qDebug() << "КРАБ: Вызов send()";
    if (p_is_active == false)
        return;

    // 1. В переменной m_bitsDataField будем формировать поле данных пакета запроса на запись
    // от КРАБ к АПАК. Для начала заполняем его нулями:
    m_bitsDataField.fill (false, m_params.data_len*8);

    // 2. В цикле for () будем проходить по всем номерам битов (от 0 до m_params.data_len*8 - 1)
    // из битового массива "m_bitsDataField". Данный массив битов представляет собой поле данных
    // пакета запроса на запись от КРАБ к АПАК. В переменной цикла "bitNumberInDataField" будем
    // хранить текущий номер бита массива "m_bitsDataField".
    for (quint16 bitNumberInDataField = 0; bitNumberInDataField <= m_params.data_len*8 - 1;
         bitNumberInDataField++)
    {
        // 2.1. Проверим, должен ли по индексу "bitNumberInDataField" в массиве "m_bitsDataField" храниться
        // бит какого-либо сигнала (по протоколу там есть "пустоты"):
        if (m_signal_by_bitNumberInDataField.contains(bitNumberInDataField) == false)
        { // Если не должен, то оставляем соответствующий бит массива "m_bitsDataField" нулевым
            //(переходим на следующую итерацию цикла):

            continue;
        }

        // 2.2. Если по индексу "bitNumberInDataField" в массиве "m_bitsDataField" должен храниться
        // бит какого-либо сигнала, то:
        // Получаем указатель на сигнал, один из битов которого должен храниться в массиве
        // "m_bitsDataField" по индексу "bitNumberInDataField":
        modus::SvSignal* signal = m_signal_by_bitNumberInDataField.value( bitNumberInDataField);

        // 2.3. Проверяем актуальна ли информация, содержащаяся в сигнале:
        if(!signal->value().isValid() || signal->value().isNull())
        {
            // Информация в сигнале не актуальна -> оставляем соответствующий бит массива
            // "m_bitsDataField" нулевым (переходим на следующую итерацию цикла):

            continue;
        }

        bool ok;

        // 2.4. Теперь проверим представима ли информация в сигнале беззнаковым целым числом:
        quint8 intValueSignal = signal->value().toUInt(&ok);
        if(ok == false )
        {
            // Информация в сигнале не представима беззнаковым целым числом ->
            // оставляем соответствующий бит массива "m_bitsDataField" нулевым
            // (переходим на следующую итерацию цикла):

            continue;
         }

        // 2.5. Получаем номер бита сигнала, который должен храниться в массиве бит
        // "m_bitsDataField" по индексу "bitNumberInDataField":
        quint8 m_bitNumberInSignal =
                m_bitNumberInSignal_by_bitNumberInDataField.value (bitNumberInDataField);

        // 2.6. Получаем и сохраняем бит сигнала в массиве битов "m_bitsDataField":
        m_bitsDataField.setBit(bitNumberInDataField,
                                  (intValueSignal>>m_bitNumberInSignal) & 0x01 ? true: false);
    }

    qDebug() << m_bitsDataField;
     // 3. Теперь преобразуем массив бит "m_bitsDataField" в массив байт:
     // В этом массиве байт будет содержаться поле данных пакета запроса на запись от КРАБ к АПАК.
     // 3.1. Для начала заполняем этот массив байт нулями:
     QByteArray dataField (m_params.data_len, 0);

     // 3.2. Конвертируем данные из массива бит "m_bitsDataField" в массив байт "dataField".
     // В переменной цикла "bitNumberInDataField" будем хранить текущий номер бита в
     // массиве "m_bitsDataField".
     for(quint16  bitNumberInDataField = 0; bitNumberInDataField <m_params.data_len * 8;
                bitNumberInDataField++)
            dataField[bitNumberInDataField / 8] = dataField.at(bitNumberInDataField / 8) |
                  ((m_bitsDataField [bitNumberInDataField] ? 1 : 0) << (bitNumberInDataField % 8));

     // 4. В переменной m_send_data формируем пакет запроса на запись для передачи от КРАБ
     // в систему АПАК в соответствии с протоколом обмена:

     // 4.1. Формируем "заголовок" (6 байт):
     m_send_data.resize(6);

     // 4.1.1. Нулевой байт - адрес АПАК (0x02):
     m_send_data [0] = 0x02;

     // 4.1.2. Первый байт - код фукции (0x10):
     m_send_data [1] = 0x10;

     // 4.1.3. Второй и третий байты - адрес первого регистра (0х0000):
     m_send_data [2] = 0;
     m_send_data [3] = 0;

     // 4.1.4. Количество байт данных (длина поля данных в байтах).
     // Согласно протоколу MODBUS - сначала идёт старший байт (нулевой),
     // а затем младший (содержит значение m_params.data_len):
     // четвёртый байт - нулевой
     // пятый байт - содержит значение m_params.data_len:
     m_send_data [4] = 0;
     m_send_data [5] = m_params.data_len;

     // 4.2. Добавляем к заголовку поле данных, сформированное нами в массиве байт dataField[]:
     m_send_data.append(dataField);

     // 4.3. Считаем контрольную сумму CRC-16/MODBUS по "заголовку" и полю данных:
     quint16 crc = crc::crc16ibm(m_send_data);

     // 4.4. Добавляем контрольную сумму к пакету - сначала младший байт, затем старший:
     m_send_data.append(quint8(crc & 0xFF));
     m_send_data.append(quint8(crc >> 8));

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

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvKrabImitator();
  return protocol;
}
