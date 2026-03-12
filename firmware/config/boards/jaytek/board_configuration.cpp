/**
 * @file boards/JAYTEK/board_configuration.cpp
 *
 * @brief Configuration defaults for the JAYTEK board
 *
 * @author Jason Roughley, (c) 2025
 */

#include "pch.h"
#include "jaytek_meta.h"
#include "board_overrides.h"

static const brain_pin_e injPins[] = {
    Gpio::JAYTEK_LS_1,
	Gpio::JAYTEK_LS_2,
	Gpio::JAYTEK_LS_3,
	Gpio::JAYTEK_LS_4,
	Gpio::JAYTEK_LS_5,
	Gpio::JAYTEK_LS_6,
	Gpio::JAYTEK_LS_7,
	Gpio::JAYTEK_LS_8,
	Gpio::JAYTEK_LS_9,
	Gpio::JAYTEK_LS_10,
	Gpio::JAYTEK_LS_11,
	Gpio::JAYTEK_LS_12
};

static const brain_pin_e ignPins[] = {
	Gpio::JAYTEK_IGN_1,
	Gpio::JAYTEK_IGN_2,
	Gpio::JAYTEK_IGN_3,
	Gpio::JAYTEK_IGN_4,
	Gpio::JAYTEK_IGN_5,
	Gpio::JAYTEK_IGN_6,
	Gpio::JAYTEK_IGN_7,
	Gpio::JAYTEK_IGN_8,
	Gpio::JAYTEK_IGN_9,
	Gpio::JAYTEK_IGN_10,
	Gpio::JAYTEK_IGN_11,
	Gpio::JAYTEK_IGN_12,
};

static void setInjectorPins() {
	copyArray(engineConfiguration->injectionPins, injPins);
}

static void setIgnitionPins() {
	copyArray(engineConfiguration->ignitionPins, ignPins);
}

// PE3 is error LED, configured in board.mk
Gpio getCommsLedPin() {
	return Gpio::C9;
}

Gpio getRunningLedPin() {
	return Gpio::A8;
}

Gpio getWarningLedPin() {
	return Gpio::C7;
}

static void setupVbatt() {
	// 5.6k high side/10k low side = 1.56 ratio divider
	engineConfiguration->analogInputDividerCoefficient = 1.56f;

  // 1.41V = 
  // 39k high side/8.2k low side -> 1.56 ->
	engineConfiguration->vbattDividerCoeff = 9.06f;

	// Battery sense on PB1
	engineConfiguration->vbattAdcChannel = EFI_ADC_9;

	engineConfiguration->adcVcc = 3.3f;
}

static void setupEtb() {
	// TLE9201 driver
	// This chip has three control pins:
	// DIR - sets direction of the motor
	// PWM - pwm control (enable high, coast low)
	// DIS - disables motor (enable low)

	// Throttle #1
	// PWM pin
	engineConfiguration->etbIo[0].controlPin = Gpio::D6;
	// DIR pin
	engineConfiguration->etbIo[0].directionPin1 = Gpio::D7;
	// Disable pin
	engineConfiguration->etbIo[0].disablePin = Gpio::D5;

	// Throttle #2
	// PWM pin
	engineConfiguration->etbIo[1].controlPin = Gpio::D3;
	// DIR pin
	engineConfiguration->etbIo[1].directionPin1 = Gpio::D4;
	// Disable pin
	engineConfiguration->etbIo[1].disablePin = Gpio::D2;

	// we only have pwm/dir, no dira/dirb
	engineConfiguration->etb_use_two_wires = false;
}

static void setupDefaultSensorInputs() {
	// trigger inputs
	// Digital channel 1 as default - others not set
	engineConfiguration->triggerInputPins[0] = JAYTEK_DIGITAL_1;
	engineConfiguration->camInputs[0] = Gpio::Unassigned;

	engineConfiguration->triggerInputPins[1] = Gpio::Unassigned;


	engineConfiguration->clt.adcChannel = JAYTEK_IN_CLT;
	engineConfiguration->iat.adcChannel = JAYTEK_IN_IAT;
	engineConfiguration->tps1_1AdcChannel = JAYTEK_IN_TPS;
	engineConfiguration->map.sensor.hwChannel = JAYTEK_IN_MAP;

    // see also enableAemXSeries
	// pin #28 WBO AFR "Analog Volt 10"
	engineConfiguration->afr.hwChannel = JAYTEK_IN_ANALOG_VOLT_10;
}

