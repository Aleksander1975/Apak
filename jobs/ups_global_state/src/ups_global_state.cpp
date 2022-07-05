#include "ups_global_state.h"


apak::UPSGlobalState::UPSGlobalState():
  m_global_state_signal(nullptr)
{
}


bool apak::UPSGlobalState::configure(modus::JobConfig* config)
{
  try {

    p_config = config;

//    m_params = moxa::SignalParams::fromJsonString(p_config->params);


  }
  catch(SvException& e) {

    p_last_error = e.error;
    return false;
  }

  return true;
}

bool apak::UPSGlobalState::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
{
  try {

    bool r = modus::SvAbstractJob::bindSignal(signal, binding);

    apak::ups::SignalParams params = apak::ups::SignalParams::fromJsonString(binding.params);

    if(r) {

      if(binding.mode == modus::Master) {

        if(signal->config()->tag == TAG_MOXA_GLOBAL_STATE) {

          if(!m_global_state_signal)
            m_global_state_signal = signal;

          else
            emit message(QString("К модулю \"%1\" может быть привязан только один сигнал с тегом \"%2\"")
                         .arg(p_config->name).arg(TAG_MOXA_GLOBAL_STATE), llerr, mterr);
        }

      }
      else {

        if(!m_params_by_signals.contains(signal))
          m_params_by_signals.insert(signal, params);

      }
    }

    return r;
  }

  catch(SvException& e) {
    p_last_error = e.error;
    return false;
  }
}

bool apak::UPSGlobalState::signalIsSupported(modus::SvSignal* signal)
{
  return apak::ups::SupportedSignalTags.contains(signal->config()->tag.toUpper());
}

void apak::UPSGlobalState::start()
{
  if(!m_global_state_signal) {

    emit message(QString("Запуск модуля \"%1\" невозможен, так как не определен сигнал с тегом \"%2\"")
                 .arg(p_config->name).arg(TAG_MOXA_GLOBAL_STATE));
    return;

  }

  QTimer* m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &UPSGlobalState::updateGlobalState);
  m_timer->start(m_params.interval);

}

void apak::UPSGlobalState::updateGlobalState()
{
  quint16 global_state = 0;
  foreach (modus::SvSignal* signal, m_params_by_signals.keys()) {

    if(m_params_by_signals.value(signal).watch) {

      bool ok;
      quint16 state = signal->value().toUInt(&ok);

      if(!ok || apak::ups::badStates.contains(state))
        global_state |= 0x0002;

      else if (state == STATE_LINK_UP)
        global_state |= 0x0001;

    }
  }

  if(m_global_state_signal)
    m_global_state_signal->setValue(QVariant(global_state));

}

void apak::UPSGlobalState::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::UPSGlobalState::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}


/** ********** EXPORT ************ **/
modus::SvAbstractJob* create()
{
  modus::SvAbstractJob* job = new apak::UPSGlobalState();
  return job;
}
