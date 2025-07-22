#include "extdll.h"
#include "util.h"
#include "cbase.h"

class CTriggerPrint : public CBaseEntity
{
public:
	void Spawn(void) override;
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value) override;
};

LINK_ENTITY_TO_CLASS(trigger_print, CTriggerPrint);

void CTriggerPrint::Spawn(void)
{
	ALERT(at_console, "AKU ENTITY TRIGGERPRINT\n\n");
}

void CTriggerPrint::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	ALERT(at_console, "AKU ENTITY TRIGGERPRINT %s\n\n", STRING( pev->message) );
}
