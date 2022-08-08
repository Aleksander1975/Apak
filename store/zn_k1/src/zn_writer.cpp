#include "zn_writer.h"

QMutex mutex;

QSemaphore sem(1);

zn1::ZNWriter::ZNWriter():
  m_socket(nullptr),
  m_state_signal(nullptr)
{

}

zn1::ZNWriter::~ZNWriter()
{
  delete m_socket;

//  m_test_file.close();

}

bool zn1::ZNWriter::configure(modus::StorageConfig* config)
{
  try {

    p_config = config;
    m_params = zn1::StorageParams::fromJsonString(p_config->params);

  }
  catch(SvException& e) {

    p_last_error = e.error;
    return false;
  }

  return true;
}

bool zn1::ZNWriter::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
{
  bool r = modus::SvAbstractStorage::bindSignal(signal, binding);

  if(r) {

    if(binding.mode == modus::Master) {

      if(signal->config()->type.toLower() == TYPE_STAT) {

        if(m_state_signal) {

          p_last_error = TOO_MUCH(p_config->name, TYPE_STAT);
//          qDebug() << QString("Second: %1").arg(signal->config()->name);
          return false;
        }
        else {
//          qDebug() << QString("First: %1").arg(signal->config()->name);
          m_state_signal = signal;

        }
      }
    }
    else {

      try {

        zn1::SignalParams params = zn1::SignalParams::fromJson(binding.params);

        m_signal_params.insert(signal, params);

      }
      catch(SvException& e) {

        p_last_error = e.error;
        return false;

      }
    }
  }

  return r;

}

void zn1::ZNWriter::start()
{
//  m_timer = new QTimer;
//  connect(m_timer, &QTimer::timeout, this, &ZNWriter::checkupSignals);
//  m_timer->start(DEFAULT_INTERVAL);

  m_socket = new sv::tcp::Client(); // обязательно создаем здесь, чтобы объект принадлежал этому потоку

  m_zn_state.c = STATE_NO_CONNECTION;
  m_zn_state.a = STATE_NO_AUTHORITY;
  m_zn_state.w = STATE_NO_WRITING;

//  m_test_file.setFileName("/home/user/ProjectData/APAK/ZNRecovery/test_writing.zndata");
//  if(!m_test_file.open(QIODevice::WriteOnly))
//    emit message(m_test_file.errorString(), sv::log::llError, sv::log::mtError);

}

//void zn1::ZNWriter::checkupSignals()
//{
//  if(m_state_signal)
//    m_state_signal->approve(false);
//}

//void zn1::ZNWriter::setState(int writeState, int authorization, int connectionState)
//{
//  m_zn_state.c = connectionState;
//  m_zn_state.a = authorization;
//  m_zn_state.w = writeState;

//  if(m_state_signal)
//    m_state_signal->setValue(m_zn_state.state());

////  m_timer->start();

//}

