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
 

/*============================ INCLUDES ======================================*/
#include ".\app_cfg.h"
#include ".\interface.h"
#include "..\..\page\page.h"

#if USE_COMPONENT_MAL_SDF_W25QXXXX == ENABLED

/*============================ MACROS ========================================*/
#define this                                (*ptThis)

#ifndef     MAL_SDF_W25QXXX_COUNT
#warning    MAL_SDF_W25QXXX_COUNT is not define,now use default 0x2u
#define     MAL_SDF_W25QXXX_COUNT           (0x2u)
#endif
#if         MAL_SDF_W25QXXX_COUNT == 0
#error      MAL_SDF_W25QXXX_COUNT  does not equal 0,use default 0x1u
#define     MAL_SDF_W25QXXX_COUNT           (0x1u)
#endif

#define SDF_PAGE_SIZE                       (256ul)
#define SDF_PAGE_COUNT                      (32768ul)

#define SDF_SECTOR_SIZE                     (4096ul)
#define SDF_SECTOR_COUNT                    (2048ul)

#define SDF_PAGE_SIZE_BIT                   (13u)

#define SDF_READY_MSK                       _BV(0)
#define SDF_BUSY_MSK                        _BV(1)

/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/

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
WEAK bool ENTER_CRITICAL_SECTOR_SPI(void);

/*! \brief leave critical spi bus
 *! 
 *! \param none
 *! 
 *! \retval none
 */
WEAK void LEAVE_CRITICAL_SECTOR_SPI(void);

/*! \brief initialize w25qx mal module
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \retval fsm_rt_cpl initialize succeed.
 *! \retval fsm_rt_err initialize failed
 */
static fsm_rt_t w25qx_init(mem_t *ptMal, void *ptCFG);

/*! \brief finish w25qx mal module
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \retval fsm_rt_cpl w25qx mal finish succeed.
 *! \retval fsm_rt_on_going w25qx mal finish on-going
 */
static fsm_rt_t w25qx_finish(mem_t *ptMal);

/*! \brief get w25qx mal module infomation
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \return memory infomation
 */
static mem_info_t w25qx_info(mem_t *ptMal);

/*! \brief open w25qx mal module
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \retval fsm_rt_cpl w25qx mal open succeed.
 *! \retval fsm_rt_on_going w25qx mal open on-going
 */
static fsm_rt_t w25qx_open(mem_t *ptMal);

/*! \brief close w25qx mal module
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \retval fsm_rt_cpl w25qx mal finish succeed.
 *! \retval fsm_rt_on_going w25qx mal finish on-going
 */
static fsm_rt_t w25qx_close(mem_t *ptMal);

/*! \brief get w25qx mal module status
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \return w25qx mal status
 */
static em_mem_status_t w25qx_status(mem_t *ptMal);

/*! \brief write a sector
 *! 
 *! \param ptMal memory abstract layer pointer
 *! \param wPageAddress page address
 *! \param ptBuffer page bufer 
 *! 
 *! \retval fsm_rt_err current state is busy or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY system is busy
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_page_write( mem_t *ptMal, 
                                 uint32_t wPageAddress, 
                                 void *ptBuffer);

/*! \brief read specified data
 *! 
 *! \param ptMal memory abstract layer pointer
 *! \param wPageAddress page address
 *! \param hwOffset offset within a page
 *! \param pchStream data buffer
 *! \param hwSize buffer size
 *! 
 *! \retval fsm_rt_err current state is busy or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY system is busy
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_page_read( mem_t *ptMal, 
                                    uint32_t wPageAddress, 
                                    uint_fast16_t hwOffset, 
                                    uint8_t *pchStream, 
                                    uint_fast16_t hwSize);

/*! \brief verify specified data 
 *! 
 *! \param ptMal memory abstract layer pointer
 *! \param wPageAddress page address
 *! \param hwOffset offset within a page
 *! \param pchStream data buffer
 *! \param hwSize buffer size
 *! 
 *! \retval fsm_rt_err current state is busy or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY system is busy
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_page_verify( mem_t *ptMal, 
                                  uint32_t wPageAddress, 
                                  uint_fast16_t hwOffset, 
                                  uint8_t *pchStream, 
                                  uint_fast16_t hwSize);

/*! \brief erase a sector
 *! 
 *! \param ptMal memory abstract layer pointer
 *! \param wPageAddress page address
 *! 
 *! \retval fsm_rt_err current state is busy or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY system is busy
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_page_erase( mem_t *ptMal, 
                                 uint32_t wPageAddress);

/*! \brief chip erase
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \retval fsm_rt_err current state is busy or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY system is busy
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_chip_erase(mem_t *ptMal);

/*! \brief read unique ID
 *! 
 *! \param pdwUID unique ID
 *! 
 *! \retval fsm_rt_err access failed or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY serial data flash is not ready
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_read_unique_ID(mem_t *ptMal,uint64_t *pdwUID);

/*! \brief read device ID
 *! 
 *! \param pdwUID device ID
 *! 
 *! \retval fsm_rt_err access failed or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY serial data flash is not ready
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_read_device_ID(mem_t *ptMal,uint16_t *phwDID);

/*! \brief W25Qx CMD: enable write
 *!
 *! \para none
 *! \retval fsm_rt_on_going device is busy
 *! \retval fsm_rt_cpl device is free
 */
