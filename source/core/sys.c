#include <stdio.h>
#include <ogcsys.h>
#include "sys.h"
#include "wiipad.h"
#include "mload.h"
#include "mload_modules.h"
#include "wdvd.h"
#include "usbstorage.h"
#include "wbfs.h"
#include "disc.h"
#include "coverflow.h"
#include "sdhc.h"
#include "fat.h"
#include "restart.h"
#include "TrackedMemoryManager.h"

extern s32 wbfsDev;

/* Constants */
#define CERTS_LEN	0x280

extern s_self self;
extern s_gameSettings gameSetting;
extern s_path dynPath;
extern s_settings settings;

/* Variables */
static const char certs_fs[] ATTRIBUTE_ALIGN(32) = "/sys/cert.sys";

u8 shutdown = 0;
u8 reset    = 0;

void __Sys_ResetCallback(void)
{
	/* Reboot console */
	reset = 1;
}

void __Sys_PowerCallback(void)
{
	/* Poweroff console */
	shutdown = 1;
}


void Sys_Init(void)
{
	/* Initialize video subsytem */
	//VIDEO_Init();

	/* Set RESET/POWER button callback */
	SYS_SetResetCallback(__Sys_ResetCallback);
	SYS_SetPowerCallback(__Sys_PowerCallback);
}

static void _ExitApp() {
    //ExitGUIThreads();
    //StopGX();
    //ShutdownAudio();

    Fat_UnmountSDHC();
    Fat_UnmountUSB();
    mload_set_ES_ioctlv_vector(NULL);
    mload_close();
}

void Sys_Reboot(void)
{
	/* Restart console */
	_ExitApp();
	STM_RebootSystem();
}

void Sys_Shutdown(void)
{
	Wpad_Disconnect();
	/* Poweroff console */
	if(CONF_GetShutdownMode() == CONF_SHUTDOWN_IDLE) {
		s32 ret;

		/* Set LED mode */
		ret = CONF_GetIdleLedMode();
		if(ret >= 0 && ret <= 2)
			STM_SetLedMode(ret);

		/* Shutdown to idle */
		STM_ShutdownToIdle();
	} else {
		/* Shutdown to standby */
		STM_ShutdownToStandby();
	}
}

void Sys_LoadMenu(void)
{
	_ExitApp();
	/* Return to the Wii system menu */
	SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
}

s32 Sys_GetCerts(signed_blob **certs, u32 *len)
{
	static signed_blob certificates[CERTS_LEN] ATTRIBUTE_ALIGN(32);

	s32 fd, ret;

	/* Open certificates file */
	fd = IOS_Open(certs_fs, 1);
	if (fd < 0)
		return fd;

	/* Read certificates */
	ret = IOS_Read(fd, certificates, sizeof(certificates));

	/* Close file */
	IOS_Close(fd);

	/* Set values */
	if (ret > 0) {
		*certs = certificates;
		*len   = sizeof(certificates);
	}

	return ret;
}

int Sys_IosReload(int IOS) {
    s32 ret = -1;

    //shutdown SD and USB before IOS Reload in DiscWait
    Fat_UnmountSDHC();
    Fat_UnmountUSB();

    WPAD_Flush(0);
    WPAD_Disconnect(0);
    WPAD_Shutdown();

    WDVD_Close();

    USBStorage_Deinit();

    if (IOS == 249 || IOS == 222 || IOS == 223) {
        int i;
		for (i = 0; i < 10; i++) {
            ret = IOS_ReloadIOS(IOS);
            u32 timeout = 30;
            if (ret < 0) return ret;
            if (IOS == 222 || IOS == 223) load_ehc_module();
            ret = WBFS_Init(WBFS_DEVICE_USB,timeout);
            if (!(ret < 0)) break;
            sleep(1);
            USBStorage_Deinit();
        }
        if (ret>=0) {
            ret = Disc_Init();
            if (ret>=0) {
                ret = WBFS_Open();
            }
        } else Sys_BackToLoader();
    }

    PAD_Init();
    Wpad_Init();
    WPAD_SetDataFormat(WPAD_CHAN_ALL,WPAD_FMT_BTNS_ACC_IR);
//    WPAD_SetVRes(WPAD_CHAN_ALL, screenwidth, screenheight);
    //reinitialize SD and USB
    Fat_MountSDHC();
    Fat_MountUSB();

    return ret;
}

