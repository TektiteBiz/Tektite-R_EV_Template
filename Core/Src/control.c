/*
 * control.c
 *
 *  Created on: Aug 11, 2024
 *      Author: nv
 */


#include "control.h"

float ticks_per_m; // 48ticks/rev = 300.8, 24ticks/rev = 150.4, 12ticks/rev = 75.2, address: 0x0802 0000
float dist; // Must be >1.5 (8.53 m = 28ft), address: 0x0802 0004
float accel_dist; // meters
float decel_dist; // meters

void EStop() {
	while (STOPPressed()) {
		ESCWrite(1.5);
		LEDWrite(255, 0, 0);
		HAL_GPIO_WritePin(BOOSTON_GPIO_Port, BOOSTON_Pin, GPIO_PIN_RESET);
	}
}

int Distance() {
	return abs(M1Ticks) + abs(M2Ticks);
}

void Go() {
	HAL_GPIO_WritePin(BOOSTON_GPIO_Port, BOOSTON_Pin, GPIO_PIN_SET);

	ESCWrite(1.5);
	LEDWrite(0, 255, 0);
	while (GOPressed()) {
		EncoderReset();
	}
	EncoderReset();

	// CHANGE THIS PART

	/* Documentation:
	 *
	 * Functions:
	 * 	use Distance() to get the distance travelled in ticks, NOT METERS
	 * 	use ESCWrite(<ms>) to write a PWM frequency to the ESC
	 * 	use LEDWrite(r, g, b) to write to the LEDs, where r, g, and b are integers from 0-255
	 *
	 * Variables:
	 * 	accel_dist has the acceleration distance in meters
	 * 	decel_dist has the deceleration distance in meters
	 * 	dist has the total distance in meters
	 *
	 * Communicating with the ESC:
	 * 	ESCWrite(1.5) will be zero power (1500us)
	 * 	ESCWrite(1.9) will be full speed (1900us)
	 * 	ESCWrite(1.6) will be the minimum speed (1600us)
	 * 	ESCWrite(1.1) will trigger motor braking (1100us)
	 *
	 * 	You can interpolate between 1.5 and 1.9 for the acceleration curve!
	 *
	 * 	I've outlined the code below:
	 */

	// TODO: Implement accelerate with a square root curve for accel_dist
	while (true /* change this */) {
		// TODO: Put an ESCWrite and LEDWrite statement here

		EncoderUpdate();
		if (STOPPressed()) {
			EStop();
			return;
		}
	}

	// TODO: Maintain full speed until decel_dist away from end
	while (true /* change this */) {
		// TODO: Put ESCWrite and LEDWrite statement here

		EncoderUpdate();
		if (STOPPressed()) {
			EStop();
			return;
		}
	}

	// TODO: Reduce speed to minimum speed and keep going until the end
	while (true /* change this */) {
		// TODO: Put ESCWrite and LEDWrite statement here

		EncoderUpdate();
		if (STOPPressed()) {
			EStop();
			return;
		}
	}

	// TODO: Stop! (put ESCWrite and LEDWrite statement here)

	HAL_Delay(500);
}

