#include "moxa.h"

apak::SvMoxa::SvMoxa():
  modus::SvAbstractProtocol()
{

}

apak::SvMoxa::~SvMoxa()
{
  deleteLater();
}

bool apak::SvMoxa::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
{
  try {

    p_config = config;
    p_io_buffer = iobuffer;

    m_params = moxa::ProtocolParams::fromJson(p_config->protocol.params);

    return true;

  } catch (SvException& e) {

    p_last_error = e.error;
    return false;

  }
}

bool apak::SvMoxa::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    moxa::SignalParams params = moxa::SignalParams::fromJsonString(binding.params);

    if(r) {

      if(binding.mode == modus::Master) {

        if(signal->config()->type.toLower() == TYPE_STAT) {

          if(!m_master_signals_by_registers.contains(params.registr))
            m_master_signals_by_registers.insert(params.registr, signal);

        }
      }
    }

    return r;
  }

  catch(SvException& e) {
    p_last_error = e.error;
    return false;
  }
}

void apak::SvMoxa::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvMoxa::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvMoxa::start()
{
  if(!(bool(p_config) && bool(p_io_buffer))) {

    emit message(QString("Panic! Устройство %1 не проинициализировано").arg(p_config->name), sv::log::llError, sv::log::mtError);
    return;
  }

  QTimer* m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &apak::SvMoxa::sendRequest);
  m_timer->start(m_params.interval);

  connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &apak::SvMoxa::parse);

}

void apak::SvMoxa::sendRequest()
{
  p_io_buffer->output->mutex.lock();

//  0102 0000 0006 01 04 1000 000C

  QByteArray request;
  QDataStream s(&request, QIODevice::ReadWrite);
  s.writeRawData(&REQUEST[0], sizeof(REQUEST));
  s << m_params.start_register << m_params.register_count;

  quint16 crc = crc::crc16ccitt(request);
  request.append(crc & 0xFF).append(crc >> 8);

  p_io_buffer->output->setData(request);

  p_io_buffer->output->setReady(true);
  emit p_io_buffer->readyWrite(p_io_buffer->output);

  p_io_buffer->output->mutex.unlock();
}

void apak::SvMoxa::parse(modus::BUFF* buffer)
{
  buffer->mutex.lock();

  if(buffer->isReady()) {

    QByteArray ba(buffer->data, buffer->offset);
    QDataStream st(ba);
    MOXAReplay r;

    st >> r.id >> r.proto >> r.msglen >> r.address >> r.func >> r.bytecnt;

    quint16 regdata;
    int cnt = 0;
    for(quint16 reg = m_params.start_register; reg < m_params.start_register + r.bytecnt / 2; reg++) {

      st >> regdata;

      if(m_master_signals_by_registers.contains(reg)) {

        m_master_signals_by_registers[reg]->setValue(QVariant(regdata));
        cnt++;
      }
    }

    emit message(QString("Пакет обработан, %1 сигналов обновлено").arg(cnt), sv::log::llDebug, sv::log::mtParse);

    buffer->reset();

  }

  buffer->mutex.unlock();

}

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvMoxa();
  return protocol;
}

