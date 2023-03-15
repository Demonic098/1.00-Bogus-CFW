#include <string.h>

#include <psptypes.h>
#include <pspmoduleinfo.h>
#include <pspmodulemgr.h>
#include <pspkernel.h>

#include "../include/macros.h"
#include "../include/systemctrl.h"

PSP_MODULE_INFO("vshex", 0, 1, 0);

char curParamKey[100];

STMOD_HANDLER previous = NULL;

int (*_getParamSfoKeyData)(u8 * buf, int a1, u8 *paramData);

int getParamSfoKeyDataPatched(u8 * buf, int a1, u8 *paramData)
{
    int res = _getParamSfoKeyData(buf, a1, paramData);

    if (res < 0)
    {
        if (strcmp(curParamKey, "DRIVER_PATH") == 0)  {
            return 0;
        }
        else if(strcmp(curParamKey, "DISC_ID") == 0) {
            strcpy((char *)paramData, "UCJS10041");
            return 0;
        }
        else if(strcmp(curParamKey, "DISC_VERSION") == 0 || strcmp(curParamKey, "PSP_SYSTEM_VER") == 0) {
            strcpy((char *)paramData, "1.00");
            return 0;
        }
        else if(strcmp(curParamKey, "PARENTAL_LEVEL") == 0) {
            paramData[0] = 1;
            return 0;
        }
    }

    return res;
}

int (*_getParamSfoKeyInfo)(u8 * buf, char *key, int* a2);

int getParamSfoKeyInfoPatched(u8 *buf, char *key, int* a2)
{
    strcpy(curParamKey, key);

    return _getParamSfoKeyInfo(buf, key, a2);
}

void clearCaches(void)
{
    sceKernelDcacheWritebackAll();
    sceKernelIcacheInvalidateAll();
}

int OnModuleStart(SceModule *mod)
{
	if (strcmp(mod->modname, "game_plugin_module") == 0)
	{
        HIJACK_FUNCTION(mod->text_addr + 0xa614, getParamSfoKeyDataPatched, _getParamSfoKeyData);
        HIJACK_FUNCTION(mod->text_addr + 0xd270, getParamSfoKeyInfoPatched, _getParamSfoKeyInfo);

        clearCaches();
	}

	if (!previous)
		return 0;

	return previous(mod);
}

 int module_start(SceSize args, void *argp)
{
	previous = sctrlHENSetStartModuleHandler(OnModuleStart);

	return 0;
}
