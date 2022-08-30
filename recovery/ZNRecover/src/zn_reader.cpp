#include "zn_reader.h"

QMutex mutex;

zn1::ZNReader::ZNReader():
  m_socket(nullptr)
{

}

/*bool zn1::ZNReader::configure(const QString& json, const QString& password) //, double zone_size)
{
  QFile f(json);

  try {

    if(!f.open(QIODevice::ReadOnly))
      throw SvException(f.errorString());

    QJsonParseError jerr;
    QJsonDocument jd = QJsonDocument::fromJson(f.readAll(), &jerr);

    if(jerr.error != QJsonParseError::NoError)
      throw SvException(jerr.errorString());

    m_config.readerParams = zn1::ReaderParams::fromJsonObject(jd.object());

    m_config.readerParams.pass = password;
    m_buff_size_bytes = m_config.readerParams.buff_size_mb * 1024 * 1024; // пересчитываем в байты

    return true;

  }
  catch(SvException& e) {

    if(f.isOpen())
      f.close();

    m_last_error = e.error;
    return false;
  }
}*/

bool zn1::ZNReader::configure(const zn1::RecoveryConfig& config, const QString& password)
{
  try {

    m_config = config;

    m_config.readerParams.pass = password;
    m_buff_size_bytes = m_config.readerParams.buff_size_mb * 1024 * 1024; // пересчитываем в байты

    return true;

  }
  catch(SvException& e) {

    m_last_error = e.error;
    return false;
  }
}

void zn1::ZNReader::conmessage(const QString msg, int level, int type)
{
  emit message(msg, level, type);
}

