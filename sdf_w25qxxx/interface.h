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

#ifndef __MAL_SPI_FLASH_INTERFACES_H__
#define __MAL_SPI_FLASH_INTERFACES_H__

/*============================ INCLUDES ======================================*/
#include "..\interface.h"

/*============================ DEPENDENT TYPES ===============================*/
/*============================ INCLUDES ======================================*/
/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
typedef struct{ 
    void (*fnInitCS)(void);         //!< init #CS pin func pointer
    void (*fnDeInitCS)(void);       //!< deinit #CS pin func pointer
    void (*fnSetCs)(void);          //!< set pin #cs IO func pointer
    void (*fnClrCs)(void);          //!< clr pin #cs  IO func pointer
}w25qx_io_cfg_t;
  
//! \brief define abstract class w25qxxx_t
DEF_CLASS_IMPLEMENT(w25qxxx_t, i_w25qxxx_t,

        //! \name memory instruction block
        //! @{
        DEF_INTERFACE(i_mib_t)
                                                                                                  
            fsm_rt_t        (*power_down)   (mem_t *ptMal);                             
            fsm_rt_t        (*read_uid)     (mem_t *ptMal,uint64_t *pdwUID);
            fsm_rt_t        (*read_did)     (mem_t *ptMal,uint16_t *phwDID);
            
        END_DEF_INTERFACE(i_mib_t)   
        //! @}
        
        //! \name Memory Abstraction Layers
        //! @{
        DEF_INTERFACE(i_w25qxxx_t) INHERIT(i_mem_t)

            IMPLEMENT(i_mib_t) 
            
        END_DEF_INTERFACE(i_w25qxxx_t)
        //! @}
    )
    page_t *ptPage;
    em_mem_status_t         tCurrentState;      //!< system state  
    const w25qx_io_cfg_t    *ptw25qx_io;        //!< #cs pin IO
    uint64_t                dwUID;              //!< UID

END_DEF_CLASS_IMPLEMENT(w25qxxx_t, i_w25qxxx_t)

/*============================ GLOBAL VARIABLES ==============================*/
/*============================ PROTOTYPES ====================================*/

#endif
/* EOF */
