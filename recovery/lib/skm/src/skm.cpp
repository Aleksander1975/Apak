#include "skm.h"


zn1::SKM::SKM()
{
}

bool zn1::SKM::getPeriodValues(const QString& data_file, const QList<modus::SignalConfig>& signal_config_list, QMap<modus::SignalConfig, QList<zn1::InstantValue>>& result)
{
  QFile f(data_file);

  try {

    // парсим параметры сигналов, чтобы знать, где их искать в массиве данных
//    // если во время парсинга возникнет ошибка, выходим и райзим ошибку наверх
//    QList<skm::SignalParams>& signal_params_list;
//    for(auto signal_config: signal_config_list) {

//      skm::SignalParams p = skm::SignalParams::fromJson(signal_config.params);

//      signal_params_list.append(p);

//    }

    if(!f.open(QIODevice::ReadOnly))
      throw SvException(f.errorString());

    qint64 total_size = QFileInfo(f).size();

    zn1::ZNRHeader znrhead = zn1::ZNRHeader::fromByteArray(f.readAll());

    // проверяем корректность маркера и определяем тип данных в файле
//    int buf_size = BUF_SIZE_BY_MARKER.value(znrhead.marker, -1);

    if(znrhead.marker != "SKM20386")
      throw SvException(QString("Неверный маркер \"%1\" в файле \"%2\"").arg(znrhead.marker).arg(data_file));

    // к размеру буфера добавляем длину заголовка CKM 32 байта и 2 байта длина crc!
//    buf_size += skm::HEADER_LEN + 2;

    // если попали сюда, значит файл корректный и заголовок с данными прочитаны
    f.seek(znrhead.len);
    QDataStream s(&f);

    qint64 total_progress = 0;
    p_running = true;
    while(!s.atEnd() && p_running) {

      // вначале идет дата/время, а потом сами данные
      qint64 dt;
      s >> dt;

      // готовим буфер для чтения
      char header_buf[skm::HEADER_LEN]; // длина заголовка CKM и 2 байта длина crc!
      memset(&header_buf[0], 0, skm::HEADER_LEN);

      if(s.readRawData(&header_buf[0], skm::HEADER_LEN) < skm::HEADER_LEN)
        throw SvException(QString("Неверный размер файла. Данные не могут быть прочитаны корректно"));

      total_progress += skm::HEADER_LEN;
      emit progress(int((float(total_progress) / total_size) * 100));

      skm::Header skm_header;
      if(!skm_header.fromRawData(&header_buf[0]))
        throw SvException(QString("Ошибка чтения заголовка файла. Данные не могут быть прочитаны корректно"));

      char skm_data[skm_header.data_len];
      memset(&skm_data[0], 0, skm_header.data_len);

      if(s.readRawData(&skm_data[0], skm_header.data_len) < skm_header.data_len)
        throw SvException(QString("Неверный размер файла. Данные не могут быть прочитаны корректно"));

      quint16 crc;
      s >> crc;

      if(skm_header.data_len == 0)
        continue;

      // вытаскиваем данные сигналов
      for(auto signal_config: signal_config_list) {

        if(!p_running)
          break;

        // парсим параметры сигналов, чтобы знать, где их искать в массиве данных
        // если во время парсинга возникнет ошибка, выходим и райзим ошибку наверх
        skm::SignalParams p = skm::SignalParams::fromJson(signal_config.params);

        // определяем значение сигнала в момент времени dt
//        QVariant signal_value;
        int offset = 0;
        for(int c = 0; c < skm_header.camera_count; c++) {

          quint8 vin = skm_data[offset++];
          quint8 fcnt= skm_data[offset++];

          if(vin != p.vin) {

            offset += fcnt;
            continue;
          }

          int f = fcnt;
          while(f--) {

            quint8 factor = skm_data[offset++];

            if(factor == p.factor)
              result[signal_config].append(zn1::InstantValue(dt, 1));

          }
        }
      }
    }

    f.close();

    return true;

  }
  catch(SvException& e) {

    if(f.isOpen())
      f.close();

    last_error = e.error;

    return false;
  }

}

/** ********** EXPORT ************ **/
zn1::AbstractOuterSystem* create()
{
  zn1::AbstractOuterSystem* obj = new zn1::SKM();
  return obj;
}