static fsm_rt_t enable_write(mem_t *ptMal);

/* \brief common routine for write a 8bits command a 24bits address
 *
 * \param chCMD 8bits command
 * \param wAddr 32bits address
 *
 * \retval fsm_rt_cpl fsm is completed
 * \retval fsm_rt_on_gong fsm is running
 */
static fsm_rt_t write_command_and_address( uint8_t chCMD, uint32_t wAddr )  ;

/*! \brief wait device busy flag release
 *! 
 *! \param none
 *! 
 *! \retval pbIsBusy bool pointer : true sdf is busy ; false sdf is idle 
 *! \retval fsm_rt_on_going check device state fsm running
 *! \retval fsm_rt_cpl check device state fsm finish
 */
static fsm_rt_t check_device_state(mem_t *ptMal, bool *pbIsBusy )  ;


static fsm_rt_t w25qx_power_down(mem_t *ptMal);
/*============================ GLOBAL VARIABLES ==============================*/

//! \brief sdf mal object
const i_w25qxxx_t I_W25QX = {
  
          .base__i_mem_t = { 
                   .base__i_mcb_t = {
                    .Init       = &w25qx_init,
                    .Finish     = &w25qx_finish,
                    .Info       = &w25qx_info,
                    .Open       = &w25qx_open,
                    .Close      = &w25qx_close,
                    .GetStatus  = &w25qx_status
                },
                
                .base__i_mem_page_t = {
                    .PageWrite  = &w25qx_page_write,
                    .PageRead   = &w25qx_page_read,
                    .PageVerify = &w25qx_page_verify,
                    .PageErase  = &w25qx_page_erase,
                    .Erase      = &w25qx_chip_erase
                },
        },  
  
        .base__i_mib_t = {            
          .read_uid   = &w25qx_read_unique_ID,    
          .read_did   = &w25qx_read_device_ID, 
          .power_down = &w25qx_power_down,   
        },
};
       
union {
        CLASS(w25qxxx_t) __SDF[MAL_SDF_W25QXXX_COUNT];
        w25qxxx_t   SDF[MAL_SDF_W25QXXX_COUNT];
    } = {

        .__SDF[0] = {
                .ptMethod = &I_W25QX,
                .tCurrentState = MEM_NOT_READY,
            },                
        .__SDF[1] = {
                .ptMethod = &I_W25QX,
                .tCurrentState = MEM_NOT_READY,
            },
    };

/*============================ IMPLEMENTATION ================================*/
/*! \brief initialize w25qx mal module
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \retval fsm_rt_cpl initialize succeed.
 *! \retval fsm_rt_err initialize failed
 */
static fsm_rt_t w25qx_init(mem_t *ptMal, void *ptCFG)
{
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    w25qx_io_cfg_t *ptConfig = (w25qx_io_cfg_t *)ptCFG;
       
    if((NULL == ptThis) || (NULL == ptConfig)) {
        return fsm_rt_err;
    }
    
    SAFE_ATOM_CODE (
        if (MEM_NOT_READY == ptThis->tCurrentState) {  
          
            //!< config init obj
            ptThis->ptw25qx_io  = ptConfig;
            ptThis->tCurrentState = MEM_READY_IDLE;     

            //!< init spi bus
            spi_bus_low_init(); 

            //!< init #CS pin
           ptThis->ptw25qx_io->fnInitCS();        
        }
    )
   
    return fsm_rt_cpl;
}

/*! \brief finish w25qx mal module
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \retval fsm_rt_cpl w25qx mal finish succeed.
 *! \retval fsm_rt_on_going w25qx mal finish on-going
 */
static fsm_rt_t w25qx_finish(mem_t *ptMal)
{
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    fsm_rt_t tReturn = fsm_rt_on_going;
    
    SAFE_ATOM_CODE (
        ptThis->tCurrentState = MEM_NOT_READY;
        
    //!< deinit spi bus
    
    //!< deinit #CS pin

        tReturn = fsm_rt_cpl
    )
    
    return tReturn;
}


/*! \brief open w25qx mal module
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \retval fsm_rt_cpl w25qx mal open succeed.
 *! \retval fsm_rt_on_going w25qx mal open on-going
 */
static fsm_rt_t w25qx_open(mem_t *ptMal)
{
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    uint16_t hwDeviceID;
    
    if( NULL == ptThis ) {
        return fsm_rt_err;
    }
    
    //!< open spi bus clock
    
    //!< wakeup sdf,read sdf Device ID
    if(fsm_rt_cpl == w25qx_read_device_ID(ptMal,&hwDeviceID)) {
        return fsm_rt_cpl;
    }
    
    return fsm_rt_on_going;
}

