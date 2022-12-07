#include "pu_apak.h"


apak::Sv_PU_APAK::Sv_PU_APAK():
  modus::SvAbstractProtocol(),
  m_state_signal(nullptr)
//Инициализация структур данных, необходимых для посылки РОST-запроса на
// сервер другого ПУ АПАК
{
    // Очистим список указателей на сигналы, подлежащих передаче на другой ПУ АПАК.
    m_signals.clear();

    // Очистим словарь, который каждому сигналу ставит в соответствие структуру SignalParams,
    // хранящую параметры для этого сигнала:
    m_params_by_signal.clear();
}


apak::Sv_PU_APAK::~Sv_PU_APAK()
{
  deleteLater();
}


bool apak::Sv_PU_APAK::configure(modus::DeviceConfig *config, modus::IOBuffer *iobuffer)
// Эта функция вызывается серевером "mdserver" для библиотек всех устройств.
// Её цель - инициализировать все структуры, необходимые нам для сигналов библиотеки
// конкретного устройства (в данном случае, библиотеки обмена между ПУ АПАК).
{
    try
    {
        p_config = config;
        p_io_buffer = iobuffer;

        // Заполняем структуру "m_params" параметром протокола обмена между ПУ АПАК
        // (периодом посылки запроса POST к серверу другого ПУ АПАК)
        m_params = apak::ProtocolParams::fromJson(p_config->protocol.params);

        return true;
    }
    catch (SvException& e)
    {
        // Отображаем оператору сообщение о месте ошибке. Сообщение о самой ошибке
        // хранится в исключении (e.error) и будет передано в "mdserver" через "p_last_error":
        emit message(QString("ПУ АПАК: Исключение в функции \"configure\""), sv::log::llError, sv::log::mtError);
        qDebug() << "ПУ АПАК: Исключение в функции \"configure\"";

        p_last_error = e.error;
        return false;
    }
}


bool apak::Sv_PU_APAK::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
// Эта функция вызывается сервером "mdserver" для всех сигналов, связанных с данным ПУ АПАК.
// Цель этой функции состоит в получении всех параметров для каждого сигнала и заполнении в соответствии
// с ними структур данных (перечисленных в файле "pu_apak.h"), необходимых для формирования
// POST-запроса к серверу другого ПУ АПАК.
{
  try {

    bool r = modus::SvAbstractProtocol::bindSignal(signal, binding);

    if(r)
    {
        if(binding.mode == modus::Master)
        { // Привязку "мастер" для ПУ АПАК имеет только один сигнал - это сигнал состояния
          // ПУ АПАК.

            if(signal->config()->type.toLower() == TYPE_STAT)
            {// Если тип сигнала - "STAT", то это сигнал о состоянии ПУ АПАК.
                // (0 - не работает/ 1 - работает).
                // Мы устанавливаем этот сигнал в "1" один раз в секунду в функции "send()"
                // перед посылкой запроса POST на другой АПАК ПУ.

                if(m_state_signal)
                { // Если указатель "m_state_signal" на "сигнал состояния" - ненулевой, то это означает,
                    // что функция "bindSignal" для "сигнала состояния" уже вызывалась. То есть,
                    // в файлах конфигурации сигналов для данного устройства ПУ АПАК "сигналов состояния"
                    // - НЕСКОЛЬКО, что является ошибкой.

                    p_last_error = TOO_MUCH(p_config->name, TYPE_STAT);                
                    return false;
                }

                // Присваиваем указателю "m_state_signal" указатель на сигнал состояния:
                m_state_signal = signal;

                // Так как сведения о состоянии этого ПУ АПАК подлежат передаче (в
                // составе запроса POST) на другой ПУ АПАК, то получим и занесём в структуру
                // "signalParams" - параметры "сигнала состояния", в список "m_signals" -
                // указатель на "сигнал состояния" и в словарь "m_params_by_signal" занесём
                // строку, которая указателю на "сигнал состояния" ставит в соответствие
                // cтруктуру "signalParams" с его параметрами:

                // Заполняем структуру "SignalParams" параметром сигналов, отсылаемых в запросе
                // POST на другой АПАК ПУ: этим параметром является тип данных сигнала (параметр "data_type").

                // Параметры сигналов описываются в подразделе "params", следующем за номером устройства,
                // к которому они привязаны.
                apak::SignalParams signal_params = apak::SignalParams::fromJson(binding.params);

                // Заносим указатель на сигнал в список "m_signals":
                m_signals.append(signal);

                // Заносим в словарь (который каждому сигналу ставит в соответствие структуру SignalParams,
                // хранящую параметры для этого сигнала) указатель на сигнал и структуру SignalParams:
                m_params_by_signal.insert (signal, signal_params);
            } // Если тип сигнала - "STAT"
      } // if(binding.mode == modus::Master)
      else
      { // Сигналы, привязанные к ПУ АПАК, которые имеют привязку "bindings".
            // Заполняем структуру "SignalParams" параметром сигналов, отсылаемых в запросе
            // POST на другой АПАК ПУ: этим параметром является тип данных сигнала (параметр "data_type").

            // Параметры сигналов описываются в подразделе "params", следующем за номером устройства,
            // к которому они привязаны.
            apak::SignalParams signal_params = apak::SignalParams::fromJson(binding.params);

            // Заносим указатель на сигнал в список "m_signals":
            m_signals.append(signal);

            // Заносим в словарь (который каждому сигналу ставит в соответствие структуру SignalParams,
            // хранящую параметры для этого сигнала) указатель на сигнал и структуру SignalParams:
            m_params_by_signal.insert (signal, signal_params);
      }
    } // if(r)

    return r;
  }

  catch(SvException& e) {
    p_last_error = e.error;

    // Отображаем оператору сообщение о месте ошибке. Сообщение о самой ошибке
    // хранится в исключении (e.error) и будет передано в "mdserver" через "p_last_error":
    emit message(QString("ПУ АПАК: Исключение в функции \"bindSignal\""), sv::log::llError, sv::log::mtError);
    qDebug() << "ПУ АПАК: Исключение в функции \"bindSignal\"";

    return false;
  }
}

