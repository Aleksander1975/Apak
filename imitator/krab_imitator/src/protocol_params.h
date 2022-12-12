#ifndef KRAB_PROTOCOL_PARAMS
#define KRAB_PROTOCOL_PARAMS

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"

// Имя параметра "адрес ведомого устройства",
// используемое при описании имитатора устройства КРАБ в файле описания
// имитаторов:
#define ADDRESS         "address"

// Имя параметра "код функции",
// используемое при описании имитатора устройства КРАБ в файле описания
// имитаторов:
#define FUNC_CODE       "func_code"

// Имя параметра "период поступления данных от КРАБ'а к АПАК",
// используемое при описании имитатора устройства КРАБ в файле описания
// имитаторов:
#define SEND_INTERVAL   "interval"

// Имя параметра "размер поля данных в пакете запроса на запись от КРАБ'а к АПАК",
// используемое при описании имитатора устройства КРАБ в файле описания
// имитаторов:
#define DATA_LEN        "data_len"


// Имя параметра "смещение байта, в котором хранится значение сигнала от начала пакета
// запроса на запись от КРАБ'а к АПАК"
// используемое при описании сигналов имитатора КРАБ в файле описания сигналов КРАБ'a:
#define BYTE          "byte"

// Имя параметра "смещение области битов, в которой хранится значение сигнала от начала байта
// в пакете запроса на запись от КРАБ'а к АПАК",
// используемое при описании сигналов имитатора КРАБ в файле описания сигналов КРАБ'a:
#define OFFSET        "offset"

// Имя параметра "размер области бит, которая хранит значение сигнала (количество бит)",
// в пакете запроса на запись от КРАБ'а к АПАК",
// используемое при описании сигналов имитатора КРАБ в файле описания сигналов КРАБ'a:
#define LEN           "len"

// Значение адреса системы АПАК ПО УМОЛЧАНИЮ
// (данное значение берётся в том случае,
// если в конфигурационном файле имитаторов усторойств для устройства КРАБ не задано
// другое значение):
#define KRAB_DEFAULT_ADDRESS       2

// Значение периода поступления данных от устройства КРАБ в систему АПАК ПО УМОЛЧАНИЮ
// (данное значение берётся в том случае,
// если в конфигурационном файле имитаторов усторойств для устройства КРАБ не задано
// другое значение):
#define KRAB_DEFAULT_FUNC_CODE     0x10

// Значение периода поступления данных от устройства КРАБ в систему АПАК ПО УМОЛЧАНИЮ
// (данное значение берётся в том случае,
// если в конфигурационном файле имитаторов усторойств для устройства КРАБ не задано
// другое значение):
#define KRAB_DEFAULT_SEND_INTERVAL 1000

// Значение длины поля данных ПО УМОЛЧАНИЮ пакета
// запроса на запись от КРАБ'a к системе АПАК (данное значение берётся в том случае,
// если в конфигурационном файле имитаторов усторойств для устройства КРАБ не задано
// другое значение):
#define KRAB_DEFAULT_DATA_LEN      66

// Значение количества байт от начала пакета до поля данных пакета запроса на запись:
#define KRAB_HEADER_LEN     6

namespace krab {

  struct ProtocolParams
   // Структура, хранящая параметры протокола обмена КРАБ'a c АПАК:
   //       - адрес системы АПАК (ведомого устройства),
   //       - код функции,
   //       - период в мс поступления данных от КРАБ'а в систему АПАК,
   //       - размер поля данных в пакете запроса на запись от КРАБ'а к АПАК.
  {
    // Адрес системы АПАК (ведомого устройства):
    quint8  address       = 0;

    // Код функции:
    quint8  func_code     = KRAB_DEFAULT_FUNC_CODE;

    // Период в мс поступления данных от КРАБ'а в систему АПАК:
    quint16 send_interval = KRAB_DEFAULT_SEND_INTERVAL;

    // Размер поля данных в пакете запроса на запись от КРАБ'а к АПАК:
    quint8  data_len = KRAB_DEFAULT_DATA_LEN;

    static ProtocolParams fromJson(const QString& json_string) //throw (SvException)
    {
      QJsonParseError err;
      QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

      if(err.error != QJsonParseError::NoError)
        throw SvException(QString ("Имитатор КРАБ: Ошибка при разборе параметров протокола: ") + err.errorString());

      try {

        return fromJsonObject(jd.object());

      }
      catch(SvException& e) {
        throw e;
      }
    }

    static ProtocolParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    // Конструктор:

     // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла имитаторов для
     // сервера "mdserver", и содержащем информацию только об устройстве КРАБ.

     // Выход: Структура ProtocolParams, заполненная параметрами устройства КРАБ
        // (адресом системы АПАК (ведомого устройства),
        // кодом функции,
        // периодом поступления данных от устройства КРАБ,
        // размером поля данных в пакете запроса на запись от КРАБ'а к АПАК)
    {
      ProtocolParams p;
      QString P;

      // Считываем адрес системы АПАК:
      P = ADDRESS;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("Имитатор КРАБ: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Адрес устройства Modbus должен быть задан целым числом"));

        p.address = object.value(P).toInt(KRAB_DEFAULT_ADDRESS);

      }
      else
        p.address = KRAB_DEFAULT_ADDRESS;