/*! \brief close w25qx mal module
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \retval fsm_rt_cpl w25qx mal finish succeed.
 *! \retval fsm_rt_on_going w25qx mal finish on-going
 */
static fsm_rt_t w25qx_close(mem_t *ptMal)
{
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    fsm_rt_t tResult = fsm_rt_on_going;

    SAFE_ATOM_CODE (
        if (MEM_READY_BUSY != ptThis->tCurrentState) {
            ptThis->tCurrentState = MEM_NOT_READY;
            
            //!< use sdf low power model
            
            //!< close spi clock
            
            tResult = fsm_rt_cpl;
        }
    )

    return tResult;
}

/*! \brief get w25qx mal module infomation
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \return memory infomation
 */  
static mem_info_t w25qx_info(mem_t *ptMal)
{
    static const mem_info_t tReturn = {
        .hwProperty = MEM_BLOCK_ACCESS,             //!< memory property
        .hwPageSize = SDF_PAGE_SIZE,                //!< memory page size (type)
        .wPageCount = SDF_PAGE_COUNT,               //!< memory page count
        .chPageSizeBit = SDF_PAGE_SIZE_BIT,
    };
    
    return tReturn;   
}

/*! \brief get w25qx mal module status
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \return w25qx mal status
 */
static em_mem_status_t w25qx_status(mem_t *ptMal)
{
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    em_mem_status_t tResult;
    
    SAFE_ATOM_CODE(
        tResult = ptThis->tCurrentState;
    )

    return tResult;
}

