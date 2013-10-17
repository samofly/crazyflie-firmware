/**
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
 *
 * Crazyflie Firmware
 *
 * Copyright (C) 2011-2012 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 */
#define DEBUG_MODULE "STABILIZER"

#include <string.h>

#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "task.h"

#include "mpu6050.h"
#include "hmc5883l.h"
#include "system.h"
#include "stabilizer.h"
#include "commander.h"
#include "controller.h"
#include "sensfusion6.h"
#include "imu.h"
#include "motors.h"
#include "log.h"
#include "usec_time.h"
#include "debug.h"
#include "crtp.h"

static void stabilizerTask(void *param);

static bool isInit;

void stabilizerInit(void) {
  if (isInit)
    return;

  imu6Init();

  xTaskCreate(stabilizerTask, (const signed char * const) "STABILIZER",
              2 * configMINIMAL_STACK_SIZE, NULL, /*Piority*/ 2, NULL);

  isInit = TRUE;
}

bool stabilizerTest(void) {
  return imu6Test();
}

static CRTPPacket p;
static int16_t mpuOut[9];

static void stabilizerTask(void *param) {
  uint32_t lastWakeTime;

  vTaskSetApplicationTaskTag(0, (void *)TASK_STABILIZER_ID_NBR);

  // Wait for the system to be fully started to start stabilization loop
  systemWaitStart();

  lastWakeTime = xTaskGetTickCount();

  hmc5883lSetMode(HMC5883L_MODE_SINGLE);
  vTaskDelay(M2T(HMC5883L_ST_DELAY_MS));

  uint32_t count = 0;
  while (1) {
    vTaskDelayUntil(&lastWakeTime, F2T(IMU_UPDATE_FREQ));

    mpu6050GetMotion6(&mpuOut[0], &mpuOut[1], &mpuOut[2], &mpuOut[3], &mpuOut[4], &mpuOut[5]);
    hmc5883lGetHeading(&mpuOut[6], &mpuOut[7], &mpuOut[8]);
    uint64_t imuReadTime = usecTimestamp();

    p.size = 30;
    p.data[0] = 133;
    memcpy(&p.data[1], &count, 4);
    memcpy(&p.data[5], mpuOut, 18);
    memcpy(&p.data[23], &imuReadTime, 8);
    crtpSendPacket(&p);
    count++;
  }
}