void Sys_BackToLoader(void) {
    if (*((u32*) 0x80001800)) {
        _ExitApp();
        exit(0);
    }
    // Channel Version
    Sys_LoadMenu();
}

// mload from uloader by Hermes
// uLoader 2.5:
#define size_ehcmodule2 20340
#define size_dip_plugin2 3304
//extern unsigned char ehcmodule2[size_ehcmodule2];
extern unsigned char dip_plugin2[size_dip_plugin2];

// uLoader 2.8D:
#define size_ehcmodule3 22264
#define size_dip_plugin3 3352
//extern unsigned char ehcmodule3[size_ehcmodule3];
extern unsigned char dip_plugin3[size_dip_plugin3];

// uLoader 3.0B:
//#define size_ehcmodule4 32384
//#define size_dip_plugin4 3512
// uLoader 3.0C:
//#define size_ehcmodule4 32432
// uLoader 3.1:
#define size_ehcmodule4 32400
//#define size_dip_plugin4 3080 // uloader 3.1
#define size_dip_plugin4 3224 // uloader 3.2
//extern unsigned char ehcmodule4[size_ehcmodule4];
extern unsigned char dip_plugin4[size_dip_plugin4];
// EHCFAT module:
//#define size_ehcmodule_frag 70529 // cfg50-52
#define size_ehcmodule_frag 70613 // cfg53
//#include "../ehcsize.h"
extern unsigned char ehcmodule_frag[size_ehcmodule_frag];
int mload_ehc_fat = 0;
int mload_need_fat = 0;

// current
void *ehcmodule;
int size_ehcmodule;

void *dip_plugin;
int size_dip_plugin;

// external2
char ehc_path[200];
void *external_ehcmodule2 = NULL;
int size_external_ehcmodule2 = 0;

// external3
char ehc_path3[200];
void *external_ehcmodule3 = NULL;
int size_external_ehcmodule3 = 0;

// external4
char ehc_path4[200];
void *external_ehcmodule4 = NULL;
int size_external_ehcmodule4 = 0;

// external_fat
char ehc_path_fat[200];
void *external_ehcmodule_frag = NULL;
int size_external_ehcmodule_frag = 0;

static u32 ios_36[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022DDAC, // dvd_read_controlling_data
	0x20201010+1, // handle_di_cmd_reentry (thumb)
	0x20200b9c+1, // ios_shared_alloc_aligned (thumb)
	0x20200b70+1, // ios_shared_free (thumb)
	0x20205dc0+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202b4c+1, // ios_doReadHashEncryptedState (thumb)
	0x20203934+1, // ios_printf (thumb)
};

static u32 ios_38[16] ATTRIBUTE_ALIGN(32)=
{
	0, // DI_EmulateCmd
	0,
	0x2022cdac, // dvd_read_controlling_data
	0x20200d38+1, // handle_di_cmd_reentry (thumb)
	0x202008c4+1, // ios_shared_alloc_aligned (thumb)
	0x20200898+1, // ios_shared_free (thumb)
	0x20205b80+1, // ios_memcpy (thumb)
	0x20200048+1, // ios_fatal_di_error (thumb)
	0x20202874+1, // ios_doReadHashEncryptedState (thumb)
	0x2020365c+1, // ios_printf (thumb)
};


u32 patch_datas[8] ATTRIBUTE_ALIGN(32);

data_elf my_data_elf;
int my_thread_id=0;

