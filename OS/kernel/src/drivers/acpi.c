/*
// Hojuix ACPI Driver
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <kern/kprintf.h>
#include <kern/libkern.h>
#include <drivers/acpi.h>
#include <memory/pmmgr.h>

#include <uacpi/uacpi.h>
#include <uacpi/event.h>
#include <uacpi/types.h>
#include <uacpi/tables.h>
#include <uacpi/log.h>
#include <kernel_ext/limine.h>

__attribute__((used, section(".requests")))
static volatile struct limine_rsdp_request rsdp_request = {
    .id = LIMINE_RSDP_REQUEST,
    .revision = 0
};

void acpi_init() {
    printf("[ACPI] init.\n");

    uintptr_t tempbuf = pmmgr_kcalloc_contiguous(1) + 0xFFFF800000000000;
    uacpi_status ret = uacpi_setup_early_table_access((void*)tempbuf, 4096);
    if (uacpi_unlikely_error(ret)) {
        printf("uacpi_initialize error: %s", uacpi_status_to_string(ret));
    }

    uacpi_table tbl;
    uacpi_status st;
    const char* signature = "RDST";

    st = uacpi_table_find_by_signature(signature, &tbl);
    if (st != UACPI_STATUS_OK) {
        printf("fucky wucky\n");
            } else {
                printf("hih\n");
            }
}

void acpi_reboot(){}
void acpi_shutdown(){}


#define ACPI_HHDM_OFFSET 0xFFFF800000000000

// uACPI Hook Functions (From uacpi/kernel_api.h>
uacpi_status uacpi_kernel_get_rsdp(uacpi_phys_addr *out_rsdp_address) {
    if (rsdp_request.response == NULL || 
        rsdp_request.response->address == 0) {
        return UACPI_STATUS_NOT_FOUND;
    }

    *out_rsdp_address = (uacpi_phys_addr)(uintptr_t)rsdp_request.response->address;
    return UACPI_STATUS_OK;
}

void uacpi_kernel_log(uacpi_log_level level, const uacpi_char* message) {
    printf("[uACPI] (Level %d) - Log: %s", level, message);
}

void *uacpi_kernel_map(uacpi_phys_addr addr, uacpi_size len) {
    if (addr > 0xFFFF800000000000) {
        return (void*)addr;
    } else {
        return (void*)(addr + 0xFFFF800000000000);
    }
}

void uacpi_kernel_unmap(void *addr, uacpi_size len) {
    //
}