/*! \brief write a sector no erase
 *! 
 *! \param ptMal memory abstract layer pointer
 *! \param wPageAddress page address
 *! \param ptBuffer page bufer 
 *! 
 *! \retval fsm_rt_err current state is busy or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY system is busy
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_page_write( mem_t *ptMal, 
                                 uint32_t wPageAddress, 
                                 void *ptBuffer)
{
    #define PAGE_WRITE_RESET() do { s_tState = PAGE_WRITE_START; } while(0)
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    static uint16_t s_hwCounter;
    uint8_t *pchData = ptBuffer;
    static enum {
        PAGE_WRITE_START = 0,
        PAGE_WRITE_READ_STATUS,
        PAGE_WRITE_ENTER_CRITICL,
        PAGE_WRITE_ENABLE_WRITE,
        PAGE_WRITE_WRITE_ERASE_CMD,
        PAGE_WRITE_WRITE_DATA,
        PAGE_WRITE_WRITE_READY ,
        PAGE_WRITE_WAITE_IDLE
    }s_tState = PAGE_WRITE_START;

    switch (s_tState) {
        case PAGE_WRITE_START:
            if ((wPageAddress >= SDF_PAGE_COUNT) 
                || (NULL == ptThis) || ( NULL == ptBuffer)){
                return fsm_rt_err;
            }
            s_tState = PAGE_WRITE_READ_STATUS;
            //break;
            
        case PAGE_WRITE_READ_STATUS:
            SAFE_ATOM_CODE(           
                //! whether system is initialized
                if (MEM_NOT_READY == ptThis->tCurrentState) {
                    //EXIT_SAFE_ATOM_CODE();
                    return fsm_rt_err;
                } else if (MEM_READY_BUSY == ptThis->tCurrentState) {
                    //EXIT_SAFE_ATOM_CODE();
                    return (fsm_rt_t)GSF_ERR_NOT_READY;
                }     
                ptThis->tCurrentState = MEM_READY_BUSY;     
                s_tState = PAGE_WRITE_ENTER_CRITICL;
            )
            break;
            
        case PAGE_WRITE_ENTER_CRITICL:
            if( ENTER_CRITICAL_SECTOR_SPI() ) {
                s_tState = PAGE_WRITE_ENABLE_WRITE;                 //!< enter critical spi bus
            }
            break;
            
        case PAGE_WRITE_ENABLE_WRITE:       
            if( fsm_rt_cpl == enable_write(ptMal) ) {               //!< enable write
                ptThis->ptw25qx_io->fnClrCs();                      //!< clr #cs pin
                s_tState = PAGE_WRITE_WRITE_ERASE_CMD;
            }
            break;
            
        case PAGE_WRITE_WRITE_ERASE_CMD:
            if ( fsm_rt_cpl ==  write_command_and_address           //!< write page cmd 0x02 and page address
                    ( SDF_CMD_PROG_PAGE,wPageAddress * SDF_PAGE_SIZE) ) {
                s_hwCounter = 0;
                s_tState = PAGE_WRITE_WRITE_DATA;
            }
            break;
            
        case PAGE_WRITE_WRITE_DATA:
                                                                    //!< write data to sdf
            if( fsm_rt_cpl == spi_exchange_byte( ((uint8_t*)(pchData))[s_hwCounter], NULL )) {    
                if ( (++s_hwCounter) >= SDF_PAGE_SIZE) {
                    ptThis->ptw25qx_io->fnSetCs();                  //!< set #cs pin
                    LEAVE_CRITICAL_SECTOR_SPI();                    //!< leave critical spi bus
                    s_tState = PAGE_WRITE_WRITE_READY;
                } 
            }
            break;
        
        case PAGE_WRITE_WRITE_READY:
            if( ENTER_CRITICAL_SECTOR_SPI() ) {
                s_tState = PAGE_WRITE_WAITE_IDLE;
            }
            break;
            
        case PAGE_WRITE_WAITE_IDLE:{
                bool bIsBusy;
                if( fsm_rt_cpl == check_device_state(ptMal, &bIsBusy ) ) {
                    LEAVE_CRITICAL_SECTOR_SPI();
                    if( bIsBusy ) {
                        s_tState = PAGE_WRITE_WRITE_READY;
                    } else {
                        SAFE_ATOM_CODE(
                            ptThis->tCurrentState = MEM_READY_IDLE;
                        ) 
                        PAGE_WRITE_RESET();
                        return fsm_rt_cpl;                    
                    }
                }
                break;         
            }
    }     

    return fsm_rt_on_going;
}

/*! \brief read specified data
 *! 
 *! \param ptMal memory abstract layer pointer
 *! \param wPageAddress page address
 *! \param hwOffset offset within a page
 *! \param pchStream data buffer
 *! \param hwSize buffer size
 *! 
 *! \retval fsm_rt_err current state is busy or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY system is busy
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_page_read( mem_t *ptMal, 
                                    uint32_t wPageAddress, 
                                    uint_fast16_t hwOffset, 
                                    uint8_t *pchStream, 
                                    uint_fast16_t hwSize)
{
    #define PAGE_READ_RESET() do { s_tState = PAGE_READ_START; } while(0)
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    static enum {
        PAGE_READ_START = 0,
        PAGE_READ_STATUS,
        PAGE_READ_ENTER_CRITICL,
        PAGE_READ_WRITE_CMD,
        PAGE_READ_FETCH_DATA,
    }s_tState = PAGE_READ_START;
    static uint16_t  s_hwSize ,hwIndex;
    uint32_t wOffset = wPageAddress * SDF_PAGE_SIZE + hwOffset;

    switch(s_tState) {
        case PAGE_READ_START:
            if (    (NULL == pchStream) 
                ||  (wOffset + hwSize > SDF_PAGE_SIZE * SDF_PAGE_COUNT)) {
                return fsm_rt_err;
            } else if (0 == hwSize) {
                return fsm_rt_cpl;
            }
            s_tState = PAGE_READ_STATUS;
            //break;
        case PAGE_READ_STATUS:
            SAFE_ATOM_CODE(           
                //! whether system is initialized
                if (MEM_NOT_READY == ptThis->tCurrentState) {
                    EXIT_SAFE_ATOM_CODE();
                    return fsm_rt_err;
                } else if (MEM_READY_BUSY == ptThis->tCurrentState) {
                    EXIT_SAFE_ATOM_CODE();
                    return (fsm_rt_t)GSF_ERR_NOT_READY;
                }
                s_hwSize = hwSize;
                ptThis->tCurrentState = MEM_READY_BUSY;     
                s_tState = PAGE_READ_ENTER_CRITICL;
            )
            break;
            
            
        case PAGE_READ_ENTER_CRITICL:
            if( ENTER_CRITICAL_SECTOR_SPI() ) {
                s_tState = PAGE_READ_WRITE_CMD;
                ptThis->ptw25qx_io->fnClrCs();
            }
            break;
            
        case PAGE_READ_WRITE_CMD:
            if ( fsm_rt_cpl ==  write_command_and_address( SDF_CMD_READ,wOffset) ) {
                s_tState = PAGE_READ_FETCH_DATA;
                hwIndex = 0;
            }
            break;
            
        case PAGE_READ_FETCH_DATA:
            if( fsm_rt_cpl == spi_exchange_byte( 0, (pchStream+hwIndex) )) {
                if(--s_hwSize){
                    hwIndex++;
                } else {
                    ptThis->ptw25qx_io->fnSetCs();
                    LEAVE_CRITICAL_SECTOR_SPI();
                    SAFE_ATOM_CODE(
                        ptThis->tCurrentState = MEM_READY_IDLE;
                    ) 
                    PAGE_READ_RESET();
                    return fsm_rt_cpl;    
                }
            }
            break;
    }
    
    return fsm_rt_on_going;
}

/*! \brief verify specified data 
 *! 
 *! \param ptMal memory abstract layer pointer
 *! \param wPageAddress page address
 *! \param hwOffset offset within a page
 *! \param pchStream data buffer
 *! \param hwSize buffer size
 *! 
 *! \retval fsm_rt_err current state is busy or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY system is busy
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_page_verify( mem_t *ptMal, 
                                      uint32_t wPageAddress, 
                                      uint_fast16_t hwOffset, 
                                      uint8_t *pchStream, 
                                      uint_fast16_t hwSize)
{
    return fsm_rt_cpl;
}

/*! \brief erase a sector
 *! 
 *! \param ptMal memory abstract layer pointer
 *! \param wPageAddress page address
 *! 
 *! \retval fsm_rt_err current state is busy or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY system is busy
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_page_erase( mem_t *ptMal, 
                                     uint32_t wPageAddress)
{
    #define PAGE_REASE_RESET()  do {s_tState = PAGE_ERASE_START;} while(0)
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;

    static enum {
        PAGE_ERASE_START = 0,
        PAGE_ERASE_READ_STATUS,
        PAGE_ERASE_ENTER_CRITICL,
        PAGE_ERASE_ENABLE_WRITE,
        PAGE_ERASE_WRITE_ERASE_CMD,
        PAGE_ERASE_WRITE_READY ,
        PAGE_ERASE_WAITE_IDLE
    }s_tState = PAGE_ERASE_START;
    
    switch (s_tState) {
        case PAGE_ERASE_START:
            if ((wPageAddress >= SDF_PAGE_COUNT) || ( NULL == ptThis )){
                return fsm_rt_err;
            }
            s_tState = PAGE_ERASE_READ_STATUS;
            //break;
            
        case PAGE_ERASE_READ_STATUS:
            SAFE_ATOM_CODE(           
                //! whether system is initialized
                if (MEM_NOT_READY == ptThis->tCurrentState) {
                    EXIT_SAFE_ATOM_CODE();
                    return fsm_rt_err;
                } else if (MEM_READY_BUSY == ptThis->tCurrentState) {
                    EXIT_SAFE_ATOM_CODE();
                    return (fsm_rt_t)GSF_ERR_NOT_READY;
                }     
                ptThis->tCurrentState = MEM_READY_BUSY;     
                s_tState = PAGE_ERASE_ENTER_CRITICL;
            )
            break;
            
        case PAGE_ERASE_ENTER_CRITICL:
            if( ENTER_CRITICAL_SECTOR_SPI() ) {
                s_tState = PAGE_ERASE_ENABLE_WRITE;
            }
            break;
            
        case PAGE_ERASE_ENABLE_WRITE:
            if( fsm_rt_cpl == enable_write(ptMal) ) {
                ptThis->ptw25qx_io->fnClrCs();
                s_tState = PAGE_ERASE_WRITE_ERASE_CMD;
            }
            break;
            
        case PAGE_ERASE_WRITE_ERASE_CMD:
            if ( fsm_rt_cpl ==  write_command_and_address
                ( SDF_CMD_SECTOR_ERASE,(wPageAddress * SDF_PAGE_SIZE) & 0xFFFFFFFFFFFFF000) ) {
                ptThis->ptw25qx_io->fnSetCs();
                LEAVE_CRITICAL_SECTOR_SPI();
                s_tState = PAGE_ERASE_WRITE_READY;
            }
            break;
            
        case PAGE_ERASE_WRITE_READY:
            if( ENTER_CRITICAL_SECTOR_SPI() ) {
                s_tState = PAGE_ERASE_WAITE_IDLE;
            }
            break;
            
        case PAGE_ERASE_WAITE_IDLE:{
                bool bIsBusy;
                if( fsm_rt_cpl == check_device_state(ptMal, &bIsBusy ) ) {
                    if( bIsBusy ) {
                        LEAVE_CRITICAL_SECTOR_SPI();
                        s_tState = PAGE_ERASE_WRITE_READY;
                    } else {
                        SAFE_ATOM_CODE(
                            ptThis->tCurrentState = MEM_READY_IDLE;
                        ) 
                        LEAVE_CRITICAL_SECTOR_SPI();
                        PAGE_REASE_RESET();
                        return fsm_rt_cpl;                    
                    }
                }
                break;         
            }
    }     

    return fsm_rt_on_going;
}

/*! \brief chip erase
 *! 
 *! \param ptMal memory abstract layer pointer
 *! 
 *! \retval fsm_rt_err current state is busy or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY system is busy
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_chip_erase(mem_t *ptMal)
{
    #define CHIP_ERASE_RESET()  do {s_tState = CHIP_ERASE_START;} while(0)
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;

    static enum {
        CHIP_ERASE_START = 0,
        CHIP_ERASE_READ_STATUS,
        CHIP_ERASE_ENTER_CRITICL,
        CHIP_ERASE_ENABLE_WRITE,
        CHIP_ERASE_WRITE_ERASE_CMD,
        CHIP_ERASE_WRITE_READY ,
        CHIP_ERASE_WAITE_IDLE
    }s_tState = CHIP_ERASE_START;
    
    switch (s_tState) {
        case CHIP_ERASE_START:
            if ( NULL == ptThis ){
                return fsm_rt_err;
            }
            s_tState = CHIP_ERASE_READ_STATUS;
            //break;
            
        case CHIP_ERASE_READ_STATUS:
            SAFE_ATOM_CODE(           
                //! whether system is initialized
                if (MEM_NOT_READY == ptThis->tCurrentState) {
                    EXIT_SAFE_ATOM_CODE();
                    return fsm_rt_err;
                } else if (MEM_READY_BUSY == ptThis->tCurrentState) {
                    EXIT_SAFE_ATOM_CODE();
                    return (fsm_rt_t)GSF_ERR_NOT_READY;
                }     
                ptThis->tCurrentState = MEM_READY_BUSY;     
                s_tState = CHIP_ERASE_ENTER_CRITICL;
            )
            break;
            
        case CHIP_ERASE_ENTER_CRITICL:
            if( ENTER_CRITICAL_SECTOR_SPI() ) {
                s_tState = CHIP_ERASE_ENABLE_WRITE;
            }
            break;
            
        case CHIP_ERASE_ENABLE_WRITE:
            if( fsm_rt_cpl == enable_write(ptMal) ) {
                ptThis->ptw25qx_io->fnClrCs();
                s_tState = CHIP_ERASE_WRITE_ERASE_CMD;
            }
            break;
            
        case CHIP_ERASE_WRITE_ERASE_CMD:
            if( fsm_rt_cpl == spi_exchange_byte( SDF_CMD_CHIP_ERASE, NULL )) {
                ptThis->ptw25qx_io->fnSetCs();
                LEAVE_CRITICAL_SECTOR_SPI();
                s_tState = CHIP_ERASE_WRITE_READY;
            }
            break;
            
        case CHIP_ERASE_WRITE_READY:
            if( ENTER_CRITICAL_SECTOR_SPI() ) {
                s_tState = CHIP_ERASE_WAITE_IDLE;
            }
            break;
            
        case CHIP_ERASE_WAITE_IDLE:{
                bool bIsBusy;
                if( fsm_rt_cpl == check_device_state(ptMal, &bIsBusy ) ) {
                    if( bIsBusy ) {
                        LEAVE_CRITICAL_SECTOR_SPI();
                        s_tState = CHIP_ERASE_WRITE_READY;
                    } else {
                        SAFE_ATOM_CODE(
                            ptThis->tCurrentState = MEM_READY_IDLE;
                        ) 
                        LEAVE_CRITICAL_SECTOR_SPI();
                        CHIP_ERASE_RESET();
                        return fsm_rt_cpl;                    
                    }
                }
                break;         
            }
    }     

    return fsm_rt_on_going;  
  
}

/*! \brief read unique ID
 *! 
 *! \param pdwUID unique ID
 *! 
 *! \retval fsm_rt_err access failed or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY serial data flash is not ready
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_read_unique_ID(mem_t *ptMal,uint64_t *pdwUID)
{
    #define READ_UID_RESET() do { s_tState = READ_UID_START; } while(0)
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    static enum {
        READ_UID_START = 0,
        READ_UID_READ_STATUS,
        READ_UID_ENTER_CRITICL,
        READ_UID_WRITE_CMD,
        READ_UID_READ_UID,
    }s_tState = READ_UID_START;
    static uint8_t* s_pchUID = NULL;
    static uint8_t s_chIndex;
    
    switch(s_tState) {
        case READ_UID_START:
            if ((NULL == ptThis) || (NULL == pdwUID)) {
                return fsm_rt_err;
            } 
            s_pchUID = (uint8_t*)pdwUID;
            s_tState = READ_UID_READ_STATUS;
            //break;
        case READ_UID_READ_STATUS:
            SAFE_ATOM_CODE(           
                //! whether system is initialized
                if (MEM_NOT_READY == ptThis->tCurrentState) {
                    EXIT_SAFE_ATOM_CODE();
                    return fsm_rt_err;
                } else if (MEM_READY_BUSY == ptThis->tCurrentState) {
                    EXIT_SAFE_ATOM_CODE();
                    return (fsm_rt_t)GSF_ERR_NOT_READY;
                }     
                ptThis->tCurrentState = MEM_READY_BUSY;     
                s_tState = READ_UID_ENTER_CRITICL;
            )
            break;
            
        case READ_UID_ENTER_CRITICL:
            if( ENTER_CRITICAL_SECTOR_SPI() ) {
                s_tState = READ_UID_WRITE_CMD;
                ptThis->ptw25qx_io->fnClrCs();
            }
            break;
            
        case READ_UID_WRITE_CMD:{
                uint32_t wDumyWrite = 0x00000000;
                if ( fsm_rt_cpl ==  write_command_and_address( SDF_CMD_READ_UNIQUE_ID, wDumyWrite) ) {
                    s_tState = READ_UID_READ_UID;
                    s_chIndex = 0;
                }
            }
            break;
            
        case READ_UID_READ_UID:
            if( fsm_rt_cpl == spi_exchange_byte( 0, (s_pchUID+s_chIndex) )) {
                if(s_chIndex < 7){
                    s_chIndex++;
                } else {
                    ptThis->ptw25qx_io->fnSetCs();
                    LEAVE_CRITICAL_SECTOR_SPI();
                    SAFE_ATOM_CODE(
                        ptThis->tCurrentState = MEM_READY_IDLE;
                    ) 
                    READ_UID_RESET();
                    return fsm_rt_cpl;    
                }
            }
            break;
    }
    
    return fsm_rt_on_going;
}

static fsm_rt_t w25qx_power_down(mem_t *ptMal)
{
    return fsm_rt_cpl;
}

/*! \brief read device ID
 *! 
 *! \param phwDID device ID
 *! 
 *! \retval fsm_rt_err access failed or illegal parameter
 *! \retval fsm_rt_cpl access succeed
 *! \retval GSF_ERR_NOT_READY serial data flash is not ready
 *! \retval fsm_rt_on_going current access is on going.
 */
