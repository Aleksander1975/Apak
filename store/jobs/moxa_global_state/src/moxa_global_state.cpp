#include "moxa_global_state.h"


apak::MoxaGlobalState::MoxaGlobalState():
  m_global_state_signal(nullptr)
{
}


bool apak::MoxaGlobalState::configure(modus::JobConfig* config)
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

bool apak::MoxaGlobalState::bindSignal(modus::SvSignal* signal, modus::SignalBinding binding)
{
  try {

    bool r = modus::SvAbstractJob::bindSignal(signal, binding);

    apak::moxa::SignalParams params = apak::moxa::SignalParams::fromJsonString(binding.params);

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

bool apak::MoxaGlobalState::signalIsSupported(modus::SvSignal* signal)
{
  return apak::moxa::SupportedSignalTags.contains(signal->config()->tag.toUpper());
}

void apak::MoxaGlobalState::start()
{
  if(!m_global_state_signal) {

    emit message(QString("Запуск модуля \"%1\" невозможен, так как не определен сигнал с тегом \"%2\"")
                 .arg(p_config->name).arg(TAG_MOXA_GLOBAL_STATE));
    return;

  }

  QTimer* m_timer = new QTimer;
  connect(m_timer, &QTimer::timeout, this, &MoxaGlobalState::updateGlobalState);
  m_timer->start(m_params.interval);

}

void apak::MoxaGlobalState::updateGlobalState()
{
  quint16 global_state = 0;
  foreach (modus::SvSignal* signal, m_params_by_signals.keys()) {

    if(m_params_by_signals.value(signal).watch) {

//      bool ok;
      quint16 state = signal->value().toUInt(&ok);

      if(/*!ok || */apak::moxa::badStates.contains(state))
        global_state |= 0x0002;

      else if (state == STATE_LINK_UP)
        global_state |= 0x0001;

    }
  }

  if(m_global_state_signal)
    m_global_state_signal->setValue(QVariant(global_state));

}

void apak::MoxaGlobalState::signalUpdated(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}

void apak::MoxaGlobalState::signalChanged(modus::SvSignal* signal)
{
  Q_UNUSED(signal);
}


/** ********** EXPORT ************ **/
modus::SvAbstractJob* create()
{
  modus::SvAbstractJob* job = new apak::MoxaGlobalState();
  return job;
}