void load_ext_ehc_module(int verbose)
{
	if(!external_ehcmodule2)
	{
		snprintf(ehc_path, sizeof(ehc_path), "%s/ehcmodule.elf", dynPath.dir_usb_loader);
		size_external_ehcmodule2 = Fat_ReadFile(ehc_path, &external_ehcmodule2);
		if (size_external_ehcmodule2 < 0) {
			size_external_ehcmodule2 = 0;
		}
	}
	if(!external_ehcmodule3)
	{
		snprintf(ehc_path3, sizeof(ehc_path3), "%s/ehcmodule3.elf", dynPath.dir_usb_loader);
		size_external_ehcmodule3 = Fat_ReadFile(ehc_path3, &external_ehcmodule3);
		if (size_external_ehcmodule3 < 0) {
			size_external_ehcmodule3 = 0;
		}
	}
	if(!external_ehcmodule4)
	{
		snprintf(ehc_path4, sizeof(ehc_path4), "%s/ehcmodule4.elf", dynPath.dir_usb_loader);
		size_external_ehcmodule4 = Fat_ReadFile(ehc_path4, &external_ehcmodule4);
		if (size_external_ehcmodule4 < 0) {
			size_external_ehcmodule4 = 0;
		}
	}
	if(!external_ehcmodule_frag)
	{
		snprintf(ehc_path_fat, sizeof(ehc_path_fat), "%s/ehcmodule_frag.elf", dynPath.dir_usb_loader);
		size_external_ehcmodule_frag = Fat_ReadFile(ehc_path_fat, &external_ehcmodule_frag);
		if (size_external_ehcmodule_frag < 0) {
			size_external_ehcmodule_frag = 0;
		}
	}
}

static char mload_ver_str[40];

void mk_mload_version()
{
	mload_ver_str[0] = 0;
	if (self.ios_mload
			|| (is_ios_type(IOS_TYPE_WANIN) && IOS_GetRevision() >= 18) )
	{
		if (IOS_GetRevision() >= 4) {
			if (is_ios_type(IOS_TYPE_WANIN)) {
				sprintf(mload_ver_str, "Base: IOS%d ", wanin_mload_get_IOS_base());
			} else {
				sprintf(mload_ver_str, "Base: IOS%d ", mload_get_IOS_base());
			}
		}
		if (IOS_GetRevision() > 4) {
			int v, s;
			v = mload_get_version();
			s = v & 0x0F;
			v = v >> 4;
			sprintf(mload_ver_str + strlen(mload_ver_str), "mload v%d.%d ", v, s);
		} else {
			sprintf(mload_ver_str + strlen(mload_ver_str), "mload v%d ", IOS_GetRevision());
		}
	}
}

void print_mload_version()
{
	int new_wanin = is_ios_type(IOS_TYPE_WANIN) && IOS_GetRevision() >= 18;
	if (self.ios_mload || new_wanin) {
		printf("%s", mload_ver_str);
	}
}