void apak::Sv_PU_APAK::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::Sv_PU_APAK::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::Sv_PU_APAK::start(void)
// В этой функции мы
// 1. Привязываем к наступлению таймаута таймера посылки "m_sendTimer"
// вызова функции "sendResponse_POST". В функции "sendResponse_POST" мы устанавливаем в "1"
// сигнал состояния ПУ АПАК и выполняем формирование и передачу
// интерфейсной части запроса POST к другому ПУ АПАК.
// 2. Привязываем вызов функции "getReply_POST", к сигналу "modus::IOBuffer::dataReaded".
// Этот сигнал испускается интерфейсной частью по приходу данных от интерефейса
// (в нашем случае - от сервера другого ПУ АПАК).
// В функции "getReply_POST" мы выводим ответ сервера на посланный к нему POST-запрос.
{
    // Привязываем вызов функции "sendResponse_POST" к наступлению таймаута
    // таймера посылки "m_sendTimer":
    m_sendTimer = new QTimer;
    connect(m_sendTimer, &QTimer::timeout, this, &Sv_PU_APAK::sendResponse_POST);

    // Запускаем таймер посылки "m_sendTimer" с периодом, равным интервалу посылки
    // запроса POST на сервер доугого ПУ АПАК, заданный в конфигурационном файле
    // "config_apak.json", как параметр протокола ПУ АПАК:
    m_sendTimer->start(m_params.send_interval);

    // Привязываем вызов функции "getReply_POST", к сигналу "modus::IOBuffer::dataReaded".
    // Этот сигнал испускается интерфейсной частью по приходу данных от интерефейса
    // (в нашем случае - от сервера другого ПУ АПАК).
    connect(p_io_buffer, &modus::IOBuffer::dataReaded, this, &Sv_PU_APAK::getReply_POST);

    p_is_active = bool(p_config) && bool(p_io_buffer);
}

