#ifndef PROTOCOL_PARAMS
#define PROTOCOL_PARAMS


#include <QJsonDocument>
#include <QJsonObject>
#include <QDataStream>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"
//#include "../../../../../Modus/global/device/device_defs.h"


// === Имена параметров, используемых при описании протокольной библиотеки МОС
// (модуля обмена сигналами) "libapak_exchange_signals_module" ===
// === в файле описания устройств "config_apak.json": ===

// Имя параметра "Период посылки сигналов к другому "mdserver"у":
#define P_SEND_INTERVAL       "send_interval"

// Имя параметра "Порядок байт". Возможные значения этого
// параметра: "BigEndian" и "LittleEndian".
#define P_BYTE_ORDER          "byte_order"

// Имя параметра "Точность значений с плавающей точкой". Возможные значения
// этого параметра: "SinglePrecision"и "DoublePrecision".
#define P_FLOATING_POINT_PRECISION "fp_precision"

// Имя параметра "Номер версии формата сериализации данных библиотеки Qt".
// Возможные значения этого параметра: допустимое число из перечисления:
// "enum QDataStream::Version".
#define P_VERSION_QT "version_Qt"

// Имя параметра "Номер версии протокола обмена между МОС":
// беззнаковое шестнадцатиразрядное число.
#define P_EXCHANGE_PROTOCOL_VERSION "protocol_version"

// Имя параметра "Магическое число в заголовке посылки между МОС":
// в шестнадцатеричной системе, 32-х разрядное - оно позволит нам проверить, что
// это посылка обмена сигналами между МОС.
#define P_MAGIC_NUMBER   "magic_number"

// Значение по умолчанию для параметра "Период посылки сигналов к другому "mdserver"у":
#define DEFAULT_SEND_INTERVAL   1000

// Значение по умолчанию для параметра "Порядок байт":
#define DEFAULT_BYTE_ORDER      QDataStream::BigEndian

// Значение по умолчанию для параметра "Точность значений с плавающей точкой":
#define DEFAULT_FLOATING_POINT_PRECISION    QDataStream::DoublePrecision

// Значение по умолчанию для параметра "Номер версии формата сериализации данных библиотеки Qt"
#define DEFAULT_VERSION_QT QDataStream::Qt_5_5

// Значение по умолчанию для параметра "Номер версии протокола обмена между МОС":
#define DEFAULT_EXCHANGE_PROTOCOL_VERSION 1

// Значение по умолчанию для параметра "Магическое число в заголовке посылки между МОС":
#define DEFAULT_MAGIC_NUMBER   0xF34D5397


namespace apak {