void zn1::ZNReader::run()
{
  m_is_running = true;

  int readed_buff  = 0;
  int readed_total = 0;
  int cnt = 0;

while(m_is_running) {

    readed_buff = cnt++ * 0x100000;
    readed_total += readed_buff;

    emit message(QString("Запрос на чтение %1 мегабайт.")
                 .arg(m_config.readerParams.buff_size_mb), sv::log::llDebug, sv::log::mtDebug);


    emit total(int(double(readed_total) / 0x100000)); // пересчет в мегабайты
//    emit parted(int(double(readed_buff ) / 0x100000));
    emit loaded(QString("Загружено сегментов: %1, текущий сегмент: %2 Мб, всего: %3 Мб")
                .arg(cnt).arg(int(double(readed_buff ) / 0x100000)).arg(int(double(readed_total) / 0x100000)));

    if (cnt==16) cnt = 0;

    msleep(1000);
    qDebug() << 4 * cnt;

    if(readed_total >= 0xA00000)
      break;
  }

/*

  auto connect2zn = [=](zn1::ZNReader::ZNstate& state, QString& error)  -> bool
  { //! 1. физическое подключение к хосту

    Q_ASSERT_X(m_socket, "zn1::ZNReader::connect", "m_socket is not defined!");

    try {

      if(m_socket->state() != QAbstractSocket::ConnectedState) {

        emit message(QString("Подключаюсь к защищенному хранилищу %1:%2")
                     .arg(m_config.readerParams.host.toString()).arg(m_config.readerParams.port), sv::log::llInfo, sv::log::mtDebug);

  //      m_socket->connectToHost(m_config.readerParams.host, m_config.readerParams.port);
  //qDebug() << m_socket->state() << m_config.readerParams.host << m_config.readerParams.port << m_config.readerParams.timeout;
  //      if(!m_socket->waitForConnected()) //m_config.readerParams.timeout))
  //        throw SvException(QString("Не удалось подключиться к защищенному накопителю по адресу %1:%2.\n%3")
  //                          .arg(m_config.readerParams.host.toString()).arg(m_config.readerParams.port).arg(m_socket->errorString()));

        if(!m_socket->connectTo(m_config.readerParams.host, m_config.readerParams.port, m_config.readerParams.timeout * 1000))
          throw SvException(QString("Не удалось подключиться к защищенному накопителю по адресу %1:%2.\n%3")
                            .arg(m_config.readerParams.host.toString()).arg(m_config.readerParams.port).arg(m_socket->errorString()));
      }

      state.c = STATE_CONNECTION_OK;
      error = QString();

      return true;

    }
    catch(SvException& e) {

      m_socket->disconnectFromHost();
      state.reset();
      error = e.error;

      return false;
    }
  }; //! ~физическое подключение к хосту

  m_socket = new sv::tcp::Client(m_config.readerParams.host.toString(), m_config.readerParams.port, nullptr, 0x0); //0x1E); // QTcpSocket(); // обязательно создаем здесь, чтобы объект принадлежал этому потоку
  connect(m_socket, SIGNAL(message(QString,int,int)), this, SIGNAL(message(QString,int,int)));

  quint64 start_byte = m_config.readerParams.start_byte;
//  quint64 read_count = ; //m_config.readerParams.read_buf * 1024 * 1024; // пересчитываем в байты

//  quint64 m_config.readerParams.zone_size;

  QString error;
  zn1::ZNReader::ZNstate znstate{};

  //! запрашиваем общий объем накопителя, если он не был задан
  if(m_config.readerParams.ask_zone_size) {

    try {

      emit message(QString("Запрашиваем объем данных накопителя"), sv::log::llInfo, sv::log::mtInfo);

      if(!connect2zn(znstate, error))
        throw SvException(error);

      if(!authorization(znstate, error))
        throw SvException(error);

      QByteArray request = zn1::DataSizeRequest().toByteArray();

      emit message(QString("Запрос: %1").arg(QString(request.toHex())), sv::log::llDebug, sv::log::mtDebug);

      m_socket->write(request);

      if(!m_socket->waitForReadyRead(m_config.readerParams.timeout * 1000))
        throw SvException(QString("Ошибка запроса. Нет ответа. Таймаут."));

      // анализируем полученный ответ
      QByteArray readed = m_socket->readAll();

      zn1::DataSizeReply reply = zn1::DataSizeReply::parse(readed);

      emit message(QString("Ответ на запрос: %1\n"
                           "len: %2, reply code: %3, request code: %4, result: %5, data_size: %6, data_pointer: %7, addition len: %8 bytes")
                      .arg(QString(readed.toHex())).arg(reply.length).arg(reply.reply_code).arg(reply.request_code)
                      .arg(reply.result).arg(reply.data_size).arg(reply.data_pointer).arg(reply.additional.length()),
                   sv::log::llDebug, sv::log::mtDebug);

      if(reply.failed)
        throw SvException(QString("Полученные данные имеют неверный формат"));

      if(static_cast<ReplyCode>(reply.result) != ReplyCode::Success)
        throw SvException(QString("Накопитель вернул код ошибки %1: %2")
                          .arg(reply.result)
                          .arg(ReplyCodeMap.value(static_cast<zn1::ReplyCode>(reply.result), "Ух ты! Как ты это сделал?")));


      m_config.readerParams.zone_size_mb  = reply.data_size / 0x100000; // пересчитываем в мегабайты. 1024 * 1024 = 1 048 576 = Hex 10 0000

      emit message(QString("Успешно"), sv::log::llInfo, sv::log::mtSuccess);

    }
    catch(SvException& e) {

      delete m_socket;
      emit message(e.error, sv::log::llError, sv::log::mtError);
      return;
    }
  }
//  else
//    m_config.readerParams.zone_size_mb *= 1024; // в мегабайтах


  emit zonesize(int(m_config.readerParams.zone_size_mb));
  emit partsize(int(m_config.readerParams.buff_size_mb));

//return;
//  m_config.readerParams.zone_size = 4026531840; // 3.75 Gb
//qDebug() << (m_config.readerParams.read_type == DEFAULT_READ_TYPE);

  //! чтение данных
  m_is_running = true; //(m_config.readerParams.read_type == DEFAULT_READ_TYPE);


  QFile file(m_config.zn_data_file);

  try {

    if(!file.open(QIODevice::WriteOnly))
      throw SvException(file.errorString());

    if(!connect2zn(znstate, error))
      throw SvException(error);

//      if(!m_is_running) break;

    if(!authorization(znstate, error))
      throw SvException(error);

//      if(!m_is_running) break;

    if(!(znstate.authorized() && znstate.connected()))
      throw SvException("Невозможно произвести операцию чтения-записи");

    quint64 readed_total  = 0;
    quint64 written_total = 0;
    quint64 readed_buff   = 0;

    emit message(QString("Начинаем загрузку %1 мегабайт. Стартовый байт %2")
                 .arg(m_config.readerParams.zone_size_mb).arg(start_byte), sv::log::llInfo, sv::log::mtAttention);

    while(m_is_running) {

      //! запрос на чтение данных
      QByteArray request = zn1::ReadRequest(start_byte, m_buff_size_bytes).toByteArray();

      m_socket->write(request);

      emit message(QString("Запрос на чтение %1 мегабайт. Стартовый байт %2\n%3")
                   .arg(m_config.readerParams.buff_size_mb).arg(start_byte).arg(QString(request.toHex())),
                   sv::log::llDebug, sv::log::mtDebug);

      if(!m_socket->waitForReadyRead(m_config.readerParams.timeout * 1000)) // пересчет в миллисекунды
        throw SvException(QString("Ошибка чтения данных защищенного накопителя. Нет ответа."));

      QByteArray r = m_socket->readAll();

      zn1::ReadReply reply = zn1::ReadReply::parse(r);

      readed_buff  =  reply.data.size();
      readed_total += readed_buff;

      emit total(int(double(readed_total) / 0x100000)); // пересчет в мегабайты
      emit parted(int(double(readed_buff ) / 0x100000));

//      emit message(QString("Ответ на запрос: %1").arg(QString(r.toHex())), sv::log::llDebug2, sv::log::mtDebug);

      emit message(QString("Ответ на запрос: len: %1, reply code: %2, request code: %3, result: %4, position: %5, data len: %6 bytes")
                      .arg(reply.length).arg(reply.reply_code).arg(reply.request_code)
                      .arg(reply.result).arg(reply.start_byte).arg(readed_total),
                   sv::log::llDebug, sv::log::mtDebug);

      if(reply.result != zn1::Success)
        throw SvException(QString("Загрузка прервана. Накопитель вернул код ошибки %1: \"%2\"")
                          .arg(reply.result).arg(ReplyCodeMap.value(static_cast<ReplyCode>(reply.result))));

//      emit message(QString("Загружено %1 байт").arg(readed_buff), sv::log::llDebug2, sv::log::mtDebug);

      if(!file.isOpen())
        throw SvException(QString("Ошибка записи! Файл не открыт"));

      written_total += file.write(reply.data);

      emit message(QString("Записано %1 байт").arg(written_total), sv::log::llDebug2, sv::log::mtDebug);


      while(m_socket->waitForReadyRead(m_config.readerParams.timeout * 1000)) {  // пересчет в миллисекунды

        QByteArray r2 = m_socket->readAll();
        readed_buff  += r2.size();
        readed_total += r2.size();

        emit total(int(double(readed_total) / 0x100000)); // пересчет в мегабайты
        emit parted(int(double(readed_buff ) / 0x100000));

//        emit message(QString("Загружено %1 байт").arg(readed_total), sv::log::llDebug2, sv::log::mtData);

        if(!file.isOpen())
          throw SvException(QString("Ошибка записи! Файл не открыт"));

        written_total += file.write(r2);

        emit message(QString("Записано %1 байт").arg(written_total), sv::log::llDebug2, sv::log::mtDebug);

        if(!m_is_running || readed_buff >= m_buff_size_bytes) // условие завершения загрузки
          break;
      }

      start_byte += readed_buff;

      readed_buff = 0;
      emit parted(readed_buff);

      if(!m_is_running || readed_total >= m_config.readerParams.zone_size_mb * 0x100000) // достигли конца данных. выходим
        break;
    }

    emit message(QString("Загрузка закончена. Успешно загружено %1 байт из %2 Мб").arg(readed_total).arg(m_config.readerParams.zone_size_mb), sv::log::llInfo, sv::log::mtSuccess);

  }
  catch(SvException& e) {

    emit message(e.error, sv::log::llError, sv::log::mtError);
    znstate.r = STATE_NO_READING;
//    break;
  }

  if(m_socket->state() == QAbstractSocket::ConnectedState)
    m_socket->close();

  delete m_socket;

  if(file.isOpen())
    file.close();

  exit();
  */
}

