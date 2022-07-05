#include "data_type_1.h"

raduga::DataType1::DataType1():
  SvAbstractSignalCollection()
{

}

void raduga::DataType1::addSignal(modus::SvSignal* signal)
{
  try
  {
    raduga::SignalParams p = raduga::SignalParams::fromJson(signal->config()->params);

    m_signals.append(raduga::SignalType1(signal, p));

  }
  catch(SvException& e)
  {
    throw SvException(QString("Сигнал %1: %2").arg(signal->config()->name, e.error));
  }
}

void raduga::DataType1::updateSignals(const raduga::DATA* data)
{
  if(!data)
    return;

  foreach (raduga::SignalType1 st, m_signals) {

    if(st.params.byte < data->len)
      st.signal->setValue(int((data->data[st.params.byte] >> st.params.bit) & 1));

  }
}