int load_ehc_module_ex(int verbose)
{
	int is_ios=0;

	//extern int wbfs_part_fat;
	mload_ehc_fat = 0;
	if (mload_need_fat)
	{
		if (verbose) {
			if (strncasecmp(settings.partition, "ntfs", 4) == 0) {
				printf("[NTFS]");
			} else {
				printf("[FAT]");
			}
		}
		if (IOS_GetRevision() < 4) {
			printf("\nERROR: IOS%u rev%u\n", IOS_GetVersion(), IOS_GetRevision());
			printf("FAT mode only supported with ios 222 rev4");
			printf("\n");
			sleep(5);
			return -1;
		}
		mload_ehc_fat = 1;
		size_ehcmodule = size_ehcmodule_frag;
		ehcmodule = ehcmodule_frag;
		size_dip_plugin = size_dip_plugin4;
		dip_plugin = dip_plugin4;
		if(external_ehcmodule_frag) {
			printf("\n");
			printf("external: %s\n", ehc_path_fat);
			ehcmodule = external_ehcmodule_frag;
			size_ehcmodule = size_external_ehcmodule_frag;
		}
	}
	else if (IOS_GetRevision() <= 2)
	{
		size_dip_plugin = size_dip_plugin2;
		dip_plugin = dip_plugin2;
		//size_ehcmodule = size_ehcmodule2;
		//ehcmodule = ehcmodule2;
		if(external_ehcmodule2) {
			//if (verbose) {
				printf("\n");
				printf("external: %s\n", ehc_path);
			//}
			ehcmodule = external_ehcmodule2;
			size_ehcmodule = size_external_ehcmodule2;
		} else {
			printf("\n");
			printf("ERROR: ehcmodule2 no longer included!\n"
						"external file ehcmodule.elf must be used!");
			printf("\n");
			sleep(5);
			return -1;
		}

	} else if (IOS_GetRevision() == 3) {
			size_dip_plugin = size_dip_plugin3;
			dip_plugin = dip_plugin3;
			//size_ehcmodule = size_ehcmodule3;
			//ehcmodule = ehcmodule3;
			if(external_ehcmodule3) {
				//if (verbose) {
					printf("\n");
					printf("external: %s\n", ehc_path3);
				//}
				ehcmodule = external_ehcmodule3;
				size_ehcmodule = size_external_ehcmodule3;
			} else {
				printf("\n");
				printf("ERROR: ehcmodule3 no longer included!\n"
							"external file ehcmodule3.elf must be used!");
				printf("\n");
				sleep(5);
				return -1;
			}
	} else if (IOS_GetRevision() == 4) {
		size_dip_plugin = size_dip_plugin4;
		dip_plugin = dip_plugin4;
		//size_ehcmodule = size_ehcmodule4;
		//ehcmodule = ehcmodule4;
		// use ehcmodule_frag also for wbfs
		size_ehcmodule = size_ehcmodule_frag;
		ehcmodule = ehcmodule_frag;
		if(external_ehcmodule4) {
			//if (verbose) {
				printf("\n");
				printf("external: %s\n", ehc_path4);
			//}
			ehcmodule = external_ehcmodule4;
			size_ehcmodule = size_external_ehcmodule4;
		}
	} else if (IOS_GetRevision() >= 5) {
		size_dip_plugin = size_dip_plugin4;
		dip_plugin = dip_plugin4;
		size_ehcmodule = size_ehcmodule_frag;
		ehcmodule = ehcmodule_frag;
	/*
	} else {
		printf("\nERROR: IOS%u rev%u not supported\n",
				IOS_GetVersion(), IOS_GetRevision());
		sleep(5);
		return -1;
	*/
	}

	if (IOS_GetRevision() >= 4) {

		// NEW
		int ret = load_ehc_module();
		if (ret == 0) mk_mload_version();
		mload_close();
		return ret;
	}

	if (IOS_GetRevision() <= 2) {

		if (mload_module(ehcmodule, size_ehcmodule)<0) return -1;

	} else {

		if(mload_init()<0) return -1;
		mload_elf((void *) ehcmodule, &my_data_elf);
		my_thread_id= mload_run_thread(my_data_elf.start, my_data_elf.stack, my_data_elf.size_stack, my_data_elf.prio);
		if(my_thread_id<0) return -1;

	}

	usleep(350*1000);

	// Test for IOS

	mload_seek(0x20207c84, SEEK_SET);
	mload_read(patch_datas, 4);
	if(patch_datas[0]==0x6e657665)
		{
		is_ios=38;
		}
	else
		{
		is_ios=36;
		}

	if(is_ios==36)
		{
		// IOS 36
		memcpy(ios_36, dip_plugin, 4);		// copy the entry_point
		memcpy(dip_plugin, ios_36, 4*10);	// copy the adresses from the array

		mload_seek(0x1377E000, SEEK_SET);	// copy dip_plugin in the starlet
		mload_write(dip_plugin,size_dip_plugin);

		// enables DIP plugin
		mload_seek(0x20209040, SEEK_SET);
		mload_write(ios_36, 4);

		}
	if(is_ios==38)
		{
		// IOS 38

		memcpy(ios_38, dip_plugin, 4);	    // copy the entry_point
		memcpy(dip_plugin, ios_38, 4*10);   // copy the adresses from the array

		mload_seek(0x1377E000, SEEK_SET);	// copy dip_plugin in the starlet
		mload_write(dip_plugin,size_dip_plugin);

		// enables DIP plugin
		mload_seek(0x20208030, SEEK_SET);
		mload_write(ios_38, 4);

		}

	mk_mload_version();

	mload_close();

return 0;
}

// Reload custom ios