static void setupSdCard() {
	engineConfiguration->sdCardSpiDevice = SPI_DEVICE_1;
	engineConfiguration->sdCardCsPin = Gpio::A15;

	engineConfiguration->is_enabled_spi_1 = true;
	engineConfiguration->spi1sckPin = Gpio::B3;
	engineConfiguration->spi1misoPin = Gpio::B4;
	engineConfiguration->spi1mosiPin = Gpio::B5;
}

void jaytek_boardConfigOverrides() {
	setupSdCard();
	setupVbatt();

	engineConfiguration->clt.config.bias_resistor = JAYTEK_DEFAULT_AT_PULLUP;
	engineConfiguration->iat.config.bias_resistor = JAYTEK_DEFAULT_AT_PULLUP;

	engineConfiguration->canTxPin = Gpio::D1;
	engineConfiguration->canRxPin = Gpio::D0;
	engineConfiguration->can2RxPin = Gpio::B12;
	engineConfiguration->can2TxPin = Gpio::B13;

  engineConfiguration->is_enabled_spi_2 = false;
	engineConfiguration->spi2sckPin = Gpio::B10;
	engineConfiguration->spi2misoPin = Gpio::B14;
	engineConfiguration->spi2mosiPin = Gpio::B15;

	engineConfiguration->lps25BaroSensorScl = Gpio::B6;
	engineConfiguration->lps25BaroSensorSda = Gpio::B7;
}

/**
 * @brief   Board-specific configuration defaults.
 *
 * See also setDefaultEngineConfiguration
 *

 */
void jaytek_boardDefaultConfiguration() {
	setInjectorPins();
	setIgnitionPins();
	setupEtb();

	engineConfiguration->isSdCardEnabled = true;

	// "required" hardware is done - set some reasonable defaults
	setupDefaultSensorInputs();

	engineConfiguration->enableSoftwareKnock = true;

#if HW_JAYTEK & EFI_PROD_CODE
	engineConfiguration->mainRelayPin = Gpio::JAYTEK_LS_12;
	engineConfiguration->fanPin = Gpio::JAYTEK_LS_11;
	engineConfiguration->fuelPumpPin = Gpio::JAYTEK_LS_10;
#endif // HW_JAYTEK

	// If we're running as hardware CI, borrow a few extra pins for that
#ifdef HARDWARE_CI
	engineConfiguration->triggerSimulatorPins[0] = Gpio::G3;
	engineConfiguration->triggerSimulatorPins[1] = Gpio::G2;
#endif
}

void boardPrepareForStop() {
	// Wake on the CAN RX pin
	palEnableLineEvent(PAL_LINE(GPIOD, 0), PAL_EVENT_MODE_RISING_EDGE);
}

#if HW_JAYTEK
static Gpio JAYTEK_SLINGSHOT_OUTPUTS[] = {
    Gpio::JAYTEK_LS_1, // inj 1
    Gpio::JAYTEK_LS_2, // inj 2
    Gpio::JAYTEK_LS_3, // inj 3
    Gpio::JAYTEK_LS_4, // inj 4
};

static Gpio JAYTEK_SBC_OUTPUTS[] = {
    Gpio::JAYTEK_LS_14, // inj 1 four times
    Gpio::JAYTEK_LS_14, // inj 1 four times
    Gpio::JAYTEK_LS_14, // inj 1 four times
    Gpio::JAYTEK_LS_14, // inj 1 four times

    Gpio::JAYTEK_LS_15, // inj 4 four times
    Gpio::JAYTEK_LS_15, // inj 4 four times
    Gpio::JAYTEK_LS_15, // inj 4 four times
    Gpio::JAYTEK_LS_15, // inj 4 four times

};

