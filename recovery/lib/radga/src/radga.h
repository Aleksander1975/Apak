#ifndef RADGA_H
#define RADGA_H

#include "../raduga_defs.h"
#include "../../../../zn_abstract_outer_system.h"
#include "../../../../recovery_defs.h"
#include "radga_global.h"

extern "C" {

    ZN_RADGASHARED_EXPORT zn1::AbstractOuterSystem* create();

}

namespace zn1 {

  const QMap<QString, int> BUF_SIZE_BY_MARKER = {{"RADGA101", 1152 },
                                                 {"RADGA102", 768  },
                                                 {"RADGA103", 1284 },
                                                 {"RADGA104", 700  },
                                                 {"RADGA105", 1152 },
                                                 {"RADGA106", 1152 },
                                                 {"RADGA107", 768  },
                                                 {"RADGA108", 1284 },
                                                 {"RADGA109", 700  },
                                                 {"RADGA110", 1152 },
                                                 {"RADGA111", 40   },
                                                 {"RADGA112", 105  },
                                                 {"RADGA113", 122  },
                                                 {"RADGA114", 30   },
                                                 {"RADGA115", 336  },
                                                 {"RADGA116", 37   },
                                                 {"RADGA117", 37   },
                                                 {"RADGA118", 49   },
                                                 {"RADGA119", 49   },
                                                 {"RADGA120", 240  }};

  class RADGA: public zn1::AbstractOuterSystem
  {
    Q_OBJECT

  public:
    RADGA();
    ~RADGA() { }

    bool getPeriodValues(const QString& data_file, const QList<modus::SignalConfig>& signal_config_list, QMap<modus::SignalConfig, QList<InstantValue>>& result);
  };

}

#endif // RADGA_H
