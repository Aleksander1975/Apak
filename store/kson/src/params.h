#ifndef RADUGA_PROTOCOL_PARAMS
#define RADUGA_PROTOCOL_PARAMS

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../svlib/SvException/svexception.h"
#include "../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"

//#define P_ABONENT  "abonent"
#define P_BYTE    "byte"
#define P_OFFSET  "offset"
#define P_LEN     "len"

namespace apak {

  struct ProtocolParams {

    static ProtocolParams fromJson(const QString& json_string) //throw (SvException)
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

    static ProtocolParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    {
      ProtocolParams p;
      QString P;

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

      return j;

    }
  };

  struct SignalParams
  {
    quint16 byte     = 0;
    quint8  offset   = 0;
    quint8  len      = 0;

    static SignalParams fromJson(const QString& json_string)
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

    static SignalParams fromJsonObject(const QJsonObject &object)
    {
      SignalParams p;
      QString P;

      /* byte */
      P = P_BYTE;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                            .arg("Номер байта должен быть задан целым числом в десятичном формате"));

        p.byte = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      /* offset */
      P = P_OFFSET;
      if(object.contains(P)) {

        if((object.value(P).toInt(-1) == -1) || (object.value(P).toInt(-1) > 7))
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                            .arg("Сдвиг должен быть задан целым числом в десятичном формате в диапазоне [0..7]"));

        p.offset = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM).arg(P));

      /* len */
      P = P_LEN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) == -1)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact)))
                            .arg("Кол-во бит должено быть задано целым числом в десятичном формате"));

        p.len = object.value(P).toInt();

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

      j.insert(P_BYTE,   QJsonValue(byte).toInt());
      j.insert(P_OFFSET, QJsonValue(offset).toInt());
      j.insert(P_LEN,    QJsonValue(len).toInt());

      return j;

    }
  };
}

#endif // RADUGA_PROTOCOL_PARAMS

