// Copyright 2023 gamaPhy (https://gamaphy.com/)
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include_next <mcuconf.h>

#undef RP_PWM_USE_PWM6
#define RP_PWM_USE_PWM6 TRUE

#undef RP_ADC_USE_ADC1
#define RP_ADC_USE_ADC1 TRUE
