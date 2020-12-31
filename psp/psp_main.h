typedef void (*pPowerCB)(int powerflag);

#define POWER_CB_POWER		0x80000000
#define POWER_CB_HOLDON		0x40000000
#define POWER_CB_STANDBY	0x00080000
#define POWER_CB_RESCOMP	0x00040000
#define POWER_CB_RESUME		0x00020000
#define POWER_CB_SUSPEND	0x00010000
#define POWER_CB_EXT		0x00001000
#define POWER_CB_BATLOW		0x00000100
#define POWER_CB_BATTERY	0x00000080
#define POWER_CB_BATTPOWER	0x0000007F


int  PSP_Is(void);
int  PSP_IsEsc(void);
void PSP_GoHome(void);
void PSP_SetCallback(pPowerCB);