void apak::Sv_PU_APAK::sendResponse_POST(void)
// В этой функции мы:
// 1. Устанавливаем в "1" сигнал состояния ПУ АПАК
// 2. Выполняем формирование и передачу интерфейсной части запроса POST к другому ПУ АПАК.
{
    qDebug() << "Вызов send()";

    if (p_is_active == false)
        return;

    // 1. Устанавливаем в "1" сигнал состояния ПУ АПАК:
    if(m_state_signal)
    {// Если в файле конфигурации сигналов ПУ АПАК имеется "сигнал состояния",
     // перед каждой посылкой запроса POST к другому ПУ АПАК устанавливаем его значение в 1:

       m_state_signal->setValue(int(1));
    }

    // 2. Формируем запрос POST к другому ПУ АПАК.
    // Проходим по списку всех сигналов, значения которых подлежат передаче на другой ПУ АПАК.
    // Для каждого сигнала формируем JSON-объект "signal_JSON", содержащий
    // информацию о идентификаторе сигнала и его значении.
    // После этого - добавляем сформированный JSON-объект к JSON-массиву "signalsArray_JSON":
    QJsonArray signalsArray_JSON;

    foreach ( modus::SvSignal* signal, m_signals)
    {
        // Для каждого сигнала в переменной "signal_JSON" формируем объект JSON:
        //{"id":ХХХХХХХ, "value":Y},
        // где ХХХХХХХ - идентификатор сигнала из переменной: "identOfSignal", а
        // Y - значение сигнала, формируемое в переменной: "valueOfSignal".
        QJsonObject signal_JSON;
        QJsonValue valueOfSignal;

        // 2.1. Получим идентификатор сигнала:
        int identOfSignal = signal ->config()->id;

        // 2.2. Получим параметры сигнала:
        apak::SignalParams signal_params = m_params_by_signal.value(signal);

        // 2.3. Выясним тип сигнала, и в случае, если он отличен от "boolean",
        // выведем сообщение об ошибке и переходим к следующему сигналу:
        if (signal_params.data_type != boolType)
        {
            // Получаем имя сигнала для отображения в сообщении об ошибке:
            QString signalName = signal ->config() ->name;

            // Отображаем оператору сообщение об ошибке:
            emit message(QString("ПУ АПАК: Сигнал %1 имеет тип, отличный от булевского: %2").arg(signalName).arg(signal_params.data_type), sv::log::llError, sv::log::mtError);
            qDebug() << QString("ПУ АПАК: Сигнал %1 имеет тип, отличный от булевского: %2").arg(signalName).arg(signal_params.data_type);

            // Этот сигнал в запрос POST - не помещаем -> переходим к следующему сигналу:
            continue;
        }

        // 2.4. Получим значение сигнала:
        // Проверим актуальна ли информация, содержащаяся в сигнале:
        if(!signal->value().isValid() || signal->value().isNull())
        {
            // Информация в сигнале не актуальна -> устанавливаем значение Null.
            valueOfSignal = QJsonValue();
        }
        else
        {
            valueOfSignal = QJsonValue(signal ->value().toBool());
        }

        // 2.5. Формируем объект "signal_JSON":
        // 2.5.1. Вставляем в пустой JSON-объект пару: "id" и JSON-значение
        //        переменной "identOfSignal":
        signal_JSON.insert(QString("id"), QJsonValue(identOfSignal));

        // 2.5.2. Вставляем пару: "value" и значение переменной "valueOfSignal":
        signal_JSON.insert(QString("value"), valueOfSignal);

        // Добавляем сформированный JSON-объект "signal_JSON" к
        // JSON-массиву "signalArray_JSON":
        signalsArray_JSON.append(QJsonValue(signal_JSON));
    }

    // 2.6. Формируем из JSON-массива "signalsArray_JSON" JSON-объект "signalsObject_JSON" вида:
    // {"signals": JSON-массиву "signalArray_JSON"}
    QJsonObject signalsObject_JSON;
    signalsObject_JSON.insert(QString ("signals"), QJsonValue (signalsArray_JSON));

    // 2.7. Cформируем из JSON-объекта "signalsObject_JSON" JSON-документ
    // "signalsDocument_JSON", чтобы его можно было перевести в массив байт в кодировке UTF-8:
    QJsonDocument signalsDocument_JSON (signalsObject_JSON);

    // 2.8. Переведём JSON-документ "signalsDocument_JSON" в массив байт
    // "signalsByteArray_JSON" в кодировке UTF-8:
    QByteArray signalsByteArray_JSON = signalsDocument_JSON.toJson(QJsonDocument::Compact);

    // 2.9. Сформируем POST-запрос вида:
    // POST /set_signal_values HTTP/1.1
    // Content-Length: ХХ
    // (пустая строка)
    // Массив байт "signalsByteArray_JSON",
    // где ХХ - это длина массива байт "signalsByteArray_JSON".

    QByteArray POST_request;
    POST_request.append(QString("POST /set_signal_values HTTP/1.1\r\n") +
                        QString("Content-Length: %1\r\n\r\n").arg(signalsByteArray_JSON.length()));
    POST_request.append(signalsByteArray_JSON);

    // 3. Передаём данные от протокольной к интерфейcной части (для передачи по линии связи):
    transferToInterface (POST_request);
    qDebug() << "POST-запрос: " << POST_request;
}


