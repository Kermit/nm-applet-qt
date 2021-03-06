// This file is generated by kconfig_compiler from cdma.kcfg.
// All changes you do to this file will be lost.
#ifndef KNM_CDMAPERSISTENCE_H
#define KNM_CDMAPERSISTENCE_H

#include <kdebug.h>
#include "settingpersistence.h"
#include "internals/settings/cdma.h"

namespace Knm {

class CdmaSetting;

class CdmaPersistence : public SettingPersistence
{
  public:
    CdmaPersistence( CdmaSetting * setting, KSharedConfig::Ptr config, SettingPersistence::SecretStorageMode mode = SettingPersistence::Secure);
    ~CdmaPersistence();
    void load();
};
}

#endif

