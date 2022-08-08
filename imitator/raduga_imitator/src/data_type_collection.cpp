#include "data_type_collection.h"

raduga::DataTypeCollection::DataTypeCollection():
  SvAbstractSignalCollection()
{

}

void raduga::DataTypeCollection::addSignal(modus::SvSignal* signal, modus::SignalBinding binding, quint16 bufsize)
{
  try
  {

    raduga::SignalParams p = raduga::SignalParams::fromJson(binding.params);

    if(p.byte >= bufsize)
      throw SvException(QString("Номер байта %1 выходит за границы буфера (%2)").arg(p.byte).arg(bufsize));

    m_signals.append(raduga::SignalStruct(signal, p));

  }
  catch(SvException& e)
  {
    throw SvException(QString("Сигнал %1: %2").arg(signal->config()->name, e.error));
  }
}

void raduga::DataTypeCollection::updateSignals(const raduga::DATA* data)
{
  Q_UNUSED(data);
//    foreach (raduga::SignalStruct st, m_signals) {

//      if(st.params.byte < data->len)
//        st.signal->setValue(int((data->data[st.params.byte] >> st.params.offset) & 1));

//    }
}

void raduga::DataTypeCollection::updateOutput(const modus::BUFF* data)
{
  if(!data)
    return;

  bool ok;

  foreach (raduga::SignalStruct s, m_signals) {

    switch (s.params.tip) {

      case raduga::TIP::Discrete:
      case raduga::TIP::Ustavka:
      {
        quint8 v = quint8(data->data[s.params.byte]) & ~(((1 << s.params.len) - 1) << s.params.offset);
//        quint8 v = quint8(data->data[data->offset + s.params.byte]) & ~(1 << s.params.offset);
  //      v &= ~(((1 << s.params.len) - 1) << s.params.offset); // если больше 1го бита

        if(s.signal->value().isValid() && !s.signal->value().isNull()) {

          quint8 signal_value = s.signal->value().toUInt(&ok);

          if(ok) // && signal_value == 1)
            v |= (signal_value << s.params.offset);

        }

//        data->data[data->offset + s.params.byte] = v;
        data->data[s.params.byte] = v;

//        if(s.params.byte == 236)
//          qDebug() << int(data->data[offset + s.params.byte]) << v << offset << s.params.byte;

        break;
      }

      case raduga::TIP::Short:
      {
        qint16 signal_value = 0;

        if(s.signal->value().isValid() && !s.signal->value().isNull())
          signal_value = s.signal->value().toInt();

        memcpy(&data->data[data->offset + s.params.byte], &signal_value, sizeof(qint16));

        break;
      }

      case raduga::TIP::Long:
      {
        qint32 signal_value = 0;

        if(s.signal->value().isValid() && !s.signal->value().isNull())
          signal_value = s.signal->value().toInt();

        memcpy(&data->data[data->offset + s.params.byte], &signal_value, sizeof(qint32));

        break;
      }

      case raduga::TIP::Analog:
      case raduga::TIP::Float:
      {
        float signal_value = 0;

        if(s.signal->value().isValid() && !s.signal->value().isNull())
          signal_value = s.signal->value().toFloat();

        memcpy(&data->data[data->offset + s.params.byte], &signal_value, sizeof(float));

        break;
      }

      default:
        break;
    }
  }
}
