﻿/*
    Egaroucid Project

    @date 2021-2023
    @author Takuto Yamana
    @license GPL-3.0 license
*/

#pragma once
#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "./../../engine/setting.hpp"

#define EGAROUCID_GUI_VERSION "0"
#if USE_BETA_VERSION
    #if GUI_PORTABLE_MODE
        const String EGAROUCID_VERSION = Unicode::Widen(EGAROUCID_ENGINE_VERSION + (std::string)"." + EGAROUCID_GUI_VERSION + (std::string)" " + EGAROUCID_ENGINE_ENV_VERSION + (std::string)" Portable" + (std::string)" beta");
    #else
        const String EGAROUCID_VERSION = Unicode::Widen(EGAROUCID_ENGINE_VERSION + (std::string)"." + EGAROUCID_GUI_VERSION + (std::string)" " + EGAROUCID_ENGINE_ENV_VERSION + (std::string)" beta");
    #endif
#else
    #if GUI_PORTABLE_MODE
        const String EGAROUCID_VERSION = Unicode::Widen(EGAROUCID_ENGINE_VERSION + (std::string)"." + EGAROUCID_GUI_VERSION + (std::string)" " + EGAROUCID_ENGINE_ENV_VERSION + (std::string)" Portable");
    #else
        const String EGAROUCID_VERSION = Unicode::Widen(EGAROUCID_ENGINE_VERSION + (std::string)"." + EGAROUCID_GUI_VERSION + (std::string)" " + EGAROUCID_ENGINE_ENV_VERSION);
    #endif
#endif

const String EGAROUCID_NUM_VERSION = Unicode::Widen(EGAROUCID_ENGINE_VERSION + (std::string)"." + EGAROUCID_GUI_VERSION);