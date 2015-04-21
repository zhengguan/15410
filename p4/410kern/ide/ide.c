/** @file ide.c
 *  @brief PIO driver for IDE controllers
 *
 *  "Well, you can implement it using the [disk drive] if you want, but you
 *  are allowed to use the ramdisk too..." (or something similar)
 *         -- de0u, 20071128
 *
 *  So I did.
 *
 *  The IDE interface doesn't actually expose a drive in Pebbles -- it
 *  exposes a linear partition that one can read sectors from.  That way,
 *  only ide.c has to know about partition tables -- everyone else just sees
 *  a big block of 512-byte sectors allocated to Pebbles (or nothing at
 *  all).
 *
 *  @author Joshua Wise (jwise) <joshua@joshuawise.com>
 *  @bug Only supports drives that support LBA.
 *  @bug Only supports 28-bit LBA.
 *  @bug Arguably, the partition table finder shouldn't be in here.
 */

//#define IDE_INLINE_ASM

#include <simics.h>
#include <x86/asm.h>
#include <x86/pic.h>
#include <string.h>
#include <ide.h>
#include <ide-config.h>
#include <stdint.h>

static int ide_drive0_present = 0, ide_pebpartition_present = 0;
static unsigned long ide_drive0_sectors;
static unsigned long ide_pebpartition_start;
static unsigned long ide_pebpartition_size;
static int ide_lba48_enabled = 0;

static int _reset(void);
static int _identify(void);
static void _find_pebbles(void);
static int _wait_busy(void);
static int _wait_drq(void);
static int _read_sector(unsigned short *buf, int words);
static int _write_sector(unsigned short *buf, int words);
static void _swizzle(unsigned char *buf, int words);
static int _lba_read(unsigned long addr, void *buf, int count, int lba48);
static int _lba_write(unsigned long addr, void *buf, int count, int lba48);
static void _iopause();

/** @brief Initializes the IDE layer.
 *
 *  Attempts to reset and identify a drive.  If we can find a drive, looks
 *  for the Pebbles partition.
 */
int ide_init(void)
{
    ide_drive0_present = 0;
    ide_pebpartition_present = 0;

    _IDE_DEBUG("ide_init: trying to reset drive 0...");
    if (_reset() < 0)
    {
        _IDE_DEBUG("ide_init: _reset failed -- are you sure there's a drive there?");
        return -1;
    }

    _IDE_DEBUG("ide_init: trying to identify drive 0...");
    if (_identify() < 0)
    {
        _IDE_DEBUG("ide_init: drive identify failed");
        return -1;
    }

    if (!ide_drive0_present) {
        _IDE_DEBUG("IDE drive not present :(\n");
        return -1;
    }

    _IDE_DEBUG("ide_init: found BM base addr, looking for Pebbles partition...");

    _find_pebbles();

    if (dma_init() < 0)
    {
        _IDE_DEBUG("ide_init: dma_init failed.\n");
        return -1;
    }

    return 0;
}

/** @brief Determine if there is a Pebbles partition available.
 *  @return 1 if there is a Pebbles partition to read from, 0 otherwise.
 */
int ide_present(void)
{
    return ide_pebpartition_present;
}

/** @brief Size the Pebbles partition.
 *  @retval number of (512-byte) sectors available in the Pebbles partition.
 *  @retval -1 if no Pebbles partition exists.
 */
int ide_size(void)
{
    if (!ide_pebpartition_present)
        return -1;

    return ide_pebpartition_size;
}

/** @brief Attempts to reset the IDE drive.
 *  @retval 0 if the drive came back to life in a reasonable amount of time.
 *  @retval -1 otherwise.
 */
static int _reset(void)
{
    int counter = 1000;

    /* Select the drive. */
    outb(IDE_SELECT, IDE_SELECT_RSVD);
    if (_wait_busy() < 0)
    {
        _IDE_DEBUG("_reset: first BUSY wait timed out");
        return -1;
    }

    /* Reset it. */
    //outb(IDE_DCR, IDE_DCR_RSVD | IDE_DCR_SRST | IDE_DCR_nIE);
    outb(IDE_DCR, IDE_DCR_RSVD | IDE_DCR_SRST);

    /* Wait for the drive to say it's working on it. */
    while (!(inb(IDE_STATUS) & IDE_STATUS_BUSY))
        if (counter-- == 0)
        {
            _IDE_DEBUG("_reset: BUSY wait for SRST timed out");
            return -1;
        }
    /* Deassert SRST. */
    //outb(IDE_DCR, IDE_DCR_RSVD | IDE_DCR_nIE);
    outb(IDE_DCR, IDE_DCR_RSVD);
    _wait_busy();

    /* Wait for drive ready. */
    while (!(inb(IDE_STATUS) & IDE_STATUS_DRDY))
        ;

    return 0;
}