static fsm_rt_t w25qx_read_device_ID(mem_t *ptMal,uint16_t *phwDID)
{
    #define READ_DID_RESET() do { s_tState = READ_DID_START; } while(0)
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    static enum {
        READ_DID_START = 0,
        READ_DID_READ_STATUS,
        READ_DID_ENTER_CRITICL,
        READ_DID_WRITE_CMD,
        READ_DID_READ_UDD,
    }s_tState = READ_DID_START;
    static uint8_t s_chIndex;
    
    switch(s_tState) {
        case READ_DID_START:
            if ((NULL == ptThis) || (NULL == phwDID)) {
                return fsm_rt_err;
            } 
            s_tState = READ_DID_READ_STATUS;
            //break;
        case READ_DID_READ_STATUS:
            SAFE_ATOM_CODE(           
                //! whether system is initialized
                if (MEM_NOT_READY == ptThis->tCurrentState) {
                    EXIT_SAFE_ATOM_CODE();
                    return fsm_rt_err;
                } else if (MEM_READY_BUSY == ptThis->tCurrentState) {
                    EXIT_SAFE_ATOM_CODE();
                    return (fsm_rt_t)GSF_ERR_NOT_READY;
                }     
                ptThis->tCurrentState = MEM_READY_BUSY;     
                s_tState = READ_DID_ENTER_CRITICL;
            )
            break;
            
        case READ_DID_ENTER_CRITICL:
            if( ENTER_CRITICAL_SECTOR_SPI() ) {
                s_tState = READ_DID_WRITE_CMD;
                ptThis->ptw25qx_io->fnClrCs();
            }
            break;
            
        case READ_DID_WRITE_CMD:{
                uint32_t wDumyWrite = 0x00000000;
                if ( fsm_rt_cpl ==  write_command_and_address( SDF_CMD_READ_DEVICE_ID, wDumyWrite) ) {
                    s_tState = READ_DID_READ_UDD;
                    s_chIndex = 0;
                }
            }
            break;
            
        case READ_DID_READ_UDD:
            if( fsm_rt_cpl == spi_exchange_byte( 0, ((uint8_t *)phwDID + s_chIndex) )) {
                if(s_chIndex < 1){
                    s_chIndex++;
                } else {
                    ptThis->ptw25qx_io->fnSetCs();
                    LEAVE_CRITICAL_SECTOR_SPI();
                    SAFE_ATOM_CODE(
                        ptThis->tCurrentState = MEM_READY_IDLE;
                    ) 
                    READ_DID_RESET();
                    return fsm_rt_cpl;    
                }
            }
            break;
    }
    
    return fsm_rt_on_going;
}