      // Считываем код функции:
      P = FUNC_CODE;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("Имитатор КРАБ: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Код функции Modbus должен быть задан целым числом"));

        p.func_code = object.value(P).toInt(KRAB_DEFAULT_FUNC_CODE);

      }
      else
        p.func_code = KRAB_DEFAULT_FUNC_CODE;

      // Считываем период поступления данных от устройства КРАБ:
      P = SEND_INTERVAL;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("Имитатор КРАБ: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Интервал обновления данных должен быть задан целым числом в миллисекундах"));

        p.send_interval = object.value(P).toInt(KRAB_DEFAULT_SEND_INTERVAL);

      }
      else
        p.send_interval = KRAB_DEFAULT_SEND_INTERVAL;

      // Считываем размер поля данных в пакете запроса на запись от КРАБ'а к АПАК:
      P = DATA_LEN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString ("Имитатор КРАБ: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toVariant().toString())
                            .arg("Длина поля данных запроса на запись от КРАБа к АПАК должна быть задана целым числом байт"));

        p.data_len = object.value(P).toInt(KRAB_DEFAULT_DATA_LEN);

      }
      else
        p.data_len = KRAB_DEFAULT_DATA_LEN;

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

      j.insert (ADDRESS,       QJsonValue(address).toString());
      j.insert (FUNC_CODE,    QJsonValue (func_code).toString());
      j.insert (SEND_INTERVAL,      QJsonValue(send_interval).toInt());
      j.insert (DATA_LEN,   QJsonValue(data_len).toInt());

      return j;

    }
  };

  struct SignalParams
  // Структура, хранящая параметры сигнала устройства КРАБ:
  // - Смещение байта, в котором хранится значение сигнала, от начала пакета
  //                "запроса на запись" (количество байт от начала пакета),
  // - Смещение области битов, в которой хранится значение сигнала, от начала байта
  //                 (количество бит от начала байта),
  // - Размер области бит, которая хранит значение сигнала (количество бит).
  {
    // Смещение байта, в котором хранится значение сигнала, от начала пакета
    // "запроса на запись" (количество байт от начала пакета):
    quint8 byte = 0;

    // Смещение области битов, в которой хранится значение сигнала, от начала байта
    // (количество бит от начала байта):
    quint8 offset = 0;

    // Размер области бит, которая хранит значение сигнала (количество бит).
    quint8 len = 0;

    static SignalParams fromJson(const QString& json_string) //throw (SvException)
    // Конструктор:

    // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла для имитатора
    // устройства КРАБ, и содержащем информацию об одном сигнале.

    // Выход: Заполненная параметрами сигнала устройства КРАБ структура SignalParams.
    {
        QJsonParseError err;
        QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

        if(err.error != QJsonParseError::NoError)
        {
            throw SvException(QString("Имитатор КРАБ: Ошибка при разборе параметров сигнала: ") + err.errorString());
        }
        try
        {
            return fromJsonObject(jd.object());
        }
        catch(SvException& e)
        {
            throw e;
        }
    }

    static SignalParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    // Конструктор:

    // Вход: объект QJsonObject, описывающий один сигнал устройства КРАБ.

    // Выход: Заполненная параметрами сигнала устройства КРАБ структура SignalParams.
    {
      SignalParams p;
      QString P;

      // Считываем значение параметра "смещение байта от начала пакета":
      P = BYTE;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString("Имитатор КРАБ: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Смещение байта от начала пакета должно быть задано целым числом"));

        p.byte = object.value(P).toInt();

      }
      else
        throw SvException(QString("Имитатор КРАБ: ") + QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "смещение области битов в которой хранится значение
      // сигнала от начала байта":
      P = OFFSET;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString("Имитатор КРАБ: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Cмещение области битов от начала байта должнo быть задано целым числом"));

        p.offset = object.value(P).toInt();

      }
      else
        throw SvException(QString("Имитатор КРАБ: ") + QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

      // Считываем значение параметра "размер области битов":
      P = LEN;
      if(object.contains(P)) {

        if(object.value(P).toInt(-1) < 0)
          throw SvException(QString("Имитатор КРАБ: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toVariant().toString())
                                 .arg("Размер области битов должен быть задан целым числом"));

        p.len = object.value(P).toInt();

      }
      else
        throw SvException(QString("Имитатор КРАБ: ") + QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

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
      return j;

    }
  };
}

#endif // KRAB_PROTOCOL_PARAMS