void DataAvailable() {
	dataAvailable = false;
	LEDWrite(0, 0, 255);
	printf("Entering programming mode.\n");
	HAL_Delay(1);
	printf("Enter 'd' to change the distance (currently %.3f meters)\n", dist);
	HAL_Delay(1);
	printf("Enter 't' to change the ticks per meter (currently %.1f)\n", ticks_per_m);
	HAL_Delay(1);
	printf("Enter 'b' to read the battery voltage (make sure the jumper is in the ON position)\n");
	HAL_Delay(1);
	printf("Enter 'a' to change the acceleration distance (currently %.3f meters)\n", accel_dist);
	HAL_Delay(1);
	printf("Enter 'e' to change the deceleration distance (currently %.3f meters)\n", decel_dist);
	HAL_Delay(1);
	printf("Enter 'f' to finish\n");
	bool finish = false;
	while (!finish) {
		while (!dataAvailable) {
			HAL_Delay(1);
		}
		switch (data[0]) {
		case 'd':
			memset(data, '0', sizeof(data));
			printf("Enter the distance as 5 characters, must be over 1.500. Example: 7.000\n");
			dataAvailable = false;
			while (!dataAvailable) {
				HAL_Delay(1);
			}
			dist = atof(data);
			DataWrite();
			printf("Saved distance as %.3f meters.\n", dist);
			break;

		case 't':
			memset(data, '0', sizeof(data));
			printf("Enter the ticks per meter as 5 characters. Example: 150.4\n");
			dataAvailable = false;
			while (!dataAvailable) {
				HAL_Delay(1);
			}
			data[5] = 0;

			ticks_per_m = atof(data);
			DataWrite();
			printf("Saved ticks per meter as %.1f.\n", ticks_per_m);
			break;

		case 'a':
			memset(data, '0', sizeof(data));
			printf("Enter the acceleration distance as 5 characters. Example: 0.500\n");
			dataAvailable = false;
			while (!dataAvailable) {
				HAL_Delay(1);
			}
			data[5] = 0;

			accel_dist = atof(data);
			DataWrite();
			printf("Saved acceleration distance per meter as %.3f.\n", accel_dist);
			break;

		case 'e':
			memset(data, '0', sizeof(data));
			printf("Enter the deceleration distance as 5 characters. Example: 1.500\n");
			dataAvailable = false;
			while (!dataAvailable) {
				HAL_Delay(1);
			}
			data[5] = 0;

			decel_dist = atof(data);
			DataWrite();
			printf("Saved deceleration distance per meter as %.3f.\n", decel_dist);
			break;

		case 'f':
			printf("Finished.\n");
			finish = true;
			break;

		case 'b':
			printf("Battery voltage: %0.2f (Recommended minimum of 10V)\n", BattVoltage());
			break;

		default:
			printf("Invalid character. Enter 'd', 't', 'b', 'a', 'e', or 'f'.\n");
			break;
		}
		dataAvailable = false;
	}
}

void DataWrite() {
	printf("Clearing and writing to sector 5...\n");

	static FLASH_EraseInitTypeDef EraseInitStruct;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector = FLASH_SECTOR_5;
	EraseInitStruct.NbSectors = 1;
	uint32_t SECTORError;

	HAL_FLASH_Unlock();
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError) != HAL_OK) {
		HAL_FLASH_Lock();
		while (1) {printf("F\n");};
		Error("FLASH Failure");
		return;
	}

	uint32_t tpm;
	uint32_t dst;
	uint32_t ad;
	uint32_t dd;
	memcpy(&tpm, &ticks_per_m, sizeof(ticks_per_m));
	memcpy(&dst, &dist, sizeof(dist));
	memcpy(&ad, &accel_dist, sizeof(accel_dist));
	memcpy(&dd, &decel_dist, sizeof(decel_dist));

	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08020000, tpm) != HAL_OK) {
		HAL_FLASH_Lock();
		Error("FLASH Failure");
		return;
	}
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08020004, dst) != HAL_OK) {
		HAL_FLASH_Lock();
		Error("FLASH Failure");
		return;
	}
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08020008, 1) != HAL_OK) { // Initialized byte
		HAL_FLASH_Lock();
		Error("FLASH Failure");
		return;
	}
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x0802000C, ad) != HAL_OK) {
		HAL_FLASH_Lock();
		Error("FLASH Failure");
		return;
	}
	if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08020010, dd) != HAL_OK) {
		HAL_FLASH_Lock();
		Error("FLASH Failure");
		return;
	}

	HAL_FLASH_Lock();
}

void DataInit() {
	uint32_t tpm = *(__IO uint32_t *)(0x08020000);
	uint32_t dst = *(__IO uint32_t *)(0x08020004);
	uint32_t ad = *(__IO uint32_t *)(0x0802000C);
	uint32_t dd = *(__IO uint32_t *)(0x08020010);
	memcpy(&ticks_per_m, &tpm, sizeof(tpm));
	memcpy(&dist, &dst, sizeof(dst));
	memcpy(&accel_dist, &ad, sizeof(ad));
	memcpy(&decel_dist, &dd, sizeof(dd));

	uint32_t initialized = *(__IO uint32_t *)(0x08020008);
	if (initialized != 1) {
		LEDWrite(255, 0, 0);
		ticks_per_m = 150.4;
		dist = 7.0f;
		accel_dist = 0.5;
		decel_dist = 1.5;
		DataWrite();
	}
}
