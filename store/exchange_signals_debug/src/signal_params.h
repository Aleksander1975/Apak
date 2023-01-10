#ifndef SIGNAL_PARAMS_H
#define SIGNAL_PARAMS_H

#include <QString>
#include <QtCore>

#include <QJsonDocument>
#include <QJsonObject>

#include "../../../../svlib/SvException/svexception.h"
#include "../../../../Modus/global/global_defs.h"

// === Имена параметров сигналов МП МОС, подлежащих передаче (эти сигналы имеют ===
// === привязку типа "master" к модулю МП МОС). ===

// Имя параметра "тип данных сигнала" - задаётся целым числом от 1 до 50
// из перечисления QMetaType::Type:
#define DATA_TYPE     "data_type"

// Имя параметра "значение сигнала":
#define DATA_VALUE    "data_value"



// Значение по умолчанию для параметра "тип данных сигнала" (int):
#define DEFAULT_DATA_TYPE   2

// Значение по умолчанию для параметра "значение сигнала"
#define DEFAULT_DATA_VALUE  0


namespace apak
{
    struct SignalParams
    // Структура, хранящая параметры сигналов с привязкой master к МП МОС:
    // - Тип данных сигнала (задаётся целым числом от 1 до 50 из перечисления QMetaType::Type);
    // - Значение сигнала.
    {
        // Тип данных сигнала:
        quint8 data_type;

        // Значение сигнала:
        QJsonValue data_value;

        static SignalParams fromJson(const QString& json_string) //throw (SvException)
        // Конструктор:

        // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла сигналов
        // модуля МП МОС, содержащим сведения только об одном сигнале, подлежащем передаче.

        // Выход: Заполненная параметрами сигнала, подлежащего передаче, модуля МП МОС структура SignalParams.
        {
            QJsonParseError err;
            QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

            if(err.error != QJsonParseError::NoError)
            {
                throw SvException(QString("МП МОС: Ошибка при разборе параметров сигнала: ") + err.errorString());
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

        // Вход: объект QJsonObject, описывающий один подлежащий передаче сигнал модуля МП МОС.

        // Выход: Заполненная параметрами подлежащего передаче сигнала модуля МП МОС
        // структура SignalParams.
        {
            SignalParams p;
            QString P;

            // Считываем значение параметра "тип данных сигнала":
            P = DATA_TYPE;
            if(object.contains(P))
            {
                if(object.value(P).toInt(-1) < 0)
                    throw SvException(QString("МП МОС: ") + QString(IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toInt())
                               .arg("Тип данных сигнала задаётся целым числом от 1 до 50 из перечисления QMetaType::Type"));

                p.data_type = object.value(P).toInt();
            }
            else
                throw SvException(QString("МП МОС: ") + QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

            // Считываем значение параметра "значение сигнала":
            P = DATA_VALUE;
            if(object.contains(P))
            {
                p.data_value = object.value(P);
            }
            else
                throw SvException(QString("МП МОС: ") + QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

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

            j.insert(DATA_TYPE,  QJsonValue(data_type));
            j.insert(DATA_VALUE,  QJsonValue(data_value));

            return j;
        }
    };
}

#endif // SIGNAL_PARAMS_H