static Gpio JAYTEK_M73_OUTPUTS[] = {
    Gpio::JAYTEK_LS_1, // inj 1
    Gpio::JAYTEK_LS_2, // inj 2
    Gpio::JAYTEK_LS_3,
    Gpio::JAYTEK_LS_4,
    Gpio::JAYTEK_LS_5,
    Gpio::JAYTEK_LS_6,
    Gpio::JAYTEK_LS_7,
    Gpio::JAYTEK_LS_8,
    Gpio::JAYTEK_LS_9, // inj 9
    Gpio::JAYTEK_LS_10, // inj 10
    Gpio::JAYTEK_LS_11, // inj 11
    Gpio::JAYTEK_LS_12, // inj 12
    Gpio::JAYTEK_LS_14, // starter control or aux output
    Gpio::JAYTEK_LS_15, // radiator fan relay output white


    //Gpio::JAYTEK_LS_13, // main relay
    //Gpio::JAYTEK_LS_16, // main relay
};

static Gpio JAYTEK_SUBARU_OUTPUTS[] = {
    Gpio::JAYTEK_LS_1, // inj 1
    Gpio::JAYTEK_LS_2, // inj 2
    Gpio::JAYTEK_LS_3, // inj 3
    Gpio::JAYTEK_LS_4, // inj 4
    Gpio::JAYTEK_LS_12, // main relay
    Gpio::JAYTEK_LS_14, // starter
};

static Gpio JAYTEK_CANAM_OUTPUTS[] = {
    Gpio::JAYTEK_LS_1, // inj 1
    Gpio::JAYTEK_LS_2, // inj 2
    Gpio::JAYTEK_LS_3, // inj 3
    Gpio::JAYTEK_LS_12, // main relay
    Gpio::JAYTEK_LS_14, // starter
    Gpio::JAYTEK_LS_15, // intercooler fan
    Gpio::JAYTEK_LS_4, // accessories relay
	Gpio::JAYTEK_IGN_1,
	Gpio::JAYTEK_IGN_2,
	Gpio::JAYTEK_IGN_3,
};

static Gpio JAYTEK_HARLEY_OUTPUTS[] = {
    Gpio::JAYTEK_LS_1,
    Gpio::JAYTEK_LS_2,
	Gpio::JAYTEK_IGN_1,
	Gpio::JAYTEK_IGN_2,
	Gpio::JAYTEK_IGN_8, // ACR
	Gpio::JAYTEK_IGN_9, // ACR2
};

int getBoardMetaLowSideOutputsCount() {
    if (engineConfiguration->engineType == engine_type_e::SUBARU_2011) {
        return getBoardMetaOutputsCount();
    }
    if (engineConfiguration->engineType == engine_type_e::MAVERICK_X3) {
        return getBoardMetaOutputsCount();
    }
    if (engineConfiguration->engineType == engine_type_e::HARLEY) {
        return getBoardMetaOutputsCount();
    }
    if (engineConfiguration->engineType == engine_type_e::GM_SBC) {
        return getBoardMetaOutputsCount();
    }
    if (engineConfiguration->engineType == engine_type_e::ME17_9_MISC) {
        return getBoardMetaOutputsCount();
    }
    return 16;
}

