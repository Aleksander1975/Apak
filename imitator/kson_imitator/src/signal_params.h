#ifndef SIGNAL_PARAMS_H
#define SIGNAL_PARAMS_H

// === Имена параметров сигналов имитатора КСОН ===
// === Все сигналы имитатора КСОН описаны в файле "kson.json"===

// Имя параметра "смещение байта, в котором хранится значение сигнала от начала информацинного
// блока информационного кадра":
#define BYTE          "byte"

// Имя параметра "смещение области битов, в которой хранится значение сигнала от начала байта":
#define OFFSET        "offset"

// Имя параметра "размер области бит, которая хранит значение сигнала (количество бит),
//                  в информационном блоке информационного кадра":
#define LEN           "len"

// Имя параметра "тип данных сигнала" - задаётся значением (boolType, unsignedType, floatType)
//                  из перечиcления "dataType":
#define DATA_TYPE     "data_type"

// Имя параметра "максимальное значение сигнала". Этот параметр может использоваться для проверки
// допустимости значений сигналов параметрических данных, получаемых нами в информационном
// кадре от АРАК (на данный момент - все данные, приходящие от АПАК - булевы, поэтому этот параметр
// не используется).
#define MAX         "max"

// Имя параметра "минимальное значение сигнала". Этот параметр может используется для проверки
// допустимости значений сигналов параметрических данных, получаемых нами в информационном
// кадре от АПАК (на данный момент - все данные, приходящие от АПАК - булевы, поэтому этот параметр
// не используется).
#define MIN         "min"

// Имя параметра "номер группы". Это номер группы к которой принадлежит сигнал.
// В протоколaх передачи информационных кадров между КСОН к АПАК указывается
// актуальность сигналов каждой из групп.
#define GROUP       "group"

// Значение по умолчанию для параметра "тип данных сигнала" ("unsigned", "float", "boolean")":
#define DEFAULT_DATA_TYPE   floatType

// Значение по умолчанию для параметра "максимальное значение сигнала"
#define DEFAULT_MAX 0

