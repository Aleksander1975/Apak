#include "apak_raduga_sheksna.h"

raduga::SvRaduga::SvRaduga():
  modus::SvAbstractProtocol()
{
  input_signal_collections.insert(TYPE_1,   &type1_input_signals);
  input_signal_collections.insert(TYPE_2,   &type2_input_signals);
  input_signal_collections.insert(TYPE_3,   &type3_input_signals);
  input_signal_collections.insert(TYPE_5,   &type5_input_signals);
  input_signal_collections.insert(TYPE_9,   &type9_input_signals);
  input_signal_collections.insert(TYPE_53,  &type53_input_signals);

  output_signal_collections.insert(TYPE_1,  &type1_output_signals);
  output_signal_collections.insert(TYPE_2,  &type2_output_signals);
  output_signal_collections.insert(TYPE_3,  &type3_output_signals);
  output_signal_collections.insert(TYPE_5,  &type5_output_signals);
  output_signal_collections.insert(TYPE_9,  &type9_output_signals);
  output_signal_collections.insert(TYPE_53, &type53_output_signals);

}

raduga::SvRaduga::~SvRaduga()
{
  deleteLater();
}

bool raduga::SvRaduga::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
{
  try {

    p_config = config;
    p_io_buffer = iobuffer;

    m_params = raduga::ProtocolParams::fromJson(p_config->protocol.params);

//    if(!m_data.resize(p_config->bufsize))
//      throw SvException(QString("Не удалось выделить %1 байт памяти для буфера").arg(p_config->bufsize));

//    foreach (raduga::DataTypeCollection* dtc, input_signal_collections.values())
//      dtc->setBufsize(p_config->bufsize);

//    foreach (raduga::DataTypeCollection* dtc, output_signal_collections.values())
//      dtc->setBufsize(p_config->bufsize);

    return true;

  } catch (SvException& e) {

    p_last_error = e.error;
    return false;

  }
}

void raduga::SvRaduga::disposeSignal (modus::SvSignal* signal)
{
  if(!signal->config()->bindings.contains(duid(p_config->id)))
    return;

  if(signal->isMaster(P_DEVICE, p_config->id)) {

    bool ok;
    quint16 type = signal->config()->type.toUInt(&ok, 0);

    if(ok && input_signal_collections.contains(type))
      input_signal_collections.value(type)->addSignal(signal, p_config->bufsize);

    else {

      emit message(QString("Сигнал '%1': Неопознанный тип данных \"%2\"").arg(signal->config()->name).arg(signal->config()->type),
                   sv::log::llError, sv::log::mtError);
    }

  }
  else if(signal->hasBindingTo(P_DEVICE, p_config->id)) {

    bool ok;
    quint16 type = signal->config()->type.toUInt(&ok, 0);

    if(ok && output_signal_collections.contains(type)) {

      output_signal_collections.value(type)->addSignal(signal, p_config->bufsize);
      connect(signal, &modus::SvSignal::updated, this, &SvRaduga::output_queue);
    }

  }
  else
    emit message(QString("В конфигурации не определена привязка сигнала '%1' к устройству '%2'").arg(signal->config()->name).arg(signal->config()->type),
                 sv::log::llError, sv::log::mtError);
}

void raduga::SvRaduga::run()
{
  p_is_active = bool(p_config) && bool(p_io_buffer);

  while(p_is_active) {

    p_io_buffer->confirm->mutex.lock();     // если нужен ответ квитирование
    p_io_buffer->input->mutex.lock();

    raduga::PARSERESULT result = parse();

    if(result.do_reset == DO_RESET)
      p_io_buffer->input->reset();

    p_io_buffer->input->mutex.unlock();
    p_io_buffer->confirm->mutex.unlock();   // если нужен ответ квитирование

    if(result.parse_time.isValid())
      checkupSignals(result.parse_time);

//    p_io_buffer->output->mutex.lock();

//    putout();

//    p_io_buffer->output->mutex.unlock();

//    emit message("protocol before yield", sv::log::llDebug, sv::log::mtReceive);
//    QThread::yieldCurrentThread();
    if(p_is_active)
      msleep(m_params.parse_interval);

  }
}

