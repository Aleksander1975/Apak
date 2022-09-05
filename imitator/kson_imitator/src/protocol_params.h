#ifndef KSON_IMITATOR_PARAMS
#define KSON_IMITATOR_PARAMS

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"

// Имя параметра "период поступления данных от КСОН к АПАК",
// используемое при описании имитатора устройства КСОН в файле описания
// имитаторов:
#define SEND_INTERVAL   "interval"

// Имя параметра "размер поля данных в пакете запроса на запись от КCОН'а к АПАК",
// используемое при описании имитатора устройства КСОН в файле описания
// имитаторов:
#define DATA_LEN        "data_len"


// Имя параметра "смещение байта, в котором хранится значение сигнала от начала поля
// данных информационного кадра от сети КСОН к системе АПАК
// используемое при описании сигналов имитатора КСОН в файле описания сигналов КСОН'a:
#define BYTE          "byte"

// Имя параметра "смещение области битов, в которой хранится значение сигнала от начала поля
// данных информационного кадра от сети КСОН к системе АПАК,
// используемое при описании сигналов имитатора КСОН в файле описания сигналов КСОН'a:
#define OFFSET        "offset"

// Имя параметра "размер области бит, которая хранит значение сигнала (количество бит)",
// в поле данных информационного кадра от сети КСОН к системе АПАК
// используемое при описании сигналов имитатора КРАБ в файле описания сигналов КРАБ'a:
#define LEN           "len"

// Имя параметра "Тип данных сигнала" ("unsigned", "float", "boolean")". Это тип данных,
// который используется для представления параметрического данного (сигнала), передаваемого
// в информационном блоке от КСОН в АПАК.
#define TYPE        "type"

// Имя параметра "Номер группы". Это номер группы к которой принадлежит сигнал. Сигналы от
// сети КСОН в систему АПАК принадлежат к 11, 12 или 13 группе. В протоколе передачи от
// КСОН к АПАК указывается актуальность сигналов каждой из групп.
#define GROUP       "group"

// Значение периода поступления данных от сети КСОН в систему АПАК ПО УМОЛЧАНИЮ
// (данное значение берётся в том случае,
// если в конфигурационном файле имитаторов усторойств для устройства КСОН не задано
// другое значение):
#define KSON_DEFAULT_SEND_INTERVAL 1000

namespace kson {

  struct ProtocolParams
  // Структура, хранящая параметр протокола - период в мс поступления данных от сети КСОН в систему АПАК.
  {
    // Период поступления данных в мс от сети КСОН в систему АПАК:
    quint16 send_interval = KSON_DEFAULT_SEND_INTERVAL;

    static ProtocolParams fromJson(const QString& json_string) //throw (SvException)
    // Конструктор:

    // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла имитаторов для
    // сервера "mdserver", и содержащем информацию только об устройстве КСОН.

    // Выход: Структура ProtocolParams, заполненная параметром устройства КСОН (периодом поступления
    // данных от сети КСОН в систему АПАК).
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
    // Конструктор:

    // Вход: объект QJsonObject, описывающий устройство КСОН.

    // Выход: Структура ProtocolParams, заполненная параметром устройства КСОН (периодом поступления
    // данных от устройства КСОН).
   {
      ProtocolParams p;
      QString P;

      P = SEND_INTERVAL;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Интервал отправки данных должен быть задан целым числом в миллисекундах"));

        p.send_interval = object.value(P).toInt(KSON_DEFAULT_SEND_INTERVAL);

      }
      else
        p.send_interval = KSON_DEFAULT_SEND_INTERVAL;

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

      j.insert(SEND_INTERVAL,   QJsonValue(send_interval).toInt());

      return j;

    }
  };

  struct SignalParams
  // Структура, хранящая параметры сигнала устройства КСОН:
  // - Смещение байта, в котором хранится значение сигнала, от начала пакета
  //                "запроса на запись" (количество байт от начала пакета),
  // - Смещение области битов, в которой хранится значение сигнала, от начала байта
  //                 (количество бит от начала байта),
  // - Размер области бит, которая хранит значение сигнала (количество бит).
  // - Тип данных сигнала ("unsigned", "float", "boolean").
  // - Номер группы, к которой относится данный сигнал (с 10-ой по 12-ую).
  {
    // Смещение байта, в котором хранится значение сигнала, от начала пакета
    // "запроса на запись" (количество байт от начала пакета):
    quint8 byte = 0;

    // Смещение области битов, в которой хранится значение сигнала, от начала байта
    // (количество бит от начала байта):
    quint8 offset = 0;

    // Размер области бит, которая хранит значение сигнала (количество бит).
    quint8 len = 0;

    // Тип данных сигнала ("unsigned", "float", "boolean"):
    QString type;

    // Номер группы:
    quint8 group;

    static SignalParams fromJson(const QString& json_string) //throw (SvException)
    // Конструктор:

    // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла для имитатора
    // устройства КСОН, и содержащем информацию об одном сигнале.

    // Выход: Заполненная параметрами сигнала устройства КСОН структура SignalParams.
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
    // Конструктор:

    // Вход: объект QJsonObject, описывающий один сигнал устройства КСОН.

    // Выход: Заполненная параметрами сигнала устройства КСОН структура SignalParams.
    {
      SignalParams p;
      QString P;

      // Считываем значение параметра "смещение байта от начала пакета":
      P = BYTE;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Смещение байта от начала пакета должно быть задано целым числом"));

        p.byte = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "смещение области битов в которой хранится значение
      // сигнала от начала байта":
      P = OFFSET;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Cмещение области битов от начала байта должнo быть задано целым числом"));

        p.offset = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "размер области битов":
      P = LEN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Размер области битов должен быть задан целым числом"));

        p.len = object.value(P).toInt();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "Тип данных сигнала":
      P = TYPE;
      if(object.contains(P)) {

        QString t = object.value(P).toString();
        int cmp_boolean = t.compare ("boolean", Qt::CaseInsensitive);
        int cmp_float = t.compare("float", Qt::CaseInsensitive);
        int cmp_unsigned = t.compare("unsigned", Qt::CaseInsensitive);

        if (cmp_boolean != 0 && cmp_float != 0 && cmp_unsigned != 0)
            throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Тип данных сигнала может быть: boolean, unsigned или float"));

        p.type = object.value(P).toString();

      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "номер группы":
      P = GROUP;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Номер группы должен быть задан целым числом"));

        p.group = object.value(P).toInt();
      }
      else
        throw SvException(QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));


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

      j.insert(BYTE,  QJsonValue(byte).toInt());
      j.insert(OFFSET,  QJsonValue(offset).toInt());
      j.insert(LEN,  QJsonValue(len).toInt());
      j.insert(TYPE, QJsonValue(type).toString());
      j.insert(GROUP, QJsonValue(group).toInt());
      return j;

    }
  };
}

#endif // KSON_PROTOCOL_PARAMS