// Значение по умолчанию для параметра "минимальное значение сигнала"
#define DEFAULT_MIN 0

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
    // Структура, хранящая параметры сигналов устройства КСОН:
    // - Смещение байта, в котором хранится значение сигнала, от начала информацинного
    //                  блока информационного кадра,
    // - Смещение области битов, в которой хранится значение сигнала, от начала байта
    //                 (количество бит от начала байта),
    // - Размер области бит, которая хранит значение сигнала (количество бит),
    //                  в информационном блоке информационного кадра.
    // - Тип данных сигнала - задаётся значением (boolType, unsignedType, floatType)
    //                  из перечиcления "dataType".
    // - Максимальное значение сигнала
    // - Минимальное значение сигнала
    // - Номер группы, к которой принадлежит сигнал.
    {
        // Смещение байта, в котором хранится значение сигнала, от начала информацинного
        // блока информационного кадра:
        quint8 byte;

        // Смещение области битов, в которой хранится значение сигнала, от начала байта
        // (количество бит от начала байта):
        quint8 offset;

        // Размер области бит, которая хранит значение сигнала (количество бит),
        //                  в информационном блоке информационного кадра":
        quint8 len;

        // Тип данных сигнала - задаётся значением (boolType, unsignedType, floatType)
        //                  из перечиcления "dataType":
        quint8 data_type;

        // Максимальное значение сигнала:
        float max;

        // Минамальное значение сигнала:
        float min;

        // Номер группы, к которой принадлежит сигнал:
        quint8 group;


        static SignalParams fromJson(const QString& json_string) //throw (SvException)
        // Конструктор:

        // Вход: строка, являющаяся отрывком, взятым из конфигурационного файла "KSON.json"
        // имитатора устройства КСОН, и содержащем информацию об одном сигнале.

        // Выход: Заполненная параметрами сигнала устройства КСОН структура SignalParams.
        {
            QJsonParseError err;
            QJsonDocument jd = QJsonDocument::fromJson(json_string.toUtf8(), &err);

            if(err.error != QJsonParseError::NoError)
                throw SvException(QString("Имитатор КСОН: Ошибка при разборе параметров сигнала: ") + err.errorString());

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

        // Вход: объект QJsonObject, описывающий один сигнал устройства КСОН.

        // Выход: Заполненная параметрами сигнала устройства КСОН структура SignalParams.
        {
            SignalParams p;
            QString P;

            // Считываем значение параметра "смещение байта, в котором хранится значение сигнала
            // от начала информацинного блока информационного кадра":
            P = BYTE;
            if(object.contains(P))
            {
                if(object.value(P).toInt(-1) < 0)
                    throw SvException(QString("Имитатор КСОН: ") + QString(IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toInt())
                               .arg("Смещение байта, в котором хранится значение сигнала "
                                 "от начала информацинного блока должно быть задано целым числом"));

                p.byte = object.value(P).toInt();
            }
            else
                throw SvException(QString("Имитатор КСОН: ") + QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

            // Считываем значение параметра "смещение области битов, в которой хранится значение
            // сигнала, от начала байта":
            P = OFFSET;
            if(object.contains(P))
            {
                if(object.value(P).toInt(-1) < 0)
                    throw SvException(QString("Имитатор КСОН: ") + QString(IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toInt())
                               .arg("Смещение области битов, в которой хранится значение "
                                    "сигнала, от начала байта должно быть задано целым числом"));

                p.offset = object.value(P).toInt();
            }
            else
                throw SvException(QString("Имитатор КСОН: ") + QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

            // Считываем значение параметра "размер области битов":
            P = LEN;
            if(object.contains(P))
            {
                if(object.value(P).toInt(-1) < 0)
                    throw SvException(QString("Имитатор КСОН: ") + QString(IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toInt())
                               .arg("Размер области битов должен быть задан целым числом"));

                p.len = object.value(P).toInt();
            }
            else
                throw SvException(QString("Имитатор КСОН: ") + QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

            // Считываем значение параметра "Тип данных сигнала":
            P = DATA_TYPE;
            if(object.contains(P))
            {
                QString t = object.value(P).toString("");
                int cmp_boolean = t.compare ("boolean", Qt::CaseInsensitive);
                int cmp_unsigned = t.compare("unsigned", Qt::CaseInsensitive);
                int cmp_float = t.compare("float", Qt::CaseInsensitive);

                if (cmp_boolean != 0 && cmp_unsigned != 0 && cmp_float != 0)
                    throw SvException(QString("Имитатор КСОН: ") + QString(IMPERMISSIBLE_VALUE)
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

            // Считываем значение параметра "Максимальное значение сигнала":
            P = MAX;
            if(object.contains(P))
            {
                if(object.value(P).isDouble() == false)
                    throw SvException(QString("Имитатор КСОН: ") + QString(IMPERMISSIBLE_VALUE)
                                .arg(P)
                                .arg(object.value(P).toDouble())
                                .arg("Максимальное значение сигнала должено быть задано числом"));

                p.max = object.value(P).toDouble();
            }
            else
                p.max = DEFAULT_MAX;

            // Считываем значение параметра "Минимальное значение сигнала":
            P = MIN;
            if(object.contains(P))
            {
                if(object.value(P).isDouble() == false)
                    throw SvException(QString("Имитатор КСОН: ") + QString(IMPERMISSIBLE_VALUE)
                                 .arg(P)
                                 .arg(object.value(P).toDouble())
                                 .arg("Минимальное значение сигнала должено быть задано числом"));

                p.min = object.value(P).toDouble();
            }
            else
                p.min = DEFAULT_MIN;

            // Считываем значение параметра "Номер группы, к которой прнадлежит сигнал":
            P = GROUP;
            if(object.contains(P))
            {
                if(object.value(P).toInt(-1) < 0)
                    throw SvException(QString("Имитатор КСОН: ") + QString(IMPERMISSIBLE_VALUE)
                               .arg(P)
                               .arg(object.value(P).toInt())
                               .arg("Номер группы должен быть задан целым числом"));

                p.group = object.value(P).toInt();
            }
            else
                throw SvException(QString("Имитатор КСОН: ") + QString(MISSING_PARAM_DESC).arg(QString(QJsonDocument(object).toJson(QJsonDocument::Compact))).arg(P));

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
            j.insert(DATA_TYPE, QJsonValue(data_type));
            j.insert(MAX, QJsonValue(max).toDouble());
            j.insert(MIN, QJsonValue(min).toDouble());
            j.insert(GROUP, QJsonValue(group).toInt());

            return j;
        }
    };
}
#endif // SIGNAL_PARAMS_H