raduga::PARSERESULT raduga::SvRaduga::parse()
{
  // проверяем, что длина данных в буфере не меньше длины заголовка
  if(p_io_buffer->input->offset < m_hsz)
    return raduga::PARSERESULT(DO_NOT_RESET);

  // разбираем заголовок. если адрес или код функции не тот, значит это чужой пакет
  memcpy(&m_header, &p_io_buffer->input->data[0], m_hsz);

  emit message(QString("header: %1, abonent: %2, activity: %3, p.packid: %4")
               .arg(QString(QByteArray(&(m_header.system_name[0]), SYSTEM_NAME_LEN)))
               .arg(m_header.abonent_id)
               .arg(m_header.activity_id)
               .arg(m_header.pack_id),
      lldbg, sv::log::mtDebug2);


  if((QByteArray(&(m_header.system_name[0]), RDGA_NAME_LEN) != m_raduga20386) ||
                                     (m_header.pack_id != m_params.packid))
    return raduga::PARSERESULT(DO_RESET);

  quint16 packsz = pack_size.value(m_header.pack_id);

  // проверяем размер пакета, согласно полученному идентификатору пакета (pack id)
  if(p_io_buffer->input->offset < packsz)
    return raduga::PARSERESULT(DO_NOT_RESET);

  /**
   *  в этой точке в буфере должны находиться правильные данные
   *  производим непосредственно разбор данных и назначаем значения сигналам
  **/
  // ставим состояние данной линии
//  line_status_signals.updateSignals();

  // проверяем crc
  quint16 got_crc;
  memcpy(&got_crc,  &p_io_buffer->input->data[m_hsz + packsz], 2); // crc полученная

  quint16 calc_crc = CRC::MODBUS_CRC16((const quint8*)&p_io_buffer->input->data[m_hsz], packsz); // вычисляем crc из данных

  if(calc_crc != got_crc) {

    // если crc не совпадает, то выходим без обработки и ответа
    emit message(QString("Ошибка crc! Ожидалось %1%2, получено %3%4")
                 .arg(quint8(calc_crc),      2, 16, QChar('0'))
                 .arg(quint8(calc_crc >> 8), 2, 16, QChar('0'))
                 .arg(quint8(got_crc),       2, 16, QChar('0'))
                 .arg(quint8(got_crc  >> 8), 2, 16, QChar('0')),
                 sv::log::llError, sv::log::mtError);

    return raduga::PARSERESULT(DO_RESET);
  }

  // если все корректно, то разбираем данные в зависимости от типа
  switch (m_header.pack_id)
  {
    case PACK_ID_101:
    case PACK_ID_105:
    case PACK_ID_106:
    case PACK_ID_110:
    {
      type1_input_signals.updateSignals(&p_io_buffer->input->data[m_hsz], packsz);
      break;
    }

    case PACK_ID_103:
    case PACK_ID_108:
    {
      type5_input_signals.updateSignals(&p_io_buffer->input->data[m_hsz], packsz);
      break;
    }

    case PACK_ID_102:
    case PACK_ID_107:
    {
      type2_input_signals.updateSignals(&p_io_buffer->input->data[m_hsz], type_size.value(TYPE_2));
      type3_input_signals.updateSignals(&p_io_buffer->input->data[m_hsz + type_size.value(TYPE_2)], type_size.value(TYPE_3));
      break;
    }

    case PACK_ID_104:
    case PACK_ID_109:
    {
      type9_input_signals. updateSignals(&p_io_buffer->input->data[m_hsz], type_size.value(TYPE_9));
      type53_input_signals.updateSignals(&p_io_buffer->input->data[m_hsz + type_size.value(TYPE_9)], type_size.value(TYPE_53));
      break;
    }

    default:
        break;

  }

  //  qDebug() << QString(QByteArray((const char*)&p_io_buffer->input->data[0], p_io_buffer->input->offset).toHex());
  emit message(QString(QByteArray((const char*)&(p_io_buffer->input->data[m_hsz]), packsz).toHex()), lldbg, sv::log::mtParse);

  return raduga::PARSERESULT(DO_RESET, QDateTime::currentDateTime());

}

void raduga::SvRaduga::confirmation()
{
//  QByteArray confirm;
//  confirm.append((const char*)(&m_header), 6);

//  // вычисляем crc ответа
//  quint16 crc = CRC::MODBUS_CRC16((uchar*)(&m_header), 6);
//  confirm.append(quint8(crc & 0xFF));
//  confirm.append(quint8(crc >> 8));

//  memcpy(&p_io_buffer->confirm->data[0], confirm.data(), confirm.length());
//  p_io_buffer->confirm->offset = confirm.length();

}

void raduga::SvRaduga::putout()
{
////  qDebug() << "putout" << m_params.packid;
//  p_io_buffer->output->offset = 0;

//  memcpy(&p_io_buffer->output->data[p_io_buffer->output->offset], &APAK_NAME[0], SYSTEM_NAME_LEN);
//  p_io_buffer->output->offset += SYSTEM_NAME_LEN;

//  memcpy(&p_io_buffer->output->data[p_io_buffer->output->offset], &m_params.abonent, sizeof(quint16));
//  p_io_buffer->output->offset += sizeof(quint16);

//  p_io_buffer->output->offset += 14;  // резерв

//  quint16 control = 777;
//  memcpy(&p_io_buffer->output->data[p_io_buffer->output->offset], &control, sizeof(quint16));

//  quint16 crc = CRC::MODBUS_CRC16((quint8*)&p_io_buffer->output->data[p_io_buffer->output->offset], sizeof(quint16));

//  p_io_buffer->output->offset += sizeof(quint16);
//  memcpy(&p_io_buffer->output->data[p_io_buffer->output->offset], &crc, sizeof(quint16));

//  p_io_buffer->output->offset += sizeof(quint16);

}

/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* device = new raduga::SvRaduga();
  return device;
}
