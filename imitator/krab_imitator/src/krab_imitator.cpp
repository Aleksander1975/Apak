#include "krab_imitator.h"

apak::SvKrabImitator::SvKrabImitator():
  modus::SvAbstractProtocol()
{
    // Очистим словарь, который каждому номеру бита (от 0 до 66*8-1) из битового массива
    // "m_bitsPacket" ставит в соответствие сигнал, бит которого должен быть записан
    // в массив "m_bitsPacket":
    m_signal_by_bitNumberInPacket.clear();

    // Очистим словарь, который каждому номеру бита (от 0 до 66*8-1) из битового массива
    // "m_bitsPacket" ставит в соответствие номер бита сигнала, который
    // должен быть записан в массив "m_bitsPacket":
    m_bitNumberInSignal_by_bitNumberInPacket.clear();
}

apak::SvKrabImitator::~SvKrabImitator()
{
  deleteLater();
}

bool apak::SvKrabImitator::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
{
  try {

    p_config = config;
    p_io_buffer = iobuffer;

    m_params = krab::ProtocolParams::fromJson(p_config->protocol.params);

    return true;

  } catch (SvException& e) {

    p_last_error = e.error;
    return false;
  }
}

bool apak::SvKrabImitator::bindSignal(modus::SvSignal *signal, modus::SignalBinding binding)
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    // Заполняем структуру SignalParams параметрами конкретного сигнала устройства КРАБ:
    // количеством байт от начала пакета (параметр "byte"), количеством бит от начала байта
    // (параметр "оffset"), размером области бит, которая хранит значение сигнала (параметр "len").

    // Для КРАБ'a параметры сигнала ("byte", "offset" и "len") задаются не в разделе "bindings"
    // каждого сигнала из файла сигналов "krab.json", а прямо в разделе сигнала.
    // Поэтому аргументом функции SignalParams::fromJson() будет не "binding.params",
    // а "signal->config()->params".
    krab::SignalParams signal_params = krab::SignalParams::fromJson(signal->config()->params);

    if(r)
    {

      if(binding.mode == modus::Master)
      {
      }
      else
      {
          // Заполняем соответствующие сигналу, на который указывает аргумент "signal"
          // функции  "bindSignal", поля в структурах данных "m_signal_by_bitNumberInPacket"
          // и "m_bitNumberInSignal_by_bitNumberInPacket".

          // В цикле for () будем проходить по всем номерам битов сигнала (от 0 до "len").
          // В переменной цикла "bitNumberInSignal" будем хранить текущий номер бита сигнала.
          for (uint8_t bitNumberInSignal = 0; bitNumberInSignal < signal_params.len; bitNumberInSignal++)
          {
              m_signal_by_bitNumberInPacket [signal_params.byte * 8 + signal_params.offset +
                      bitNumberInSignal] = signal;

              m_bitNumberInSignal_by_bitNumberInPacket [signal_params.byte * 8 +
                      signal_params.offset + bitNumberInSignal] = bitNumberInSignal;
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
{
  m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &SvKrabImitator::send);
  m_timer->start(m_params.interval);

  p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvKrabImitator::send()
{
  std::vector<uint8_t> values;
  values.reserve(m_max_byte + 1);

  for(const auto& p: m_signals_by_byte_number) {

    bool ok;
    uint8_t signal_value = p.second->value().toUInt(&ok);
    if(!ok)
      signal_value = 0;

    values.insert(values.end(),signal_value);

  }

  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);

  stream << quint8(m_params.address)              // Адрес устройства
         << quint8(m_params.func_code)            // Функциональный код. у нас 0x10
         << 0x0000                                // Адрес первого регистра
//         << quint16(values.capacity())          // Количество регистров
         << quint8 (values.capacity());           // Количество байт

  int i = 0;
  qDebug() << values.size();
  for(auto v: values) {
//    qDebug() << v << i++;
    stream << v;                                  // данные
  }

  quint16 crc = CRC::MODBUS_CRC16((const quint8*)data.data(), data.length()); // Контрольная сумма CRC
  stream << quint8(crc & 0xFF);
  stream << quint8(crc >> 8);

  p_io_buffer->output->mutex.lock();

  p_io_buffer->output->setData(data);

  p_io_buffer->output->mutex.unlock();

  emit p_io_buffer->readyWrite(p_io_buffer->output);
}

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvKrabImitator();
  return protocol;
}
