/***************************************************************************
 *   Copyright(C)2009-2014 by zhao_li <lizhao15431230@qq.com@qq.com>       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

//! \brief import up level configuration
#include "..\app_cfg.h"

//! \brief component configuration
#ifndef _USE_SERIAL_DATA_FLASH_CONFIG_H_
#define _USE_SERIAL_DATA_FLASH_CONFIG_H_

/*============================ INCLUDES ======================================*/
#include ".\interface.h"
/*============================ DEPENDENT TYPES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ MACROS ========================================*/
#define MAL_SDF_W25QXXX_COUNT       (0x2u)

//! \brief status register  bit definition
//! @{
#define BUSY                        (1 << 0)        //!< busy flag
#define WEL                         (1 << 1)        //!< write enable latch flag
//! @}
                                    
//! \name data flash access command
//! @{
#define SDF_CMD_PROG_PAGE           (0x02u)         //!< page programming
#define SDF_CMD_READ                (0x03u)         //!< read memory
#define SDF_CMD_DISABLE_WRITE       (0x04u)         //!< disable write
#define SDF_CMD_READ_STATUS_REG     (0x05u)         //!< read status register 1
#define SDF_CMD_ENABLE_WRITE        (0x06u)         //!< enable write
#define SDF_CMD_SECTOR_ERASE        (0x20u)         //!< erase sector
#define SDF_CMD_CHIP_ERASE          (0xC7u)         //!< erase chip 
#define SDF_CMD_READ_UNIQUE_ID      (0x4Bu)         //!< read unique-id
#define SDF_CMD_READ_DEVICE_ID      (0x90u)         //!< read device ID
//! @}

#define IS_SDF_CMD(CMD)             (((CMD) == SDF_CMD_PROG_PAGE )              \
                                    || ((CMD) == SDF_CMD_READ)                  \
                                    || ((CMD) == SDF_CMD_DISABLE_WRITE)         \
                                    || ((CMD) == SDF_CMD_READ_STATUS_REG)       \
                                    || ((CMD) == SDF_CMD_ENABLE_WRITE)          \
                                    || ((CMD) == SDF_CMD_SECTOR_ERASE)          \
                                    || ((CMD) == SDF_CMD_READ_UNIQUE_ID)        \
                                    || ((CMD) == SDF_CMD_READ_DEVICE_ID) )

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/
#endif