/* \brief common routine for write a 8bits command a 24bits address
 *
 * \param chCMD 8bits command
 * \param wAddr 24bits address
 *
 * \retval fsm_rt_cpl fsm is completed
 * \retval fsm_rt_on_gong fsm is running
 */
static fsm_rt_t write_command_and_address( uint8_t chCMD, uint32_t wAddr )
{
    #define WRITE_CMD_ADDR_RESET() do { s_tState = WRITE_CMD_ADDR_START; } while(0)
    static uint8_t s_chAddrCounter;
    static enum {
        WRITE_CMD_ADDR_START = 0,
        WRITE_CMD_ADDR_WRITE_CMD,
        WRITE_CMD_ADDR_WRITE_ADDR
    }s_tState = WRITE_CMD_ADDR_START;
    
    switch(s_tState) {
        case WRITE_CMD_ADDR_START:
            s_tState = WRITE_CMD_ADDR_WRITE_CMD;
            //break;
            
        case WRITE_CMD_ADDR_WRITE_CMD:
            if( fsm_rt_cpl == spi_exchange_byte( chCMD, NULL )) {                                   //!< send 8bits command
                s_chAddrCounter = 2;
                s_tState = WRITE_CMD_ADDR_WRITE_ADDR;
            }
            break;
            
        case WRITE_CMD_ADDR_WRITE_ADDR:
            if( fsm_rt_cpl == spi_exchange_byte( ((uint8_t*)(&wAddr))[s_chAddrCounter], NULL )) {   //!< send 24bit address
                if ( !s_chAddrCounter) {
                    WRITE_CMD_ADDR_RESET();
                    return fsm_rt_cpl;
                } else {
                    s_chAddrCounter--;
                }
            }
            break;
    }
    
    return fsm_rt_on_going;
}
 
