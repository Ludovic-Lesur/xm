/*
 * sht3x_driver_flags.h
 *
 *  Created on: 12 jan. 2025
 *      Author: Ludo
 */

#ifndef __SHT3X_DRIVER_FLAGS_H__
#define __SHT3X_DRIVER_FLAGS_H__

#include "i2c.h"
#include "lptim.h"

/*** SHT3x driver compilation flags ***/

#ifndef SM
#define SHT3X_DRIVER_DISABLE
#endif

#define SHT3X_DRIVER_I2C_ERROR_BASE_LAST    I2C_ERROR_BASE_LAST
#define SHT3X_DRIVER_DELAY_ERROR_BASE_LAST  LPTIM_ERROR_BASE_LAST

#endif /* __SHT3X_DRIVER_FLAGS_H__ */
