#include "skm_imitator.h"

apak::SvSkmImitator::SvSkmImitator():
  modus::SvAbstractProtocol()
{

}

apak::SvSkmImitator::~SvSkmImitator()
{
  deleteLater();
}

bool apak::SvSkmImitator::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
{
  try {

    p_config = config;
    p_io_buffer = iobuffer;

    m_params = skm::ProtocolParams::fromJson(p_config->protocol.params);

    return true;

  } catch (SvException& e) {

    p_last_error = e.error;
    return false;

  }
}

bool apak::SvSkmImitator::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    skm::SignalParams signal_params = skm::SignalParams::fromJson(binding.params);

    if(r) {

      if(binding.mode == modus::Master) {


      }
      else {

        if(!m_signals_by_vin.contains(signal_params.vin))
          m_signals_by_vin.insert(signal_params.vin, QList<modus::SvSignal*>());

        m_signals_by_vin[signal_params.vin].append(signal);

        if(!m_params_by_signal.contains(signal))
          m_params_by_signal.insert(signal, signal_params);

      }
    }

    return r;
  }

  catch(SvException& e) {
    p_last_error = e.error;
    return false;
  }
}

void apak::SvSkmImitator::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvSkmImitator::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvSkmImitator::start()
{
  m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &SvSkmImitator::send);
  m_timer->start(m_params.send_interval);

  p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvSkmImitator::send()
{
  if(p_is_active) {

    QByteArray vin_data;
    QDataStream vin_stream(&vin_data, QIODevice::WriteOnly);

    QByteArray factors_data;

    quint8 vin_count = 0;

    for(quint8 vin: m_signals_by_vin.keys()) {

      factors_data.clear();
      QDataStream factors_stream(&factors_data, QIODevice::WriteOnly);

      for(modus::SvSignal* signal: m_signals_by_vin.value(vin)) {

        if(!signal->value().isValid() || signal->value().isNull())
          continue;

        bool ok;
        quint8 v = signal->value().toUInt(&ok);

        if(!ok || v <= 0)
          continue;

        factors_stream << m_params_by_signal.value(signal).factor;

      }

      if(!factors_data.isEmpty()) {

        vin_stream << vin;
        vin_stream << static_cast<quint8>(factors_data.length());

        vin_stream.writeRawData(factors_data.data(), factors_data.length());

        vin_count++;

      }
    }

    QByteArray header2;
    QDataStream h2stream(&header2, QIODevice::WriteOnly);
    h2stream.setByteOrder(QDataStream::LittleEndian);

    h2stream << quint32(9);                 //Размер заголовка (=9 байт)
    h2stream << quint32(vin_data.length()); // Размер данных (см. таблицу 3)
    h2stream << quint8(vin_count);          // Количество камер в пакете

    send_data.clear(); //!

    QDataStream send_stream(&send_data, QIODevice::WriteOnly);

    send_stream << quint8(0x12) << quint8(0x34) << quint8(0x56) << quint8(0x78);
    send_stream.writeRawData(header2.data(), header2.length());
    send_stream.writeRawData(vin_data.data(), vin_data.length());

    quint16 crc = crc::crc16ccitt(send_data);
    send_stream << quint8(crc & 0xFF) << quint8(crc >> 8);

    p_io_buffer->output->mutex.lock();
    p_io_buffer->output->setData(send_data);

//    memcpy(&p_io_buffer->output->data[0], send_data.data(), send_data.length());
//    p_io_buffer->output->offset = send_data.length();

    emit message(QString(send_data), lldbg, sv::log::mtNew);

    p_io_buffer->output->setReady(true);
    emit p_io_buffer->readyWrite(p_io_buffer->output);

    p_io_buffer->output->mutex.unlock();

  }
}

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvSkmImitator();
  return protocol;
}

