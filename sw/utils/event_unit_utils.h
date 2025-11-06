/*
 * Copyright (C) 2024 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Authors: Luca Balboni <luca.balboni10@studio.unibo.it>
 *
 * MAGIA Event Unit - Generic utilities for all accelerators
 * Supports RedMulE, FSync, iDMA and custom events
 */

#ifndef EVENT_UNIT_UTILS_H
#define EVENT_UNIT_UTILS_H

#include <stdint.h>
#include "magia_tile_utils.h"

//=============================================================================
// Event Unit Register Map - Base addresses and offsets
//=============================================================================

#define EU_BASE                      EVENT_UNIT_BASE       

// Core Event Unit registers - Main control and status
#define EU_CORE_MASK                 (EU_BASE + 0x00)          // R/W: Event mask (enables event lines)
#define EU_CORE_MASK_AND             (EU_BASE + 0x04)          // W: Clear bits in mask
#define EU_CORE_MASK_OR              (EU_BASE + 0x08)          // W: Set bits in mask
#define EU_CORE_IRQ_MASK             (EU_BASE + 0x0C)          // R/W: IRQ event mask
#define EU_CORE_IRQ_MASK_AND         (EU_BASE + 0x10)          // W: Clear IRQ mask bits
#define EU_CORE_IRQ_MASK_OR          (EU_BASE + 0x14)          // W: Set IRQ mask bits
#define EU_CORE_STATUS               (EU_BASE + 0x18)          // R: Core clock status
#define EU_CORE_BUFFER               (EU_BASE + 0x1C)          // R: Event buffer
#define EU_CORE_BUFFER_MASKED        (EU_BASE + 0x20)          // R: Buffer with mask applied
#define EU_CORE_BUFFER_IRQ_MASKED    (EU_BASE + 0x24)          // R: Buffer with IRQ mask
#define EU_CORE_BUFFER_CLEAR         (EU_BASE + 0x28)          // W: Clear received events
#define EU_CORE_SW_EVENTS_MASK       (EU_BASE + 0x2C)          // R/W: SW event target mask
#define EU_CORE_SW_EVENTS_MASK_AND   (EU_BASE + 0x30)          // W: Clear SW target bits
#define EU_CORE_SW_EVENTS_MASK_OR    (EU_BASE + 0x34)          // W: Set SW target bits

// Core Event Unit wait registers - Sleep functionality
#define EU_CORE_EVENT_WAIT           (EU_BASE + 0x38)          // R: Sleep until event
#define EU_CORE_EVENT_WAIT_CLEAR     (EU_BASE + 0x3C)          // R: Sleep + clear buffer

// Hardware barrier registers (0x20 * barr_id offset)
#define HW_BARR_TRIGGER_MASK         (EU_BASE + 0x400)         // R/W: Barrier trigger mask
#define HW_BARR_STATUS               (EU_BASE + 0x404)         // R: Barrier status
#define HW_BARR_TARGET_MASK          (EU_BASE + 0x40C)         // R/W: Barrier target mask
#define HW_BARR_TRIGGER              (EU_BASE + 0x410)         // W: Manual barrier trigger
#define HW_BARR_TRIGGER_SELF         (EU_BASE + 0x414)         // R: Automatic trigger
#define HW_BARR_TRIGGER_WAIT         (EU_BASE + 0x418)         // R: Trigger + sleep
#define HW_BARR_TRIGGER_WAIT_CLEAR   (EU_BASE + 0x41C)         // R: Trigger + sleep + clear

// Software event trigger registers (0x04 * sw_event_id offset)
#define EU_CORE_TRIGG_SW_EVENT       (EU_BASE + 0x600)         // W: Generate SW event
#define EU_CORE_TRIGG_SW_EVENT_WAIT  (EU_BASE + 0x640)         // R: Generate event + sleep
#define EU_CORE_TRIGG_SW_EVENT_WAIT_CLEAR (EU_BASE + 0x680)    // R: Generate event + sleep + clear

// SoC event FIFO register
#define EU_CORE_CURRENT_EVENT        (EU_BASE + 0x700)         // R: SoC event FIFO

// Hardware mutex registers (0x04 * mutex_id offset)
#define EU_CORE_HW_MUTEX             (EU_BASE + 0x0C0)         // R/W: HW mutex management

//=============================================================================
// Event Bit Mapping - Based on cluster_event_map.sv
//=============================================================================

// DMA Events [3:2] - dma_events_i mapping
#define EU_DMA_EVT_0_BIT             2                         // DMA event 0 (completion)
#define EU_DMA_EVT_1_BIT             3                         // DMA event 1 (error/status)
#define EU_DMA_EVT_MASK              0x0000000C                // bits 3:2