  struct ProtocolParams
   // Структура, хранящая параметры протокола обмена между МОС:
   //       - Период посылки сигналов к другому "mdserver"у,
   //       - Порядок байт,
   //       - Точность значений с плавающей точкой,
   //       - Номер версии формата сериализации данных библиотеки Qt,
   //       - Номер версии протокола обмена между МОС,
   //       - Магическое число в заголовке посылки между МОС.
    {
    // Период посылки сигналов к другому "mdserver"у:
    quint16 send_interval  = 0;

    // Порядок байт:
    quint8 byte_order = 0;

    // Точность значений с плавающей точкой:
    quint8 fp_precision = 0;

    // Номер версии формата сериализации данных библиотеки Qt:
    quint8 version_Qt = 0;

    // Номер версии протокола обмена между МОС:
    quint16 protocol_version = 0;

    // Магическое число в заголовке посылки между МОС:
    qint32 magic_number = 0;

    static ProtocolParams fromJson(const QString& json_string) //throw (SvException)
    {
        QJsonParseError err;
        QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

        if(err.error != QJsonParseError::NoError)
        {
            throw SvException(QString ("МОС: Ошибка при разборе параметров протокола: ") + err.errorString());
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

    static ProtocolParams fromJsonObject(const QJsonObject &object) //throw (SvException)
    // Конструктор:

    // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла
    // "config_apak.json" для сервера "mdserver", и содержащем информацию только
    // о модуле МОС.

    // Выход: Структура ProtocolParams, заполненная параметрами протокольной части
    // устройства МОС.
    {
        ProtocolParams p;
        QString P;

        // Считываем значение параметра "Период посылки сигналов к другому "mdserver"у":
        P = P_SEND_INTERVAL;
        if(object.contains(P))
        {
            if(object.value(P).toInt(-1) < 0)
                throw SvException(QString ("МОС: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                            .arg(P)
                            .arg(object.value(P).toString())
                            .arg("Период посылки данных должен быть задан целым числом в миллисекундах"));
            p.send_interval = object.value(P).toInt(DEFAULT_SEND_INTERVAL);
        }
        else
            p.send_interval = DEFAULT_SEND_INTERVAL;

        // Считываем значение параметра "Порядок байт":
        P = P_BYTE_ORDER;
        if(object.contains(P))
        {
            QString t = object.value(P).toString("");
            int cmp_BigEndian = t.compare ("BigEndian", Qt::CaseInsensitive);
            int cmp_LittleEndian = t.compare("LittleEndian", Qt::CaseInsensitive);

            if (cmp_BigEndian != 0 && cmp_LittleEndian != 0)
                throw SvException(QString("МОС: ") + QString(IMPERMISSIBLE_VALUE)
                          .arg(P)
                          .arg(object.value(P).toString())
                          .arg("Порядок байт может быть: BigEndian или LittleEndian"));

            if (cmp_BigEndian == 0)
                p.byte_order = QDataStream::BigEndian;
            if (cmp_LittleEndian == 0)
                p.byte_order = QDataStream::LittleEndian;
        }
        else
            p.byte_order = DEFAULT_BYTE_ORDER;

        // Считываем значение параметра "Точность значений с плавающей точкой":
        P = P_FLOATING_POINT_PRECISION;
        if(object.contains(P))
        {
            QString t = object.value(P).toString("");
            int cmp_SinglePrecision = t.compare ("SinglePrecision", Qt::CaseInsensitive);
            int cmp_DoublePrecision = t.compare("DoublePrecision", Qt::CaseInsensitive);

            if (cmp_SinglePrecision != 0 && cmp_DoublePrecision != 0)
                throw SvException(QString("МОС: ") + QString(IMPERMISSIBLE_VALUE)
                          .arg(P)
                          .arg(object.value(P).toString())
                          .arg("Точность значений с плавающей точкой может быть: SinglePrecision или DoublePrecision"));

            if (cmp_SinglePrecision == 0)
                p.fp_precision = QDataStream::SinglePrecision;
            if (cmp_DoublePrecision == 0)
                p.fp_precision = QDataStream::DoublePrecision;
        }
        else
            p.fp_precision = DEFAULT_FLOATING_POINT_PRECISION;

        // Считываем значение параметра "Номер версии формата сериализации данных библиотеки Qt":
        P = P_VERSION_QT;
        if(object.contains(P))
        {
            if(object.value(P).toInt(-1) < 0)
                throw SvException(QString ("MOC: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Номер версии формата сериализации данных библиотеки Qt должен быть задан целым числом"));

            p.version_Qt = object.value(P).toInt(DEFAULT_VERSION_QT);
        }
        else
            p.version_Qt = DEFAULT_VERSION_QT;

        // Считываем значение параметра "Номер версии протокола обмена между МОС должен быть задан целым числом":
        P = P_EXCHANGE_PROTOCOL_VERSION;
        if(object.contains(P))
        {
            if(object.value(P).toInt(-1) < 0)
                throw SvException(QString ("МОС: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Номер версии протокола обмена между МОС должен быть задан целым числом"));

            p.protocol_version = object.value(P).toInt(DEFAULT_EXCHANGE_PROTOCOL_VERSION);
        }
        else
            p.protocol_version = DEFAULT_EXCHANGE_PROTOCOL_VERSION;

        // Считываем значение параметра "Магическое число в заголовке посылки между МОС":
        P = P_MAGIC_NUMBER;
        if(object.contains(P))
        {
            QString magicNumberString = object.value(P).toString();

            if ( magicNumberString.isNull() == true)
            {
                throw SvException(QString ("МОС: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Магическое число в заголовке посылки между МОС должно быть задано целым шестнадцатеричрым числом"));
            }

            bool ok;
            quint32 magicNumber = magicNumberString.toUInt(&ok, 16);

            if ( ok == false)
            {
                throw SvException(QString ("МОС: Ошибка при разборе параметров протокола: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toString())
                                 .arg("Магическое число в заголовке посылки между МОС должно быть задано целым шестнадцатеричрым числом"));

            }

            p.magic_number = magicNumber;
        }
        else
            p.magic_number = DEFAULT_MAGIC_NUMBER;

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

        j.insert (P_SEND_INTERVAL,     QJsonValue(send_interval));
        j.insert (P_BYTE_ORDER,  QJsonValue(byte_order));
        j.insert (P_FLOATING_POINT_PRECISION,  QJsonValue(fp_precision));
        j.insert (P_VERSION_QT,     QJsonValue(version_Qt));
        j.insert (P_EXCHANGE_PROTOCOL_VERSION,  QJsonValue(protocol_version));
        j.insert (P_MAGIC_NUMBER,  QJsonValue(magic_number));

        return j;
    }
  };
}

#endif // PROTOCOL_PARAMS

