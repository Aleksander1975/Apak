#include "radga.h"


zn1::RADGA::RADGA()
{
}

bool zn1::RADGA::getPeriodValues(const QString& data_file, const QList<modus::SignalConfig>& signal_config_list, QMap<modus::SignalConfig, QList<zn1::InstantValue>>& result)
{
  QFile f(data_file);

  try {

    if(!f.open(QIODevice::ReadOnly))
      throw SvException(f.errorString());

    qint64 total_size = QFileInfo(f).size();

    zn1::ZNRHeader znrhead = zn1::ZNRHeader::fromByteArray(f.readAll());

    // проверяем корректность маркера и определяем тип данных в файле
    int buf_size = BUF_SIZE_BY_MARKER.value(znrhead.marker, -1);

    if(buf_size == -1)
      throw SvException(QString("Неверный маркер \"%1\" в файле \"%2\"").arg(znrhead.marker).arg(data_file));

    // к размеру буфера добавляем длину заголовка радуги 32 байта и 2 байта длина crc!
    buf_size += raduga::HEADER_LEN + 2;

    // если попали сюда, значит файл корректный и заголовок с данными прочитаны
    f.seek(znrhead.len);
    QDataStream s(&f);

    qint64 total_progress = 0;
    p_running = true;
    while(!s.atEnd() && p_running) {

      // вначале идет дата/время, а потом сами данные
      qint64 dt;
      s >> dt;

      // готовим буфер для чтения данных. размер буфера определяется для каждой системы ОТДЕЛЬНО!!
      char buf[buf_size];
      memset(&buf[0], 0, buf_size);

      if(s.readRawData(&buf[0], buf_size) < buf_size)
        throw SvException(QString("Неверный размер файла. Данные не могут быть прочитаны корректно"));

      total_progress += buf_size;
      emit progress(int((float(total_progress) / total_size) * 100));

      // вытаскиваем данные сигналов, которые переданы
      for(auto signal_config: signal_config_list) {

        if(!p_running)
          break;

        // если сигнал не выбран в списке сигналов, то пропускаем его
//        if(signal_config.enable)
//          continue;

        // парсим параметры сигнала, чтобы знать, где его искать в массиве данных
        // если во время парсинга возникнет ошибка, выходим и райзим ошибку наверх
        raduga::SignalParams params = raduga::SignalParams::fromJson(signal_config.params);

        // определяем значение сигнала в момент времени dt
        QVariant signal_value;
        switch (params.tip) {

          case raduga::TIP::Discrete:
          case raduga::TIP::Ustavka:
          {
            signal_value = QVariant((buf[raduga::HEADER_LEN + params.byte] >> params.offset) & ((1 << params.len) - 1));
            break;
          }

          case raduga::TIP::Short:
          {
            qint16 v = 0;
            memcpy(&v, &buf[raduga::HEADER_LEN + params.byte], sizeof(qint16));

            signal_value = QVariant(v);

            break;
          }

          case raduga::TIP::Long:
          {
            qint32 v = 0;
            memcpy(&v, &buf[raduga::HEADER_LEN + params.byte], sizeof(qint32));

            signal_value = QVariant(v);

            break;
          }

          case raduga::TIP::Analog:
          case raduga::TIP::Float:
          {
            float v = 0;
            memcpy(&v, &buf[raduga::HEADER_LEN + params.byte], sizeof(float));

            signal_value = QVariant(v);

            break;
          }

          default:
            break;
        }

        result[signal_config].append(zn1::InstantValue(dt, signal_value));

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
  zn1::AbstractOuterSystem* obj = new zn1::RADGA();
  return obj;
}