// Timer Events [5:4] - timer_events_i mapping  
#define EU_TIMER_EVT_0_BIT           4                         // Timer event 0
#define EU_TIMER_EVT_1_BIT           5                         // Timer event 1
#define EU_TIMER_EVT_MASK            0x00000030                // bits 5:4

// Accelerator Events [11:8] - acc_events_i mapping
#define EU_ACC_EVT_0_BIT             8                         // Accelerator event 0 (always zero)
#define EU_ACC_EVT_1_BIT             9                         // Accelerator event 1 (busy)
#define EU_ACC_EVT_2_BIT             10                        // Accelerator event 2 (completion)
#define EU_ACC_EVT_3_BIT             11                        // Accelerator event 3 (additional)
#define EU_ACC_EVT_MASK              0x00000F00                // bits 11:8

// RedMulE specific event mapping (within accelerator events)
#define EU_REDMULE_BUSY_BIT          EU_ACC_EVT_1_BIT          // bit 9 - RedMulE busy
#define EU_REDMULE_DONE_BIT          EU_ACC_EVT_2_BIT          // bit 10 - RedMulE completion
#define EU_REDMULE_EVT1_BIT          EU_ACC_EVT_3_BIT          // bit 11 - RedMulE additional event
#define EU_REDMULE_DONE_MASK         (1 << EU_REDMULE_DONE_BIT) // 0x400
#define EU_REDMULE_BUSY_MASK         (1 << EU_REDMULE_BUSY_BIT) // 0x200
#define EU_REDMULE_EVT1_MASK         (1 << EU_REDMULE_EVT1_BIT) // 0x800
#define EU_REDMULE_ALL_MASK          (EU_ACC_EVT_MASK)         // 0xF00

// iDMA specific event mapping (within DMA events)
// Based on magia_tile.sv: assign dma_events_array[0] = {idma_o2a_done, idma_a2o_done};
#define EU_IDMA_A2O_DONE_BIT         EU_DMA_EVT_0_BIT          // bit 2 - iDMA AXI2OBI (L2->L1) completion
#define EU_IDMA_O2A_DONE_BIT         EU_DMA_EVT_1_BIT          // bit 3 - iDMA OBI2AXI (L1->L2) completion
#define EU_IDMA_A2O_DONE_MASK        (1 << EU_IDMA_A2O_DONE_BIT) // 0x04 - L2->L1 done
#define EU_IDMA_O2A_DONE_MASK        (1 << EU_IDMA_O2A_DONE_BIT) // 0x08 - L1->L2 done
#define EU_IDMA_ALL_DONE_MASK        (EU_IDMA_A2O_DONE_MASK | EU_IDMA_O2A_DONE_MASK) // 0x0C
#define EU_IDMA_ALL_MASK             (EU_DMA_EVT_MASK)         // 0x0C

// Legacy compatibility (uses A2O done by default)
#define EU_IDMA_DONE_BIT             EU_IDMA_A2O_DONE_BIT      // bit 2 - Default to A2O done
#define EU_IDMA_ERROR_BIT            EU_IDMA_O2A_DONE_BIT      // bit 3 - Legacy "error" was O2A done
#define EU_IDMA_DONE_MASK            EU_IDMA_A2O_DONE_MASK     // 0x04 - Legacy compatibility
#define EU_IDMA_ERROR_MASK           EU_IDMA_O2A_DONE_MASK     // 0x08 - Legacy compatibility

// iDMA extended status via cluster events [31:26]
#define EU_IDMA_A2O_ERROR_BIT        26                        // iDMA AXI2OBI error
#define EU_IDMA_O2A_ERROR_BIT        27                        // iDMA OBI2AXI error  
#define EU_IDMA_A2O_START_BIT        28                        // iDMA AXI2OBI start
#define EU_IDMA_O2A_START_BIT        29                        // iDMA OBI2AXI start
#define EU_IDMA_A2O_BUSY_BIT         30                        // iDMA AXI2OBI busy
#define EU_IDMA_O2A_BUSY_BIT         31                        // iDMA OBI2AXI busy
#define EU_IDMA_A2O_ERROR_MASK       (1 << EU_IDMA_A2O_ERROR_BIT) // 0x04000000
#define EU_IDMA_O2A_ERROR_MASK       (1 << EU_IDMA_O2A_ERROR_BIT) // 0x08000000
#define EU_IDMA_A2O_START_MASK       (1 << EU_IDMA_A2O_START_BIT) // 0x10000000
#define EU_IDMA_O2A_START_MASK       (1 << EU_IDMA_O2A_START_BIT) // 0x20000000
#define EU_IDMA_A2O_BUSY_MASK        (1 << EU_IDMA_A2O_BUSY_BIT)  // 0x40000000
#define EU_IDMA_O2A_BUSY_MASK        (1 << EU_IDMA_O2A_BUSY_BIT)  // 0x80000000
#define EU_IDMA_STATUS_MASK          0xFC000000                // All iDMA status bits [31:26]

