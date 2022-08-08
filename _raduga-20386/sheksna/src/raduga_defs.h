#ifndef RADUGA_DEFS_H
#define RADUGA_DEFS_H

#include <QMap>

#include "../../../../../../Modus/global/signal/sv_signal.h"
#include "../../../../../../Modus/global/device/protocol/sv_abstract_protocol.h"
#include "../../../../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"

#define TYPE_1  1
#define TYPE_2  2
#define TYPE_3  3
#define TYPE_5  5
#define TYPE_9  9
#define TYPE_53 53


#define PACK_ID_101  101
#define PACK_ID_102  102
#define PACK_ID_103  103
#define PACK_ID_104  104
#define PACK_ID_105  105
#define PACK_ID_106  106
#define PACK_ID_107  107
#define PACK_ID_108  108
#define PACK_ID_109  109
#define PACK_ID_110  110


#define PACK_SZ_1  1152
#define PACK_SZ_2  384
#define PACK_SZ_3  384
#define PACK_SZ_5  1284
#define PACK_SZ_9  316
#define PACK_SZ_53 384

#define PACK_SZ_101  1152
#define PACK_SZ_102  768
#define PACK_SZ_103  1284
#define PACK_SZ_104  700
#define PACK_SZ_105  1152
#define PACK_SZ_106  1152
#define PACK_SZ_107  768
#define PACK_SZ_108  1284
#define PACK_SZ_109  700
#define PACK_SZ_110  1152

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

  const QMap<QString, TIP> tips =  {{"discrete", TIP::Discrete  }, {"short",    TIP::Short    }, {"word",  TIP::Short },
                                    {"ushort", TIP::Short       }, {"ustavka",  TIP::Ustavka  }, {"long",  TIP::Long  },
                                    {"ulong",  TIP::Long        }, {"analog",   TIP::Analog   }, {"float", TIP::Float }};

  const QMap<int, int> type_size = {{TYPE_1, PACK_SZ_1}, {TYPE_2, PACK_SZ_2}, {TYPE_3, PACK_SZ_3},
                                    {TYPE_5, PACK_SZ_5}, {TYPE_9, PACK_SZ_9}, {TYPE_53, PACK_SZ_53}};


  const QMap<int, int> pack_size = {{101, PACK_SZ_101}, {102, PACK_SZ_102}, {103, PACK_SZ_103},
                                    {104, PACK_SZ_104}, {105, PACK_SZ_105}, {106, PACK_SZ_106},
                                    {107, PACK_SZ_107}, {108, PACK_SZ_108}, {109, PACK_SZ_109},
                                    {110, PACK_SZ_110}};

//  struct DATA
//  {
//    DATA():
//      data(nullptr),
//      bufsize(0)
//    {  }

//    DATA(quint16 size):
//      data(nullptr),
//      bufsize(size)
//    {
//      data = (quint8*)malloc(size);
//    }

//    ~DATA()
//    {
//      if(data)
//        free(data);
//    }

//    bool resize(quint16 size)
//    {
//      if(data)
//        free(data);

//      data = nullptr;

//      bufsize = size;
//      data = (quint8*)malloc(size);

//      return bool(data);
//    }

//    quint8* data = nullptr;
//    quint8  type;
//    quint8  len;
//    quint16 crc;

//    quint16 bufsize;

//  };

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

  class SvAbstractSignalCollection: public QObject
  {
    Q_OBJECT

  public:
    SvAbstractSignalCollection()
    {  }

    virtual ~SvAbstractSignalCollection()
    {  }

    virtual void addSignal(modus::SvSignal* signal, quint16 bufsize) = 0;

    virtual void updateSignals(const char* data, quint16 len) = 0;

    virtual void updateOutput(const modus::BUFF* data = nullptr) = 0;

  };
}

#endif // RADUGA_DEFS_H