void zn1::ZNReader::stop()
{
  m_is_running = false;
}

bool zn1::ZNReader::authorization(zn1::ZNReader::ZNstate& state, QString& error)
{
  try {

    if((state.authorized()) && (state.connected())) {

      error = QString();
      return true;
    }

    else if((!state.authorized()) && (state.connected())) {

      QByteArray request = zn1::AuthorizeRequest (m_config.readerParams.zone, m_config.readerParams.pass, ACC_CODE_READ).toByteArray();

      emit message(QString("Запрос на авторизацию (пароль скрыт): %1")
                   .arg(QString(request.toHex()) // заменяем пароль нулями для вывода на экран
                            .replace(QString(m_config.readerParams.pass.toUtf8().toHex()), QString(m_config.readerParams.pass.length() * 2, QChar('0')))
                        ),
                   sv::log::llInfo, sv::log::mtRequest);

      m_socket->write(request);

      if(!m_socket->waitForReadyRead(m_config.readerParams.timeout * 1000))
        throw SvException(QString("Ошибка авторизации на защищенном накопителе. Нет ответа. Таймаут."));

      // анализируем полученный ответ
      QByteArray replay = m_socket->readAll();

      zn1::AuthorizeReply reply = zn1::AuthorizeReply::parse(replay);

      emit message(QString("Ответ на запрос авторизации: %1\n"
                           "len: %2, reply code: %3, request code: %4, result: %5, addition len: %6 bytes")
                      .arg(QString(replay.toHex())).arg(reply.length).arg(reply.reply_code).arg(reply.request_code)
                      .arg(reply.result).arg(reply.additional.length()),
                   sv::log::llDebug2, sv::log::mtDebug);

      if(static_cast<ReplyCode>(reply.result) != ReplyCode::Success)
        throw SvException(QString("Ошибка авторизации: %1")
                          .arg(ReplyCodeMap.value(static_cast<zn1::ReplyCode>(reply.result), "Ух ты! Как ты это сделал?")));

      emit message(QString("Успешная авторизация"), sv::log::llInfo, sv::log::mtSuccess);

      state.a = STATE_AUTHORITY_OK;

      return true;

    }
    else
      throw SvException("Неопределенное состояние авторизации или отсутствует подключение к накопителю.");

  }
  catch(SvException& e) {

    error = e.error;
    emit message(e.error, sv::log::llError, sv::log::mtError);
    m_zn_state.a = STATE_NO_AUTHORITY;

    return false;

  }
} //! ~авторизация на устройстве