// FSync specific event mapping (via cluster_events_i[25:24])
// Based on magia_tile.sv: fsync_error, fsync_done at bits [25:24]
#define EU_FSYNC_DONE_BIT            24                        // FSync completion event
#define EU_FSYNC_ERROR_BIT           25                        // FSync error event
#define EU_FSYNC_DONE_MASK           (1 << EU_FSYNC_DONE_BIT)  // 0x01000000
#define EU_FSYNC_ERROR_MASK          (1 << EU_FSYNC_ERROR_BIT) // 0x02000000
#define EU_FSYNC_ALL_MASK            (EU_FSYNC_DONE_MASK | EU_FSYNC_ERROR_MASK) // 0x03000000

// Legacy compatibility - use DONE by default
#define EU_FSYNC_EVT_BIT             EU_FSYNC_DONE_BIT         // bit 24 - Legacy compatibility
#define EU_FSYNC_EVT_MASK            EU_FSYNC_DONE_MASK        // 0x01000000 - Legacy compatibility

// Synchronization and barrier events [1:0]
#define EU_SYNC_EVT_BIT              0                         // Synchronization/barrier event
#define EU_DISPATCH_EVT_BIT          1                         // Dispatch event
#define EU_SYNC_EVT_MASK             0x00000001                // bit 0
#define EU_DISPATCH_EVT_MASK         0x00000002                // bit 1

//=============================================================================
// Event Type Definitions
//=============================================================================

typedef enum {
    EU_WAIT_MODE_POLLING = 0,     // Busy wait polling
    EU_WAIT_MODE_WFE,             // Wait For Event (RISC-V)
} eu_wait_mode_t;

//=============================================================================
// Core Control Functions
//=============================================================================

// Initialization
static inline void eu_init(void) {
    mmio32(EU_CORE_BUFFER_CLEAR) = 0xFFFFFFFF;
    mmio32(EU_CORE_MASK) = 0x00000000;
    mmio32(EU_CORE_IRQ_MASK) = 0x00000000;
}

// Event mask control
static inline void eu_enable_events(uint32_t event_mask) {
    mmio32(EU_CORE_MASK_OR) = event_mask;
}

static inline void eu_disable_events(uint32_t event_mask) {
    mmio32(EU_CORE_MASK_AND) = event_mask;
}

// IRQ control
static inline void eu_enable_irq(uint32_t irq_mask) {
    mmio32(EU_CORE_IRQ_MASK_OR) = irq_mask;
}

static inline void eu_disable_irq(uint32_t irq_mask) {
    mmio32(EU_CORE_IRQ_MASK_AND) = irq_mask;
}

// Event buffer operations
static inline void eu_clear_events(uint32_t event_mask) {
    mmio32(EU_CORE_BUFFER_CLEAR) = event_mask;
}

static inline uint32_t eu_get_events(void) {
    return mmio32(EU_CORE_BUFFER);
}

static inline uint32_t eu_get_events_masked(void) {
    return mmio32(EU_CORE_BUFFER_MASKED);
}

static inline uint32_t eu_get_events_irq_masked(void) {
    return mmio32(EU_CORE_BUFFER_IRQ_MASKED);
}

static inline uint32_t eu_check_events(uint32_t event_mask) {
    return mmio32(EU_CORE_BUFFER_MASKED) & event_mask;
}

//=============================================================================
// Wait Functions
//=============================================================================

// Polling mode wait
static inline uint32_t eu_wait_events_polling(uint32_t event_mask, uint32_t timeout_cycles) {
    uint32_t cycles = 0;
    uint32_t detected_events;
    
    do {
        detected_events = eu_check_events(event_mask);
        if (detected_events) {
            eu_clear_events(event_mask);
            return detected_events;
        }
        
        wait_nop(10);
        cycles += 10;
        
    } while (timeout_cycles == 0 || cycles < timeout_cycles);
    
    return 0;
}

