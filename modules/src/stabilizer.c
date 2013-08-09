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

#include "stm32f10x_conf.h"
#include "FreeRTOS.h"
#include "task.h"

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

/**
 * Defines in what divided update rate should the attitude
 * control loop run relative the rate control loop.
 */
#define ATTITUDE_UPDATE_RATE_DIVIDER 2
#define FUSION_UPDATE_DT                                                       \
  (float)(1.0 / (IMU_UPDATE_FREQ / ATTITUDE_UPDATE_RATE_DIVIDER))

#define LOGGING_ENABLED
#ifdef LOGGING_ENABLED
#define PRIVATE
#else
#define PRIVATE static
#endif

PRIVATE Axis3f gyro; // Gyro axis data in deg/s
PRIVATE Axis3f acc;  // Accelerometer axis data in mG

PRIVATE float eulerRollActual;
PRIVATE float eulerPitchActual;
PRIVATE float eulerYawActual;
PRIVATE float eulerRollDesired;
PRIVATE float eulerPitchDesired;
PRIVATE float eulerYawDesired;
PRIVATE float rollRateDesired;
PRIVATE float pitchRateDesired;
PRIVATE float yawRateDesired;
PRIVATE float fusionDt;

RPYType rollType;
RPYType pitchType;
RPYType yawType;

uint16_t actuatorThrust;
int16_t actuatorRoll;
int16_t actuatorPitch;
int16_t actuatorYaw;

uint32_t motorPowerM4;
uint32_t motorPowerM2;
uint32_t motorPowerM1;
uint32_t motorPowerM3;

LOG_GROUP_START(stabilizer)
LOG_ADD(LOG_FLOAT, roll, &eulerRollActual)
LOG_ADD(LOG_FLOAT, pitch, &eulerPitchActual)
LOG_ADD(LOG_FLOAT, yaw, &eulerYawActual)
LOG_ADD(LOG_UINT16, thrust, &actuatorThrust)
LOG_GROUP_STOP(stabilizer)

LOG_GROUP_START(motor)
LOG_ADD(LOG_INT32, m4, &motorPowerM4)
LOG_ADD(LOG_INT32, m1, &motorPowerM1)
LOG_ADD(LOG_INT32, m2, &motorPowerM2)
LOG_ADD(LOG_INT32, m3, &motorPowerM3)
LOG_GROUP_STOP(motor)

LOG_GROUP_START(acc)
LOG_ADD(LOG_FLOAT, x, &acc.x)
LOG_ADD(LOG_FLOAT, y, &acc.y)
LOG_ADD(LOG_FLOAT, z, &acc.z)
LOG_GROUP_STOP(acc)

LOG_GROUP_START(gyro)
LOG_ADD(LOG_FLOAT, x, &gyro.x)
LOG_ADD(LOG_FLOAT, y, &gyro.y)
LOG_ADD(LOG_FLOAT, z, &gyro.z)
LOG_GROUP_STOP(gyro)

static bool isInit;

static void distributePower(const uint16_t thrust, const int16_t roll,
                            const int16_t pitch, const int16_t yaw);
static void stabilizerTask(void *param);

// TODO(d2rk): verify the performance. This is not portable implementation.
static uint16_t limitThrust(int32_t thrust) {
  thrust -= thrust & (thrust >> 31);
  return (uint16_t)(((thrust - UINT16_MAX) & ((thrust - UINT16_MAX) >> 31)) +
                    UINT16_MAX);
}

void stabilizerInit(void) {
  if (isInit)
    return;

  motorsInit();
  imu6Init();
  sensfusion6Init();
  controllerInit();

  rollRateDesired = 0;
  pitchRateDesired = 0;
  yawRateDesired = 0;

  xTaskCreate(stabilizerTask,
              (const signed char * const) "STABILIZER",
              configMINIMAL_STACK_SIZE << 1,
              NULL,
              2, /*Piority*/
              NULL);

  isInit = TRUE;
}

bool stabilizerTest(void) {
  return motorsTest() & imu6Test() & sensfusion6Test() & controllerTest();
}

