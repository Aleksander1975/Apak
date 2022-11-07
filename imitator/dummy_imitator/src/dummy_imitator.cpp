#include "dummy_imitator.h"

apak::SvDummyImitator::SvDummyImitator():
  modus::SvAbstractProtocol()
{
}

apak::SvDummyImitator::~SvDummyImitator()
{
  deleteLater();
}

bool apak::SvDummyImitator::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
// Эта функция вызывается серевером "mdserver" для всех имитаторов устройств.
// Её цель - инициализировать все структуры, необходимые нам для конкретного
// имитатора (для "заглушки" единственная структура данных - это номер посылки "m_transferNumber").
{
    m_transferNumber = 0;

    try
    {

    p_config = config;
    p_io_buffer = iobuffer;

    // Заполняем структуру m_params параметром протокола обмена "заглушки" с системой АПАК
    // (периодом поступления данных в мс от "заглушки" в систему АПАК)
    m_params = Dummy::ProtocolParams::fromJson(p_config->protocol.params);

    return true;
    }
    catch (SvException& e) {

    p_last_error = e.error;
    return false;
  } 
}

bool apak::SvDummyImitator::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
// Эта функция вызывается сервером "mdserver" для всех сигналов "заглушки".
// Так как у "заглушки" нет сигналов, то эта функция вызыватья не будет.
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);


    Dummy::SignalParams signal_params = Dummy::SignalParams::fromJson(binding.params);

    if(r)
    {

      if(binding.mode == modus::Master)
      {
      }
      else
      {
      }
    } // if (r)

    return r;
  } // try

  catch(SvException& e)
  {
    p_last_error = e.error;
    return false;
  }
}

void apak::SvDummyImitator::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvDummyImitator::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::SvDummyImitator::start()
// В этой функции мы осуществляем привязку вызова функции send() к наступлению таймаута таймера "m_timer".
{
  m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &SvDummyImitator::send);
  m_timer->start(m_params.send_interval);

  p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::SvDummyImitator::send()
// В этой функции мы формируем и помещаем в массив байт "m_send_data" пакет для передачи от "заглушки"
// в систему АПАК (в соответствии с протоколом обмена) и инициируем передачу
// этого пакета от протокольной к интерфейcной части имитатора (для передачи по линии связи).
{
    //qDebug () << "Вызов send()";

    if(p_is_active)
    {
        // 1. В переменной "m_send_data" формируем пакет данных от "заглушки":
        // то есть посылку, длиной в 250 байт, каждый байт которой содержит
        // значение номера посылки "m_transferNumber".
        m_send_data.clear();
        m_send_data.resize (250);
        m_send_data.fill(m_transferNumber, 250);

        // 2. Увеличиваем значение "m_transferNumber" на 1.
        if (m_transferNumber == 255)
            m_transferNumber = 0;
        else
            m_transferNumber++;


       //qDebug() << "Пакет (в окончательном виде) от ЗАГЛУШКИ-протокола";
       //qDebug() << "Размер: " << m_send_data.length();
       //qDebug() << "Содержание: " << m_send_data.toHex();


       // 3. Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
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
  modus::SvAbstractProtocol* protocol = new apak::SvDummyImitator();
  return protocol;
}

