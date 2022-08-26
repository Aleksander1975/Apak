#include "coarse_data_picker.h"

zn1::CoarseDataPicker::CoarseDataPicker()
{

}

bool zn1::CoarseDataPicker::configure(zn1::RecoveryConfig& config, const QList<CoarseFilter> coarseFilters) // const QString& json)
{
  try {

    m_config = config;
    m_coarse_filters = coarseFilters;

    return true;

  }
  catch(SvException& e) {

//    if(f.isOpen())
//      f.close();

    m_last_error = e.error;
    return false;
  }
}

void zn1::CoarseDataPicker::run()
{
  //! чтение данных
  m_is_running = true;

  QFile srcfile(m_config.zn_data_file);

  QHash<QString, QFile*> dstfiles;


  try {

    QFileInfo fi = QFileInfo(srcfile);
    m_total_read_size = fi.size();

    if(m_total_read_size <= 0)
      throw SvException(QString("Файл пуст или не существует: %1")
                        .arg(fi.fileName()));

    if(!srcfile.open(QIODevice::ReadOnly))
      throw SvException(srcfile.errorString());

    qint64 pos = m_config.pickerParams.start_byte; //446867787;
    qint64 found_count = 0;

    int mrksz = sizeof(zn1::BunchMarker);
    int headsz = sizeof(zn1::BunchHeader);
    char bunch[headsz];
//    char test_marker[4] = {char(0xAA), char(0x0), char(0x0), char(0x0)};

    while(m_is_running && !srcfile.atEnd()) {

      srcfile.seek(pos);
//qDebug() << pos << m_total_read_size;

      srcfile.read(&bunch[0], headsz);

      emit read_progress(int(((pos + headsz) / m_total_read_size) * 100));

      bool is_marker = true;
      bool is_possible_marker = false;
//      qDebug() << QString(QByteArray(1, bunch[0]).toHex()) << QString(QByteArray(1, bunch[1]).toHex()) << QString(QByteArray(1, bunch[2]).toHex()) << QString(QByteArray(1, bunch[3]).toHex());
      int i = 0;
      for(; i < mrksz; i++) {

        is_marker = is_marker && (bunch[i] == zn1::BunchMarker[i]); // test_marker[i]); //
        is_possible_marker |= (bunch[i] == zn1::BunchMarker[i]);

      }

      if(!is_marker) {

        if(!is_possible_marker) {

          for(; i < headsz; i++) {

            if(bunch[i] == zn1::BunchMarker[0]) {

              is_possible_marker = true;
              break;
            }
          }

          if(is_possible_marker)
            pos += i;
          else
            pos += headsz;

          continue;

        }
        else
          pos++;

        continue;
      }

      emit message(QString("Найден маркер %1 в позиции %2")
                   .arg(QString(QByteArray(&zn1::BunchMarker[0], sizeof(zn1::BunchMarker)).toHex()).toUpper())
                   .arg(pos), sv::log::llDebug2);

      // маркер найден. проверяем crc
      zn1::BunchHeader h;
      h.fromRawData(&bunch[0]);

      if(h.crc16 != crc::crc16ccitt(&bunch[0], headsz - 2)) {

        emit message(QString("Контрольная сумма в позиции %1 не совпадает").arg(pos));

        pos++;
        continue;

      }

      // если считанная и рассчитанная crc совпадают,
      // считаем, что это правильный заголовок и начинаем выбирать данные
      char data[h.dataLength];
      srcfile.read(&data[0], h.dataLength);

      qint64 offset = 0;

      while(m_is_running & (offset < h.dataLength)) {

        zn1::Record r;

        if(!r.fromRawData(&data[offset])) {

          emit message(QString("Неверный заголовок записи. %1").arg(QString(QByteArray::fromRawData(&data[offset], zn1::Record::headsz()).toHex())));
          break;

        }

        // ищем, есть ли задача, которая соответствует прочитанным данным
        for(zn1::CoarseFilter& coarse_filter: m_coarse_filters) {

          if(coarse_filter.period().contains(r.dateTime()) && coarse_filter.marker_hash() == r.marker_hash()) {

//            emit message(QString("Найдено соответствие %1 %2 в позиции %3")
//                         .arg(QDateTime::fromMSecsSinceEpoch(r.dateTime()).toString(DEFAULT_DATETIME_FORMAT))
//                         .arg(r.marker()).arg(pos), sv::log::llDebug, sv::log::mtInfo);

            // пытаемся открыть нужный файл для записи
            QFile* f = dstfiles.value(coarse_filter.save_path(), nullptr);

            if(!f) {

              f = new QFile(coarse_filter.save_path(m_config.data_dir));
              dstfiles.insert(coarse_filter.save_path(), f);

            }

            if(!f->isOpen()) {

              if(!f->open(QIODevice::WriteOnly))
                throw SvException(QString("Ошибка при попытке открыть файл для записи: %1/n%2")
                                  .arg(f->errorString()).arg(coarse_filter.save_path()));

              f->write(ZNRHeader(coarse_filter.marker(), coarse_filter.begin().toString(DEFAULT_DATETIME_FORMAT), coarse_filter.end().toString(DEFAULT_DATETIME_FORMAT)).toByteArray());

//              f->write(task.marker().toStdString().c_str(), task.marker().length());
//              f->write('\0');
//              f->write(task.begin().toString("dd.MM.yyyy hh:mm:ss").toStdString().c_str());
//              f->write('\0');
//              f->write(task.end().toString("dd.MM.yyyy hh:mm:ss").toStdString().c_str());
//              f->write('\0');

              emit message(QString("Открыт файл для записи %1").arg(f->fileName()), sv::log::llInfo, sv::log::mtInfo);

            }

            if(r.data().length() > 0) {

//              QByteArray dt;
              QDataStream s(f); //&dt, QIODevice::WriteOnly);
              s << r.dateTime();
              int w = s.writeRawData(r.data().data(), r.data().length());
//              qint64 w = f->write(dt);
//              w += f->write()
              if(w == -1)
                throw SvException(QString("Ошибка записи файла: %1").arg(f->errorString()));

              emit message(QString("Найдено соответствие [%1 %2] в позиции %3. Записано %4 байт в %5")
                           .arg(QDateTime::fromMSecsSinceEpoch(r.dateTime()).toString(DEFAULT_DATETIME_FORMAT))
                           .arg(r.marker()).arg(pos).arg(w).arg(f->fileName()), sv::log::llInfo, sv::log::mtSuccess);

              emit find_progress(found_count++);

              // даем основному потоку время, чтобы переварить все события в очереди
              msleep(1);

            }
          }
        }

        // передвигаем указатель на следующую запись (Record) в пучке
        offset += r.size();

      }

      pos += h.dataLength <= 0 ? mrksz : h.dataLength;

    }
  }
  catch(SvException& e) {

    emit message(e.error, sv::log::llError, sv::log::mtError);
  }

  for(QFile* file: dstfiles.values()) {

    if(file->isOpen())
      file->close();

    delete file;
  }

  if(srcfile.isOpen())
    srcfile.close();

}

void zn1::CoarseDataPicker::stop()
{
  m_is_running = false;
}
