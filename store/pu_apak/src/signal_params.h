#ifndef SIGNAL_PARAMS_H
#define SIGNAL_PARAMS_H

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"

// Имена параметров сигналов, которыми обмениваются между собой ПУ АПАК:

// === ПУ АПАК-1 и ПУ АПАК-2 обмениваются между собой сведениями (сигналами),
// === содержащими информацию о собственном состоянии (работает / не работает),
// === a также о состоянии их сопряжения с другими устройствами (СКМ, ИВ-1, КРАБ...).
// === Все сигналы состояния имеют тип "STAT", который указывается для конкретного
// === сигнала или для группы сигналов. Поскольку все сигналы состояния (кроме
// === сигналов о сопряжении с КСОН) передаются на КСОН, то все они имеют ряд
// === параметров, которые:
// === - необходимы для проверки допустимости их значений перед передачей на КСОН
// ===   (параметр "data_type", "min", "max")
// === - необходимы для указания актуальности группы, к которой они принадлежат
// ===   (параметр "group")
// === - необходимы для определения их места (очерёдности) в информационном кадре
// ===   (параметры "byte", "offset", "len").
// Для обмена между ПУ АПАК-1 и ПУ АПАК-2 нами используется только параметр "data_type",
// который должен иметь тип "boolean".


// Имя параметра "тип данных сигнала" - задаётся значением (boolType, unsignedType, floatType)
//                  из перечиcления "dataType":
#define DATA_TYPE     "data_type"

// Значение по умолчанию для параметра "тип данных сигнала" ("unsigned", "float", "boolean"):
#define DEFAULT_DATA_TYPE   boolType

// Перечисление возможных для сигнала КСОН типов данных:
enum dataType : quint8
{
    boolType, // булевский тип (1 бит)
    unsignedType, // тип беззнакового целого (2 байта)
    floatType // число с плавающей запятой (4 байта)
};

namespace apak
{
    struct SignalParams
    // Структура, хранящая параметры сигналов, используемых для обмена между собой ПУ АПАК:
    // тип данных сигнала ("unsigned", "float", "boolean")
    {
        // Тип данных сигнала - задаётся значением (boolType, unsignedType, floatType)
        //                  из перечиcления "dataType":
        quint8 data_type;

        static SignalParams fromJson(const QString& json_string) //throw (SvException)
        // Конструктор:

        // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла
        // "config_apak.json" (в котором содержится, в частности, описание сигналов,
        // используемых ПУ АПАК), и содержащем информацию об одном сигнале.

        // Выход: Заполненная параметрами сигнала ПУ АПАК структура SignalParams.
        {
            QJsonParseError err;
            QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

            if(err.error != QJsonParseError::NoError)
                throw SvException(err.errorString());

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

        // Вход: объект QJsonObject, описывающий один сигнал, использумый при обмене
        // между ПУ АПАК.

        // Выход: Заполненная параметрами сигналов, используемых для
        // обмена между собой ПУ АПАК, структура SignalParams.
        {
            SignalParams p;
            QString P;

            // Считываем значение параметра "Тип данных сигнала":
            P = DATA_TYPE;
            if(object.contains(P))
            {
                QString t = object.value(P).toString("");
                int cmp_boolean = t.compare ("boolean", Qt::CaseInsensitive);
                int cmp_float = t.compare("float", Qt::CaseInsensitive);
                int cmp_unsigned = t.compare("unsigned", Qt::CaseInsensitive);

                if (cmp_boolean != 0 && cmp_float != 0 && cmp_unsigned != 0)
                    throw SvException(QString(IMPERMISSIBLE_VALUE)
                                .arg(P)
                                .arg(object.value(P).toString())
                                .arg("Тип данных сигнала может быть: boolean, unsigned или float"));

                //p.data_type = object.value(P).toString();
                if (cmp_boolean == 0)
                    p.data_type = boolType;
                if (cmp_unsigned == 0)
                    p.data_type = unsignedType;
                if (cmp_float == 0)
                    p.data_type = floatType;
            }
            else
                p.data_type = DEFAULT_DATA_TYPE;


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

            j.insert(DATA_TYPE, QJsonValue(data_type));

            return j;
        }
    };
}

#endif // SIGNAL_PARAMS_H
