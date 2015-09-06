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
#include "..\..\..\driver\driver.h"
/*============================ MACROS ========================================*/
/*============================ MACROFIED FUNCTIONS ===========================*/
/*============================ TYPES =========================================*/
/*============================ LOCAL VARIABLES ===============================*/
/*============================ PROTOTYPES ====================================*/
static void SetCs1(void);
static void SetCs2(void);
static void ClrCs1(void);
static void ClrCs2(void);
static void InitCS1(void);
static void InitCS2(void);

typedef struct{ 
    void (*fnInitCS)(void);         //!< init #CS pin func pointer
    void (*fnDeInitCS)(void);       //!< deinit #CS pin func pointer
    void (*fnSetCs)(void);          //!< set pin #cs IO func pointer
    void (*fnClrCs)(void);          //!< clr pin #cs  IO func pointer
    void (*fnInitBus)(void);
    void (*fnStartDma)(void);
}w25qx_io_cfg_t;

const w25qx_io_cfg_t sdf_io[2] = {
    
    {.fnInitCS = InitCS1,
    .fnClrCs = ClrCs1,
    .fnSetCs = SetCs1,},
    
    {.fnInitCS = InitCS2,
    .fnClrCs = ClrCs2,
    .fnSetCs = SetCs2,},
    
};
/*============================ GLOBAL VARIABLES ==============================*/
/*============================ IMPLEMENTATION ================================*/
fsm_rt_t spi_exchange_byte( uint8_t chTxByte, uint8_t *pchRxByte )
{
   #define EXCHANGE_RESET() do { s_tState = EXCHANGE_START;} while(0)
    static enum {
        EXCHANGE_START = 0,
        EXCHANGE_CHECK_TXE,
        EXCHANGE_BYTE
    }s_tState = EXCHANGE_START;
    uint8_t chTemp;

    switch(s_tState) {
        case EXCHANGE_START:
            s_tState = EXCHANGE_CHECK_TXE;
            //break;
        case EXCHANGE_CHECK_TXE:
            if((SPI1->SR) & (1 << 1)) {
                SPI1->DR = chTxByte;
                s_tState = EXCHANGE_BYTE;
            }
            break;
            
        case EXCHANGE_BYTE:
            if((SPI1->SR) & (1 << 0)) {
                chTemp = SPI1->DR; 
                if(NULL != pchRxByte) {
                    *pchRxByte = chTemp;
                }
                EXCHANGE_RESET();
                return fsm_rt_cpl;
            }        
            break;
    }
    
    return fsm_rt_on_going;  
}

void deinit_spi_bus(void)
{

}

void spi_bus_low_init( void ) 
{
    SPI_InitTypeDef  SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* Enable SPI1 and GPIOA clocks */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1|RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    /* Configure SPI1 pins: NSS, SCK, MISO and MOSI */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
   
    //init w25qx pin #cs 1 
    /*GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);  
    GPIO_SetBits(GPIOC, GPIO_Pin_4); 
    
    //init w25qx pin #cs 2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_4); */
    
    /* SPI1 configuration */ 
    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; //SPI1…Ë÷√Œ™¡Ωœﬂ»´À´π§
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	                   //…Ë÷√SPI1Œ™÷˜ƒ£ Ω
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;                  //SPI∑¢ÀÕΩ” ’8Œª÷°Ω·ππ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;	 		               //¥Æ–– ±÷”‘⁄≤ª≤Ÿ◊˜ ±£¨ ±÷”Œ™∏ﬂµÁ∆Ω
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;		               //µ⁄∂˛∏ˆ ±÷”—ÿø™ º≤…—˘ ˝æ›
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;			               //NSS–≈∫≈”…»Ìº˛£® π”√SSIŒª£©π‹¿Ì
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16; //∂®“Â≤®Ãÿ¬ ‘§∑÷∆µµƒ÷µ:≤®Ãÿ¬ ‘§∑÷∆µ÷µŒ™8
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   // ˝æ›¥´ ‰¥”MSBŒªø™ º
    SPI_InitStructure.SPI_CRCPolynomial = 7;						   //CRC÷µº∆À„µƒ∂‡œÓ Ω
    SPI_Init(SPI1, &SPI_InitStructure);
    
    /* Enable SPI1  */
    SPI_Cmd(SPI1, ENABLE); 											  // πƒ‹SPI1Õ‚…Ë
}

static void InitCS1(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    
    //!< init GPIOC clock
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    //init w25qx pin #cs 1 
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);  
    GPIO_SetBits(GPIOC, GPIO_Pin_4);     
}

static void SetCs1(void)
{
    GPIO_SetBits(GPIOC, GPIO_Pin_4);  
}

static void ClrCs1(void)
{
    GPIO_ResetBits(GPIOC, GPIO_Pin_4);  
}



static void InitCS2(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    //init w25qx pin #cs 2
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);     
}
    
static void SetCs2(void)
{
    GPIO_SetBits(GPIOA, GPIO_Pin_4);  
}

static void ClrCs2(void)
{
    GPIO_ResetBits(GPIOA, GPIO_Pin_4);  
}

