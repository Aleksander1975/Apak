#ifndef SKM_H
#define SKM_H

#include "../skm_defs.h"
#include "../../../zn_abstract_outer_system.h"
#include "../../../recovery_defs.h"
#include "skm_global.h"

extern "C" {

    ZN_SKMSHARED_EXPORT zn1::AbstractOuterSystem* create();

}

namespace zn1 {

  const QMap<QString, int> BUF_SIZE_BY_MARKER = {{"SKM20386", 1152 }};

  class SKM: public zn1::AbstractOuterSystem
  {
    Q_OBJECT

  public:
    SKM();
    ~SKM() { }

    bool getPeriodValues(const QString& data_file, const QList<modus::SignalConfig>& signal_config_list, QMap<modus::SignalConfig, QList<InstantValue>>& result);
  };

}

#endif // SKM_H
