#include "krab_imitator.h"

apak::SvKrabImitator::SvKrabImitator():
  modus::SvAbstractProtocol()
{

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

    m_params = hmi::ProtocolParams::fromJson(p_config->protocol.params);

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

    m_max_byte = 0;

    if(r) {

      if(binding.mode == modus::Master) {

        m_input_signals.insert(signal, hmi::SignalParams());

      }
      else {

        //! параметры params для каждого свои! для master'a свои, а для bindings - свои
        hmi::SignalParams params = hmi::SignalParams::fromJson(binding.params);
        m_params_by_signals.insert(signal, params);

        std::pair<uint8_t, modus::SvSignal*> pair(params.byte, signal);
        m_signals_by_byte_number.insert(pair);

        if(params.byte > m_max_byte)
          m_max_byte = params.byte;

      }
    }

    return r;

  }
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
  QTimer* m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &SvKrabImitator::putout);
  m_timer->start(m_params.interval);

//  connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &SvGammaOpaImitator::parse);

  p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvKrabImitator::putout()
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
