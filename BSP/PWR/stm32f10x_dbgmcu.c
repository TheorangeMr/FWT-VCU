 /**
00002   ******************************************************************************
00003   * @file    stm32f10x_dbgmcu.c
00004   * @author  MCD Application Team
00005   * @version V3.5.0
00006   * @date    11-March-2011
00007   * @brief   This file provides all the DBGMCU firmware functions.
00008   ******************************************************************************
00009   * @attention
00010   *
00011   * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
00012   * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
00013   * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
00014   * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
00015   * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
00016   * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
00017   *
00018   * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
00019   ******************************************************************************
00020   */
 
 /* Includes ------------------------------------------------------------------*/
 #include "stm32f10x_dbgmcu.h"
 
 /** @addtogroup STM32F10x_StdPeriph_Driver
00026   * @{
00027   */
 
 /** @defgroup DBGMCU 
00030   * @brief DBGMCU driver modules
00031   * @{
00032   */ 
 
 /** @defgroup DBGMCU_Private_TypesDefinitions
00035   * @{
00036   */
 
 /**
00039   * @}
00040   */
 
 /** @defgroup DBGMCU_Private_Defines
00043   * @{
00044   */
 
 #define IDCODE_DEVID_MASK    ((uint32_t)0x00000FFF)
 /**
00048   * @}
00049   */
 
 /** @defgroup DBGMCU_Private_Macros
00052   * @{
00053   */

 /**
00056   * @}
00057   */
 
 /** @defgroup DBGMCU_Private_Variables
00060   * @{
00061   */
 
 /**
00064   * @}
00065   */
 
 /** @defgroup DBGMCU_Private_FunctionPrototypes
00068   * @{
00069   */
 
 /**
00072   * @}
00073   */
 
 /** @defgroup DBGMCU_Private_Functions
   * @{
   */

 /**
   * @brief  Returns the device revision identifier.
   * @param  None
   * @retval Device revision identifier
   */
 uint32_t DBGMCU_GetREVID(void)
 {
    return(DBGMCU->IDCODE >> 16);
 }
 
 /**
00090   * @brief  Returns the device identifier.
00091   * @param  None
00092   * @retval Device identifier
00093   */
 uint32_t DBGMCU_GetDEVID(void)
 {
    return(DBGMCU->IDCODE & IDCODE_DEVID_MASK);
 }
 
 /**
00100   * @brief  Configures the specified peripheral and low power mode behavior
00101   *   when the MCU under Debug mode.
00102   * @param  DBGMCU_Periph: specifies the peripheral and low power mode.
00103   *   This parameter can be any combination of the following values:
00104   *     @arg DBGMCU_SLEEP: Keep debugger connection during SLEEP mode              
00105   *     @arg DBGMCU_STOP: Keep debugger connection during STOP mode               
00106   *     @arg DBGMCU_STANDBY: Keep debugger connection during STANDBY mode            
00107   *     @arg DBGMCU_IWDG_STOP: Debug IWDG stopped when Core is halted          
00108   *     @arg DBGMCU_WWDG_STOP: Debug WWDG stopped when Core is halted          
00109   *     @arg DBGMCU_TIM1_STOP: TIM1 counter stopped when Core is halted          
00110   *     @arg DBGMCU_TIM2_STOP: TIM2 counter stopped when Core is halted          
00111   *     @arg DBGMCU_TIM3_STOP: TIM3 counter stopped when Core is halted          
00112   *     @arg DBGMCU_TIM4_STOP: TIM4 counter stopped when Core is halted          
00113   *     @arg DBGMCU_CAN1_STOP: Debug CAN2 stopped when Core is halted           
00114   *     @arg DBGMCU_I2C1_SMBUS_TIMEOUT: I2C1 SMBUS timeout mode stopped when Core is halted
00115   *     @arg DBGMCU_I2C2_SMBUS_TIMEOUT: I2C2 SMBUS timeout mode stopped when Core is halted
00116   *     @arg DBGMCU_TIM5_STOP: TIM5 counter stopped when Core is halted          
00117   *     @arg DBGMCU_TIM6_STOP: TIM6 counter stopped when Core is halted          
00118   *     @arg DBGMCU_TIM7_STOP: TIM7 counter stopped when Core is halted          
00119   *     @arg DBGMCU_TIM8_STOP: TIM8 counter stopped when Core is halted
00120   *     @arg DBGMCU_CAN2_STOP: Debug CAN2 stopped when Core is halted 
00121   *     @arg DBGMCU_TIM15_STOP: TIM15 counter stopped when Core is halted
00122   *     @arg DBGMCU_TIM16_STOP: TIM16 counter stopped when Core is halted
00123   *     @arg DBGMCU_TIM17_STOP: TIM17 counter stopped when Core is halted                
00124   *     @arg DBGMCU_TIM9_STOP: TIM9 counter stopped when Core is halted
00125   *     @arg DBGMCU_TIM10_STOP: TIM10 counter stopped when Core is halted
00126   *     @arg DBGMCU_TIM11_STOP: TIM11 counter stopped when Core is halted
00127   *     @arg DBGMCU_TIM12_STOP: TIM12 counter stopped when Core is halted
00128   *     @arg DBGMCU_TIM13_STOP: TIM13 counter stopped when Core is halted
00129   *     @arg DBGMCU_TIM14_STOP: TIM14 counter stopped when Core is halted
00130   * @param  NewState: new state of the specified peripheral in Debug mode.
00131   *   This parameter can be: ENABLE or DISABLE.
00132   * @retval None
00133   */
void DBGMCU_Config(uint32_t DBGMCU_Periph, FunctionalState NewState)
 {
   /* Check the parameters */
   assert_param(IS_DBGMCU_PERIPH(DBGMCU_Periph));
   assert_param(IS_FUNCTIONAL_STATE(NewState));
 
   if (NewState != DISABLE)
   {
     DBGMCU->CR |= DBGMCU_Periph;
   }
   else
   {
     DBGMCU->CR &= ~DBGMCU_Periph;
   }
 }
 
 /**
   * @}
   */
 
 /**
   * @}
   */
 
 /**
   * @}
   */

 /******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/