/*! \brief W25Qx CMD: enable write
 *!
 *! \para none
 *! \retval fsm_rt_on_going device is busy
 *! \retval fsm_rt_cpl device is free
 */
static fsm_rt_t enable_write(mem_t *ptMal)
{
    #define ENABLE_WRITE_RESET() do {s_tState = ENABLE_WRITE_START;} while(0)
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    static enum {
        ENABLE_WRITE_START = 0,
        ENABLE_WRITE_WRITE_CMD
    }s_tState = ENABLE_WRITE_START;
  
    switch( s_tState ) {
        case ENABLE_WRITE_START:
            if( NULL == ptThis ) {
                return fsm_rt_err;
            }
            //!< set #CS low
            ptThis->ptw25qx_io->fnClrCs();              
            s_tState = ENABLE_WRITE_WRITE_CMD;
            //break;
            
        case ENABLE_WRITE_WRITE_CMD:
            if( fsm_rt_cpl == spi_exchange_byte( SDF_CMD_ENABLE_WRITE, NULL) ){   //!< write command
                ptThis->ptw25qx_io->fnSetCs();
                //!< set #CS high
                ENABLE_WRITE_RESET();
                return fsm_rt_cpl;
            }
            break;
    }

    return fsm_rt_on_going;
}

/*! \brief wait device busy flag release
 *! 
 *! \param none
 *! 
 *! \retval pbIsBusy bool pointer : true sdf is busy ; false sdf is idle 
 *! \retval fsm_rt_on_going check device state fsm running
 *! \retval fsm_rt_cpl check device state fsm finish
 */
