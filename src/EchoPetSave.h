#pragma once

#include "EchoPetModel.h"

namespace echopet {

bool loadSave(PetSave& save);
bool writeSave(const PetSave& save);

}  // namespace echopet