/** @brief Attempts to identify the attached drive.
 *
 *  Sends the identify command, and then prints some information about the
 *  output.  If it detects a drive that it knows how to talk to, sets the
 *  'drive present' bit, and sets the sector count.  ide.c only supports
 *  LBA, so if your drive is too old to support LBA, then you can't use the
 *  Pebbles hard drive support.  How sad.
 */
static int _identify(void)
{
    unsigned short identbuf[256];
    unsigned char asciibuf[41];
    int counter = 1000;

    /* Select the drive. */
    outb(IDE_SELECT, IDE_SELECT_RSVD | IDE_SELECT_LBA);

    /* wait about 400ns */
    for (counter = 0; counter < 50; counter++)
        inb(IDE_ALTSTATUS);
    if (_wait_busy() < 0)
    {
        _IDE_DEBUG("_identify: drive busy wait failed");
        goto faileas;
    }

    /* Wait for drive ready. */
    counter = 5000;
    while (!(inb(IDE_ALTSTATUS) & IDE_STATUS_DRDY))
    {
        if (counter-- == 0)
        {
            _IDE_DEBUG("_identify: drive ready wait failed");
            goto faileas;
        }
        _iopause();
    }
    if (inb(IDE_ALTSTATUS) & IDE_STATUS_ERROR)
    {
        _IDE_DEBUG("_identify: IDE status error trying to identify");
        goto faileas;
    }
    _iopause();
    _IDE_DEBUG("_identify: reading sector from disk: ");
    outb(IDE_SECCNT, 0);
    outb(IDE_SECNUM, 0);
    outb(IDE_LBAL, 0);
    outb(IDE_LBAM, 0);
    outb(IDE_COMMAND, IDE_COMMAND_IDENTIFY);
    if (inb(IDE_ALTSTATUS) == 0)
    {
        _IDE_DEBUG("_identify: drive doesn't *really* exist?");
        goto faileas;
    }
    if (_read_sector(identbuf, 256) < 0)
    {
        _IDE_DEBUG("_identify: sector read failed");
        goto faileas;
    }

    memcpy(asciibuf, &identbuf[0x1B], 40);
    asciibuf[40] = 0;
    _swizzle(asciibuf, 20);

    _IDE_DEBUG("_identify: drive information:");
    _IDE_DEBUG("_identify: drive 0 is \"%s\"", asciibuf);
    if (identbuf[49] & (1 << 9))
    {
        int sectors;
        _IDE_DEBUG("_identify: drive 0 supports LBA");
        sectors = identbuf[61];
        sectors <<= 16;
        sectors |= identbuf[60];
        _IDE_DEBUG("_identify: drive 0 is %d sectors = %d MiB", sectors, sectors / (2 * 1024));

        ide_drive0_present = 1;
        ide_drive0_sectors = sectors;
    } else
        _IDE_DEBUG("_identify: drive 0 doesn't support LBA; we don't know how to deal with it");

    return 0;

faileas:
    return -1;
}

/** @brief The on-disk layout of the partition table. */
struct ptab {
    unsigned char flags;
    unsigned char sh, scl, sch;
    unsigned char type;
    unsigned char eh, ecl, ech;
    unsigned long lba, size;
};

/** @brief Looks for a Pebbles partition.
 *
 *  Pebbles's partition type is 0xAA. Only partitions with correct LBA bits
 *  are supported.
 */
static void _find_pebbles(void)
{
    unsigned char buf[512];
    struct ptab *table;
    int i;

    if (_lba_read(0x0, buf, 1, 0) < 0)
    {
        _IDE_DEBUG("_find_pebbles: partition table read failed");
        return;
    }

    table = (struct ptab *)&buf[HD_MBR_PTAB];
    for (i=0; i<4; i++)
        if (table[i].type == HD_PARTITION_PEBBLES)
        {
            _IDE_DEBUG("_find_pebbles: %lu sector (%lu KiB) Pebbles partition found at LBA %08lx!", table[i].size, table[i].size / 2, table[i].lba);

            if (table[i].lba >= (1L<<28)
             || table[i].lba + table[i].size < table[i].lba
             || table[i].lba + table[i].size >= (1L<<28))
            {
                _IDE_DEBUG("_find_pebbles: ... but it's too big for LBA28.");
                continue;
            }

            ide_pebpartition_present = 1;
            ide_pebpartition_start = table[i].lba;
            ide_pebpartition_size = table[i].size;
        }
}

