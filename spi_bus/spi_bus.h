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


#ifndef __SPI_BUS_H__
#define __SPI_BUS_H__
/*============================ INCLUDES ======================================*/
#include ".\app_cfg.h"

/*============================ MACROS ========================================*/

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
extern const w25qx_io_cfg_t sdf_io[2];

/*! \brief initializing spi interface
 *! 
 *! \param none
 *! 
 *! \retval void.
 */  
extern void spi_bus_low_init( void ) ;

/*! \brief exchanging a single byte with spi
 *! 
 *! \param pchRxByte recive byte pointer
 *! \param chTxByte send byte 
 *! 
 *! \retval fsm_rt_cpl exchanging succeed.
 *! \retval fsm_rt_on_going exchanging on-going
 */  
extern fsm_rt_t spi_exchange_byte( uint8_t chTxByte, uint8_t *pchRxByte );

/*! \brief enter critical spi bus
 *! 
 *! \param none
 *! 
 *! \retval true enter critical spi bus succeed.
 *! \retval false enter critical spi bus failed.
 */
 
 

#endif