static void stabilizerTask(void *param) {
  uint32_t attitudeCounter = 0;
  uint32_t lastWakeTime;
  uint64_t lastUpdateTime;

  vTaskSetApplicationTaskTag(0, (void *)TASK_STABILIZER_ID_NBR);

  // Wait for the system to be fully started to start stabilization loop.
  systemWaitStart();

  lastWakeTime = xTaskGetTickCount();
  lastUpdateTime = usecTimestamp();

  while (1) {
    vTaskDelayUntil(&lastWakeTime, F2T(IMU_UPDATE_FREQ));

    imu6Read(&gyro, &acc);
    uint64_t imuReadTime = usecTimestamp();

    if (!imu6IsCalibrated()) {
      continue;
    }

    commanderGetRPY(&eulerRollDesired, &eulerPitchDesired, &eulerYawDesired);
    commanderGetRPYType(&rollType, &pitchType, &yawType);

    if (++attitudeCounter >= ATTITUDE_UPDATE_RATE_DIVIDER) {
      float dtSec = ((float)(imuReadTime - lastUpdateTime) / 1000000);
      lastUpdateTime = imuReadTime;
      sensfusion6UpdateQ(gyro.x, gyro.y, gyro.z, acc.x, acc.y, acc.z, dtSec);
      sensfusion6GetEulerRPY(&eulerRollActual, &eulerPitchActual, &eulerYawActual);

      controllerCorrectAttitudePID(eulerRollActual, eulerPitchActual,
                                   eulerYawActual, eulerRollDesired,
                                   eulerPitchDesired, -eulerYawDesired,
                                   &rollRateDesired, &pitchRateDesired,
                                   &yawRateDesired);
      attitudeCounter = 0;
    }

    if (!(rollType ^ RATE)) {
      rollRateDesired = eulerRollDesired;
    }
    if (!(pitchType ^ RATE)) {
      pitchRateDesired = eulerPitchDesired;
    }
    if (!(yawType ^ RATE)) {
      yawRateDesired = -eulerYawDesired;
    }

    // TODO: Investigate possibility to subtract gyro drift.
    controllerCorrectRatePID(gyro.x, -gyro.y, gyro.z, rollRateDesired,
                             pitchRateDesired, yawRateDesired);

    controllerGetActuatorOutput(&actuatorRoll, &actuatorPitch, &actuatorYaw);

    commanderGetTrust(&actuatorThrust);
    if (actuatorThrust) {
#if defined(TUNE_ROLL)
      distributePower(actuatorThrust, actuatorRoll, 0, 0);
#elif defined(TUNE_PITCH)
      distributePower(actuatorThrust, 0, actuatorPitch, 0);
#elif defined(TUNE_YAW)
      distributePower(actuatorThrust, 0, 0, -actuatorYaw);
#else
      distributePower(actuatorThrust, actuatorRoll, actuatorPitch,-actuatorYaw);
#endif
      continue;
    }
    distributePower(0, 0, 0, 0);
    controllerResetAllPID();
  }
}

static void distributePower(const uint16_t thrust, const int16_t roll,
                            const int16_t pitch, const int16_t yaw) {
#ifdef QUAD_FORMATION_X
  roll >>= 1;
  pitch >>= 1;
  motorPowerM1 = limitThrust(thrust - roll + pitch + yaw);
  motorPowerM2 = limitThrust(thrust - roll - pitch - yaw);
  motorPowerM3 = limitThrust(thrust + roll - pitch + yaw);
  motorPowerM4 = limitThrust(thrust + roll + pitch - yaw);
#else // QUAD_FORMATION_NORMAL
  motorPowerM1 = limitThrust(thrust + pitch + yaw);
  motorPowerM2 = limitThrust(thrust - roll - yaw);
  motorPowerM3 = limitThrust(thrust - pitch + yaw);
  motorPowerM4 = limitThrust(thrust + roll - yaw);
#endif

  motorsSetRatio(MOTOR_M1, motorPowerM1);
  motorsSetRatio(MOTOR_M2, motorPowerM2);
  motorsSetRatio(MOTOR_M3, motorPowerM3);
  motorsSetRatio(MOTOR_M4, motorPowerM4);
}