/** @brief Reads a sector from the Pebbles partition.
 *
 *  All reads from the IDE device outside this driver are relative to the
 *  start of the Pebbles partition, in 512 byte sectors.
 */
int ide_read(unsigned long addr, void *buf, int count)
{
    int rv;

    if (!ide_pebpartition_present || (addr + count > ide_pebpartition_size))
        return -1;

    rv = _lba_read(addr + ide_pebpartition_start,
                   buf, count, ide_lba48_enabled);

    return rv;
}

/** @brief Writes a chunk to the Pebbles partition.
 *
 *  All accesses to the IDE device outside this driver are relative to the
 *  start of the Pebbles partition, in 512 byte sectors.
 */
int ide_write(unsigned long addr, void *buf, int count)
{
    int rv;

    if (!ide_pebpartition_present || (addr + count > ide_pebpartition_size))
        return -1;

    rv = _lba_write(addr + ide_pebpartition_start,
                    buf, count, ide_lba48_enabled);

    return rv;
}

/** @brief Prepare a drive for reading.
 *
 *  This prepares the drive to receive a read or write command and sends addr
 *  (address) and count (sector count) in appropriate form for LBA28 or LBA48.
 *  The caller must then send the actual read or write command.
 *
 *  @return Negative if addr or count are out of range for the specified mode.
 */
int lba_setup(uint64_t addr, int count, int lba48)
{
    if ((count > (lba48 ? (1<<16) : (1<<8))) || (count < 1))
    {
        _IDE_DEBUG("lba_setup: count out of range");
        return -1;
    }

    if (addr > (lba48 ? (1LL<<48) : (1LL<<28)))
    {
        _IDE_DEBUG("lba_setup: address out of range");
        return -1;
    }

    /* Select the drive. */
    outb(IDE_SELECT, IDE_SELECT_RSVD | IDE_SELECT_LBA);

    if (_wait_busy() < 0)
    {
        _IDE_DEBUG("lba_setup: drive busy wait failed");
        return -1;
    }

    /* The current and previous values of the Features register (same
     * address as err) are reserved / must be zero in LBA48 mode. The
     * spec says it is ignored in LBA28 mode, but clear it anyway. */
    outb(IDE_ERR, 0);

    if (lba48) {
        outb(IDE_ERR, 0);
        outb(IDE_SECCNT, (count >> 8) & 0xFF);
        outb(IDE_SECCNT, count & 0xFF);
        outb(IDE_SECNUM, (addr >> 24) & 0xFF);
        outb(IDE_SECNUM, addr & 0xFF);
        outb(IDE_LBAL, (addr >> 32) & 0xFF);
        outb(IDE_LBAL, (addr >> 8) & 0xFF);
        outb(IDE_LBAM, (addr >> 40) & 0xFF);
        outb(IDE_LBAM, (addr >> 16) & 0xFF);
        outb(IDE_SELECT, IDE_SELECT_RSVD | IDE_SELECT_LBA);
    } else {
        outb(IDE_SECCNT, count);
        outb(IDE_SECNUM, addr & 0xFF);
        outb(IDE_LBAL, (addr >> 8) & 0xFF);
        outb(IDE_LBAM, (addr >> 16) & 0xFF);
        outb(IDE_SELECT, IDE_SELECT_RSVD | IDE_SELECT_LBA | ((addr >> 24) & 0xF));
    }

    return 0;
}

/** @brief Sends the LBA Read command to the hard drive and reads the results.
 *
 *  Reads a sector from any location on the drive, not just the Pebbles
 *  partition.  Handles all the IDE interfacing.
 */
static int _lba_read(unsigned long addr, void *buf, int count, int lba48)
{
    int i;

    if (lba_setup(addr, count, lba48) < 0)
        return -1;

    outb(IDE_COMMAND, lba48 ? IDE_COMMAND_READ48
                            : IDE_COMMAND_READ28);
    for (i = 0; i < count; i++)
        if (_read_sector((unsigned short *)(buf+(i*512)), 256) < 0)
            return -1;
    return 0;
}

