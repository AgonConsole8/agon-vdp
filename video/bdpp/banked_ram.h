//
// Title:			Agon Video BIOS
// Author:			Dean Belfield
// Subtitle:		Banked RAM (SPI HIMEM)
// Author:			Curtis Whitley
// Contributors:	
// Created:			17/03/2024
// Last Updated:	17/03/2024
//

#ifndef BANKED_RAM_H
#define BANKED_RAM_H

#include "esp32/himem.h"
#include <memory.h>

#define MEM_AREA_SIZE   (4*1024*1024) // upper 4MB
#define MEM_VIEW_SIZE   (8*ESP_HIMEM_BLKSZ) // 8 pages of 32KB, or 256KB

extern void debug_log(const char* fmt, ...);

esp_himem_handle_t mem_area_handle;
esp_himem_rangehandle_t mem_view_handle;

// Initialize access to banked RAM
//
void banked_ram_initialize() {
    if (esp_himem_alloc(MEM_AREA_SIZE, &mem_area_handle) == ESP_OK) {
        debug_log("Allocated %u (0x%X) bytes in HIMEM\n", MEM_AREA_SIZE, MEM_AREA_SIZE);
    } else {
        debug_log("ERROR: Could not allocate HIRAM area!\n");
    }

    if (esp_himem_alloc_map_range(MEM_VIEW_SIZE, &mem_view_handle) == ESP_OK) {
        debug_log("Allocated %u (0x%X) bytes as HIMEM view\n", MEM_VIEW_SIZE, MEM_VIEW_SIZE);
    } else {
        debug_log("ERROR: Could not allocate HIRAM view!\n");
    }
}

// Read from banked RAM
//
// This function expects that the copied memory range will lie completely
// within a 256KB area, and not cross a 256KB boundary.
//
void banked_ram_read(uint32_t ram_offset, uint8_t* dst_buffer, uint16_t size) {
    uint32_t base_offset = ram_offset & (~(MEM_VIEW_SIZE - 1));
    void *ptr = NULL;
    if (esp_himem_map(mem_area_handle, mem_view_handle, base_offset, 0, MEM_VIEW_SIZE, 0, &ptr) == ESP_OK) {
        uint32_t view_offset = ram_offset & (MEM_VIEW_SIZE - 1);
        memcpy(dst_buffer, ptr, size);
        if (esp_himem_unmap(mem_view_handle, ptr, MEM_VIEW_SIZE)) {
            debug_log("ERROR: Could not unmap HIMEM view!\n");
        }
    } else {
        debug_log("ERROR: Could not map HIMEM view for 0x%X at base 0x%X!\n", ram_offset, base_offset);
    }
}

// Write to banked RAM
//
// This function expects that the copied memory range will lie completely
// within a 256KB area, and not cross a 256KB boundary.
//
void banked_ram_write(uint32_t ram_offset, const uint8_t* src_buffer, uint16_t size) {
    uint32_t base_offset = ram_offset & (~(MEM_VIEW_SIZE - 1));
    void *ptr = NULL;
    if (esp_himem_map(mem_area_handle, mem_view_handle, base_offset, 0, MEM_VIEW_SIZE, 0, &ptr) == ESP_OK) {
        uint32_t view_offset = ram_offset & (MEM_VIEW_SIZE - 1);
        memcpy(ptr, src_buffer, size);
        if (esp_himem_unmap(mem_view_handle, ptr, MEM_VIEW_SIZE)) {
            debug_log("ERROR: Could not unmap HIMEM view!\n");
        }
    } else {
        debug_log("ERROR: Could not map HIMEM view for 0x%X at base 0x%X!\n", ram_offset, base_offset);
    }    
}

#endif // BANKED_RAM_H
