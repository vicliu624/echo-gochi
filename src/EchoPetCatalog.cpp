#include "EchoPetCatalog.h"

#include <stdio.h>

namespace echopet {

namespace {

struct CatalogSegment {
  uint8_t start;
  uint8_t end;
  const char* prefix;
  CatalogKind kind;
  uint8_t behaviorBase;
  uint8_t behaviorModulo;
  uint16_t basePrice;
  uint8_t priceBase;
  uint8_t priceModulo;
  uint8_t priceStep;
  uint8_t flags;
  uint8_t sourceGroup;
  uint8_t useScene;
};

struct CatalogOverride {
  uint8_t index;
  const char* label;
  CatalogKind kind;
  uint8_t behavior;
  uint16_t price;
  uint8_t icon;
  uint8_t flags;
  uint8_t sourceGroup;
  uint8_t useScene;
};

struct ShopRestockWindow {
  uint16_t startMinute;
  uint8_t period;
};

struct ShopVendorWindow {
  uint16_t startMinute;
  uint16_t endMinute;
  uint8_t visitIndex;
};

struct ShopSaleRule {
  uint8_t dayMultiplier;
  uint8_t dayOffset;
  uint8_t modulo;
  uint8_t pricePercent;
};

const CatalogSegment kCatalogSegments[] = {
    {0, 24, "MEAL", CatalogKind::kFood, 0, 8, 60, 0, 12, 10, 0x00, 1,
     kCatalogUseMeal},
    {24, 48, "SNACK", CatalogKind::kFood, 0, 8, 60, 0, 12, 10,
     kCatalogFlagSnack, 2, kCatalogUseSnack},
    {48, 72, "TOY", CatalogKind::kItem, 48, 8, 100, 48, 24, 20, 0x00, 3,
     kCatalogUsePlay},
    {72, 96, "STUDY", CatalogKind::kItem, 48, 8, 100, 48, 24, 20, 0x00, 4,
     kCatalogUseStudy},
    {96, 120, "MUSIC", CatalogKind::kItem, 48, 8, 100, 48, 24, 20, 0x00, 5,
     kCatalogUseMusic},
    {120, 144, "TRAVEL", CatalogKind::kItem, 48, 8, 100, 48, 24, 20,
     kCatalogFlagTravel, 6, kCatalogUseTravel},
    {144, 152, "SECRET", CatalogKind::kItem, 144, 8, 500, 144, 255, 50,
     kCatalogFlagSecret, 7, kCatalogUsePlay},
    {152, 160, "MEMORY", CatalogKind::kSouvenir, 152, kCatalogSouvenirCount,
     500, 144, 255, 50, kCatalogFlagSecret, 8, kCatalogUseMemory},
};

const CatalogOverride kCatalogOverrides[] = {
    {0, "APPLE PIE", CatalogKind::kFood, 6, 150, 6, kCatalogFlagSnack, 10, 2},
    {1, "BANANA", CatalogKind::kFood, 7, 130, 7, kCatalogFlagSnack, 10, 2},
    {2, "BBQ", CatalogKind::kFood, 1, 180, 1, 0x00, 10, 1},
    {3, "BEEF BOWL", CatalogKind::kFood, 1, 180, 1, 0x00, 10, 1},
    {4, "CHEESE", CatalogKind::kFood, 3, 100, 3, 0x00, 10, 1},
    {5, "CHEESE CAKE", CatalogKind::kFood, 6, 150, 6,
     kCatalogFlagSnack, 10, 2},
    {6, "CHERRY", CatalogKind::kFood, 7, 90, 7, kCatalogFlagSnack, 10, 2},
    {7, "CHOCOLATE", CatalogKind::kFood, 5, 150, 5,
     kCatalogFlagSnack, 10, 2},
    {8, "COOKIE", CatalogKind::kFood, 6, 150, 6, kCatalogFlagSnack, 10, 2},
    {9, "CORN", CatalogKind::kFood, 3, 80, 3, 0x00, 10, 1},
    {10, "CORN DOG", CatalogKind::kFood, 4, 100, 4,
     kCatalogFlagSnack, 10, 2},
    {11, "CUPCAKE", CatalogKind::kFood, 6, 120, 6,
     kCatalogFlagSnack, 10, 2},
    {12, "CURRY", CatalogKind::kFood, 1, 100, 1, 0x00, 10, 1},
    {13, "DONUT", CatalogKind::kFood, 6, 110, 6, kCatalogFlagSnack, 10, 2},
    {14, "FRIES", CatalogKind::kFood, 4, 80, 4, kCatalogFlagSnack, 10, 2},
    {15, "HAMBURGER", CatalogKind::kFood, 1, 180, 1, 0x00, 10, 1},
    {16, "HOT DOG", CatalogKind::kFood, 1, 150, 1, 0x00, 10, 1},
    {17, "MELON", CatalogKind::kFood, 7, 500, 7, kCatalogFlagSnack, 10, 2},
    {18, "NOODLE", CatalogKind::kFood, 2, 100, 2, 0x00, 10, 1},
    {19, "OMELET", CatalogKind::kFood, 3, 200, 3, 0x00, 10, 1},
    {20, "PASTA", CatalogKind::kFood, 2, 200, 2, 0x00, 10, 1},
    {21, "PEAR", CatalogKind::kFood, 7, 70, 7, kCatalogFlagSnack, 10, 2},
    {22, "PINEAPPLE", CatalogKind::kFood, 7, 140, 7,
     kCatalogFlagSnack, 10, 2},
    {23, "ROLL CAKE", CatalogKind::kFood, 6, 120, 6,
     kCatalogFlagSnack, 10, 2},
    {24, "ICE CREAM", CatalogKind::kFood, 5, 120, 5, kCatalogFlagSnack, 10, 2},
    {25, "SODA", CatalogKind::kFood, 5, 120, 5, kCatalogFlagSnack, 10, 2},
    {26, "WAFFLE", CatalogKind::kFood, 6, 0, 6, kCatalogFlagSnack, 10, 2},
    {27, "SUNDAE", CatalogKind::kFood, 5, 130, 5, kCatalogFlagSnack, 10, 2},
    {28, "SHAVED ICE", CatalogKind::kFood, 5, 530, 5, kCatalogFlagSnack, 10, 2},
    {29, "PIZZA", CatalogKind::kFood, 1, 140, 1, 0x00, 10, 1},
    {30, "SWEET POTATO", CatalogKind::kFood, 0, 600, 0, kCatalogFlagSnack, 10, 2},
    {31, "MILK", CatalogKind::kFood, 3, 80, 3, 0x00, 10, 1},
    {32, "JUICE", CatalogKind::kFood, 5, 80, 5, kCatalogFlagSnack, 10, 2},
    {33, "GRAPES", CatalogKind::kFood, 7, 160, 7, kCatalogFlagSnack, 10, 2},
    {34, "CANDY", CatalogKind::kFood, 5, 0, 5, kCatalogFlagSnack, 10, 2},
    {35, "POPCORN", CatalogKind::kFood, 4, 90, 4, kCatalogFlagSnack, 10, 2},
    {36, "GUM", CatalogKind::kFood, 5, 100, 5, kCatalogFlagSnack, 10, 2},
    {37, "DANGO", CatalogKind::kFood, 6, 100, 6, kCatalogFlagSnack, 10, 2},
    {38, "CREAM PUFF", CatalogKind::kFood, 6, 150, 6, kCatalogFlagSnack, 10, 2},
    {39, "ENERGY DRINK", CatalogKind::kFood, 5, 130, 5, kCatalogFlagSnack, 10, 2},
    {40, "SAUSAGE", CatalogKind::kFood, 1, 100, 1, 0x00, 10, 1},
    {41, "SAUSAGE", CatalogKind::kFood, 1, 110, 1, 0x00, 10, 1},
    {42, "CHIKUWA", CatalogKind::kFood, 1, 130, 1, 0x00, 10, 1},
    {43, "ESCARGOT", CatalogKind::kFood, 1, 900, 1, 0x00, 10, 1},
    {44, "MARRON CAKE", CatalogKind::kFood, 6, 520, 6, kCatalogFlagSnack, 10, 2},
    {45, "RICE BALL", CatalogKind::kFood, 0, 100, 0, 0x00, 10, 1},
    {46, "SANDWICH", CatalogKind::kFood, 2, 160, 2, 0x00, 10, 1},
    {47, "TACO", CatalogKind::kFood, 1, 150, 1, 0x00, 10, 1},
    {48, "PENCIL", CatalogKind::kItem, 1, 110, 1,
     kCatalogFlagAdultOnly, 10, 4},
    {49, "PLANT", CatalogKind::kItem, 2, 80, 2, 0x00, 10, 3},
    {50, "UMBRELLA", CatalogKind::kItem, 2, 500, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {51, "SUNGLASSES", CatalogKind::kItem, 2, 800, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {52, "BOW", CatalogKind::kItem, 2, 500, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {53, "DRUM", CatalogKind::kItem, 6, 1600, 6,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     5},
    {54, "SHOVEL", CatalogKind::kItem, 0, 80, 0, 0x00, 10, 3},
    {55, "ROLLER BLADES", CatalogKind::kItem, 0, 1800, 0,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {56, "BALLOON", CatalogKind::kItem, 0, 350, 0,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagChildPlus), 10,
     3},
    {57, "BOW TIE", CatalogKind::kItem, 2, 700, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {58, "CAP", CatalogKind::kItem, 2, 700, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {59, "MUSIC", CatalogKind::kItem, 6, 1500, 6,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     5},
    {60, "BALL", CatalogKind::kItem, 3, 200, 3,
     kCatalogFlagReusable, 10, 3},
    {61, "TICKET 2", CatalogKind::kItem, 7, 2000, 7,
     static_cast<uint8_t>(kCatalogFlagTravel | kCatalogFlagAdultOnly), 10, 6},
    {62, "ACTION FIG", CatalogKind::kItem, 0, 1400, 0,
     kCatalogFlagReusable, 10, 3},
    {63, "BALLOON", CatalogKind::kItem, 0, 350, 0,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagChildPlus), 10,
     3},
    {64, "BLDG BLOCK", CatalogKind::kItem, 4, 800, 4,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagTeenPlus), 10,
     3},
    {65, "BOOM BOX", CatalogKind::kItem, 6, 1400, 6,
     kCatalogFlagReusable, 10, 5},
    {66, "CHEST", CatalogKind::kItem, 2, 200, 2, 0x00, 10, 3},
    {67, "DARTS", CatalogKind::kItem, 0, 100, 0,
     kCatalogFlagAdultOnly, 10, 3},
    {68, "DOLL 1", CatalogKind::kItem, 0, 1400, 0,
     kCatalogFlagReusable, 10, 3},
    {69, "FISHING POLE", CatalogKind::kItem, 7, 400, 7, 0x00, 10, 3},
    {70, "LAMP", CatalogKind::kItem, 2, 700, 2,
     kCatalogFlagReusable, 10, 3},
    {71, "MIRROR", CatalogKind::kItem, 2, 3000, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {72, "MUSIC DISC", CatalogKind::kItem, 6, 500, 6, 0x00, 10, 5},
    {73, "PHONOGRAPH", CatalogKind::kItem, 6, 5000, 6,
     kCatalogFlagReusable, 10, 5},
    {74, "RC CAR 1", CatalogKind::kItem, 0, 1000, 0,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {75, "RC CAR 2", CatalogKind::kItem, 0, 1500, 0,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {76, "ROPE", CatalogKind::kItem, 5, 550, 5,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagTeenPlus), 10,
     3},
    {77, "SHIRT", CatalogKind::kItem, 2, 1000, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {78, "SHOES", CatalogKind::kItem, 0, 1900, 0,
     kCatalogFlagAdultOnly, 10, 3},
    {79, "THRONE", CatalogKind::kItem, 2, 5500, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     3},
    {80, "TICKET 1", CatalogKind::kItem, 7, 5800, 7,
     static_cast<uint8_t>(kCatalogFlagTravel | kCatalogFlagAdultOnly), 10, 6},
    {81, "TICKET 3", CatalogKind::kItem, 7, 4400, 7,
     static_cast<uint8_t>(kCatalogFlagTravel | kCatalogFlagAdultOnly), 10, 6},
    {82, "TICKET 4", CatalogKind::kItem, 7, 5000, 7,
     static_cast<uint8_t>(kCatalogFlagTravel | kCatalogFlagAdultOnly), 10, 6},
    {83, "TICKET 5", CatalogKind::kItem, 7, 2200, 7,
     static_cast<uint8_t>(kCatalogFlagTravel | kCatalogFlagAdultOnly), 10, 6},
    {84, "TRUMPET", CatalogKind::kItem, 6, 1800, 6,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     5},
    {85, "TURKEY", CatalogKind::kFood, 1, 600, kCatalogFoodIconTurkey, 0x00,
     10, kCatalogUseMeal},
    {86, "WHOLE CAKE", CatalogKind::kFood, 6, 600, kCatalogFoodIconCake,
     kCatalogFlagSnack, 10, kCatalogUseSnack},
    {87, "YOGURT", CatalogKind::kFood, 5, 550, kCatalogFoodIconYogurt,
     kCatalogFlagSnack, 10, kCatalogUseSnack},
    {88, "TV", CatalogKind::kItem, 1, 5000, kCatalogItemIconTv,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     kCatalogUseStudy},
    {89, "WEIGHTS", CatalogKind::kItem, 0, 600, kCatalogItemIconWeights,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     kCatalogUsePlay},
    {90, "WIG", CatalogKind::kItem, 2, 1000, kCatalogItemIconWig,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     kCatalogUsePlay},
    {91, "WINGS", CatalogKind::kItem, 2, 1000, kCatalogItemIconWings,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     kCatalogUsePlay},
    {92, "DOLL 2", CatalogKind::kItem, 0, 1400, kCatalogItemIconDoll,
     kCatalogFlagReusable, 10, kCatalogUsePlay},
    {93, "MAKE-UP", CatalogKind::kItem, 2, 1200, kCatalogItemIconMakeup,
     kCatalogFlagAdultOnly, 10, kCatalogUsePlay},
    {94, "SHAVER", CatalogKind::kItem, 2, 400, kCatalogItemIconShaver,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 10,
     kCatalogUsePlay},
    {95, "TAMA DRINK", CatalogKind::kItem, 2, 1200,
     kCatalogItemIconTamaDrink,
     kCatalogFlagAdultOnly, 10, kCatalogUsePlay},
    {96, "ICE CREAM", CatalogKind::kFood, 5, 120, 5,
     kCatalogFlagSnack, 11, kCatalogUseSnack},
    {97, "SODA", CatalogKind::kFood, 5, 120, 5, kCatalogFlagSnack, 11,
     kCatalogUseSnack},
    {98, "WAFFLE", CatalogKind::kFood, 6, 0, 6, kCatalogFlagSnack, 11,
     kCatalogUseSnack},
    {99, "SUNDAE", CatalogKind::kFood, 5, 130, 5, kCatalogFlagSnack, 11,
     kCatalogUseSnack},
    {100, "SHAVED ICE", CatalogKind::kFood, 5, 530, 5,
     kCatalogFlagSnack, 11, kCatalogUseSnack},
    {101, "PIZZA", CatalogKind::kFood, 1, 140, 1, 0x00, 11,
     kCatalogUseMeal},
    {102, "SWEET POTATO", CatalogKind::kFood, 0, 600, 0,
     kCatalogFlagSnack, 11, kCatalogUseSnack},
    {103, "MILK", CatalogKind::kFood, 3, 80, 3, 0x00, 11,
     kCatalogUseMeal},
    {104, "FRUIT JUICE", CatalogKind::kFood, 5, 80, 5,
     kCatalogFlagSnack, 11, kCatalogUseSnack},
    {105, "GRAPES", CatalogKind::kFood, 7, 160, 7, kCatalogFlagSnack, 11,
     kCatalogUseSnack},
    {106, "CANDY", CatalogKind::kFood, 5, 0, 5, kCatalogFlagSnack, 11,
     kCatalogUseSnack},
    {107, "POPCORN", CatalogKind::kFood, 4, 90, 4, kCatalogFlagSnack, 11,
     kCatalogUseSnack},
    {108, "GUM", CatalogKind::kFood, 5, 100, 5, kCatalogFlagSnack, 11,
     kCatalogUseSnack},
    {109, "DANGO", CatalogKind::kFood, 6, 100, 6, kCatalogFlagSnack, 11,
     kCatalogUseSnack},
    {110, "CREAM PUFF", CatalogKind::kFood, 6, 150, 6,
     kCatalogFlagSnack, 11, kCatalogUseSnack},
    {111, "ENERGY DRINK", CatalogKind::kFood, 5, 130, 5,
     kCatalogFlagSnack, 11, kCatalogUseSnack},
    {112, "PENCIL", CatalogKind::kItem, 1, 110, 1,
     kCatalogFlagAdultOnly, 11, kCatalogUseStudy},
    {113, "PLANT", CatalogKind::kItem, 2, 80, 2, 0x00, 11,
     kCatalogUsePlay},
    {114, "UMBRELLA", CatalogKind::kItem, 2, 500, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 11,
     kCatalogUsePlay},
    {115, "SUNGLASSES", CatalogKind::kItem, 2, 800, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 11,
     kCatalogUsePlay},
    {116, "BOW", CatalogKind::kItem, 2, 500, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 11,
     kCatalogUsePlay},
    {117, "DRUM", CatalogKind::kItem, 6, 1600, 6,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 11,
     kCatalogUseMusic},
    {118, "SHOVEL", CatalogKind::kItem, 0, 80, 0, 0x00, 11,
     kCatalogUsePlay},
    {119, "ROLLER BLADES", CatalogKind::kItem, 0, 1800, 0,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 11,
     kCatalogUsePlay},
    {120, "BALLOON", CatalogKind::kItem, 0, 350, 0,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagChildPlus), 12,
     kCatalogUsePlay},
    {121, "BOW TIE", CatalogKind::kItem, 2, 700, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 12,
     kCatalogUsePlay},
    {122, "CAP", CatalogKind::kItem, 2, 700, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 12,
     kCatalogUsePlay},
    {123, "MUSIC", CatalogKind::kItem, 6, 1500, 6,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 12,
     kCatalogUseMusic},
    {124, "CHIKUWA", CatalogKind::kFood, 1, 130, 1, 0x00, 12,
     kCatalogUseMeal},
    {125, "CREAM PUFF", CatalogKind::kFood, 6, 150, 6,
     kCatalogFlagSnack, 12, kCatalogUseSnack},
    {126, "DANGO", CatalogKind::kFood, 6, 100, 6, kCatalogFlagSnack, 12,
     kCatalogUseSnack},
    {127, "ESCARGOT", CatalogKind::kFood, 1, 900, 1, 0x00, 12,
     kCatalogUseMeal},
    {128, "GUM", CatalogKind::kFood, 5, 100, 5, kCatalogFlagSnack, 12,
     kCatalogUseSnack},
    {129, "MARRON CAKE", CatalogKind::kFood, 6, 520, 6,
     kCatalogFlagSnack, 12, kCatalogUseSnack},
    {130, "RICE BALL", CatalogKind::kFood, 0, 100, 0, 0x00, 12,
     kCatalogUseMeal},
    {131, "SAUSAGE", CatalogKind::kFood, 1, 100, 1, 0x00, 12,
     kCatalogUseMeal},
    {132, "ACTION FIG", CatalogKind::kItem, 0, 1400, 0,
     kCatalogFlagReusable, 12, kCatalogUsePlay},
    {133, "BLDG BLOCK", CatalogKind::kItem, 4, 800, 4,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagTeenPlus), 12,
     kCatalogUsePlay},
    {134, "BOOM BOX", CatalogKind::kItem, 6, 1400, 6,
     kCatalogFlagReusable, 12, kCatalogUseMusic},
    {135, "RC CAR 1", CatalogKind::kItem, 0, 1000, 0,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 12,
     kCatalogUsePlay},
    {136, "RC CAR 2", CatalogKind::kItem, 0, 1500, 0,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 12,
     kCatalogUsePlay},
    {137, "ROPE", CatalogKind::kItem, 5, 550, 5,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagTeenPlus), 12,
     kCatalogUsePlay},
    {138, "SHIRT", CatalogKind::kItem, 2, 1000, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 12,
     kCatalogUsePlay},
    {139, "SHOES", CatalogKind::kItem, 0, 1900, 0,
     kCatalogFlagAdultOnly, 12, kCatalogUsePlay},
    {140, "THRONE", CatalogKind::kItem, 2, 5500, 2,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 12,
     kCatalogUsePlay},
    {141, "TRUMPET", CatalogKind::kItem, 6, 1800, 6,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 12,
     kCatalogUseMusic},
    {142, "TV", CatalogKind::kItem, 1, 5000, kCatalogItemIconTv,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 12,
     kCatalogUseStudy},
    {143, "WEIGHTS", CatalogKind::kItem, 0, 600, kCatalogItemIconWeights,
     static_cast<uint8_t>(kCatalogFlagReusable | kCatalogFlagAdultOnly), 12,
     kCatalogUsePlay},
    {144, "CAKE", CatalogKind::kFood, 6, 600, 6,
     static_cast<uint8_t>(kCatalogFlagSecret | kCatalogFlagSnack), 9, 2},
    {145, "STEAK", CatalogKind::kFood, 1, 800, 1, kCatalogFlagSecret, 9, 1},
    {146, "CLOCK", CatalogKind::kItem, 1, 4000, 1,
     static_cast<uint8_t>(kCatalogFlagSecret | kCatalogFlagReusable), 9, 4},
    {147, "HAIR GEL", CatalogKind::kItem, 2, 400, 2,
     static_cast<uint8_t>(kCatalogFlagSecret | kCatalogFlagAdultOnly), 9, 3},
    {148, "HONEY", CatalogKind::kItem, 2, 2000, 2,
     static_cast<uint8_t>(kCatalogFlagSecret | kCatalogFlagAdultOnly), 9, 3},
    {149, "RC CAR 3", CatalogKind::kItem, 0, 2800, 0,
     static_cast<uint8_t>(kCatalogFlagSecret | kCatalogFlagAdultOnly |
                          kCatalogFlagReusable),
     9, 3},
    {150, "NYATCHI", CatalogKind::kItem, 2, 2500, 2,
     static_cast<uint8_t>(kCatalogFlagSecret | kCatalogFlagAdultOnly), 9, 3},
    {151, "HOHOTCHI", CatalogKind::kItem, 2, 0, 2,
     static_cast<uint8_t>(kCatalogFlagSecret | kCatalogFlagAdultOnly |
                          kCatalogFlagReusable),
     9, 3},
};

const char* const kSouvenirNames[kCatalogSouvenirCount] = {
    "FIRST HATCH", "BABY STEP",   "FIRST MEAL",  "FIRST SNACK",
    "CLEAN ROOM",  "GOOD MEDS",   "LIGHTS OFF",  "TRAINING",
    "FIRST GAME",  "SKIS",        "PALM TREE",   "SURFBOARD",
    "PANDA BEAR",  "MARACAS",     "SPRINT RIB",  "HOOP MEDAL",
    "FIRST SHOP",  "BEST BUY",    "SECRET CODE", "PASSWORD",
    "DONATION",    "GIFT BOX",    "VISIT TAG",   "FRIEND BADGE",
    "BEST FRIEND", "PARTNER",     "BABY PHOTO",  "PARENT NOTE",
    "GEN PHOTO",   "BIRTHDAY",    "ANNIV 1",     "ANNIV 2",
    "ANNIV 3",     "ANNIV 4",     "ANNIV 5",     "CARE GOOD",
    "CARE HARD",   "SICK DAY",    "TOOTH FIX",   "NO MESS",
    "FULL HEART",  "FULL FOOD",   "FULL TRAIN",  "HEAVY WIN",
    "LIGHT SLEEP", "WAKE SMILE",  "TOY PLAY",    "BOOK DAY",
    "MUSIC DAY",   "TRAVEL PASS", "TICKET",      "CHARM",
    "BLOCK SET",   "ROPE SKIP",   "TRUMPET",     "BALL PLAY",
    "SHOPPER",     "COLLECTOR",   "FAMILY TREE", "OLD FRIEND",
    "LEGACY",      "MEMORY DAY",  "NEW GEN",     "ALBUM",
};

const ShopRestockWindow kShopRestockWindows[] = {
    {0, 0},
    {15U * 60U, 1},
    {19U * 60U, 2},
};

const ShopVendorWindow kShopVendorWindows[] = {
    {11U * 60U, 12U * 60U, 0},
    {17U * 60U, 18U * 60U, 1},
};

const ShopSaleRule kShopSaleRule = {37, 11, 13, 50};

const CatalogSegment* catalogSegmentFor(uint8_t index) {
  for (const CatalogSegment& segment : kCatalogSegments) {
    if (index >= segment.start && index < segment.end) return &segment;
  }
  return nullptr;
}

const CatalogOverride* catalogOverrideFor(uint8_t index) {
  for (const CatalogOverride& item : kCatalogOverrides) {
    if (item.index == index) return &item;
  }
  return nullptr;
}

uint8_t catalogVisualTraits(uint8_t index) {
  return index;
}

uint8_t catalogItemVisualIcon(uint8_t index, uint8_t fallback) {
  switch (index) {
    case 48:
    case 112:
      return kCatalogItemIconPencil;
    case 49:
    case 113:
      return kCatalogItemIconPlant;
    case 50:
    case 114:
      return kCatalogItemIconUmbrella;
    case 51:
    case 115:
      return kCatalogItemIconGlasses;
    case 52:
    case 116:
      return kCatalogItemIconBow;
    case 53:
    case 117:
      return kCatalogItemIconDrum;
    case 54:
    case 118:
      return kCatalogItemIconShovel;
    case 55:
    case 119:
      return kCatalogItemIconSkates;
    case 56:
    case 63:
    case 120:
      return kCatalogItemIconBalloon;
    case 57:
    case 121:
      return kCatalogItemIconBowTie;
    case 58:
    case 122:
      return kCatalogItemIconCap;
    case 62:
    case 132:
      return kCatalogItemIconCharacter;
    case 66:
      return kCatalogItemIconChest;
    case 67:
      return kCatalogItemIconDarts;
    case 69:
      return kCatalogItemIconPole;
    case 70:
      return kCatalogItemIconLamp;
    case 71:
      return kCatalogItemIconMirror;
    case 73:
      return kCatalogItemIconPhonograph;
    case 74:
    case 75:
    case 135:
    case 136:
    case 149:
      return kCatalogItemIconCar;
    case 77:
    case 138:
      return kCatalogItemIconShirt;
    case 78:
    case 139:
      return kCatalogItemIconShoes;
    case 79:
    case 140:
      return kCatalogItemIconThrone;
    case 146:
      return kCatalogItemIconClock;
    case 147:
      return kCatalogItemIconHairGel;
    case 148:
      return kCatalogItemIconHoney;
    case 150:
    case 151:
      return kCatalogItemIconCharacter;
    default:
      return fallback;
  }
}

uint8_t seasonalShopIndex(uint8_t slot, uint8_t month, uint8_t day,
                          uint8_t birthdayMonth, uint8_t birthdayDay) {
  const uint8_t shopSlot = slot % kShopSlotCount;
  if (month == 0 || day == 0) {
    return 0xFF;
  }
  if (shopSlot == 0 && month == birthdayMonth && day == birthdayDay) {
    return 87;  // Yogurt birthday item.
  }
  if (shopSlot == 0 &&
      ((month == 2 && day == 14) || (month == 11 && day == 14))) {
    return 7;  // Chocolate.
  }
  if (shopSlot == 0 &&
      ((month == 3 && day == 12) || (month == 10 && day == 14))) {
    return 8;  // Cookie.
  }
  if (shopSlot == 0 &&
      ((month == 3 && day == 14) || (month == 5 && day == 14))) {
    return 18;  // Noodle.
  }
  if (month == 12 && (day == 23 || day == 24)) {
    if (shopSlot == 0) return 86;  // Whole Cake.
    if (shopSlot == 1) return 85;  // Turkey.
  }
  if (shopSlot == 0 && month == 12 && day == 25) {
    return 85;  // Turkey.
  }
  return 0xFF;
}

uint16_t shopMinuteOfDay(uint32_t clockMinutes) {
  return static_cast<uint16_t>(clockMinutes % 1440UL);
}

uint8_t shopRestockPeriodFor(uint16_t minute) {
  uint8_t period = 0;
  for (const ShopRestockWindow& window : kShopRestockWindows) {
    if (minute >= window.startMinute) period = window.period;
  }
  return period;
}

uint8_t shopVendorVisitFor(uint16_t minute) {
  for (const ShopVendorWindow& window : kShopVendorWindows) {
    if (minute >= window.startMinute && minute < window.endMinute) {
      return window.visitIndex;
    }
  }
  return 0xFF;
}

uint16_t shopSalePrice(uint16_t price) {
  return static_cast<uint16_t>(
      (static_cast<uint32_t>(price) * kShopSaleRule.pricePercent + 99U) /
      100U);
}

}  // namespace

CatalogEntry catalogEntry(uint8_t index) {
  CatalogEntry entry = {CatalogKind::kItem, 0, 100, 0, 0, 0, 0,
                        catalogVisualTraits(index)};
  const CatalogOverride* override = catalogOverrideFor(index);
  if (override) {
    entry.kind = override->kind;
    entry.behavior = override->behavior;
    entry.price = override->price;
    entry.icon = override->icon;
    entry.flags = override->flags;
    entry.sourceGroup = override->sourceGroup;
    entry.useScene = override->useScene;
    if (entry.kind == CatalogKind::kItem) {
      entry.icon = catalogItemVisualIcon(index, entry.icon);
    }
    return entry;
  }

  const CatalogSegment* segment = catalogSegmentFor(index);
  if (!segment) {
    return entry;
  }

  const uint8_t behavior =
      static_cast<uint8_t>((index - segment->behaviorBase) %
                           segment->behaviorModulo);
  const uint8_t priceStepIndex =
      static_cast<uint8_t>((index - segment->priceBase) %
                           segment->priceModulo);
  entry.kind = segment->kind;
  entry.behavior = behavior;
  entry.price = static_cast<uint16_t>(segment->basePrice +
                                      priceStepIndex * segment->priceStep);
  entry.icon = entry.behavior;
  entry.flags = segment->flags;
  entry.sourceGroup = segment->sourceGroup;
  entry.useScene = segment->useScene;
  if (entry.kind == CatalogKind::kItem) {
    entry.icon = catalogItemVisualIcon(index, entry.icon);
  }
  return entry;
}

uint8_t catalogShopIndex(uint8_t slot, uint8_t month, uint8_t day,
                         uint8_t birthdayMonth, uint8_t birthdayDay,
                         uint32_t clockMinutes) {
  static const uint8_t kShopPools[kShopSlotCount][8] = {
      {0, 1, 2, 3, 4, 5, 29, 31},
      {24, 25, 26, 27, 33, 35, 37, 38},
      {48, 49, 50, 51, 52, 54, 56, 60},
      {53, 55, 57, 58, 59, 61, 72, 88},
  };
  static const uint8_t kVendorPool[8] = {42, 38, 37, 43, 36, 44, 45, 40};
  const uint16_t minute = shopMinuteOfDay(clockMinutes);
  const uint8_t visit = shopVendorVisitFor(minute);
  if (visit != 0xFF) {
    return kVendorPool[static_cast<uint8_t>((day * 3U + visit * 5U) & 0x07)];
  }
  const uint8_t seasonal =
      seasonalShopIndex(slot, month, day, birthdayMonth, birthdayDay);
  if (seasonal != 0xFF) return seasonal;
  const uint8_t s = slot % kShopSlotCount;
  const uint8_t shopPeriod = shopRestockPeriodFor(minute);
  const uint8_t restock =
      static_cast<uint8_t>((shopPeriod + day * 3U + s * 5U) & 0x07);
  return kShopPools[s][restock];
}

uint8_t catalogShopIndex(uint8_t slot, uint8_t day, uint32_t clockMinutes) {
  return catalogShopIndex(slot, 0, day, 0, 0, clockMinutes);
}

bool catalogShopIsVendorVisit(uint32_t clockMinutes) {
  return shopVendorVisitFor(shopMinuteOfDay(clockMinutes)) != 0xFF;
}

bool catalogShopIsSale(uint8_t day, uint32_t clockMinutes) {
  if (catalogShopIsVendorVisit(clockMinutes)) return false;
  return static_cast<uint8_t>(
             (day * kShopSaleRule.dayMultiplier + kShopSaleRule.dayOffset) %
             kShopSaleRule.modulo) == 0;
}

uint16_t catalogShopPrice(uint8_t index, uint8_t day, uint32_t clockMinutes) {
  CatalogEntry entry = catalogEntry(index);
  if (catalogShopIsSale(day, clockMinutes)) {
    return shopSalePrice(entry.price);
  }
  return entry.price;
}

void catalogItemLabel(uint8_t index, char* buffer, uint8_t size) {
  if (!buffer || size == 0) {
    return;
  }
  if (index >= kCatalogItemCount) {
    snprintf(buffer, size, "ITEM---");
    return;
  }
  const CatalogOverride* override = catalogOverrideFor(index);
  if (override) {
    snprintf(buffer, size, "%s", override->label);
    return;
  }
  const CatalogSegment* segment = catalogSegmentFor(index);
  const char* prefix = segment ? segment->prefix : "ITEM";
  snprintf(buffer, size, "%s%03u", prefix,
           static_cast<unsigned>(index + 1));
}

void catalogSouvenirLabel(uint8_t index, char* buffer, uint8_t size) {
  if (!buffer || size == 0) {
    return;
  }
  if (index >= kCatalogSouvenirCount) {
    snprintf(buffer, size, "MEM---");
    return;
  }
  snprintf(buffer, size, "%s", kSouvenirNames[index]);
}

uint8_t catalogSouvenirVisualTraits(uint8_t index) {
  return static_cast<uint8_t>(0x80 | (index & 0x3F));
}

bool catalogBitTest(const uint8_t* bits, uint8_t index) {
  return bits && ((bits[index / 8] & (1U << (index & 0x07))) != 0);
}

void catalogBitSet(uint8_t* bits, uint8_t index) {
  if (!bits) {
    return;
  }
  bits[index / 8] |= static_cast<uint8_t>(1U << (index & 0x07));
}

void catalogBitClear(uint8_t* bits, uint8_t index) {
  if (!bits) {
    return;
  }
  bits[index / 8] &= static_cast<uint8_t>(~(1U << (index & 0x07)));
}

uint8_t catalogBitCount(const uint8_t* bits, uint8_t bitCount) {
  if (!bits) {
    return 0;
  }
  uint8_t count = 0;
  for (uint8_t i = 0; i < bitCount; i++) {
    if (catalogBitTest(bits, i)) {
      count++;
    }
  }
  return count;
}

}  // namespace echopet