/** @brief Sends the LBA Write command to the hard drive, along with the data.
 *
 *  Writes a sector to any location on the drive, not just the Pebbles
 *  partition.  Handles all the IDE interfacing.
 */
static int _lba_write(unsigned long addr, void *buf, int count, int lba48)
{
    int i;

    if (lba_setup(addr, count, lba48) < 0)
        return -1;

    outb(IDE_COMMAND, lba48 ? IDE_COMMAND_WRITE48
                            : IDE_COMMAND_WRITE28);
    for (i = 0; i < count; i++)
        if (_write_sector((unsigned short *)(buf+(i*512)), 256) < 0)
            return -1;
    return 0;
}

/** @brief Waits for the drive to report that it is no longer busy.
 *
 *  Times out after 1000 attempts so that the system doesn't hang if the
 *  drive goes away (and reports an error when that happens).
 */
static int _wait_busy()
{
    unsigned long counter = 1000000;
    inb(IDE_ALTSTATUS);
    inb(IDE_ALTSTATUS);
    inb(IDE_ALTSTATUS);
    inb(IDE_ALTSTATUS);
    inb(IDE_ALTSTATUS);
    while (inb(IDE_ALTSTATUS) & IDE_STATUS_BUSY)
        if (counter-- == 0)
            return -1;
    return 0;
}

/** @brief Waits for the drive to report that it is ready for data operations.
 *
 *  Times out after many, many attempts if the drive is very hung (mostly to
 *  get around an IDENTIFY bug on real hardware).
 *
 *  @bug The timeout really shouldn't be necessary.
 */
static int _wait_drq()
{
    unsigned char status;
    int counter = 10000000;

    do
    {
        --counter;
        status = inb(IDE_ALTSTATUS);
    } while (counter && !(status & (IDE_STATUS_DRQ | IDE_STATUS_ERROR)));

    if (!counter)
    {
        _IDE_DEBUG("_wait_drq: status read timed out (final status 0x%02x)", status);
        return -1;
    }
    if (status & IDE_STATUS_ERROR)
    {
        _IDE_DEBUG("_wait_drq: IDE_STATUS_ERROR (final status 0x%02x)", status);
        return -1;
    }

    return 0;
}

/** @brief Reads a number of words from the data pin on the drive.
 *
 *  Waits for the drive to tell us that it's ready to give us data, then
 *  reads the data.  Does this as many times as told to do so.
 */
static int _read_sector(unsigned short *buf, int words)
{
    if (_wait_busy() < 0)
        return -1;

    if (_wait_drq() < 0)
        return -1;

#ifdef IDE_INLINE_ASM
    asm volatile("cld ; rep insw" : "=c"(words), "=D"(buf) : "c"(words), "D"(buf), "d"(IDE_DATA) : "memory");
#else
    while (words--)
        *(buf++) = inw(IDE_DATA);
#endif

    return 0;
}

/** @brief Writes a number of words to the data pin on the drive.
 *
 *  Waits for the drive to tell us that it's ready for us to feed it data,
 *  then writes the data.  Does this as many times as told to do so.
 */
static int _write_sector(unsigned short *buf, int words)
{
    if (_wait_busy() < 0)
        return -1;

    if (_wait_drq() < 0)
        return -1;

#ifdef IDE_INLINE_ASM
    asm volatile("cld ; rep outsw" : "=c"(words), "=S"(buf) : "c"(words), "S"(buf), "d"(IDE_DATA));
#else
    while (words--)
        outw(IDE_DATA, *(buf++));
#endif

    return 0;
}

/** @brief "Swizzles" words from the drive.
 *
 *  IDENTIFY words come off the disk byteswapped; i.e., instead of "QEMU
 *  HARDDRIVE", you get "EQUMH RADDIREV".  This function byteswaps some
 *  number of words from a buffer.
 */
static void _swizzle(unsigned char *buf, int words)
{
    while (words--)
    {
        unsigned char t;
        t = buf[0];
        buf[0] = buf[1];
        buf[1] = t;
        buf += 2;
    }
}

/** @brief Pauses for a little while
 *
 *  Sadly, exactly how long an inb from 0x80 waits for is unspecified.  But
 *  this should be long enough for anything you might want to do.
 */
static void _iopause()
{
    int i;

    for (i = 0; i < 1024; i++)
        inb(0x80);
}
