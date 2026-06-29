#pragma once

#include "EchoPetDisplayDevice.h"
#include "EchoPetModel.h"
#include "EchoPetUi.h"

namespace echopet {

void drawEchoPet(EchoPetDisplayDevice& display, const Snapshot& pet,
                 uint8_t selectedMenuIndex, const UiState& ui,
                 uint8_t animationPhase);

#ifdef HOST_SCREEN_SIMULATOR
void drawEchoPetCatalogVisualProof(EchoPetDisplayDevice& display, bool souvenirs);
void drawEchoPetCharacterRosterProof(EchoPetDisplayDevice& display);
void drawEchoPetMenuIconProof(EchoPetDisplayDevice& display);
#endif

}  // namespace echopet
