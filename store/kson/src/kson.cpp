#include "kson.h"

apak::SvKson::SvKson():
  modus::SvAbstractProtocol(),
  m_data_signal(nullptr),
  m_state_signal(nullptr)
{

}

apak::SvKson::~SvKson()
{
  deleteLater();
}

bool apak::SvKson::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
{
  try {

    p_config = config;
    p_io_buffer = iobuffer;

//    m_params = apak::ProtocolParams::fromJson(p_config->protocol.params);

    return true;

  } catch (SvException& e) {

    p_last_error = e.error;
    return false;

  }
}

bool apak::SvKson::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    if(r) {

      if(binding.mode == modus::Master) {

        if(signal->config()->type.toLower() == TYPE_DATA) {

          if(m_data_signal) {

            p_last_error = TOO_MUCH(p_config->name, TYPE_DATA);
            return false;
          }

          m_data_signal = signal;

        }
        else if(signal->config()->type.toLower() == TYPE_STAT) {

          if(m_state_signal) {

            p_last_error = TOO_MUCH(p_config->name, TYPE_STAT);
            return false;
          }

          m_state_signal = signal;

        }
      }
      else if(binding.mode == modus::ReadOnly) {

        //! параметры (params) для master'a свои, а для ReadOnly - свои
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

void apak::SvKson::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvKson::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvKson::start()
{
//  QTimer* m_timer = new QTimer;
//  connect(m_timer, &QTimer::timeout, this, &SvUniversalPacket::checkupSignals);
//  m_timer->start(DEFAULT_INTERVAL);

  connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &SvKson::parse);

  p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvKson::parse(modus::BUFF* buffer)
{
  if(p_is_active) {

    buffer->mutex.lock();

    if(buffer->isReady()) {

      if(m_data_signal) {

        m_data_signal->setValue(QVariant(QByteArray(buffer->data, buffer->offset)));
        emit message(QString("signal %1 updated").arg(m_data_signal->config()->name), sv::log::llDebug, sv::log::mtParse);

      }

      if(m_state_signal)
        m_state_signal->setValue(int(1));

      buffer->reset();

//      p_last_parsed_time = QDateTime::currentDateTime();

    }

    buffer->mutex.unlock();

  }
}

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvKson();
  return protocol;
}