// WFE mode wait
static inline uint32_t eu_wait_events_wfe(uint32_t event_mask) {
    uint32_t detected_events;

    eu_enable_irq(event_mask);// Ensure IRQs are enabled for the events being waited on
    
    detected_events = eu_check_events(event_mask);
    if (detected_events) {
        eu_clear_events(event_mask);
        return detected_events;
    }
    
    __asm__ volatile (".word 0x8C000073" ::: "memory");
    
    detected_events = eu_check_events(event_mask);
    if (detected_events) {
        eu_clear_events(event_mask);
    }
    return detected_events;
}

// Generic wait with mode selection
static inline uint32_t eu_wait_events(uint32_t event_mask, eu_wait_mode_t mode, uint32_t timeout_cycles) {
    switch (mode) {
        case EU_WAIT_MODE_POLLING:
            return eu_wait_events_polling(event_mask, timeout_cycles);
        case EU_WAIT_MODE_WFE:
            return eu_wait_events_wfe(event_mask);
        default:
            return eu_wait_events_polling(event_mask, timeout_cycles);
    }
}

//=============================================================================
// RedMulE Functions
//=============================================================================

static inline void eu_redmule_init(uint32_t enable_irq) {
    eu_clear_events(0xFFFFFFFF);
    eu_enable_events(EU_REDMULE_ALL_MASK);
    
    if (enable_irq) {
        eu_enable_irq(EU_REDMULE_DONE_MASK);
    }
}

static inline uint32_t eu_redmule_wait_completion(eu_wait_mode_t mode) {
    return eu_wait_events(EU_REDMULE_DONE_MASK, mode, 1000000);
}

static inline uint32_t eu_redmule_is_busy(void) {
    return eu_check_events(EU_REDMULE_BUSY_MASK);
}

static inline uint32_t eu_redmule_is_done(void) {
    return eu_check_events(EU_REDMULE_DONE_MASK);
}

//=============================================================================
// iDMA Functions
//=============================================================================

// Initialization
static inline void eu_idma_init(uint32_t enable_irq) {
    eu_clear_events(0xFFFFFFFF);
    eu_enable_events(EU_IDMA_ALL_MASK);
    
    if (enable_irq) {
        eu_enable_irq(EU_IDMA_ALL_DONE_MASK);
    }
}

// Wait functions
static inline uint32_t eu_idma_wait_completion(eu_wait_mode_t mode) {
    return eu_wait_events(EU_IDMA_ALL_DONE_MASK, mode, 1000000);
}

static inline uint32_t eu_idma_wait_direction_completion(uint32_t direction, eu_wait_mode_t mode) {
    uint32_t wait_mask = direction ? EU_IDMA_O2A_DONE_MASK : EU_IDMA_A2O_DONE_MASK;
    return eu_wait_events(wait_mask, mode, 1000000);
}

static inline uint32_t eu_idma_wait_a2o_completion(eu_wait_mode_t mode) {
    return eu_wait_events(EU_IDMA_A2O_DONE_MASK, mode, 1000000);
}

static inline uint32_t eu_idma_wait_o2a_completion(eu_wait_mode_t mode) {
    return eu_wait_events(EU_IDMA_O2A_DONE_MASK, mode, 1000000);
}

// Status check functions
static inline uint32_t eu_idma_is_done(void) {
    return eu_check_events(EU_IDMA_ALL_DONE_MASK);
}

static inline uint32_t eu_idma_a2o_is_done(void) {
    return eu_check_events(EU_IDMA_A2O_DONE_MASK);
}

static inline uint32_t eu_idma_o2a_is_done(void) {
    return eu_check_events(EU_IDMA_O2A_DONE_MASK);
}

static inline uint32_t eu_idma_has_error(void) {
    uint32_t events = eu_get_events();
    return events & (EU_IDMA_A2O_ERROR_MASK | EU_IDMA_O2A_ERROR_MASK);
}

static inline uint32_t eu_idma_a2o_has_error(void) {
    return eu_check_events(EU_IDMA_A2O_ERROR_MASK);
}

static inline uint32_t eu_idma_o2a_has_error(void) {
    return eu_check_events(EU_IDMA_O2A_ERROR_MASK);
}

static inline uint32_t eu_idma_is_busy(void) {
    uint32_t events = eu_get_events();
    return events & (EU_IDMA_A2O_BUSY_MASK | EU_IDMA_O2A_BUSY_MASK);
}

static inline uint32_t eu_idma_a2o_is_busy(void) {
    return eu_check_events(EU_IDMA_A2O_BUSY_MASK);
}

static inline uint32_t eu_idma_o2a_is_busy(void) {
    return eu_check_events(EU_IDMA_O2A_BUSY_MASK);
}

//=============================================================================
// FSync Functions
//=============================================================================