void zn1::ZNWriter::write()
{
  Q_ASSERT_X(m_socket, "zn1::ZNWriter::write", "m_socket is not defined!");

  { //! 1. физическое подключение к хосту

    try {

      if(m_socket->state() != QAbstractSocket::ConnectedState) {

        m_zn_state.a = STATE_NO_AUTHORITY;

        emit message(QString("Подключаюсь к защищенному хранилищу %1:%2")
                     .arg(m_params.host.toString()).arg(m_params.port), sv::log::llDebug2, sv::log::mtDebug1);

        if(!m_socket->connectTo(m_params.host, m_params.port))
          throw SvException(QString("Не удалось подключиться к защищенному накопителю по адресу %1:%2.\n%3")
                            .arg(m_params.host.toString()).arg(m_params.port).arg(m_socket->lastError()));

      }

      m_zn_state.c = STATE_CONNECTION_OK;

    }
    catch(SvException& e) {

      emit message(e.error, sv::log::llDebug, sv::log::mtError);
      m_socket->disconnectFromHost();

      m_zn_state.c = STATE_NO_CONNECTION;

    }
  } //! ~физическое подключение к хосту

  { //! 2. авторизация на устройстве

    try {

      if((m_zn_state.a != STATE_AUTHORITY_OK) && (m_zn_state.c == STATE_CONNECTION_OK)) {

        zn1::AuthorizeRequest authreq(m_params.zone, m_params.pass, ACC_CODE_WRITE);

        QByteArray r = authreq.toByteArray();

        emit message(QString("Запрос на авторизацию: %1").arg(QString(r.toHex())), sv::log::llDebug2, sv::log::mtRequest);

        m_socket->write(r);

        if(!m_socket->waitForReadyRead(m_params.interval))
          throw SvException(QString("Ошибка авторизации на защищенном накопителе. Нет ответа."));

        else {

          // анализируем полученный ответ
          QByteArray r = m_socket->readAll();

          zn1::AuthorizeReply reply = zn1::AuthorizeReply::parse(r);

          emit message(QString("Ответ на запрос авторизации: %1\n"
                               "len: %2, reply code: %3, request code: %4, result: %5, addition len: %6 bytes")
                          .arg(QString(r.toHex())).arg(reply.length).arg(reply.reply_code).arg(reply.request_code)
                          .arg(reply.result).arg(reply.additional.length()),
                       sv::log::llDebug2, sv::log::mtDebug);

          if(static_cast<ReplyCode>(reply.result) != ReplyCode::Success)
            throw SvException(QString("Ошибка авторизации: %1")
                              .arg(ReplyCodeMap.value(static_cast<zn1::ReplyCode>(reply.result), "Ух ты! Как ты это сделал?")));

          else
            emit message(QString("Успешная авторизация"), sv::log::llDebug, sv::log::mtSuccess);

        }
      }

      m_zn_state.a = STATE_AUTHORITY_OK;

    }
    catch(SvException& e) {

      emit message(e.error, sv::log::llDebug, sv::log::mtError);
      m_zn_state.a = STATE_NO_AUTHORITY;

    }
  } //! ~авторизация на устройстве

  { //! 3. запись данных

    try {

      m_zn_state.w = STATE_NO_WRITING;

      if((m_zn_state.a == STATE_AUTHORITY_OK) && (m_zn_state.c == STATE_CONNECTION_OK)) {

        // если попали сюда, значит есть подключение и авторизация. можно писать данные
        if(!bunches.isEmpty()) {

//          QByteArray data = QByteArray::;
          // вычисляем длину пакета для записи
          quint32 pack_length = 0;
          foreach (Bunch* b, bunches)
            pack_length += b->length();

          QByteArray pack = QByteArray();
          QDataStream stream(&pack, QIODevice::WriteOnly);
          stream.setByteOrder(QDataStream::LittleEndian);

          stream << quint32(pack_length)
                 << quint16(CMD_WRITE);


          foreach (Bunch* b, bunches)
            b->appendToStream(&stream);

//          stream.writeRawData(data.data(), data.length());

          m_socket->write(pack);

//          m_test_file.write(pack);

          emit message(QString("Запись %1 байт в зону %2").arg(pack.length()).arg(m_params.zone),
                       sv::log::llDebug2, sv::log::mtDebug);

          emit message(QString("%1 ...").arg(QString(pack.left(64).toHex())),
                       sv::log::llDebug, sv::log::mtDebug);



          if(!m_socket->waitForReadyRead(m_params.interval))
            throw SvException(QString("Ошибка записи данных в  защищенный накопитель. Нет ответа."));

          else {

            QByteArray r = m_socket->readAll();
            zn1::WriteReply reply = zn1::WriteReply::parse(r);

            emit message(QString("Результат записи: %1\nlen: %2, reply code: %3, request code: %4, result: %5, addition: %6 bytes")
                              .arg(QString(r.toHex())).arg(reply.length).arg(reply.reply_code).arg(reply.request_code)
                         .arg(reply.result).arg(reply.additional.length()),
                         sv::log::llDebug2, sv::log::mtDebug);


            if(static_cast<ReplyCode>(reply.result ) != ReplyCode::Success)
              throw SvException(QString("Ошибка записи: %1").arg(ReplyCodeMap.value(static_cast<zn1::ReplyCode>(reply.result), "Ух ты! Как ты это сделал?")));

            else {

              while(!bunches.isEmpty())
                delete bunches.dequeue();

              emit message(QString("%1 байт успешно записаны в зону %2").arg(pack.length()).arg(m_params.zone), sv::log::llDebug, sv::log::mtSuccess);

              m_zn_state.w = STATE_WRITING_OK;

            }
          }
        }
      }
    }
    catch(SvException& e) {

      emit message(e.error, sv::log::llDebug, sv::log::mtError);

      m_zn_state.w = STATE_NO_WRITING;

    }
  } //! ~запись данных

  if(m_state_signal)
    m_state_signal->setValue(m_zn_state.state());

}

void zn1::ZNWriter::signalUpdated(modus::SvSignal* signal)
{
//  emit message(QString("1. m_mutex.lock(). bunches.count(): %1").arg(bunches.count()), sv::log::llDebug, sv::log::mtDebug1);

  if(signal->value().isNull() || !signal->value().isValid())
    return;

  if(bunches.isEmpty() || (bunches.last()->state() != Bunch::Underway)) {

    if(bunches.count() >= int(m_params.queue_len))
      delete bunches.dequeue();

    bunches.enqueue(new Bunch(QDateTime::currentMSecsSinceEpoch()));

    emit message(QString("Bunch queued. Bunch count: %1").arg(bunches.count()), sv::log::llDebug2, sv::log::mtDebug);

  }

  if(bunches.last()->state() == Bunch::Underway)
  {
    if(m_signal_params.contains(signal)) {

      zn1::SignalParams signal_params = m_signal_params.value(signal);

      if(!signal_params.marker.isEmpty()) {

        Record r(signal->lastUpdate().toMSecsSinceEpoch(), signal_params.marker, signal->value().toByteArray());

        bunches.last()->appendRecord(&r);

        emit message(QString("%1 bytes appended to bunch. Bunch size: %2, records: %3")
                     .arg(r.size()).arg(bunches.last()->length()).arg(bunches.last()->recordCount()), lldbg2, mtdbg);
      }
      else
        emit message(QString("Для сигнала \"%1\" не задан идентфикатор записи \"marker\"").arg(signal->config()->name), lldbg, mterr);

    }
  }

  if(bunches.last()->length() >= m_params.write_buf) {

    bunches.last()->setState(Bunch::Ready);

    write();

  }

//  emit message(QString("Bunch data size %1").arg(m_bunches.last()->data.length()), sv::log::llDebug2, sv::log::mtDebug);

}

/** ********** EXPORT ************ **/
modus::SvAbstractStorage* create()
{
  modus::SvAbstractStorage* storage = new zn1::ZNWriter();
  return storage;
}