int ReloadIOS(int subsys, int verbose)
{
	int ret = -1;
	int sd_m = fat_sd_mount;
	int usb_m = fat_usb_mount;

	if (CURR_IOS_IDX == gameSetting.ios_idx
		&& is_ios_type(IOS_TYPE_WANIN)) return 0;

	if (verbose) {
		printf("IOS(%d) ", self.ios);
		if (self.ios_mload) printf("mload ");
 else if (self.ios_yal) printf("yal ");
	}

	mload_need_fat = 0;
	if (strncasecmp(settings.partition, "fat", 3) == 0) {
		mload_need_fat = 1;
	}
	if (strncasecmp(settings.partition, "ntfs", 4) == 0) {
		mload_need_fat = 1;
	}
	if (wbfsDev == WBFS_DEVICE_SDHC) {
		if (self.ios_mload)
		{
			// wbfs on sd with 222/223 requires fat module
			mload_need_fat = 1;
		}
	}

	if (CURR_IOS_IDX == gameSetting.ios_idx
		&& mload_need_fat == mload_ehc_fat)
	{
		if (verbose) {
			if (mload_ehc_fat) {
				if (strncasecmp(settings.partition, "ntfs", 4) == 0) {
					printf("[NTFS]");
				} else {
					printf("[FAT]");
				}
			}
			printf("\n\n");
		}
		return 0;
	}

	*mload_ver_str = 0;

	if (self.ios_mload) {
		load_ext_ehc_module(verbose);
	}

	if (verbose) printf(".");

	// Close storage
	Fat_UnmountAll();
	USBStorage_Deinit();
	SDHC_Close();

	// Close subsystems
	if (subsys) {
		Subsystem_Close();
		WDVD_Close();
	}

	// Load Custom IOS
	usleep(100000);
	ret = IOS_ReloadIOS(self.ios);
	usleep(300000);
	if (ret < 0) {
		//if (verbose) {
			printf("\n");
			printf("ERROR:");
			printf("\n");
			printf("Custom IOS %d could not be loaded! (ret = %d)", self.ios, ret);
			printf("\n");
		//}
		goto err;
	}
	if (verbose) {
		printf(".");
	}

	// mload ehc & dip
	if (self.ios_mload) {
		ret = load_ehc_module_ex(verbose);
		if (ret < 0) {
			//if (verbose) {
				printf("\n");
				printf("ERROR: Loading EHC module! (%d)", ret);
				printf("\n");
			//}
			goto err;
		}
	}

	if (is_ios_type(IOS_TYPE_WANIN) && IOS_GetRevision() >= 18) {
		load_dip_249();
	}

	if (verbose) {
		printf(".");
		if (self.ios_mload) {
			printf("\n");
			print_mload_version();
		}
	}

	// re-Initialize Storage
	if (sd_m) Fat_MountSDHC();
	if (usb_m) Fat_MountUSB();
	if (verbose) printf(".");

	// re-initialize subsystems
	if (subsys) {
		// wpad
		Wpad_Init();
		// storage
		if (wbfsDev == WBFS_DEVICE_USB) {
			USBStorage_Init();
		}
		if (wbfsDev == WBFS_DEVICE_SDHC) {
			SDHC_Init();
		}
		// init dip
		ret = Disc_Init();
		if (ret < 0) {
			//if (verbose) {
				printf("\n");
				printf("ERROR:");
				printf("\n");
				printf("Could not initialize DIP module! (ret = %d)", ret);
				printf("\n");
			//}
			goto err;
		}
	}
	if (verbose) printf(".\n");

	CURR_IOS_IDX = gameSetting.ios_idx;

	return 0;

err:
	if (subsys) {
		Wpad_Init();
		Restart_Wait();
	}
	return ret;
}

void Block_IOS_Reload()
{
	if (self.ios_mload) {
		if (gameSetting.iosreloadblock) {
			patch_datas[0]=*((u32 *) (dip_plugin+16*4));
			mload_set_ES_ioctlv_vector((void *) patch_datas[0]);
			printf("IOS Reload: Blocked");
			printf("\n");
			sleep(1);
		}
	}
}

u8 BCA_Data[64] ATTRIBUTE_ALIGN(32);
int have_bca_data = 0;