void apak::Sv_PU_APAK::getReply_POST(modus::BUFF* buffer)
// Эта функция выводит на консоль и в утилиту "logview" ответ сервера другого ПУ АПАК
// на посланный нами POST-запрос. Это делается для целей отладки и контроля работы.
{
    //qDebug() << "getRequest_POST";

    if(p_is_active == false)
        return;

    if(!buffer->isReady())
    { // Если принятых данных нет, то не в чем и разбираться (выходим из функции):
        return;
    }

    // Блокируем  мьютекс, чтобы интерфейсная часть не имела доступа к входному буферу
    // пока мы не разберёмся с тем, что находится там сейчас.
    // Разблокировка мьютикса "buffer -> mutex" будет происходить автоматически ПРИ ВЫХОДЕ
    // из этой функции. Таким образом мы не допустим ситуации повторного вызова
    // этой функции до тех пор, пока она не закончит выполняться первый раз.
    QMutexLocker(&(buffer->mutex));

    // Скопируем пришедший от сервера ответ в массив "POST_reply":
    QByteArray POST_reply = QByteArray(buffer->data, buffer->offset);

    // Выведем ответ сервера в утилиту "logview" и на консоль:
    emit message(POST_reply, sv::log::llInfo, sv::log::mtInfo);
    qDebug() << QString(POST_reply);

    buffer->reset();

    return;
}



void apak::Sv_PU_APAK::transferToInterface (QByteArray data)
// Функция передаёт данные от протокольной к интерфейcной части (для передачи по линии связи).
// Аргумент: "data" - массив байт для передачи
{
    p_io_buffer->output->mutex.lock();

    if (p_io_buffer->output->isReady())
    {   // Если нам надо записать в буфер сообщение (информационный кадр или пакет подтверждения),
        // а интерфейсная часть ещё не прочла предыдущее сообщение (флаг "is_ready" - установлен),
        // то вместо функции "setData", мы используем фунцкцию "append", чтобы не "затирать"
        // предыдущее сообщение, а дополнить его. Но предварительно проверяем, есть ли в буфере
        // место на новые данные:

        // Если в буфере нет места на новые данные, то очищаем его содержимое и
        // сбрасываем флаг "is_ready":
        if(p_io_buffer->output->offset + data.length() > p_config->bufsize)
            p_io_buffer->output->reset();

        // Дополняем буфер новыми данными:
        p_io_buffer ->output->append(data);
    }
    else
    {   // Записываем в буфер сообщение и устанавливаем флаг готовности буфера "is_ready":
        p_io_buffer->output->setData(data);
    }

    p_io_buffer->output->setReady(true);

    // Испускаем для интерфейсной части сигнал "readyWrite", по получению которого она
    // должна начать передачу данных из буфера в интерфейс:
    emit p_io_buffer->readyWrite(p_io_buffer->output);

    p_io_buffer->output->mutex.unlock();
}


/** ********** EXPORT ************ **/
modus::SvAbstractProtocol* create()
{
  modus::SvAbstractProtocol* protocol = new apak::Sv_PU_APAK();
  return protocol;
}

