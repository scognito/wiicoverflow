#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <ogcsys.h>
#include <fat.h>
#include <sys/stat.h>

/* Constants */
//#define SDHC_MOUNT	"SD"
//#define CACHE 4
//#define SECTORS 64

/* Disc interfaces */
extern const DISC_INTERFACE __io_sdhc;


//extern bool fatMountSimple (const char* name, const DISC_INTERFACE* interface);

extern void fatUnmount (const char* name);

/*
s32 Fat_MountSDHC(void)
{
	s32 ret;

	// Initialize SDHC interface 
	ret = __io_sdhc.startup();
	if (!ret)
		return -1;

	// Mount device 
	ret = fatMount(SDHC_MOUNT, &__io_sdhc, 0, CACHE, SECTORS);
	if (!ret)
		return -2;

	return 0;
}
*/


s32 Fat_ReadFile(const char *filepath, void **outbuf)
{
	FILE *fp     = NULL;
	void *buffer = NULL;

	struct stat filestat;
	u32         filelen;

	s32 ret;

	/* Get filestats */
	stat(filepath, &filestat);

	/* Get filesize */
	filelen = filestat.st_size;

	/* Allocate memory */
	buffer = memalign(32, filelen);
	if (!buffer)
		goto err;

	/* Open file */
	fp = fopen(filepath, "rb");
	if (!fp)
		goto err;

	/* Read file */
	ret = fread(buffer, 1, filelen, fp);	
	if (ret != filelen)
		goto err;

	/* Set pointer */
	*outbuf = buffer;

	goto out;

err:
	/* Free memory */
	if (buffer)
		free(buffer);

	/* Error code */
	ret = -1;

out:
	/* Close file */
	if (fp)
		fclose(fp);

	return ret;
}

s32 Fat_ReadFileToBuffer(const char *filepath, void *outbuf, int maxsize)
{
	FILE *fp     = NULL;

	struct stat filestat;
	u32         filelen;

	s32 ret;

	/* Get filestats */
	stat(filepath, &filestat);

	/* Get filesize */
	filelen = filestat.st_size;

	if (filelen>maxsize) goto err;
	

	/* Open file */
	fp = fopen(filepath, "rb");
	if (!fp)
		goto err;

	/* Read file */
	ret = fread(outbuf, 1, filelen, fp);	
	if (ret != filelen)
		goto err;

	goto out;

err:

	/* Error code */
	ret = -1;

out:
	/* Close file */
	if (fp)
		fclose(fp);

	return ret;
}

/*
s32 Fat_UnmountSDHC(void)
{
	s32 ret;

	//  Unmount device 
	fatUnmount(SDHC_MOUNT);
	
	// Close SDHC interface 
	ret = __io_sdhc.shutdown();
	if (!ret)
		return -1;

	return 0;
}
*/