void load_bca_data(u8 *discid)
{
	if (self.disable_bca) return;
	char filename[100];
	int size = 0;
	void *buf = NULL;
	// ID6
	snprintf(D_S(filename), "%s/%.6s.bca", dynPath.dir_usb_loader, (char*)discid);
	size = Fat_ReadFile(filename, &buf);
	if (size <= 0) {
		// ID4
		snprintf(D_S(filename), "%s/%.4s.bca", dynPath.dir_usb_loader, (char*)discid);
		size = Fat_ReadFile(filename, &buf);
	}
	if (size < 64) {
		CFFree(buf);
		return;
	}
	printf("BCA: %s\n", filename);
	memcpy(BCA_Data, buf, 64);
	have_bca_data = 1;
	CFFree(buf);
}

void insert_bca_data()
{
	if (!have_bca_data) return;
	if (!self.ios_mload || IOS_GetRevision() < 4)
	{
		printf("ERROR: CIOS222/223 v4 required for BCA");
		printf("\n");
		sleep(2);
		return;
	}
	// offset 15 (bca_data area)
	mload_seek(*((u32 *) (dip_plugin+15*4)), SEEK_SET);
	mload_write(BCA_Data, 64);
	mload_close();
}


// WANINKOKO DIP PLUGIN

#if 0
void save_dip()
{
	//int dip_buf[0x5000/sizeof(int)];
	void *dip_buf = memalign(32,0x5000);
	int dip_size = 4000;
	printf("saving dip\n");
	if(mload_init()<0) {
		printf("mload init\n");
		sleep(3);
		return;
	}
	u32 base;
	int size;
	mload_get_load_base(&base, &size);
	printf("base: %08x %x\n", base, size);
	printf("mseek\n");
	mload_seek(0x13800000, SEEK_SET);
	memset(dip_buf, 0, sizeof(dip_buf));
	printf("mread\n");
	mload_read(dip_buf, dip_size);
	printf("fopen\n");
	FILE *f = fopen("sd:/dip.bin", "wb");
	if (!f) {
		printf("fopen\n");
		sleep(3);
		return;
	}
	printf("fwrite\n");
	fwrite(dip_buf, dip_size, 1, f);
	fclose(f);
	printf("dip saved\n");
	mload_close();
	sleep(3);
	printf("unmount\n");
	Fat_UnmountSDHC();
	printf("exit\n");
	exit(0);
}

void try_hello()
{
	int ret;
	printf("mload init\n");
	if(mload_init()<0) {
		sleep(3);
		return;
	}
	u32 base;
	int size;
   	mload_get_load_base(&base, &size);
	printf("base: %08x %x\n", base, size);
	mload_close();
	printf("disc init:\n");
	ret = Disc_Init();
	printf("= %d\n", ret);
	u32 x = 6;
	s32 WDVD_hello(u32 *status);
	ret = WDVD_hello(&x);
	printf("hello: %d %x %d\n", ret, x, x);
	ret = WDVD_hello(&x);
	printf("hello: %d %x %d\n", ret, x, x);
	sleep(1);
	//printf("exit\n");
	//exit(0);
}
#endif

#ifndef size_dip249
#define size_dip249 5276
#endif

extern u8 dip_plugin_249[size_dip249];


void load_dip_249()
{
	int ret;
	if (is_ios_type(IOS_TYPE_WANIN) && IOS_GetRevision() >= 18)
	{
		printf("[FRAG]");
		if(mload_init()<0) {
			printf(" ERROR\n");
			return;
		}
		/*
		u32 base;
		int size;
		mload_get_load_base(&base, &size);
		printf("base: %08x %x\n", base, size);
		*/
		ret = mload_module(dip_plugin_249, size_dip249);
		//printf("load mod: %d\n", ret);
		mk_mload_version();
		mload_close();
		//printf("OK\n");
	}
}


int get_ios_type()
{
	switch (IOS_GetVersion()) {
		case 249:
		case 250:
			return IOS_TYPE_WANIN;
		case 222:
		case 223:
			if (IOS_GetRevision() == 1)
				return IOS_TYPE_KWIIRK;
		case 224:
			return IOS_TYPE_HERMES;
	}
	return IOS_TYPE_UNK;
}

int is_ios_type(int type)
{
	return (get_ios_type() == type);
}