static Gpio JAYTEK_OUTPUTS[] = {
Gpio::JAYTEK_LS_1,
Gpio::JAYTEK_LS_2,
Gpio::JAYTEK_LS_3,
Gpio::JAYTEK_LS_4,
Gpio::JAYTEK_LS_5,
Gpio::JAYTEK_LS_6,
Gpio::JAYTEK_LS_7,
Gpio::JAYTEK_LS_8,
Gpio::JAYTEK_LS_9,
Gpio::JAYTEK_LS_10,
Gpio::JAYTEK_LS_11,
Gpio::JAYTEK_LS_12,
Gpio::JAYTEK_LS_13,
Gpio::JAYTEK_LS_14,
Gpio::JAYTEK_LS_15,
Gpio::JAYTEK_LS_16,
Gpio::JAYTEK_LS_17,
Gpio::JAYTEK_LS_18,
Gpio::JAYTEK_LS_19,
Gpio::JAYTEK_LS_20,
Gpio::JAYTEK_LS_21,
Gpio::JAYTEK_LS_22,
	Gpio::JAYTEK_IGN_1,
	Gpio::JAYTEK_IGN_2,
	Gpio::JAYTEK_IGN_3,
	Gpio::JAYTEK_IGN_4,
	Gpio::JAYTEK_IGN_5,
	Gpio::JAYTEK_IGN_6,
	Gpio::JAYTEK_IGN_7,
	Gpio::JAYTEK_IGN_8,
	Gpio::JAYTEK_IGN_9,
	Gpio::JAYTEK_IGN_10,
	Gpio::JAYTEK_IGN_11,
	Gpio::JAYTEK_IGN_12,
	Gpio::JAYTEK_HS_1,
	Gpio::JAYTEK_HS_2,
	Gpio::JAYTEK_HS_3,
	Gpio::JAYTEK_HS_4,
	Gpio::JAYTEK_HS_5,
	Gpio::JAYTEK_HS_6,
	Gpio::JAYTEK_HS_7,
	Gpio::JAYTEK_HS_8
};

int getBoardMetaOutputsCount() {
    if (engineConfiguration->engineType == engine_type_e::SUBARU_2011) {
        return efi::size(JAYTEK_SUBARU_OUTPUTS);
    }
    if (engineConfiguration->engineType == engine_type_e::MAVERICK_X3) {
        return efi::size(JAYTEK_CANAM_OUTPUTS);
    }
    if (engineConfiguration->engineType == engine_type_e::ME17_9_MISC) {
        return efi::size(JAYTEK_SLINGSHOT_OUTPUTS);
    }
    if (engineConfiguration->engineType == engine_type_e::HARLEY) {
        return efi::size(JAYTEK_HARLEY_OUTPUTS);
    }
    if (engineConfiguration->engineType == engine_type_e::GM_SBC) {
        return efi::size(JAYTEK_SBC_OUTPUTS);
    }
    if (engineConfiguration->engineType == engine_type_e::PROTEUS_BMW_M73) {
        return efi::size(JAYTEK_M73_OUTPUTS);
    }
    return efi::size(JAYTEK_OUTPUTS);
}

int getBoardMetaDcOutputsCount() {
    if (engineConfiguration->engineType == engine_type_e::PROTEUS_BMW_M73) {
        return 2;
    }
    if (engineConfiguration->engineType == engine_type_e::ME17_9_MISC ||
        engineConfiguration->engineType == engine_type_e::HARLEY ||
        engineConfiguration->engineType == engine_type_e::SUBARU_2011 ||
        engineConfiguration->engineType == engine_type_e::MAVERICK_X3
        ) {
        return 1;
    }
    return 1;
/*    return 2; JAYTEK has two h-b ridges but stim board is short on channels to test :( */
}

Gpio* getBoardMetaOutputs() {
    if (engineConfiguration->engineType == engine_type_e::SUBARU_2011) {
        return JAYTEK_SUBARU_OUTPUTS;
    }
    if (engineConfiguration->engineType == engine_type_e::MAVERICK_X3) {
        return JAYTEK_CANAM_OUTPUTS;
    }
    if (engineConfiguration->engineType == engine_type_e::ME17_9_MISC) {
        return JAYTEK_SLINGSHOT_OUTPUTS;
    }
    if (engineConfiguration->engineType == engine_type_e::HARLEY) {
        return JAYTEK_HARLEY_OUTPUTS;
    }
    if (engineConfiguration->engineType == engine_type_e::GM_SBC) {
        return JAYTEK_SBC_OUTPUTS;
    }
    if (engineConfiguration->engineType == engine_type_e::PROTEUS_BMW_M73) {
        return JAYTEK_M73_OUTPUTS;
    }
    return JAYTEK_OUTPUTS;
}
#endif // HW_JAYTEK


void setup_custom_board_overrides() {
	custom_board_DefaultConfiguration = jaytek_boardDefaultConfiguration;
	custom_board_ConfigOverrides =  jaytek_boardConfigOverrides;
}
