#include "ups.h"

apak::SvUPS::SvUPS():
  modus::SvAbstractProtocol(),
  m_state_signal(nullptr)
{

}

apak::SvUPS::~SvUPS()
{
  deleteLater();
}

bool apak::SvUPS::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
{
  try {

    p_config = config;
    p_io_buffer = iobuffer;

    m_params = ups::ProtocolParams::fromJson(p_config->protocol.params);

    return true;

  } catch (SvException& e) {

    p_last_error = e.error;
    return false;

  }
}

bool apak::SvUPS::bindSignal(modus::SvSignal *signal, modus::SignalBinding binding)
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    if(r) {

      if(binding.mode == modus::Master) {

        if(signal->config()->type.toLower() == TYPE_DATA) {

          //! параметры params для каждого свои! для master'a свои, а для bindings - свои
          ups::SignalParams params = ups::SignalParams::fromJson(binding.params);

          m_params_by_signals.insert(signal, params);

          if(!m_signals_by_registers.contains(params.registr))
            m_signals_by_registers.insert(params.registr, QList<modus::SvSignal*>());

          m_signals_by_registers[params.registr].append(signal);

        }
        else if(signal->config()->type.toLower() == TYPE_STAT) {

          if(m_state_signal)
            throw SvException(TOO_MUCH(p_config->name, TYPE_STAT));

          else {

            m_state_signal = signal;
//            m_state_params = params;

          }
        }
        else
          throw SvException(QString("Неверный тип \"%1\" для сигнала \"%2\"").arg(signal->config()->type).arg(signal->config()->name));


      }
      else {


      }
    }

    return r;

  }
  catch(SvException& e) {

    p_last_error = e.error;
    return false;

  }
}

void apak::SvUPS::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvUPS::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvUPS::start()
{
  QTimer* m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &SvUPS::queued_request);
  m_timer->start(m_params.interval);

  connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &apak::SvUPS::on_answer);
//  connect(this, &SvUPS::nextRequest, this, &apak::SvUPS::request, Qt::QueuedConnection); //! Queued

  p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvUPS::queued_request()
{

  for(quint16 registr: m_signals_by_registers.keys()) {

    QByteArray outdata;
    QDataStream outstream(&outdata, QIODevice::WriteOnly);

    outstream << quint8(m_params.address)            // Адрес устройства
              << quint8(m_params.func_code)             // Функциональный код. 0x04
              << registr                                // Адрес первого регистра
              << quint16(1);                            // Количество регистров

    quint16 crc = crc::crc16ibm((unsigned char*)(outdata.data()), outdata.length()); // Контрольная сумма CRC
    outdata.append(quint8(crc & 0xFF));
    outdata.append(quint8(crc >> 8));

    p_io_buffer->output->mutex.lock();

    p_io_buffer->output->setData(outdata);

    emit p_io_buffer->readyWrite(p_io_buffer->output);

    p_io_buffer->output->mutex.unlock();

    m_i_have_got_answer = false;

    emit message(QString("wait for answer"), lldbg, mtdat);

    //! ждем ответ
    qint64 check_time = QDateTime::currentMSecsSinceEpoch() + m_params.interval;
    while(!m_i_have_got_answer && (check_time > QDateTime::currentMSecsSinceEpoch())) {
      qApp->processEvents();
      thread()->msleep(1);
    }

    if(m_i_have_got_answer) {

      emit message(QString("got answer. isReady = %1").arg(p_io_buffer->input->isReady()), lldbg, mtdat);

      if(!p_io_buffer->input->isReady())
        continue;

      p_io_buffer->input->mutex.lock();
      QByteArray indata(p_io_buffer->input->data, p_io_buffer->input->offset);
      p_io_buffer->input->mutex.unlock();

      m_state_signal->setValue(QVariant(1));

      QDataStream instream(indata);
      apak::UPSData upsdata;

      instream >> upsdata.address >> upsdata.func >> upsdata.bytecnt >> upsdata.value >> upsdata.crc;

      emit message(QString("parsed. address: %1, func: %2, bytecnt: %3, value: %4, crc:%5")
                   .arg(upsdata.address).arg(upsdata.func).arg(upsdata.bytecnt).arg(upsdata.value).arg(upsdata.crc), lldbg, mtdat);

      for(modus::SvSignal* signal: m_signals_by_registers.value(registr, QList<modus::SvSignal*>())) {

        ups::SignalParams p = m_params_by_signals.value(signal, ups::SignalParams());

        signal->setValue(QVariant((upsdata.value >> p.offset) & ((1 << p.len) - 1)));
      }
    }
  }
}

void apak::SvUPS::on_answer(modus::BUFF* buffer)
{
  if(!buffer->isReady())
    return;

  m_i_have_got_answer = true;

//  emit nextRequest();

}

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvUPS();
  return protocol;
}
