#include "gamma_opa_imitator.h"

apak::SvGammaOpaImitator::SvGammaOpaImitator():
  modus::SvAbstractProtocol(),
  m_send_signal(nullptr)
{

}

apak::SvGammaOpaImitator::~SvGammaOpaImitator()
{
  deleteLater();
}

bool apak::SvGammaOpaImitator::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
{
  try {

    p_config = config;
    p_io_buffer = iobuffer;

    m_params = apak::ProtocolParams::fromJson(p_config->protocol.params);

    return true;

  } catch (SvException& e) {

    p_last_error = e.error;
    return false;

  }
}

bool apak::SvGammaOpaImitator::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    if(r) {

      if(binding.mode == modus::Master) {


      }
      else {

        if(m_send_signal) {

          p_last_error = TOO_MUCH(p_config->name, "send");
          return false;
        }
        else
          m_send_signal = signal;

      }
    }

    return r;
  }

  catch(SvException& e) {
    p_last_error = e.error;
    return false;
  }
}

void apak::SvGammaOpaImitator::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvGammaOpaImitator::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvGammaOpaImitator::start()
{
  m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &SvGammaOpaImitator::send);
  m_timer->start(m_params.send_interval);

  p_is_active = bool(p_config) && bool(p_io_buffer) && bool(m_send_signal);
}

void apak::SvGammaOpaImitator::send()
{
  auto sendmsg = [=](QString& s) -> void {

    // контрольная сумма
    // восьмибитная XOR-сумма всех символов (включая «,» и «^») в строке между «$» и «*»,
    // приведенная к двум ASCII-символам в верхнем регистре для 16-ричного представления байта (0–9, A–F).
    uchar crc = 0x00;
    QString d = s.mid(1, s.length() - s.indexOf("*"));

    for(QChar c: d)
      crc ^= c.cell();

    s = s.replace("<CRC>", QString(QByteArray(1, crc).toHex()));

    p_io_buffer->output->mutex.lock();

    p_io_buffer->output->setData(QByteArray().append(s).append(QChar(0x0D)).append(QChar(0x0A)));
    emit message(s, lldbg, sv::log::mtNew);

    emit p_io_buffer->readyWrite(p_io_buffer->output);

    p_io_buffer->output->mutex.unlock();

    thread()->msleep(25); // После передачи сообщения, прибор КТС ОПА «Гамма-01Ф» делает паузу в течение 25 мсек перед передачей следующего сообщения

  };

  if(p_is_active) {

    QList<QString> sl = QString(m_send_signal->value().toByteArray()).split("<CR><LF>");

    QString stat = QString("$PPAS,SystemStatus,").append((sl.count() ? "SecurityAlarm" : "Norm")).append("*<CRC>") ;

    sendmsg(stat);

    for(QString s: sl) {

      if(!(s.startsWith("$PPAS") && s.endsWith("<CRC>")))
        continue;

      sendmsg(s);

    }
  }
}

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::SvGammaOpaImitator();
  return protocol;
}

