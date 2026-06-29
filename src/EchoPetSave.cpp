#include "EchoPetSave.h"

#include <stddef.h>
#include <string.h>

#include <Adafruit_LittleFS.h>
#include <InternalFileSystem.h>

namespace echopet {

namespace {

using Adafruit_LittleFS_Namespace::FILE_O_READ;
using Adafruit_LittleFS_Namespace::FILE_O_WRITE;
using Adafruit_LittleFS_Namespace::File;

constexpr const char* kSavePath = "/echopet.sav";
constexpr size_t kSaveHeaderSize = offsetof(PetSave, ageMinutes);
bool fsReady = false;

bool ensureFs() {
  if (fsReady) {
    return true;
  }
  fsReady = InternalFS.begin();
  return fsReady;
}

}  // namespace

bool loadSave(PetSave& save) {
  if (!ensureFs()) {
    return false;
  }

  File file(InternalFS);
  if (!file.open(kSavePath, FILE_O_READ)) {
    return false;
  }

  memset(&save, 0, sizeof(save));
  const int readCount = file.read(&save, sizeof(save));
  file.close();
  if (readCount < static_cast<int>(kSaveHeaderSize)) {
    return false;
  }
  if (save.size == 0 || save.size > sizeof(save)) {
    return false;
  }
  return readCount == static_cast<int>(save.size);
}

bool writeSave(const PetSave& save) {
  if (!ensureFs()) {
    return false;
  }

  File file(InternalFS);
  if (!file.open(kSavePath, FILE_O_WRITE)) {
    return false;
  }

  const size_t written =
      file.write(reinterpret_cast<const uint8_t*>(&save), sizeof(save));
  file.close();
  return written == sizeof(save);
}

}  // namespace echopet