static fsm_rt_t check_device_state(mem_t *ptMal, bool *pbIsBusy )
{
    #define CHECK_BUSY_RESET() do {s_tState = CHECK_BUSY_START;} while(0)
    CLASS(w25qxxx_t) *ptThis = (CLASS(w25qxxx_t) *)ptMal;
    static enum {
        CHECK_BUSY_START = 0,
        CHECK_BUSY_WRITE_CMD,
        CHECK_BUSY_READ_STATUS,
        CHECK_BUSY_INIT_DISABLE_WRITE,
        CHECK_BUSY_SEND_DISABLE_WRITE
    }s_tState = CHECK_BUSY_START;
    
    switch(s_tState) {
        case CHECK_BUSY_START:
            if( ( NULL == ptThis ) || ( NULL == pbIsBusy) ){
                return fsm_rt_err;
            }
            ptThis->ptw25qx_io->fnClrCs();                                              //!< set #CS low
            s_tState = CHECK_BUSY_WRITE_CMD;
            break;
            
        case CHECK_BUSY_WRITE_CMD:
            if( fsm_rt_cpl == spi_exchange_byte(SDF_CMD_READ_STATUS_REG, NULL) ){       //!< write command 0x05 --> read status reg cmd
                s_tState = CHECK_BUSY_READ_STATUS;
            }
            break;
            
        case CHECK_BUSY_READ_STATUS:{
                uint8_t chStatus;
                
                if (fsm_rt_cpl == spi_exchange_byte(0, &chStatus)) {                    //!< recived status register value
                    ptThis->ptw25qx_io->fnSetCs();                                      //!< set #CS high
                    //! get status
                    if (!(chStatus & BUSY)) {                                           //!< check busy
                        //! device is ready
                        *pbIsBusy = false;

                        //! system is free                
                        if ((chStatus & WEL)) {                                         //!< check write enable latch
                            //! we should disable write
                            s_tState = CHECK_BUSY_INIT_DISABLE_WRITE;
                            break;
                        } 
                    } else {
                        *pbIsBusy = true;
                    }
                    CHECK_BUSY_RESET();                                                 //!< reset FSM / return fsm_rt_cpl
                    return fsm_rt_cpl;
                }       
            }
            break;
        
        case CHECK_BUSY_INIT_DISABLE_WRITE:
            ptThis->ptw25qx_io->fnClrCs();                                              //!< set #CS low
            s_tState = CHECK_BUSY_SEND_DISABLE_WRITE;
            break;
            
        case CHECK_BUSY_SEND_DISABLE_WRITE:
            if (fsm_rt_cpl == spi_exchange_byte(SDF_CMD_DISABLE_WRITE, NULL)) {         //!< write command dis enable 
                //! complete
                ptThis->ptw25qx_io->fnSetCs();                                          //!< set #CS high
                CHECK_BUSY_RESET();                                                     //!< reset FSM
                return fsm_rt_cpl;
            }
            break;
    }
    
    return fsm_rt_on_going;
}

/*! \brief enter critical spi bus
 *! 
 *! \param none
 *! 
 *! \retval true enter critical spi bus succeed.
 *! \retval false enter critical spi bus failed.
 */         
static bool ENTER_CRITICAL_SECTOR_SPI(void)
{
    return true;
}

/*! \brief leave critical spi bus
 *! 
 *! \param none
 *! 
 *! \retval none
 */
static void LEAVE_CRITICAL_SECTOR_SPI(void)
{
    return;
}

/*END OF FILE*/

#endif