bool zn1::ZNReader::read(zn1::ZNReader::ZNstate& state, QString& error)
{
  { //! 3. чтение данных

    try {
/*
      if((state.authorized()) && (state.connected())) {

        // если попали сюда, значит есть подключение и авторизация. можно читать данные

          //! запрос на чтение данных
          QByteArray request = zn1::ReadRequest(m_start_byte, m_read_count).toByteArray();

          m_socket->write(request);

          emit message(QString("Запрос на чтение %1 байт. Стартовый байт %2").arg(m_read_count).arg(m_start_byte),
                       sv::log::llDebug, sv::log::mtDebug);

          if(!m_socket->waitForReadyRead(m_config.readerParams.timeout))
            throw SvException(QString("Ошибка чтения данных защищенного накопителя. Нет ответа."));

          QByteArray r = m_socket->readAll();

          if(!m_file.isOpen())
            throw SvException(QString("Ошибка записи! Файл закрыт"));

          m_file.write(r);

          m_start_byte += m_read_count;

          if(m_start_byte + m_read_count > m_full_data_size)
            m_read_count = m_full_data_size - m_start_byte;
*/
            /*

          if(m_config.readerParams.read_type == DEFAULT_READ_TYPE) {

          }
          else {

            }
            else {

              //! ищем заданные маркеры и временные метки
//              int search_index = 0;
//              int aaaaaaaa_index;
//              while(search_index < r.size()) {

//                aaaaaaaa_index = r.indexOf(QByteArray(zn1::BunchMarker, sizeof(zn1::BunchMarker)), search_index);

//                if(aaaaaaaa_index == -1)
//                  break;

//                if(!m_current_bunch.setHeader(r.data() + aaaaaaaa_index)) {

//                  search_index = aaaaaaaa_index + 1;
//                  continue;
//                }
//                else {

//                  zn1::Record record{};
//                  record.fromRawData(r.data());
//  //                m_current_bunch.ap


//                }
          }
        }*/
      //}
      return true;
    }
    catch(SvException& e) {

      emit message(e.error, sv::log::llError, sv::log::mtError);
      error = e.error;
      state.r = STATE_NO_READING;

      return false;

    }
  } //! ~чтение данных


}

