#ifndef RADUGA_DEFS_H
#define RADUGA_DEFS_H

#include <QMap>
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "/home/user/job/Modus/global/signal/sv_signal.h"
//#include "../../../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
//#include "../../../../../../Modus/global/global_defs.h"
//#include "../../../../Modus/global/device/device_defs.h"

#define TYPE_1  1
#define TYPE_2  2
#define TYPE_3  3
#define TYPE_5  5
#define TYPE_9  9
#define TYPE_53 53

#define TYPE_11 11
#define TYPE_12 12
#define TYPE_13 13
#define TYPE_14 14
#define TYPE_15 15
#define TYPE_16 16
#define TYPE_17 17
#define TYPE_18 18
#define TYPE_19 19
#define TYPE_20 20

#define P_RADUGA_BYTE   "byte"
#define P_RADUGA_OFFSET "offset"
#define P_RADUGA_LEN    "len"
#define P_RADUGA_TIP    "tip"

namespace raduga {

  enum TIP {
    Unknown = -1,
    Discrete,
    Short,
    Ustavka,
    Long,
    Analog,
    Float
  };

  const QMap<QString, TIP> tips = {{"discrete", TIP::Discrete }, {"short",  TIP::Short },
                                   {"word", TIP::Short        }, {"ushort", TIP::Short },
                                   {"ustavka",  TIP::Ustavka  }, {"long",  TIP::Long  }, {"ulong",  TIP::Long  },
                                   {"analog",   TIP::Analog   }, {"float", TIP::Float } };

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
    quint16 byte     = 0;
    quint8  offset   = 0;
    quint8  len      = 0;
    TIP     tip      = TIP::Unknown;

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

      /* byte */
      P = P_RADUGA_BYTE;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Номер байта должен быть задан двухбайтовым целым числом в десятичном формате"));

        p.byte = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      /* offset */
      P = P_RADUGA_OFFSET;
      if(object.contains(P)) {

        if((object.value(P).toInt(-1) == -1) || (object.value(P).toInt(-1) > 7))
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Сдвиг должен быть задан однобайтовым целым числом в десятичном формате в диапазоне [0..7]"));

        p.offset = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      /* len */
      P = P_RADUGA_LEN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Кол-во бит должено быть задано однобайтовым целым числом в десятичном формате"));

        p.len = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      /* tip */
      P = P_RADUGA_TIP;
      if(object.contains(P)) {

        if(object.value(P).toString("").isEmpty())
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Тип сигнала не может быть пустым"));

        if(!tips.contains(object.value(P).toString().toLower()))
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg(QString("Неверный тип сигнала \"%1\"").arg(object.value(P).toString())));

        p.tip = tips.value(object.value(P).toString());

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

      j.insert(P_RADUGA_BYTE,   QJsonValue(byte).toInt());
      j.insert(P_RADUGA_OFFSET, QJsonValue(offset).toInt());
      j.insert(P_RADUGA_LEN,    QJsonValue(len).toInt());
      j.insert(P_RADUGA_TIP,    QJsonValue(tips.key(tip)).toString());

      return j;

    }
  };

  const int  SYSTEM_NAME_LEN  = 16;
  const int  HEADER_LEN       = 32;

  #pragma pack(push,1)
  struct Header
  {
    char    system_name[SYSTEM_NAME_LEN] = {'R','a','d','u','g','a','-','2','0','3','8','6','\0','\0','\0','\0' };
    quint16 abonent_id;
    quint16 activity_id;
    quint16 pack_id;
    char    reserv[10] = { '\0','\0','\0','\0','\0','\0','\0','\0','\0','\0' };

    bool fromRawData(const char* data)
    {
      QByteArray r{data, HEADER_LEN};
      QDataStream stream(r);
      stream.setByteOrder(QDataStream::LittleEndian); // !

      stream.readRawData(&system_name[0], SYSTEM_NAME_LEN);
      stream >> abonent_id >> activity_id >> pack_id;

      stream.readRawData(&reserv[0], 10);

      /// вводим жесткие ограничения на значения, которые могут быть в заголовке записи
      if(QString::fromUtf8(&system_name[0], 12) != QString("Raduga-20386")) {
        qDebug() << "raduga::Header::fromRawData, false";
        return false;
      }

      return true;
    }
  };
  #pragma pack(pop)

}

#endif // RADUGA_DEFS_H




