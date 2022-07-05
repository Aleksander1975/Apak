#include "hmi.h"

apak::SvHMI::SvHMI():
  modus::SvAbstractProtocol()
{

}

apak::SvHMI::~SvHMI()
{
  deleteLater();
}

bool apak::SvHMI::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
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

bool apak::SvHMI::bindSignal(modus::SvSignal *signal, modus::SignalBinding binding)
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    m_max_register = 0;

    if(r) {

      if(binding.mode == modus::Master) {

        m_input_signals.insert(signal, hmi::SignalParams());

      }
      else {

        //! параметры params для каждого свои! для master'a свои, а для bindings - свои
        hmi::SignalParams params = hmi::SignalParams::fromJson(binding.params);
        m_params_by_signals.insert(signal, params);

//        qDebug()<< signal->config()->name <<

        if(!m_signals_by_registers.contains(params.registr))
          m_signals_by_registers.insert(params.registr, QList<modus::SvSignal*>());

        m_signals_by_registers[params.registr].append(signal);

        if(params.registr > m_max_register)
          m_max_register = params.registr;

      }
    }

    return r;

  }
  catch(SvException& e) {

    p_last_error = e.error;
    return false;

  }
}

void apak::SvHMI::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvHMI::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvHMI::start()
{
  QTimer* m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &SvHMI::putout);
  m_timer->start(m_params.interval);

//  connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &SvGammaOpaImitator::parse);

  p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvHMI::putout()
{
  QMap<quint16, quint16> valueByRegister;

  for(quint16 registr = 0x0000; registr <= m_max_register; registr++) {

    if(m_signals_by_registers.contains(registr)) {

      quint16 value = 0;

      for(modus::SvSignal* signal: m_signals_by_registers.value(registr)) {

        bool ok;
        quint16 signal_value = signal->value().toUInt(&ok);
        if(!ok)
          continue;

        hmi::SignalParams params = m_params_by_signals.value(signal);

        quint16 mask = ((1 << params.len) - 1) << params.offset; // даже если больше 1го бита
        value |= mask & (signal_value << params.offset);

  //      qDebug() << "mask" << mask << "value" << value << signal->config()->name << signal->value();

      }

  //    if(registr == 1 && (value & 0xFF) == 0xFE)
  //      value |= 0xFF;

      valueByRegister.insert(registr, value);

    }
    else
      valueByRegister.insert(registr, 0x0000);

  }

  QByteArray data;
  QDataStream stream(&data, QIODevice::WriteOnly);

  stream << quint8(m_params.address)                                      // Адрес устройства
         << quint8(m_params.func_code)                                    // Функциональный код. у нас 0x10
         << valueByRegister.firstKey()                                    // Адрес первого регистра
         << quint16(valueByRegister.count())                              // Количество регистров
         << quint8 (valueByRegister.count() * m_params.register_len);     // Количество байт

  //! With QMap, the items are always sorted by key.
  for(quint16 registr: valueByRegister.keys())
    stream << quint16(valueByRegister.value(registr));                    // данные

  quint16 crc = CRC::MODBUS_CRC16((const quint8*)data.data(), data.length()); // Контрольная сумма CRC
  data.append(quint8(crc & 0xFF));
  data.append(quint8(crc >> 8));

  p_io_buffer->output->mutex.lock();

  p_io_buffer->output->setData(data.data(), data.length());
  p_io_buffer->output->setReady(true);

  p_io_buffer->output->mutex.unlock();

  emit p_io_buffer->readyWrite(p_io_buffer->output);
}

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvHMI();
  return protocol;
}
