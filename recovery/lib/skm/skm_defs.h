#ifndef RADUGA_DEFS_H
#define RADUGA_DEFS_H

#include <QMap>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "/home/user/c++/Modus/global/signal/sv_signal.h"
//#include "../../../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
//#include "../../../../../../Modus/global/global_defs.h"
//#include "../../../../Modus/global/device/device_defs.h"

#define P_SKM_VIN     "vin"
#define P_SKM_FACTOR  "factor"

namespace skm {

  struct DATA
  {
    DATA():
      data(nullptr),
      bufsize(0)
    {  }

    DATA(quint16 size):
      data(nullptr),
      bufsize(size)
    {
      data = (quint8*)malloc(size);
    }

    ~DATA()
    {
      if(data)
        free(data);
    }

    bool resize(quint16 size)
    {
      if(data)
        free(data);

      data = nullptr;

      bufsize = size;
      data = (quint8*)malloc(size);

      return bool(data);
    }

    quint8* data = nullptr;
    quint8  type;
    quint8  len;
    quint16 crc;

    quint16 bufsize;

  };

  struct SignalParams
  {
    quint8 vin    = 0;
    quint8 factor = 0;

    static SignalParams fromJson(const QString& json_string) //throw (SvException)
    {
      QJsonParseError err;
      QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

      if(err.error != QJsonParseError::NoError)
        throw SvException(err.errorString());

      try {

        return fromJsonObject(jd.object());

      }
      catch(SvException& e) {
        throw e;
      }
    }

    static SignalParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    {
      SignalParams p;
      QString P;

      /* vin */
      P = P_SKM_VIN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                            .arg("Vin номер должен быть задан однобайтовым целым числом в десятичном формате"));

        p.vin = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      /* factor */
      P = P_SKM_FACTOR;
      if(object.contains(P)) {

        if((object.value(P).toInt(-1) == -1) || (object.value(P).toInt(-1) > 7))
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                            .arg("Сдвиг должен быть задан однобайтовым целым числом в десятичном формате в диапазоне [0..7]"));

        p.factor = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      return p;

    }

    QString toString(QJsonDocument::JsonFormat format = QJsonDocument::Indented) const
    {
      QJsonDocument jd;
      jd.setObject(toJsonObject());

      return QString(jd.toJson(format));
    }

    QJsonObject toJsonObject() const
    {
      QJsonObject j;

      j.insert(P_SKM_VIN,     QJsonValue(vin).toInt());
      j.insert(P_SKM_FACTOR,  QJsonValue(factor).toInt());

      return j;

    }
  };

  const int  HEADER_LEN       = 13;

  #pragma pack(push,1)
  struct Header
  {
    quint32 begin_sign;
    quint32 header_size;
    qint32  data_len;
    quint8  camera_count;

    bool fromRawData(const char* data)
    {
      QByteArray r{data, HEADER_LEN};
      QDataStream stream(r);
      stream.setByteOrder(QDataStream::LittleEndian); // !

      stream >> begin_sign >> header_size >> data_len >> camera_count;

      /// вводим жесткие ограничения на значения, которые могут быть в заголовке записи
      if(begin_sign != 0x12345678) {
        qDebug() << "skm::Header::fromRawData, false";
        return false;
      }

      if(data_len < 0)
        return false;

      return true;
    }
  };
  #pragma pack(pop)

}

#endif // RADUGA_DEFS_H