static inline void eu_fsync_init(uint32_t enable_irq) {
    eu_clear_events(0xFFFFFFFF);
    eu_enable_events(EU_FSYNC_ALL_MASK);
    
    if (enable_irq) {
        eu_enable_irq(EU_FSYNC_DONE_MASK);
    }
}

static inline uint32_t eu_fsync_wait_completion(eu_wait_mode_t mode) {
    return eu_wait_events(EU_FSYNC_DONE_MASK, mode, 1000000);
}

static inline uint32_t eu_fsync_is_done(void) {
    return eu_check_events(EU_FSYNC_DONE_MASK);
}

static inline uint32_t eu_fsync_has_error(void) {
    return eu_check_events(EU_FSYNC_ERROR_MASK);
}

//=============================================================================
// Multi-Accelerator Functions
//=============================================================================

static inline void eu_multi_init(uint32_t redmule_enable, uint32_t idma_a2o_enable, 
                                 uint32_t idma_o2a_enable, uint32_t fsync_enable, 
                                 uint32_t enable_irq) {
    eu_clear_events(0xFFFFFFFF);
    
    uint32_t event_mask = 0;
    uint32_t irq_mask = 0;
    
    if (redmule_enable) {
        event_mask |= EU_REDMULE_ALL_MASK;
        if (enable_irq) irq_mask |= EU_REDMULE_DONE_MASK;
    }
    
    if (idma_a2o_enable) {
        event_mask |= EU_IDMA_A2O_DONE_MASK;
        if (enable_irq) irq_mask |= EU_IDMA_A2O_DONE_MASK;
    }
    
    if (idma_o2a_enable) {
        event_mask |= EU_IDMA_O2A_DONE_MASK;
        if (enable_irq) irq_mask |= EU_IDMA_O2A_DONE_MASK;
    }
    
    if (fsync_enable) {
        event_mask |= EU_FSYNC_ALL_MASK;
        if (enable_irq) irq_mask |= EU_FSYNC_DONE_MASK;
    }
    
    if (event_mask) {
        eu_enable_events(event_mask);
    }
    
    if (irq_mask) {
        eu_enable_irq(irq_mask);
    }
}

static inline uint32_t eu_multi_wait_any(uint32_t wait_redmule, uint32_t wait_idma_a2o, 
                                         uint32_t wait_idma_o2a, uint32_t wait_fsync, 
                                         eu_wait_mode_t mode) {
    uint32_t wait_mask = 0;
    
    if (wait_redmule) wait_mask |= EU_REDMULE_DONE_MASK;
    if (wait_idma_a2o) wait_mask |= EU_IDMA_A2O_DONE_MASK;
    if (wait_idma_o2a) wait_mask |= EU_IDMA_O2A_DONE_MASK;
    if (wait_fsync) wait_mask |= EU_FSYNC_DONE_MASK;
    
    return eu_wait_events(wait_mask, mode, 1000000);
}

static inline uint32_t eu_multi_wait_all(uint32_t wait_redmule, uint32_t wait_idma_a2o, 
                                         uint32_t wait_idma_o2a, uint32_t wait_fsync, 
                                         eu_wait_mode_t mode) {
    uint32_t required_mask = 0;
    uint32_t wait_mask = 0;
    
    if (wait_redmule) required_mask |= EU_REDMULE_DONE_MASK;
    if (wait_idma_a2o) required_mask |= EU_IDMA_A2O_DONE_MASK;
    if (wait_idma_o2a) required_mask |= EU_IDMA_O2A_DONE_MASK;
    if (wait_fsync) required_mask |= EU_FSYNC_DONE_MASK;
    
    wait_mask = required_mask;
    
    if (mode == EU_WAIT_MODE_WFE) {
        uint32_t accumulated_events = 0;
        
        while ((accumulated_events & required_mask) != required_mask) {
            uint32_t missing_events = required_mask & ~accumulated_events;
            uint32_t detected_events = eu_wait_events(missing_events, EU_WAIT_MODE_WFE, 0);
            accumulated_events |= detected_events;
        }
        
        eu_clear_events(accumulated_events);
        return accumulated_events;
    } else {
        uint32_t timeout_cycles = 1000000;
        uint32_t cycles = 0;
        
        while (cycles < timeout_cycles) {
            uint32_t detected_events = eu_wait_events(wait_mask, mode, 100);
            
            if ((detected_events & required_mask) == required_mask) {
                return detected_events;
            }
            
            if (detected_events) {
                eu_clear_events(detected_events);
            }
            
            cycles += 100;
        }
        
        return 0;
    }
}

#endif // EVENT_UNIT_UTILS_H