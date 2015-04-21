/** @file pci.c
 *  @brief Interface for finding + retrieving info from PCI devices from Pebbles
 *
 *  See the P4 handout.
 *
 *  Copyright (C) 2003, Lucent Technologies Inc. and others. All Rights Reserved.
 *  See PCI_LICENSE in this directory.
 *
 *  @author Caleb Levine (cjlevine)
 *  @author Chris Williamson (cdw1)
 */

#include <pci.h>
#include <ide-config.h>
#include <simics.h>
#include <x86/asm.h>

int pci_read_word(int dev_info, int reg)
{
    int type;

    if (BUSBNO(dev_info))
        type = 0x01;
    else
        type = 0x00;

    outd(PCI_ADDR, PCI_BUS_BASE | BUSBDF(dev_info) | reg | type);
    return ind(PCI_DATA);
}

/* @brief Find a class of PCI device
 * @param dev_class Device class type to search for
 * @return Packed int containing the device bus, dev, and function number */
int pci_find_device(int dev_class)
{
    int busn, devn, funcn;

    for (busn = 0; busn < PCI_NUM_BUS; busn++) {
        for (devn = 0; devn < PCI_NUM_DEV; devn++) {
            for (funcn = 0; funcn < PCI_NUM_FUN; funcn++) {

                int dev_info = MKBUS(BUS_PCI, busn, devn, funcn);

                int VID_resp = pci_read_word(dev_info, PCI_VID);
                if (VID_resp == -1 || VID_resp == 0)
                    continue;

                /* Check the device type */
                int dev_rid = pci_read_word(dev_info, PCI_RID);
                if ((dev_rid >> PCI_CLASS_OFFSET) == dev_class) {
                    /* It's the requested device, return the device info */
                    return dev_info;
                }
            }
        }
    }

    return -1;
}

int pci_find_bus_master_base()
{
    _IDE_DEBUG("Searching for Bus Master base address...\n");

    int dev_info = pci_find_device(PCI_CLASS_IDE);

    if (dev_info < 0)
        return -1;

    return pci_read_word(dev_info, PCI_BMBASE) & BM_BASE_ADDR_MASK;
}
