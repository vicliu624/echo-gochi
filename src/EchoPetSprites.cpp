#include "EchoPetSprites.h"

#include <Arduino.h>

#include "EchoPetCharacterCatalog.h"

namespace echopet {

namespace {

enum TileId : uint8_t {
  T_EMPTY,
  T_EGG_TL,
  T_EGG_TR,
  T_EGG_BL,
  T_EGG_BR,
  T_BODY_TL,
  T_BODY_TOP,
  T_BODY_TR,
  T_BODY_L,
  T_BODY_R,
  T_BODY_BL,
  T_BODY_BOTTOM,
  T_BODY_BR,
  T_EYE_PAIR,
  T_EYE_SLEEP,
  T_MOUTH_SMILE,
  T_MOUTH_SAD,
  T_FOOT_L,
  T_FOOT_R,
  T_ANT_L,
  T_ANT_R,
  T_CROWN,
  T_WRINKLE,
  T_SICK_EYES,
  T_Z_MARK,
  T_MESS,
  T_HEART,
  T_EYE_WINK,
  T_EYE_ANGRY,
  T_MOUTH_OPEN,
  T_SPARKLE,
  T_BOOK,
  T_BALL,
  T_BOW,
  T_SPIKE,
};

struct Cell {
  int8_t x;
  int8_t y;
  uint8_t tile;
};

struct BitmapAsset {
  uint8_t width;
  uint8_t height;
  uint8_t stride;
  const uint8_t* data;
};

struct FramePart {
  int8_t x;
  int8_t y;
  uint8_t asset;
  uint8_t flags;
};

struct ComposedFrame {
  uint8_t width;
  uint8_t height;
  const FramePart* parts;
  uint8_t count;
};

enum BitmapAssetId : uint8_t {
  B_EGG_BODY,
  B_EGG_EYE,
  B_EGG_MOUTH_SMILE,
  B_EGG_MOUTH_SAD,
  B_EGG_MOUTH_OPEN,
  B_EGG_CRACK,
  B_MAME_BODY,
  B_EYE_OPEN,
  B_EYE_BLINK,
  B_EYE_SAD,
  B_EYE_SICK,
  B_EYE_SLEEP,
  B_MOUTH_SMILE,
  B_MOUTH_OPEN,
  B_MOUTH_SAD,
  B_FOOT,
  B_HEART,
  B_Z_MARK,
  B_SWEAT,
  B_BALL,
  B_BOOK,
  B_BOW,
  B_SPIKE,
  B_SPARKLE,
  B_QUESTION,
  B_MUSIC,
  B_ANGRY_MARK,
  B_X_EYE,
  B_SPOON,
  B_PILL,
  B_TOILET_MARK,
  B_MESS,
  B_CRUMBS,
  B_CLEANUP_WALL,
  B_SKULL,
  B_TOOTH,
  B_FLAG,
  B_NOTE,
  B_HOOP,
  B_METER,
  B_BABY,
  B_BABY_BODY,
  B_SHOP_BOOTH,
  B_SHOP_PREVIEW,
  B_SHOP_OK,
  B_SHOP_NO_MONEY,
  B_SHOP_FULL,
  B_SHOP_SOLD_OUT,
};

constexpr uint8_t kPartFlipX = 0x01;

const uint8_t kEggBodyBitmap[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00,  // ................................
    0x00, 0x00, 0x00, 0x00,  // ................................
    0x00, 0x0F, 0xF0, 0x00,  // ............########............
    0x00, 0x3F, 0xFC, 0x00,  // ..........############..........
    0x00, 0x7F, 0xFE, 0x00,  // .........##############.........
    0x00, 0xFF, 0xFF, 0x00,  // ........################........
    0x01, 0xFF, 0xFF, 0x80,  // .......##################.......
    0x03, 0xFF, 0xFF, 0xC0,  // ......####################......
    0x03, 0xC0, 0x03, 0xC0,  // ......####............####......
    0x07, 0x80, 0x01, 0xE0,  // .....####..............####.....
    0x07, 0x00, 0x00, 0xE0,  // .....###................###.....
    0x0F, 0x00, 0x00, 0xF0,  // ....####................####....
    0x0E, 0x00, 0x00, 0x70,  // ....###..................###....
    0x0E, 0x00, 0x00, 0x70,  // ....###..................###....
    0x0E, 0x00, 0x00, 0x70,  // ....###..................###....
    0x0E, 0x00, 0x00, 0x70,  // ....###..................###....
    0x0E, 0x00, 0x00, 0x70,  // ....###..................###....
    0x0E, 0x00, 0x00, 0x70,  // ....###..................###....
    0x0F, 0x00, 0x00, 0xF0,  // ....####................####....
    0x07, 0x00, 0x00, 0xE0,  // .....###................###.....
    0x07, 0x80, 0x01, 0xE0,  // .....####..............####.....
    0x03, 0xC0, 0x03, 0xC0,  // ......####............####......
    0x03, 0xFF, 0xFF, 0xC0,  // ......####################......
    0x01, 0xFF, 0xFF, 0x80,  // .......##################.......
    0x00, 0xFF, 0xFF, 0x00,  // ........################........
    0x00, 0x7F, 0xFE, 0x00,  // .........##############.........
    0x00, 0x3F, 0xFC, 0x00,  // ..........############..........
    0x00, 0x0F, 0xF0, 0x00,  // ............########............
    0x00, 0x00, 0x00, 0x00,  // ................................
    0x00, 0x00, 0x00, 0x00,  // ................................
    0x00, 0x00, 0x00, 0x00,  // ................................
    0x00, 0x00, 0x00, 0x00,  // ................................
};

const uint8_t kEggEyeBitmap[] PROGMEM = {
    0x60,  // .##.....
    0xF0,  // ####....
    0xF0,  // ####....
    0x60,  // .##.....
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kEggMouthSmileBitmap[] PROGMEM = {
    0x00,  // ........
    0x00,  // ........
    0x81,  // #......#
    0x42,  // .#....#.
    0x3C,  // ..####..
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kEggMouthSadBitmap[] PROGMEM = {
    0x00,  // ........
    0x00,  // ........
    0x3C,  // ..####..
    0x42,  // .#....#.
    0x81,  // #......#
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kEggMouthOpenBitmap[] PROGMEM = {
    0x00,  // ........
    0x00,  // ........
    0x18,  // ...##...
    0x24,  // ..#..#..
    0x24,  // ..#..#..
    0x18,  // ...##...
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kEggCrackBitmap[] PROGMEM = {
    0x08, 0x00,  // ....#...........
    0x18, 0x00,  // ...##...........
    0x10, 0x00,  // ...#............
    0x33, 0x00,  // ..##..##........
    0x22, 0x00,  // ..#...#.........
    0x66, 0x00,  // .##..##.........
    0x44, 0x00,  // .#...#..........
    0xC4, 0x00,  // ##...#..........
};

const uint8_t kMameBodyBitmap[] PROGMEM = {
    0x00, 0x00, 0x00, 0x00,  // ................................
    0x01, 0xE0, 0x1E, 0x00,  // .......####........####.........
    0x07, 0xF8, 0x7F, 0x80,  // .....########....########.......
    0x0F, 0xFC, 0xFF, 0xC0,  // ....##########..##########......
    0x1F, 0x1F, 0xE3, 0xE0,  // ...#####...########...#####.....
    0x3C, 0x0F, 0xC0, 0xF0,  // ..####......######......####....
    0x70, 0x00, 0x00, 0x38,  // .###......................###...
    0x60, 0x00, 0x00, 0x18,  // .##........................##...
    0xE0, 0x00, 0x00, 0x1C,  // ###........................###..
    0xC0, 0x00, 0x00, 0x0C,  // ##..........................##..
    0xC0, 0x00, 0x00, 0x0C,  // ##..........................##..
    0xC0, 0x00, 0x00, 0x0C,  // ##..........................##..
    0xC0, 0x00, 0x00, 0x0C,  // ##..........................##..
    0xC0, 0x00, 0x00, 0x0C,  // ##..........................##..
    0xC0, 0x00, 0x00, 0x0C,  // ##..........................##..
    0xC0, 0x00, 0x00, 0x0C,  // ##..........................##..
    0xE0, 0x00, 0x00, 0x1C,  // ###........................###..
    0x60, 0x00, 0x00, 0x18,  // .##........................##...
    0x70, 0x00, 0x00, 0x38,  // .###......................###...
    0x3C, 0x00, 0x00, 0xF0,  // ..####..................####....
    0x1F, 0xFF, 0xFF, 0xE0,  // ...########################.....
    0x0F, 0xFF, 0xFF, 0xC0,  // ....######################......
    0x07, 0x80, 0x07, 0x80,  // .....####............####.......
    0x0F, 0x00, 0x03, 0xC0,  // ....####..............####......
    0x1E, 0x00, 0x01, 0xE0,  // ...####................####.....
    0x1C, 0x00, 0x00, 0xE0,  // ...###..................###.....
    0x0E, 0x00, 0x01, 0xC0,  // ....###................###......
    0x07, 0x86, 0x1E, 0x00,  // .....####....##....####.........
    0x03, 0xFC, 0xFF, 0x00,  // ......########..########........
    0x01, 0xF8, 0x7E, 0x00,  // .......######....######.........
    0x00, 0x00, 0x00, 0x00,  // ................................
    0x00, 0x00, 0x00, 0x00,  // ................................
};

const uint8_t kMameEyeOpenBitmap[] PROGMEM = {
    0x78,  // .####...
    0xFC,  // ######..
    0xCC,  // ##..##..
    0xCC,  // ##..##..
    0xFC,  // ######..
    0x78,  // .####...
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kMameEyeBlinkBitmap[] PROGMEM = {
    0x00,  // ........
    0x00,  // ........
    0xFC,  // ######..
    0xFC,  // ######..
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kMameEyeSadBitmap[] PROGMEM = {
    0xC0,  // ##......
    0x60,  // .##.....
    0x3C,  // ..####..
    0x1C,  // ...###..
    0x3C,  // ..####..
    0x60,  // .##.....
    0xC0,  // ##......
    0x00,  // ........
};

const uint8_t kMameEyeSickBitmap[] PROGMEM = {
    0xCC,  // ##..##..
    0x78,  // .####...
    0x30,  // ..##....
    0x78,  // .####...
    0xCC,  // ##..##..
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kMameEyeSleepBitmap[] PROGMEM = {
    0x00,  // ........
    0x00,  // ........
    0x78,  // .####...
    0xFC,  // ######..
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kMameMouthSmileBitmap[] PROGMEM = {
    0x00, 0x00,  // ................
    0x00, 0x00,  // ................
    0x30, 0x0C,  // ..##........##..
    0x18, 0x18,  // ...##......##...
    0x0F, 0xF0,  // ....########....
    0x07, 0xE0,  // .....######.....
    0x00, 0x00,  // ................
    0x00, 0x00,  // ................
};

const uint8_t kMameMouthOpenBitmap[] PROGMEM = {
    0x00, 0x00,  // ................
    0x0F, 0xF0,  // ....########....
    0x1F, 0xF8,  // ...##########...
    0x1C, 0x38,  // ...###....###...
    0x1C, 0x38,  // ...###....###...
    0x1F, 0xF8,  // ...##########...
    0x0F, 0xF0,  // ....########....
    0x00, 0x00,  // ................
};

const uint8_t kMameMouthSadBitmap[] PROGMEM = {
    0x00, 0x00,  // ................
    0x00, 0x00,  // ................
    0x0F, 0xF0,  // ....########....
    0x18, 0x18,  // ...##......##...
    0x30, 0x0C,  // ..##........##..
    0x00, 0x00,  // ................
    0x00, 0x00,  // ................
    0x00, 0x00,  // ................
};

const uint8_t kMameFootBitmap[] PROGMEM = {
    0x00,  // ........
    0x30,  // ..##....
    0x78,  // .####...
    0xF8,  // #####...
    0x70,  // .###....
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kMameHeartBitmap[] PROGMEM = {
    0x00,  // ........
    0x6C,  // .##.##..
    0xFE,  // #######.
    0xFE,  // #######.
    0x7C,  // .#####..
    0x38,  // ..###...
    0x10,  // ...#....
    0x00,  // ........
};

const uint8_t kMameZMarkBitmap[] PROGMEM = {
    0x7E,  // .######.
    0x0C,  // ....##..
    0x18,  // ...##...
    0x30,  // ..##....
    0x60,  // .##.....
    0xFC,  // ######..
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kMameSweatBitmap[] PROGMEM = {
    0x18,  // ...##...
    0x3C,  // ..####..
    0x7E,  // .######.
    0x3C,  // ..####..
    0x18,  // ...##...
    0x00,  // ........
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kMameBallBitmap[] PROGMEM = {
    0x3C,  // ..####..
    0x42,  // .#....#.
    0x99,  // #..##..#
    0xBD,  // #.####.#
    0xBD,  // #.####.#
    0x99,  // #..##..#
    0x42,  // .#....#.
    0x3C,  // ..####..
};

const uint8_t kMameBookBitmap[] PROGMEM = {
    0x7E,  // .######.
    0x42,  // .#....#.
    0x5A,  // .#.##.#.
    0x52,  // .#.#..#.
    0x5A,  // .#.##.#.
    0x42,  // .#....#.
    0x7E,  // .######.
    0x00,  // ........
};

const uint8_t kMameBowBitmap[] PROGMEM = {
    0x84,  // #....#..
    0xCC,  // ##..##..
    0x78,  // .####...
    0x30,  // ..##....
    0x78,  // .####...
    0xCC,  // ##..##..
    0x84,  // #....#..
    0x00,  // ........
};

const uint8_t kMameSpikeBitmap[] PROGMEM = {
    0x18,  // ...##...
    0x3C,  // ..####..
    0x7E,  // .######.
    0xE7,  // ###..###
    0x18,  // ...##...
    0x18,  // ...##...
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kMameSparkleBitmap[] PROGMEM = {
    0x10,  // ...#....
    0x10,  // ...#....
    0x54,  // .#.#.#..
    0x38,  // ..###...
    0xFE,  // #######.
    0x38,  // ..###...
    0x54,  // .#.#.#..
    0x10,  // ...#....
};

const uint8_t kMameQuestionBitmap[] PROGMEM = {
    0x3C,  // ..####..
    0x42,  // .#....#.
    0x02,  // ......#.
    0x0C,  // ....##..
    0x10,  // ...#....
    0x00,  // ........
    0x10,  // ...#....
    0x00,  // ........
};

const uint8_t kMameMusicBitmap[] PROGMEM = {
    0x06,  // .....##.
    0x0E,  // ....###.
    0x1A,  // ...##.#.
    0x12,  // ...#..#.
    0x12,  // ...#..#.
    0x72,  // .###..#.
    0xF2,  // ####..#.
    0x60,  // .##.....
};

const uint8_t kMameAngryMarkBitmap[] PROGMEM = {
    0x42,  // .#....#.
    0x24,  // ..#..#..
    0x18,  // ...##...
    0x7E,  // .######.
    0x18,  // ...##...
    0x24,  // ..#..#..
    0x42,  // .#....#.
    0x00,  // ........
};

const uint8_t kMameXEyeBitmap[] PROGMEM = {
    0xC6,  // ##...##.
    0x6C,  // .##.##..
    0x38,  // ..###...
    0x38,  // ..###...
    0x6C,  // .##.##..
    0xC6,  // ##...##.
    0x00,  // ........
    0x00,  // ........
};

const uint8_t kMameSpoonBitmap[] PROGMEM = {
    0x1C,  // ...###..
    0x3E,  // ..#####.
    0x1C,  // ...###..
    0x08,  // ....#...
    0x08,  // ....#...
    0x08,  // ....#...
    0x18,  // ...##...
    0x10,  // ...#....
};

const uint8_t kMamePillBitmap[] PROGMEM = {
    0x3C,  // ..####..
    0x7E,  // .######.
    0xE6,  // ###..##.
    0xC6,  // ##...##.
    0xE6,  // ###..##.
    0x7E,  // .######.
    0x3C,  // ..####..
    0x00,  // ........
};

const uint8_t kMameToiletMarkBitmap[] PROGMEM = {
    0x18,  // ...##...
    0x3C,  // ..####..
    0x18,  // ...##...
    0x7E,  // .######.
    0x42,  // .#....#.
    0x7E,  // .######.
    0x18,  // ...##...
    0x00,  // ........
};

const uint8_t kMameMessBitmap[] PROGMEM = {
    0x00,  // ........
    0x18,  // ...##...
    0x24,  // ..#..#..
    0x42,  // .#....#.
    0x7E,  // .######.
    0x7E,  // .######.
    0x3C,  // ..####..
    0x00,  // ........
};

const uint8_t kMameCrumbsBitmap[] PROGMEM = {
    0x00,  // ........
    0x00,  // ........
    0x24,  // ..#..#..
    0x00,  // ........
    0x81,  // #......#
    0x18,  // ...##...
    0x42,  // .#....#.
    0x00,  // ........
};

const uint8_t kMameCleanupWallBitmap[] PROGMEM = {
    0x42, 0x10,  // .#....#....#....
    0x24, 0x20,  // ..#..#....#.....
    0x18, 0x40,  // ...##....#......
    0x24, 0x20,  // ..#..#....#.....
    0x42, 0x10,  // .#....#....#....
    0x24, 0x20,  // ..#..#....#.....
    0x18, 0x40,  // ...##....#......
    0x24, 0x20,  // ..#..#....#.....
};

const uint8_t kMameSkullBitmap[] PROGMEM = {
    0x3C,  // ..####..
    0x7E,  // .######.
    0xDB,  // ##.##.##
    0xFF,  // ########
    0x7E,  // .######.
    0x24,  // ..#..#..
    0x24,  // ..#..#..
    0x00,  // ........
};

const uint8_t kMameToothBitmap[] PROGMEM = {
    0x3C,  // ..####..
    0x7E,  // .######.
    0xC3,  // ##....##
    0xC3,  // ##....##
    0xE7,  // ###..###
    0x7E,  // .######.
    0x24,  // ..#..#..
    0x24,  // ..#..#..
};

const uint8_t kMameFlagBitmap[] PROGMEM = {
    0x80,  // #.......
    0xFC,  // ######..
    0xFC,  // ######..
    0x80,  // #.......
    0x80,  // #.......
    0x80,  // #.......
    0x80,  // #.......
    0x80,  // #.......
};

const uint8_t kMameNoteBitmap[] PROGMEM = {
    0x1E,  // ...####.
    0x12,  // ...#..#.
    0x12,  // ...#..#.
    0x12,  // ...#..#.
    0x72,  // .###..#.
    0xF2,  // ####..#.
    0xE0,  // ###.....
    0x00,  // ........
};

const uint8_t kMameHoopBitmap[] PROGMEM = {
    0x1F, 0x80,  // ...######.......
    0x20, 0x40,  // ..#......#......
    0x40, 0x20,  // .#........#.....
    0x40, 0x20,  // .#........#.....
    0x20, 0x40,  // ..#......#......
    0x1F, 0x80,  // ...######.......
    0x04, 0x00,  // .....#..........
    0x04, 0x00,  // .....#..........
};

const uint8_t kMameMeterBitmap[] PROGMEM = {
    0xFF, 0xFE,  // ###############.
    0x80, 0x02,  // #...............
    0xBF, 0x02,  // #.######........
    0xBF, 0x02,  // #.######........
    0x80, 0x02,  // #...............
    0xFF, 0xFE,  // ###############.
    0x18, 0x00,  // ...##...........
    0x24, 0x00,  // ..#..#..........
};

const uint8_t kMameBabyBitmap[] PROGMEM = {
    0x3C,  // ..####..
    0x42,  // .#....#.
    0xA5,  // #.#..#.#
    0x81,  // #......#
    0x99,  // #..##..#
    0x42,  // .#....#.
    0x3C,  // ..####..
    0x18,  // ...##...
};

const uint8_t kBabyBodyBitmap[] PROGMEM = {
    0x00, 0x18, 0x00,
    0x00, 0x3C, 0x00,
    0x00, 0x7E, 0x00,
    0x01, 0x81, 0x80,
    0x06, 0x00, 0x60,
    0x08, 0x00, 0x10,
    0x10, 0x00, 0x08,
    0x20, 0x00, 0x04,
    0x20, 0x00, 0x04,
    0x20, 0x00, 0x04,
    0x20, 0x00, 0x04,
    0x20, 0x00, 0x04,
    0x20, 0x00, 0x04,
    0x20, 0x00, 0x04,
    0x20, 0x00, 0x04,
    0x20, 0x00, 0x04,
    0x10, 0x00, 0x08,
    0x08, 0x00, 0x10,
    0x06, 0x00, 0x60,
    0x01, 0x81, 0x80,
    0x00, 0x7E, 0x00,
    0x00, 0x18, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};

const uint8_t kMameShopBoothBitmap[] PROGMEM = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0xBF, 0x7D, 0xEF, 0xBF, 0x7D, 0xEF, 0xBD,
    0xA5, 0x52, 0xA5, 0x52, 0xA5, 0x52, 0xA5,
    0xBF, 0x7D, 0xEF, 0xBF, 0x7D, 0xEF, 0xBD,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0xBF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFD,
    0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05,
    0xA7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE5,
    0xA4, 0x92, 0x49, 0x24, 0x92, 0x49, 0x25,
    0xA4, 0x92, 0x49, 0x24, 0x92, 0x49, 0x25,
    0xA7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE5,
    0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

const uint8_t kMameShopPreviewBitmap[] PROGMEM = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x04,
    0x00, 0x00, 0x1E,
    0x3F, 0xFF, 0xFC,
    0x20, 0x00, 0x04,
    0x28, 0x00, 0x04,
    0x20, 0x00, 0x04,
    0x23, 0xFF, 0xC4,
    0x22, 0x22, 0x44,
    0x22, 0x41, 0x44,
    0x22, 0x41, 0x44,
    0x22, 0x41, 0x44,
    0x22, 0x22, 0x44,
    0x22, 0x1C, 0x44,
    0x23, 0xFF, 0xC4,
    0x20, 0x00, 0x04,
    0x20, 0x00, 0x04,
    0x3F, 0xFF, 0xFC,
    0x00, 0x3E, 0x00,
    0x00, 0x00, 0x00,
    0x01, 0xFF, 0xC0,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};

const uint8_t kMameShopOkBitmap[] PROGMEM = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x1F, 0xFF, 0xF8,
    0x10, 0x00, 0x08,
    0x10, 0x00, 0x08,
    0x10, 0x00, 0x08,
    0x10, 0x00, 0x48,
    0x10, 0x00, 0x88,
    0x10, 0x01, 0x08,
    0x10, 0x02, 0x08,
    0x10, 0x02, 0x08,
    0x11, 0x04, 0x08,
    0x10, 0x88, 0xE8,
    0x10, 0x51, 0x18,
    0x10, 0x22, 0xE8,
    0x10, 0x02, 0xE8,
    0x10, 0x02, 0xE8,
    0x10, 0x01, 0x18,
    0x10, 0x00, 0xE8,
    0x1F, 0xFF, 0xF8,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};

const uint8_t kMameShopNoMoneyBitmap[] PROGMEM = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x01, 0x08,
    0x03, 0xE1, 0x08,
    0x06, 0xB0, 0x90,
    0x08, 0x88, 0x90,
    0x1B, 0xEC, 0x60,
    0x10, 0x84, 0x60,
    0x10, 0x84, 0x60,
    0x10, 0x84, 0x60,
    0x1B, 0xEC, 0x60,
    0x08, 0x88, 0x90,
    0x06, 0xB0, 0x90,
    0x03, 0xE1, 0x08,
    0x00, 0x01, 0x08,
    0x00, 0x00, 0x00,
    0x1F, 0xFF, 0xF8,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};

const uint8_t kMameShopFullBitmap[] PROGMEM = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x04, 0x00, 0x10,
    0x08, 0x00, 0x08,
    0x10, 0x00, 0x04,
    0x00, 0x00, 0x00,
    0x01, 0xFF, 0x80,
    0x06, 0x00, 0x60,
    0x0F, 0xFF, 0xF0,
    0x08, 0x00, 0x10,
    0x08, 0x00, 0x10,
    0x08, 0xFF, 0x10,
    0x08, 0x00, 0x10,
    0x08, 0x00, 0x10,
    0x08, 0xFF, 0x10,
    0x08, 0x00, 0x10,
    0x08, 0x00, 0x10,
    0x08, 0xFF, 0x10,
    0x08, 0x00, 0x10,
    0x0F, 0xFF, 0xF0,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};

const uint8_t kMameShopSoldOutBitmap[] PROGMEM = {
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xF0,
    0x08, 0x00, 0x10,
    0x08, 0x00, 0x10,
    0x09, 0x00, 0x90,
    0x08, 0x81, 0x10,
    0x08, 0x42, 0x10,
    0x08, 0x24, 0x10,
    0x08, 0x18, 0x10,
    0x08, 0x18, 0x10,
    0x08, 0x24, 0x10,
    0x08, 0x42, 0x10,
    0x08, 0x81, 0x10,
    0x09, 0x00, 0x90,
    0x08, 0x00, 0x10,
    0x0F, 0xFF, 0xF0,
    0x00, 0x00, 0x00,
    0x04, 0x00, 0x20,
    0x03, 0xFF, 0xC0,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
};

const BitmapAsset kBitmapAssets[] = {
    {32, 32, 4, kEggBodyBitmap},         {8, 8, 1, kEggEyeBitmap},
    {8, 8, 1, kEggMouthSmileBitmap},     {8, 8, 1, kEggMouthSadBitmap},
    {8, 8, 1, kEggMouthOpenBitmap},      {16, 8, 2, kEggCrackBitmap},
    {32, 32, 4, kMameBodyBitmap},
    {8, 8, 1, kMameEyeOpenBitmap},
    {8, 8, 1, kMameEyeBlinkBitmap},      {8, 8, 1, kMameEyeSadBitmap},
    {8, 8, 1, kMameEyeSickBitmap},       {8, 8, 1, kMameEyeSleepBitmap},
    {16, 8, 2, kMameMouthSmileBitmap},   {16, 8, 2, kMameMouthOpenBitmap},
    {16, 8, 2, kMameMouthSadBitmap},     {8, 8, 1, kMameFootBitmap},
    {8, 8, 1, kMameHeartBitmap},         {8, 8, 1, kMameZMarkBitmap},
    {8, 8, 1, kMameSweatBitmap},         {8, 8, 1, kMameBallBitmap},
    {8, 8, 1, kMameBookBitmap},          {8, 8, 1, kMameBowBitmap},
    {8, 8, 1, kMameSpikeBitmap},         {8, 8, 1, kMameSparkleBitmap},
    {8, 8, 1, kMameQuestionBitmap},       {8, 8, 1, kMameMusicBitmap},
    {8, 8, 1, kMameAngryMarkBitmap},      {8, 8, 1, kMameXEyeBitmap},
    {8, 8, 1, kMameSpoonBitmap},          {8, 8, 1, kMamePillBitmap},
    {8, 8, 1, kMameToiletMarkBitmap},
    {8, 8, 1, kMameMessBitmap},           {8, 8, 1, kMameCrumbsBitmap},
    {16, 8, 2, kMameCleanupWallBitmap},   {8, 8, 1, kMameSkullBitmap},
    {8, 8, 1, kMameToothBitmap},          {8, 8, 1, kMameFlagBitmap},
    {8, 8, 1, kMameNoteBitmap},           {16, 8, 2, kMameHoopBitmap},
    {16, 8, 2, kMameMeterBitmap},         {8, 8, 1, kMameBabyBitmap},
    {24, 24, 3, kBabyBodyBitmap},
    {56, 24, 7, kMameShopBoothBitmap},
    {24, 24, 3, kMameShopPreviewBitmap},
    {24, 24, 3, kMameShopOkBitmap},
    {24, 24, 3, kMameShopNoMoneyBitmap},
    {24, 24, 3, kMameShopFullBitmap},
    {24, 24, 3, kMameShopSoldOutBitmap},
};

// Connect-style resource model:
// body outline is a variable-size bitmap; eyes, mouth, feet, and marks are
// reusable local-animation parts with explicit anchors per frame.
const FramePart kFrameEgg0Parts[] PROGMEM = {
    {0, 0, B_EGG_BODY, 0},      {9, 13, B_EGG_EYE, 0},
    {19, 13, B_EGG_EYE, 0},     {12, 20, B_EGG_MOUTH_SMILE, 0},
};

const FramePart kFrameEgg1Parts[] PROGMEM = {
    {0, 0, B_EGG_BODY, 0},      {9, 13, B_EGG_EYE, 0},
    {19, 13, B_EGG_EYE, 0},     {12, 20, B_EGG_MOUTH_SAD, 0},
    {8, 7, B_EGG_CRACK, 0},
};

const FramePart kFrameEggCrack0Parts[] PROGMEM = {
    {0, 0, B_EGG_BODY, 0},       {9, 13, B_EGG_EYE, 0},
    {19, 13, B_EGG_EYE, 0},      {12, 20, B_EGG_MOUTH_SAD, 0},
    {8, 7, B_EGG_CRACK, 0},      {22, 4, B_SPARKLE, 0},
};

const FramePart kFrameEggCrack1Parts[] PROGMEM = {
    {0, 0, B_EGG_BODY, 0},       {9, 13, B_EGG_EYE, 0},
    {19, 13, B_EGG_EYE, 0},      {12, 20, B_EGG_MOUTH_OPEN, 0},
    {5, 7, B_EGG_CRACK, 0},      {13, 11, B_EGG_CRACK, 0},
    {2, 5, B_SPARKLE, 0},        {23, 22, B_SPARKLE, 0},
};

const FramePart kFrameEggHatchParts[] PROGMEM = {
    {0, 1, B_EGG_BODY, 0},       {5, 8, B_EGG_CRACK, 0},
    {14, 12, B_EGG_CRACK, 0},    {9, 13, B_EGG_EYE, 0},
    {19, 13, B_EGG_EYE, 0},      {12, 20, B_EGG_MOUTH_OPEN, 0},
    {2, 3, B_SPARKLE, 0},        {23, 2, B_SPARKLE, 0},
    {24, 22, B_SPARKLE, 0},
};

const FramePart kFrameBaby0Parts[] PROGMEM = {
    {0, 0, B_BABY_BODY, 0},      {5, 8, B_EYE_OPEN, 0},
    {13, 8, B_EYE_OPEN, 0},      {4, 14, B_MOUTH_SMILE, 0},
    {4, 20, B_FOOT, 0},          {12, 20, B_FOOT, kPartFlipX},
};

const FramePart kFrameBaby1Parts[] PROGMEM = {
    {0, 0, B_BABY_BODY, 0},      {5, 9, B_EYE_BLINK, 0},
    {13, 9, B_EYE_BLINK, 0},     {4, 14, B_MOUTH_OPEN, 0},
    {3, 21, B_FOOT, 0},          {12, 20, B_FOOT, kPartFlipX},
    {17, 2, B_HEART, 0},
};

const FramePart kFrameStand0Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},     {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},         {19, 27, B_FOOT, kPartFlipX},
};

const FramePart kFrameStand1Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},     {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {7, 27, B_FOOT, 0},         {21, 27, B_FOOT, kPartFlipX},
};

const FramePart kFrameHappyParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},     {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},   {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},         {20, 27, B_FOOT, kPartFlipX},
    {24, 5, B_HEART, 0},
};

const FramePart kFrameSadParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},     {7, 11, B_EYE_SAD, 0},
    {18, 11, B_EYE_SAD, kPartFlipX}, {8, 18, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},         {19, 27, B_FOOT, kPartFlipX},
};

const FramePart kFrameSleepParts[] PROGMEM = {
    {0, 1, B_MAME_BODY, 0},     {7, 13, B_EYE_SLEEP, 0},
    {18, 13, B_EYE_SLEEP, 0},   {8, 19, B_MOUTH_SMILE, 0},
    {24, 2, B_Z_MARK, 0},
};

const FramePart kFrameSickParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},     {7, 11, B_EYE_SICK, 0},
    {18, 11, B_EYE_SICK, 0},    {8, 18, B_MOUTH_SAD, 0},
    {23, 8, B_SWEAT, 0},        {9, 27, B_FOOT, 0},
    {19, 27, B_FOOT, kPartFlipX},
};

const FramePart kFrameFriendParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},     {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},         {19, 27, B_FOOT, kPartFlipX},
    {1, 7, B_HEART, 0},         {24, 5, B_HEART, 0},
};

const FramePart kFrameAthleteParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},     {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},         {19, 27, B_FOOT, kPartFlipX},
    {1, 23, B_BALL, 0},
};

const FramePart kFrameScholarParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},     {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},         {19, 27, B_FOOT, kPartFlipX},
    {1, 21, B_BOOK, 0},
};

const FramePart kFrameDreamParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},     {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},         {19, 27, B_FOOT, kPartFlipX},
    {12, 2, B_BOW, 0},          {24, 7, B_SPARKLE, 0},
};

const FramePart kFrameRascalParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},     {7, 11, B_EYE_SAD, 0},
    {18, 11, B_EYE_SAD, kPartFlipX}, {8, 18, B_MOUTH_OPEN, 0},
    {8, 1, B_SPIKE, 0},         {17, 1, B_SPIKE, 0},
    {8, 27, B_FOOT, 0},         {20, 27, B_FOOT, kPartFlipX},
};

const FramePart kFrameEatMeal0Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {3, 18, B_SPOON, 0},
};

const FramePart kFrameEatMeal1Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {24, 18, B_SPOON, kPartFlipX},
};

const FramePart kFrameEatSnack0Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {1, 8, B_HEART, 0},          {24, 18, B_BALL, 0},
};

const FramePart kFrameEatSnack1Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {25, 5, B_HEART, 0},
};

const FramePart kFrameFoodCrumbsParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {1, 22, B_CRUMBS, 0},        {24, 22, B_CRUMBS, 0},
};

const FramePart kFrameFoodRefuseParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SAD, 0},
    {18, 11, B_EYE_SAD, kPartFlipX}, {8, 18, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {2, 18, B_SPOON, 0},         {24, 6, B_ANGRY_MARK, 0},
};

const FramePart kFrameFoodDoneParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {2, 7, B_SPARKLE, 0},        {25, 7, B_HEART, 0},
};

const FramePart kFrameToilet0Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {6, 12, B_EYE_SAD, 0},
    {19, 12, B_EYE_SAD, kPartFlipX}, {8, 19, B_MOUTH_SAD, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {24, 7, B_SWEAT, 0},
};

const FramePart kFrameToilet1Parts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_BLINK, 0},
    {18, 11, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {2, 5, B_SPARKLE, 0},        {25, 5, B_SPARKLE, 0},
};

const FramePart kFrameToiletMessParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {6, 12, B_EYE_SAD, 0},
    {19, 12, B_EYE_SAD, kPartFlipX}, {8, 19, B_MOUTH_SAD, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {1, 23, B_MESS, 0},          {24, 7, B_SWEAT, 0},
};

const FramePart kFrameToiletCleanupWall0Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {6, 12, B_EYE_SAD, 0},
    {19, 12, B_EYE_SAD, kPartFlipX}, {8, 19, B_MOUTH_SAD, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {22, 23, B_MESS, 0},         {4, 21, B_CLEANUP_WALL, 0},
};

const FramePart kFrameToiletCleanupWall1Parts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {5, 21, B_CLEANUP_WALL, 0},  {23, 6, B_SPARKLE, 0},
};

const FramePart kFrameToiletDoneParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_BLINK, 0},
    {18, 11, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {2, 5, B_SPARKLE, 0},        {25, 5, B_SPARKLE, 0},
    {13, 4, B_HEART, 0},
};

const FramePart kFrameToiletNoMessParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {24, 4, B_QUESTION, 0},
};

const FramePart kFrameMedicine0Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SICK, 0},
    {18, 11, B_EYE_SICK, 0},     {8, 18, B_MOUTH_OPEN, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {3, 17, B_PILL, 0},          {24, 8, B_SWEAT, 0},
};

const FramePart kFrameMedicine1Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {2, 7, B_SPARKLE, 0},        {25, 6, B_SPARKLE, 0},
};

const FramePart kFrameMedicineSickSkullParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SICK, 0},
    {18, 11, B_EYE_SICK, 0},     {8, 18, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {23, 4, B_SKULL, 0},         {2, 8, B_SWEAT, 0},
};

const FramePart kFrameMedicineSickToothParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SICK, 0},
    {18, 11, B_EYE_SICK, 0},     {8, 18, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {23, 4, B_TOOTH, 0},         {2, 8, B_SWEAT, 0},
};

const FramePart kFrameMedicineDose0Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SICK, 0},
    {18, 11, B_EYE_SICK, 0},     {8, 18, B_MOUTH_OPEN, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {3, 17, B_PILL, 0},          {24, 8, B_SKULL, 0},
};

const FramePart kFrameMedicineDose1Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {13, 17, B_PILL, 0},         {25, 7, B_SWEAT, 0},
};

const FramePart kFrameMedicineRecoverParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {2, 7, B_SPARKLE, 0},        {25, 6, B_SPARKLE, 0},
    {23, 20, B_HEART, 0},
};

const FramePart kFrameMedicineRefuseParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SAD, 0},
    {18, 11, B_EYE_SAD, kPartFlipX}, {8, 18, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {3, 17, B_PILL, 0},          {24, 6, B_ANGRY_MARK, 0},
};

const FramePart kFrameLightsOnParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {23, 5, B_SPARKLE, 0},       {2, 5, B_SPARKLE, 0},
};

const FramePart kFrameLightsOffParts[] PROGMEM = {
    {0, 1, B_MAME_BODY, 0},      {7, 13, B_EYE_SLEEP, 0},
    {18, 13, B_EYE_SLEEP, 0},    {8, 19, B_MOUTH_SMILE, 0},
    {24, 2, B_Z_MARK, 0},        {2, 4, B_Z_MARK, 0},
};

const FramePart kFrameLightsSelectorOnParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {23, 4, B_SPARKLE, 0},       {2, 5, B_QUESTION, 0},
};

const FramePart kFrameLightsSelectorOffParts[] PROGMEM = {
    {0, 1, B_MAME_BODY, 0},      {7, 13, B_EYE_SLEEP, 0},
    {18, 13, B_EYE_SLEEP, 0},    {8, 19, B_MOUTH_SMILE, 0},
    {24, 2, B_Z_MARK, 0},        {2, 5, B_QUESTION, 0},
};

const FramePart kFrameLightsWakeParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_BLINK, 0},
    {18, 11, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {2, 5, B_MUSIC, 0},          {24, 5, B_SPARKLE, 0},
};

const FramePart kFrameLightsInvalidParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SAD, 0},
    {18, 11, B_EYE_SAD, kPartFlipX}, {8, 18, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {2, 6, B_QUESTION, 0},       {24, 6, B_ANGRY_MARK, 0},
};

const FramePart kFrameDisciplineTimeoutParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SAD, 0},
    {18, 11, B_EYE_SAD, kPartFlipX}, {8, 18, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {24, 5, B_ANGRY_MARK, 0},
};

const FramePart kFrameDisciplinePraiseParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {2, 6, B_HEART, 0},          {25, 5, B_MUSIC, 0},
};

const FramePart kFrameAttentionCallParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {25, 3, B_QUESTION, 0},
};

const FramePart kFrameDisciplineInvalidParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SAD, 0},
    {18, 11, B_EYE_SAD, kPartFlipX}, {8, 18, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {2, 7, B_QUESTION, 0},       {24, 6, B_ANGRY_MARK, 0},
};

const FramePart kFrameAttentionMissedParts[] PROGMEM = {
    {0, 1, B_MAME_BODY, 0},      {7, 12, B_EYE_SAD, 0},
    {18, 12, B_EYE_SAD, kPartFlipX}, {8, 19, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {24, 8, B_SWEAT, 0},         {2, 6, B_QUESTION, 0},
};

const FramePart kFrameItemPlayParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {1, 22, B_BALL, 0},          {24, 5, B_SPARKLE, 0},
};

const FramePart kFrameShopBoothParts[] PROGMEM = {
    {0, 0, B_SHOP_BOOTH, 0},
};

const FramePart kFrameShopkeeperIdleParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {0, 18, B_BOOK, 0},
};

const FramePart kFrameShopkeeperSurpriseParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {23, 4, B_QUESTION, 0},      {0, 18, B_BOOK, 0},
};

const FramePart kFrameShopkeeperHappyParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_SMILE, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {23, 5, B_HEART, 0},         {0, 18, B_BOOK, 0},
};

const FramePart kFrameShopItemPreviewParts[] PROGMEM = {
    {0, 0, B_SHOP_PREVIEW, 0},
};

const FramePart kFrameShopBuyOkParts[] PROGMEM = {
    {0, 0, B_SHOP_OK, 0},
};

const FramePart kFrameShopBuyNoMoneyParts[] PROGMEM = {
    {0, 0, B_SHOP_NO_MONEY, 0},
};

const FramePart kFrameShopBuyFullParts[] PROGMEM = {
    {0, 0, B_SHOP_FULL, 0},
};

const FramePart kFrameShopSoldOutParts[] PROGMEM = {
    {0, 0, B_SHOP_SOLD_OUT, 0},
};

const FramePart kFrameLinkSendParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {24, 5, B_HEART, 0},         {2, 16, B_MUSIC, 0},
};

const FramePart kFrameLinkReceiveParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {1, 5, B_HEART, 0},          {25, 6, B_SPARKLE, 0},
};

const FramePart kFrameLovePartnerParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {1, 6, B_HEART, 0},          {23, 5, B_HEART, 0},
    {14, 2, B_HEART, 0},
};

const FramePart kFrameLoveBabyParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {3, 22, B_BABY, 0},          {24, 5, B_HEART, 0},
};

const FramePart kFrameParentDepartParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SAD, 0},
    {18, 11, B_EYE_SAD, kPartFlipX}, {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {1, 22, B_BABY, 0},          {24, 6, B_SPARKLE, 0},
};

const FramePart kFrameGameWinParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_BLINK, 0},
    {18, 11, B_EYE_BLINK, 0},    {8, 17, B_MOUTH_OPEN, 0},
    {7, 26, B_FOOT, 0},          {21, 26, B_FOOT, kPartFlipX},
    {1, 5, B_SPARKLE, 0},        {25, 4, B_HEART, 0},
};

const FramePart kFrameGameLoseParts[] PROGMEM = {
    {0, 1, B_MAME_BODY, 0},      {7, 12, B_EYE_SAD, 0},
    {18, 12, B_EYE_SAD, kPartFlipX}, {8, 19, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {24, 9, B_SWEAT, 0},
};

const FramePart kFrameGameGetNoteParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {3, 3, B_NOTE, 0},           {24, 12, B_NOTE, 0},
};

const FramePart kFrameGameGetBadParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SAD, 0},
    {18, 11, B_EYE_SAD, kPartFlipX}, {8, 18, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {2, 22, B_MESS, 0},          {24, 8, B_SWEAT, 0},
};

const FramePart kFrameGameGetCatchParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_BLINK, 0},
    {18, 11, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {1, 8, B_NOTE, 0},           {24, 7, B_SPARKLE, 0},
};

const FramePart kFrameGameGetMissParts[] PROGMEM = {
    {0, 1, B_MAME_BODY, 0},      {7, 12, B_EYE_SAD, 0},
    {18, 12, B_EYE_SAD, kPartFlipX}, {8, 19, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {23, 7, B_NOTE, 0},          {2, 8, B_SWEAT, 0},
};

const FramePart kFrameGameBumpMeterParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_OPEN, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {8, 2, B_METER, 0},
};

const FramePart kFrameGameBumpPushParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_SAD, 0},
    {18, 11, B_EYE_SAD, kPartFlipX}, {8, 18, B_MOUTH_OPEN, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {24, 5, B_ANGRY_MARK, 0},    {1, 22, B_BALL, 0},
};

const FramePart kFrameGameBumpFallParts[] PROGMEM = {
    {0, 2, B_MAME_BODY, 0},      {7, 14, B_EYE_SAD, 0},
    {18, 14, B_EYE_SAD, kPartFlipX}, {8, 21, B_MOUTH_SAD, 0},
    {24, 7, B_SWEAT, 0},         {2, 23, B_CRUMBS, 0},
};

const FramePart kFrameGameFlagLeftParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {1, 4, B_FLAG, 0},
};

const FramePart kFrameGameFlagRightParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {24, 4, B_FLAG, kPartFlipX},
};

const FramePart kFrameGameFlagBothParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 12, B_EYE_BLINK, 0},
    {18, 12, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {8, 27, B_FOOT, 0},          {20, 27, B_FOOT, kPartFlipX},
    {1, 4, B_FLAG, 0},           {24, 4, B_FLAG, kPartFlipX},
};

const FramePart kFrameGameFlagGoodParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_BLINK, 0},
    {18, 11, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {2, 5, B_SPARKLE, 0},        {24, 5, B_FLAG, kPartFlipX},
};

const FramePart kFrameGameFlagMissParts[] PROGMEM = {
    {0, 1, B_MAME_BODY, 0},      {7, 12, B_EYE_SAD, 0},
    {18, 12, B_EYE_SAD, kPartFlipX}, {8, 19, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {24, 7, B_FLAG, kPartFlipX}, {2, 8, B_SWEAT, 0},
};

const FramePart kFrameGameHeadingBallParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {13, 2, B_BALL, 0},
};

const FramePart kFrameGameHeadingHitParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_BLINK, 0},
    {18, 11, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {13, 1, B_BALL, 0},          {24, 6, B_SPARKLE, 0},
};

const FramePart kFrameGameHeadingMissParts[] PROGMEM = {
    {0, 1, B_MAME_BODY, 0},      {7, 12, B_EYE_SAD, 0},
    {18, 12, B_EYE_SAD, kPartFlipX}, {8, 19, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {24, 3, B_BALL, 0},          {2, 8, B_SWEAT, 0},
};

const FramePart kFrameGameMemoryRevealParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {2, 7, B_BOOK, 0},           {24, 7, B_SPARKLE, 0},
};

const FramePart kFrameGameMemoryCursorParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_OPEN, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {2, 7, B_QUESTION, 0},       {24, 7, B_BOOK, 0},
};

const FramePart kFrameGameMemoryGoodParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_BLINK, 0},
    {18, 11, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {2, 6, B_BOOK, 0},           {24, 6, B_HEART, 0},
};

const FramePart kFrameGameMemoryWrongParts[] PROGMEM = {
    {0, 1, B_MAME_BODY, 0},      {7, 12, B_EYE_SAD, 0},
    {18, 12, B_EYE_SAD, kPartFlipX}, {8, 19, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {2, 7, B_X_EYE, 0},          {24, 7, B_BOOK, 0},
};

const FramePart kFrameGameSprintRunner0Parts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {24, 4, B_FLAG, kPartFlipX},
};

const FramePart kFrameGameSprintRunner1Parts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_OPEN, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {24, 4, B_FLAG, kPartFlipX},
};

const FramePart kFrameGameSprintFinishParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_BLINK, 0},
    {18, 11, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {1, 5, B_FLAG, 0},           {24, 5, B_SPARKLE, 0},
};

const FramePart kFrameGameHoopsHoopParts[] PROGMEM = {
    {0, 0, B_MAME_BODY, 0},      {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_SMILE, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {16, 2, B_HOOP, 0},
};

const FramePart kFrameGameHoopsShootParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_OPEN, 0},
    {18, 11, B_EYE_OPEN, 0},     {8, 18, B_MOUTH_OPEN, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {16, 2, B_HOOP, 0},          {3, 6, B_BALL, 0},
};

const FramePart kFrameGameHoopsMadeParts[] PROGMEM = {
    {0, -1, B_MAME_BODY, 0},     {7, 11, B_EYE_BLINK, 0},
    {18, 11, B_EYE_BLINK, 0},    {8, 18, B_MOUTH_OPEN, 0},
    {7, 27, B_FOOT, 0},          {21, 27, B_FOOT, kPartFlipX},
    {16, 2, B_HOOP, 0},          {21, 4, B_BALL, 0},
    {2, 6, B_SPARKLE, 0},
};

const FramePart kFrameGameHoopsMissParts[] PROGMEM = {
    {0, 1, B_MAME_BODY, 0},      {7, 12, B_EYE_SAD, 0},
    {18, 12, B_EYE_SAD, kPartFlipX}, {8, 19, B_MOUTH_SAD, 0},
    {9, 27, B_FOOT, 0},          {19, 27, B_FOOT, kPartFlipX},
    {16, 2, B_HOOP, 0},          {2, 22, B_BALL, 0},
};

const FramePart kFramePassedParts[] PROGMEM = {
    {0, 2, B_MAME_BODY, 0},      {7, 14, B_X_EYE, 0},
    {18, 14, B_X_EYE, 0},        {8, 21, B_MOUTH_SAD, 0},
    {24, 3, B_Z_MARK, 0},
};

const ComposedFrame kFrameStand0 = {
    32, 32, kFrameStand0Parts,
    sizeof(kFrameStand0Parts) / sizeof(kFrameStand0Parts[0])};
const ComposedFrame kFrameStand1 = {
    32, 32, kFrameStand1Parts,
    sizeof(kFrameStand1Parts) / sizeof(kFrameStand1Parts[0])};
const ComposedFrame kFrameHappy = {
    32, 32, kFrameHappyParts,
    sizeof(kFrameHappyParts) / sizeof(kFrameHappyParts[0])};
const ComposedFrame kFrameSadComposed = {
    32, 32, kFrameSadParts,
    sizeof(kFrameSadParts) / sizeof(kFrameSadParts[0])};
const ComposedFrame kFrameSleepComposed = {
    32, 32, kFrameSleepParts,
    sizeof(kFrameSleepParts) / sizeof(kFrameSleepParts[0])};
const ComposedFrame kFrameSickComposed = {
    32, 32, kFrameSickParts,
    sizeof(kFrameSickParts) / sizeof(kFrameSickParts[0])};
const ComposedFrame kFrameFriendComposed = {
    32, 32, kFrameFriendParts,
    sizeof(kFrameFriendParts) / sizeof(kFrameFriendParts[0])};
const ComposedFrame kFrameAthleteComposed = {
    32, 32, kFrameAthleteParts,
    sizeof(kFrameAthleteParts) / sizeof(kFrameAthleteParts[0])};
const ComposedFrame kFrameScholarComposed = {
    32, 32, kFrameScholarParts,
    sizeof(kFrameScholarParts) / sizeof(kFrameScholarParts[0])};
const ComposedFrame kFrameDreamComposed = {
    32, 32, kFrameDreamParts,
    sizeof(kFrameDreamParts) / sizeof(kFrameDreamParts[0])};
const ComposedFrame kFrameRascalComposed = {
    32, 32, kFrameRascalParts,
    sizeof(kFrameRascalParts) / sizeof(kFrameRascalParts[0])};
const ComposedFrame kFrameEgg0Composed = {
    32, 32, kFrameEgg0Parts,
    sizeof(kFrameEgg0Parts) / sizeof(kFrameEgg0Parts[0])};
const ComposedFrame kFrameEgg1Composed = {
    32, 32, kFrameEgg1Parts,
    sizeof(kFrameEgg1Parts) / sizeof(kFrameEgg1Parts[0])};
const ComposedFrame kFrameEggCrack0Composed = {
    32, 32, kFrameEggCrack0Parts,
    sizeof(kFrameEggCrack0Parts) / sizeof(kFrameEggCrack0Parts[0])};
const ComposedFrame kFrameEggCrack1Composed = {
    32, 32, kFrameEggCrack1Parts,
    sizeof(kFrameEggCrack1Parts) / sizeof(kFrameEggCrack1Parts[0])};
const ComposedFrame kFrameEggHatchComposed = {
    32, 32, kFrameEggHatchParts,
    sizeof(kFrameEggHatchParts) / sizeof(kFrameEggHatchParts[0])};
const ComposedFrame kFrameBaby0Composed = {
    24, 28, kFrameBaby0Parts,
    sizeof(kFrameBaby0Parts) / sizeof(kFrameBaby0Parts[0])};
const ComposedFrame kFrameBaby1Composed = {
    24, 28, kFrameBaby1Parts,
    sizeof(kFrameBaby1Parts) / sizeof(kFrameBaby1Parts[0])};
const ComposedFrame kFrameEatMeal0Composed = {
    32, 32, kFrameEatMeal0Parts,
    sizeof(kFrameEatMeal0Parts) / sizeof(kFrameEatMeal0Parts[0])};
const ComposedFrame kFrameEatMeal1Composed = {
    32, 32, kFrameEatMeal1Parts,
    sizeof(kFrameEatMeal1Parts) / sizeof(kFrameEatMeal1Parts[0])};
const ComposedFrame kFrameEatSnack0Composed = {
    32, 32, kFrameEatSnack0Parts,
    sizeof(kFrameEatSnack0Parts) / sizeof(kFrameEatSnack0Parts[0])};
const ComposedFrame kFrameEatSnack1Composed = {
    32, 32, kFrameEatSnack1Parts,
    sizeof(kFrameEatSnack1Parts) / sizeof(kFrameEatSnack1Parts[0])};
const ComposedFrame kFrameFoodCrumbsComposed = {
    32, 32, kFrameFoodCrumbsParts,
    sizeof(kFrameFoodCrumbsParts) / sizeof(kFrameFoodCrumbsParts[0])};
const ComposedFrame kFrameFoodRefuseComposed = {
    32, 32, kFrameFoodRefuseParts,
    sizeof(kFrameFoodRefuseParts) / sizeof(kFrameFoodRefuseParts[0])};
const ComposedFrame kFrameFoodDoneComposed = {
    32, 32, kFrameFoodDoneParts,
    sizeof(kFrameFoodDoneParts) / sizeof(kFrameFoodDoneParts[0])};
const ComposedFrame kFrameToilet0Composed = {
    32, 32, kFrameToilet0Parts,
    sizeof(kFrameToilet0Parts) / sizeof(kFrameToilet0Parts[0])};
const ComposedFrame kFrameToilet1Composed = {
    32, 32, kFrameToilet1Parts,
    sizeof(kFrameToilet1Parts) / sizeof(kFrameToilet1Parts[0])};
const ComposedFrame kFrameToiletMessComposed = {
    32, 32, kFrameToiletMessParts,
    sizeof(kFrameToiletMessParts) / sizeof(kFrameToiletMessParts[0])};
const ComposedFrame kFrameToiletCleanupWall0Composed = {
    32, 32, kFrameToiletCleanupWall0Parts,
    sizeof(kFrameToiletCleanupWall0Parts) / sizeof(kFrameToiletCleanupWall0Parts[0])};
const ComposedFrame kFrameToiletCleanupWall1Composed = {
    32, 32, kFrameToiletCleanupWall1Parts,
    sizeof(kFrameToiletCleanupWall1Parts) / sizeof(kFrameToiletCleanupWall1Parts[0])};
const ComposedFrame kFrameToiletDoneComposed = {
    32, 32, kFrameToiletDoneParts,
    sizeof(kFrameToiletDoneParts) / sizeof(kFrameToiletDoneParts[0])};
const ComposedFrame kFrameToiletNoMessComposed = {
    32, 32, kFrameToiletNoMessParts,
    sizeof(kFrameToiletNoMessParts) / sizeof(kFrameToiletNoMessParts[0])};
const ComposedFrame kFrameMedicine0Composed = {
    32, 32, kFrameMedicine0Parts,
    sizeof(kFrameMedicine0Parts) / sizeof(kFrameMedicine0Parts[0])};
const ComposedFrame kFrameMedicine1Composed = {
    32, 32, kFrameMedicine1Parts,
    sizeof(kFrameMedicine1Parts) / sizeof(kFrameMedicine1Parts[0])};
const ComposedFrame kFrameMedicineSickSkullComposed = {
    32, 32, kFrameMedicineSickSkullParts,
    sizeof(kFrameMedicineSickSkullParts) / sizeof(kFrameMedicineSickSkullParts[0])};
const ComposedFrame kFrameMedicineSickToothComposed = {
    32, 32, kFrameMedicineSickToothParts,
    sizeof(kFrameMedicineSickToothParts) / sizeof(kFrameMedicineSickToothParts[0])};
const ComposedFrame kFrameMedicineDose0Composed = {
    32, 32, kFrameMedicineDose0Parts,
    sizeof(kFrameMedicineDose0Parts) / sizeof(kFrameMedicineDose0Parts[0])};
const ComposedFrame kFrameMedicineDose1Composed = {
    32, 32, kFrameMedicineDose1Parts,
    sizeof(kFrameMedicineDose1Parts) / sizeof(kFrameMedicineDose1Parts[0])};
const ComposedFrame kFrameMedicineRecoverComposed = {
    32, 32, kFrameMedicineRecoverParts,
    sizeof(kFrameMedicineRecoverParts) / sizeof(kFrameMedicineRecoverParts[0])};
const ComposedFrame kFrameMedicineRefuseComposed = {
    32, 32, kFrameMedicineRefuseParts,
    sizeof(kFrameMedicineRefuseParts) / sizeof(kFrameMedicineRefuseParts[0])};
const ComposedFrame kFrameLightsOnComposed = {
    32, 32, kFrameLightsOnParts,
    sizeof(kFrameLightsOnParts) / sizeof(kFrameLightsOnParts[0])};
const ComposedFrame kFrameLightsOffComposed = {
    32, 32, kFrameLightsOffParts,
    sizeof(kFrameLightsOffParts) / sizeof(kFrameLightsOffParts[0])};
const ComposedFrame kFrameLightsSelectorOnComposed = {
    32, 32, kFrameLightsSelectorOnParts,
    sizeof(kFrameLightsSelectorOnParts) / sizeof(kFrameLightsSelectorOnParts[0])};
const ComposedFrame kFrameLightsSelectorOffComposed = {
    32, 32, kFrameLightsSelectorOffParts,
    sizeof(kFrameLightsSelectorOffParts) /
        sizeof(kFrameLightsSelectorOffParts[0])};
const ComposedFrame kFrameLightsWakeComposed = {
    32, 32, kFrameLightsWakeParts,
    sizeof(kFrameLightsWakeParts) / sizeof(kFrameLightsWakeParts[0])};
const ComposedFrame kFrameLightsInvalidComposed = {
    32, 32, kFrameLightsInvalidParts,
    sizeof(kFrameLightsInvalidParts) / sizeof(kFrameLightsInvalidParts[0])};
const ComposedFrame kFrameDisciplineTimeoutComposed = {
    32, 32, kFrameDisciplineTimeoutParts,
    sizeof(kFrameDisciplineTimeoutParts) / sizeof(kFrameDisciplineTimeoutParts[0])};
const ComposedFrame kFrameDisciplinePraiseComposed = {
    32, 32, kFrameDisciplinePraiseParts,
    sizeof(kFrameDisciplinePraiseParts) / sizeof(kFrameDisciplinePraiseParts[0])};
const ComposedFrame kFrameDisciplineInvalidComposed = {
    32, 32, kFrameDisciplineInvalidParts,
    sizeof(kFrameDisciplineInvalidParts) / sizeof(kFrameDisciplineInvalidParts[0])};
const ComposedFrame kFrameAttentionCallComposed = {
    32, 32, kFrameAttentionCallParts,
    sizeof(kFrameAttentionCallParts) / sizeof(kFrameAttentionCallParts[0])};
const ComposedFrame kFrameAttentionMissedComposed = {
    32, 32, kFrameAttentionMissedParts,
    sizeof(kFrameAttentionMissedParts) / sizeof(kFrameAttentionMissedParts[0])};
const ComposedFrame kFrameItemPlayComposed = {
    32, 32, kFrameItemPlayParts,
    sizeof(kFrameItemPlayParts) / sizeof(kFrameItemPlayParts[0])};
const ComposedFrame kFrameShopBoothComposed = {
    56, 24, kFrameShopBoothParts,
    sizeof(kFrameShopBoothParts) / sizeof(kFrameShopBoothParts[0])};
const ComposedFrame kFrameShopkeeperIdleComposed = {
    32, 32, kFrameShopkeeperIdleParts,
    sizeof(kFrameShopkeeperIdleParts) / sizeof(kFrameShopkeeperIdleParts[0])};
const ComposedFrame kFrameShopkeeperSurpriseComposed = {
    32, 32, kFrameShopkeeperSurpriseParts,
    sizeof(kFrameShopkeeperSurpriseParts) /
        sizeof(kFrameShopkeeperSurpriseParts[0])};
const ComposedFrame kFrameShopkeeperHappyComposed = {
    32, 32, kFrameShopkeeperHappyParts,
    sizeof(kFrameShopkeeperHappyParts) / sizeof(kFrameShopkeeperHappyParts[0])};
const ComposedFrame kFrameShopItemPreviewComposed = {
    24, 24, kFrameShopItemPreviewParts,
    sizeof(kFrameShopItemPreviewParts) / sizeof(kFrameShopItemPreviewParts[0])};
const ComposedFrame kFrameShopBuyOkComposed = {
    24, 24, kFrameShopBuyOkParts,
    sizeof(kFrameShopBuyOkParts) / sizeof(kFrameShopBuyOkParts[0])};
const ComposedFrame kFrameShopBuyNoMoneyComposed = {
    24, 24, kFrameShopBuyNoMoneyParts,
    sizeof(kFrameShopBuyNoMoneyParts) / sizeof(kFrameShopBuyNoMoneyParts[0])};
const ComposedFrame kFrameShopBuyFullComposed = {
    24, 24, kFrameShopBuyFullParts,
    sizeof(kFrameShopBuyFullParts) / sizeof(kFrameShopBuyFullParts[0])};
const ComposedFrame kFrameShopSoldOutComposed = {
    24, 24, kFrameShopSoldOutParts,
    sizeof(kFrameShopSoldOutParts) / sizeof(kFrameShopSoldOutParts[0])};
const ComposedFrame kFrameLinkSendComposed = {
    32, 32, kFrameLinkSendParts,
    sizeof(kFrameLinkSendParts) / sizeof(kFrameLinkSendParts[0])};
const ComposedFrame kFrameLinkReceiveComposed = {
    32, 32, kFrameLinkReceiveParts,
    sizeof(kFrameLinkReceiveParts) / sizeof(kFrameLinkReceiveParts[0])};
const ComposedFrame kFrameLovePartnerComposed = {
    32, 32, kFrameLovePartnerParts,
    sizeof(kFrameLovePartnerParts) / sizeof(kFrameLovePartnerParts[0])};
const ComposedFrame kFrameLoveBabyComposed = {
    32, 32, kFrameLoveBabyParts,
    sizeof(kFrameLoveBabyParts) / sizeof(kFrameLoveBabyParts[0])};
const ComposedFrame kFrameParentDepartComposed = {
    32, 32, kFrameParentDepartParts,
    sizeof(kFrameParentDepartParts) / sizeof(kFrameParentDepartParts[0])};
const ComposedFrame kFrameGameWinComposed = {
    32, 32, kFrameGameWinParts,
    sizeof(kFrameGameWinParts) / sizeof(kFrameGameWinParts[0])};
const ComposedFrame kFrameGameLoseComposed = {
    32, 32, kFrameGameLoseParts,
    sizeof(kFrameGameLoseParts) / sizeof(kFrameGameLoseParts[0])};
const ComposedFrame kFrameGameGetNoteComposed = {
    32, 32, kFrameGameGetNoteParts,
    sizeof(kFrameGameGetNoteParts) / sizeof(kFrameGameGetNoteParts[0])};
const ComposedFrame kFrameGameGetBadComposed = {
    32, 32, kFrameGameGetBadParts,
    sizeof(kFrameGameGetBadParts) / sizeof(kFrameGameGetBadParts[0])};
const ComposedFrame kFrameGameGetCatchComposed = {
    32, 32, kFrameGameGetCatchParts,
    sizeof(kFrameGameGetCatchParts) / sizeof(kFrameGameGetCatchParts[0])};
const ComposedFrame kFrameGameGetMissComposed = {
    32, 32, kFrameGameGetMissParts,
    sizeof(kFrameGameGetMissParts) / sizeof(kFrameGameGetMissParts[0])};
const ComposedFrame kFrameGameBumpMeterComposed = {
    32, 32, kFrameGameBumpMeterParts,
    sizeof(kFrameGameBumpMeterParts) / sizeof(kFrameGameBumpMeterParts[0])};
const ComposedFrame kFrameGameBumpPushComposed = {
    32, 32, kFrameGameBumpPushParts,
    sizeof(kFrameGameBumpPushParts) / sizeof(kFrameGameBumpPushParts[0])};
const ComposedFrame kFrameGameBumpFallComposed = {
    32, 32, kFrameGameBumpFallParts,
    sizeof(kFrameGameBumpFallParts) / sizeof(kFrameGameBumpFallParts[0])};
const ComposedFrame kFrameGameFlagLeftComposed = {
    32, 32, kFrameGameFlagLeftParts,
    sizeof(kFrameGameFlagLeftParts) / sizeof(kFrameGameFlagLeftParts[0])};
const ComposedFrame kFrameGameFlagRightComposed = {
    32, 32, kFrameGameFlagRightParts,
    sizeof(kFrameGameFlagRightParts) / sizeof(kFrameGameFlagRightParts[0])};
const ComposedFrame kFrameGameFlagBothComposed = {
    32, 32, kFrameGameFlagBothParts,
    sizeof(kFrameGameFlagBothParts) / sizeof(kFrameGameFlagBothParts[0])};
const ComposedFrame kFrameGameFlagGoodComposed = {
    32, 32, kFrameGameFlagGoodParts,
    sizeof(kFrameGameFlagGoodParts) / sizeof(kFrameGameFlagGoodParts[0])};
const ComposedFrame kFrameGameFlagMissComposed = {
    32, 32, kFrameGameFlagMissParts,
    sizeof(kFrameGameFlagMissParts) / sizeof(kFrameGameFlagMissParts[0])};
const ComposedFrame kFrameGameHeadingBallComposed = {
    32, 32, kFrameGameHeadingBallParts,
    sizeof(kFrameGameHeadingBallParts) / sizeof(kFrameGameHeadingBallParts[0])};
const ComposedFrame kFrameGameHeadingHitComposed = {
    32, 32, kFrameGameHeadingHitParts,
    sizeof(kFrameGameHeadingHitParts) / sizeof(kFrameGameHeadingHitParts[0])};
const ComposedFrame kFrameGameHeadingMissComposed = {
    32, 32, kFrameGameHeadingMissParts,
    sizeof(kFrameGameHeadingMissParts) / sizeof(kFrameGameHeadingMissParts[0])};
const ComposedFrame kFrameGameMemoryRevealComposed = {
    32, 32, kFrameGameMemoryRevealParts,
    sizeof(kFrameGameMemoryRevealParts) / sizeof(kFrameGameMemoryRevealParts[0])};
const ComposedFrame kFrameGameMemoryCursorComposed = {
    32, 32, kFrameGameMemoryCursorParts,
    sizeof(kFrameGameMemoryCursorParts) / sizeof(kFrameGameMemoryCursorParts[0])};
const ComposedFrame kFrameGameMemoryGoodComposed = {
    32, 32, kFrameGameMemoryGoodParts,
    sizeof(kFrameGameMemoryGoodParts) / sizeof(kFrameGameMemoryGoodParts[0])};
const ComposedFrame kFrameGameMemoryWrongComposed = {
    32, 32, kFrameGameMemoryWrongParts,
    sizeof(kFrameGameMemoryWrongParts) / sizeof(kFrameGameMemoryWrongParts[0])};
const ComposedFrame kFrameGameSprintRunner0Composed = {
    32, 32, kFrameGameSprintRunner0Parts,
    sizeof(kFrameGameSprintRunner0Parts) / sizeof(kFrameGameSprintRunner0Parts[0])};
const ComposedFrame kFrameGameSprintRunner1Composed = {
    32, 32, kFrameGameSprintRunner1Parts,
    sizeof(kFrameGameSprintRunner1Parts) / sizeof(kFrameGameSprintRunner1Parts[0])};
const ComposedFrame kFrameGameSprintFinishComposed = {
    32, 32, kFrameGameSprintFinishParts,
    sizeof(kFrameGameSprintFinishParts) / sizeof(kFrameGameSprintFinishParts[0])};
const ComposedFrame kFrameGameHoopsHoopComposed = {
    32, 32, kFrameGameHoopsHoopParts,
    sizeof(kFrameGameHoopsHoopParts) / sizeof(kFrameGameHoopsHoopParts[0])};
const ComposedFrame kFrameGameHoopsShootComposed = {
    32, 32, kFrameGameHoopsShootParts,
    sizeof(kFrameGameHoopsShootParts) / sizeof(kFrameGameHoopsShootParts[0])};
const ComposedFrame kFrameGameHoopsMadeComposed = {
    32, 32, kFrameGameHoopsMadeParts,
    sizeof(kFrameGameHoopsMadeParts) / sizeof(kFrameGameHoopsMadeParts[0])};
const ComposedFrame kFrameGameHoopsMissComposed = {
    32, 32, kFrameGameHoopsMissParts,
    sizeof(kFrameGameHoopsMissParts) / sizeof(kFrameGameHoopsMissParts[0])};
const ComposedFrame kFramePassedComposed = {
    32, 32, kFramePassedParts,
    sizeof(kFramePassedParts) / sizeof(kFramePassedParts[0])};

const uint8_t kTiles[][8] PROGMEM = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // empty
    {0x03, 0x0C, 0x10, 0x20, 0x40, 0x40, 0x80, 0x80},  // egg tl
    {0xC0, 0x30, 0x08, 0x04, 0x02, 0x02, 0x01, 0x01},  // egg tr
    {0x80, 0x80, 0x40, 0x40, 0x20, 0x10, 0x0C, 0x03},  // egg bl
    {0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0x30, 0xC0},  // egg br
    {0x0F, 0x10, 0x20, 0x40, 0x40, 0x80, 0x80, 0x80},  // body tl
    {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // body top
    {0xF0, 0x08, 0x04, 0x02, 0x02, 0x01, 0x01, 0x01},  // body tr
    {0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80},  // body left
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},  // body right
    {0x80, 0x80, 0x80, 0x40, 0x40, 0x20, 0x10, 0x0F},  // body bl
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},  // body bottom
    {0x01, 0x01, 0x01, 0x02, 0x02, 0x04, 0x08, 0xF0},  // body br
    {0x00, 0x00, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00},  // eyes
    {0x00, 0x00, 0x3C, 0x00, 0x00, 0x3C, 0x00, 0x00},  // sleep eyes
    {0x00, 0x00, 0x00, 0x00, 0x42, 0x24, 0x18, 0x00},  // smile
    {0x00, 0x00, 0x00, 0x00, 0x18, 0x24, 0x42, 0x00},  // sad
    {0x00, 0x00, 0x00, 0x00, 0x70, 0x70, 0x00, 0x00},  // foot l
    {0x00, 0x00, 0x00, 0x00, 0x0E, 0x0E, 0x00, 0x00},  // foot r
    {0x00, 0x04, 0x08, 0x10, 0x20, 0x20, 0x00, 0x00},  // antenna l
    {0x00, 0x20, 0x10, 0x08, 0x04, 0x04, 0x00, 0x00},  // antenna r
    {0x00, 0x24, 0x7E, 0x5A, 0x7E, 0x42, 0x00, 0x00},  // crown
    {0x00, 0x7E, 0x00, 0x24, 0x00, 0x7E, 0x00, 0x00},  // wrinkle
    {0x00, 0x42, 0x24, 0x18, 0x18, 0x24, 0x42, 0x00},  // sick eyes
    {0x7E, 0x04, 0x08, 0x10, 0x20, 0x40, 0x7E, 0x00},  // z
    {0x00, 0x18, 0x24, 0x42, 0x7E, 0x7E, 0x3C, 0x00},  // mess
    {0x00, 0x66, 0xFF, 0xFF, 0x7E, 0x3C, 0x18, 0x00},  // heart
    {0x00, 0x00, 0x20, 0x24, 0x20, 0x00, 0x00, 0x00},  // wink
    {0x00, 0x42, 0x24, 0x18, 0x24, 0x42, 0x00, 0x00},  // angry eyes
    {0x00, 0x00, 0x00, 0x18, 0x24, 0x24, 0x18, 0x00},  // open mouth
    {0x10, 0x10, 0x54, 0x38, 0xFE, 0x38, 0x54, 0x10},  // sparkle
    {0x7E, 0x42, 0x5A, 0x52, 0x5A, 0x42, 0x7E, 0x00},  // book
    {0x3C, 0x42, 0x99, 0xA5, 0xA5, 0x99, 0x42, 0x3C},  // ball
    {0x42, 0xE7, 0x7E, 0x3C, 0x3C, 0x7E, 0xE7, 0x42},  // bow
    {0x18, 0x3C, 0x7E, 0xDB, 0x18, 0x18, 0x00, 0x00},  // spike
};

const Cell kEgg0[] PROGMEM = {
    {0, 0, T_BODY_TL}, {1, 0, T_BODY_TOP}, {2, 0, T_BODY_TR},
    {0, 1, T_BODY_L},  {1, 1, T_EYE_PAIR}, {2, 1, T_BODY_R},
    {0, 2, T_BODY_BL}, {1, 2, T_MOUTH_SMILE}, {2, 2, T_BODY_BR}};

const Cell kEgg1[] PROGMEM = {
    {0, 0, T_BODY_TL}, {1, 0, T_BODY_TOP}, {2, 0, T_BODY_TR},
    {0, 1, T_BODY_L},  {1, 1, T_EYE_PAIR}, {2, 1, T_BODY_R},
    {0, 2, T_BODY_BL}, {1, 2, T_MOUTH_SAD}, {2, 2, T_BODY_BR}};

const Cell kChild0[] PROGMEM = {
    {0, 0, T_BODY_TL}, {1, 0, T_BODY_TOP}, {2, 0, T_BODY_TR},
    {0, 1, T_BODY_L},  {1, 1, T_EYE_PAIR}, {2, 1, T_BODY_R},
    {0, 2, T_BODY_BL}, {1, 2, T_MOUTH_SMILE}, {2, 2, T_BODY_BR},
    {0, 3, T_FOOT_L},  {2, 3, T_FOOT_R}};

const Cell kChild1[] PROGMEM = {
    {0, 0, T_BODY_TL}, {1, 0, T_BODY_TOP}, {2, 0, T_BODY_TR},
    {0, 1, T_BODY_L},  {1, 1, T_EYE_PAIR}, {2, 1, T_BODY_R},
    {0, 2, T_BODY_BL}, {1, 2, T_MOUTH_SMILE}, {2, 2, T_BODY_BR},
    {0, 3, T_FOOT_R},  {2, 3, T_FOOT_L}};

const Cell kTeen0[] PROGMEM = {
    {1, -1, T_ANT_L},    {2, -1, T_ANT_R},    {0, 0, T_BODY_TL},
    {1, 0, T_BODY_TOP},  {2, 0, T_BODY_TOP},  {3, 0, T_BODY_TR},
    {0, 1, T_BODY_L},    {1, 1, T_EYE_PAIR},  {3, 1, T_BODY_R},
    {0, 2, T_BODY_L},    {2, 2, T_MOUTH_SMILE}, {3, 2, T_BODY_R},
    {0, 3, T_BODY_BL},   {1, 3, T_BODY_BOTTOM}, {2, 3, T_BODY_BOTTOM},
    {3, 3, T_BODY_BR},   {1, 4, T_FOOT_L},    {3, 4, T_FOOT_R}};

const Cell kTeen1[] PROGMEM = {
    {1, -1, T_ANT_L},    {2, -1, T_ANT_R},    {0, 0, T_BODY_TL},
    {1, 0, T_BODY_TOP},  {2, 0, T_BODY_TOP},  {3, 0, T_BODY_TR},
    {0, 1, T_BODY_L},    {1, 1, T_EYE_PAIR},  {3, 1, T_BODY_R},
    {0, 2, T_BODY_L},    {2, 2, T_MOUTH_SMILE}, {3, 2, T_BODY_R},
    {0, 3, T_BODY_BL},   {1, 3, T_BODY_BOTTOM}, {2, 3, T_BODY_BOTTOM},
    {3, 3, T_BODY_BR},   {0, 4, T_FOOT_L},    {2, 4, T_FOOT_R}};

const Cell kAdult0[] PROGMEM = {
    {1, -1, T_ANT_L},    {3, -1, T_ANT_R},    {0, 0, T_BODY_TL},
    {1, 0, T_BODY_TOP},  {2, 0, T_BODY_TOP},  {3, 0, T_BODY_TOP},
    {4, 0, T_BODY_TR},   {0, 1, T_BODY_L},    {2, 1, T_EYE_PAIR},
    {4, 1, T_BODY_R},    {0, 2, T_BODY_L},    {2, 2, T_MOUTH_SMILE},
    {4, 2, T_BODY_R},    {0, 3, T_BODY_BL},   {1, 3, T_BODY_BOTTOM},
    {2, 3, T_BODY_BOTTOM}, {3, 3, T_BODY_BOTTOM}, {4, 3, T_BODY_BR},
    {1, 4, T_FOOT_L},    {4, 4, T_FOOT_R}};

const Cell kAdult1[] PROGMEM = {
    {1, -1, T_CROWN},    {0, 0, T_BODY_TL},   {1, 0, T_BODY_TOP},
    {2, 0, T_BODY_TOP},  {3, 0, T_BODY_TOP},  {4, 0, T_BODY_TR},
    {0, 1, T_BODY_L},    {2, 1, T_EYE_PAIR},  {4, 1, T_BODY_R},
    {0, 2, T_BODY_L},    {2, 2, T_MOUTH_SMILE}, {4, 2, T_BODY_R},
    {0, 3, T_BODY_BL},   {1, 3, T_BODY_BOTTOM}, {2, 3, T_BODY_BOTTOM},
    {3, 3, T_BODY_BOTTOM}, {4, 3, T_BODY_BR}, {0, 4, T_FOOT_L},
    {3, 4, T_FOOT_R}};

const Cell kElder0[] PROGMEM = {
    {0, 0, T_BODY_TL},   {1, 0, T_BODY_TOP},  {2, 0, T_BODY_TOP},
    {3, 0, T_BODY_TOP},  {4, 0, T_BODY_TR},   {0, 1, T_BODY_L},
    {2, 1, T_WRINKLE},   {4, 1, T_BODY_R},    {0, 2, T_BODY_L},
    {2, 2, T_MOUTH_SMILE}, {4, 2, T_BODY_R},  {0, 3, T_BODY_BL},
    {1, 3, T_BODY_BOTTOM}, {2, 3, T_BODY_BOTTOM}, {3, 3, T_BODY_BOTTOM},
    {4, 3, T_BODY_BR},   {1, 4, T_FOOT_L},    {4, 4, T_FOOT_R}};

const Cell kElder1[] PROGMEM = {
    {0, 0, T_BODY_TL},   {1, 0, T_BODY_TOP},  {2, 0, T_BODY_TOP},
    {3, 0, T_BODY_TOP},  {4, 0, T_BODY_TR},   {0, 1, T_BODY_L},
    {2, 1, T_WRINKLE},   {4, 1, T_BODY_R},    {0, 2, T_BODY_L},
    {2, 2, T_MOUTH_SAD}, {4, 2, T_BODY_R},    {0, 3, T_BODY_BL},
    {1, 3, T_BODY_BOTTOM}, {2, 3, T_BODY_BOTTOM}, {3, 3, T_BODY_BOTTOM},
    {4, 3, T_BODY_BR},   {0, 4, T_FOOT_L},    {3, 4, T_FOOT_R}};

const Cell kSleep[] PROGMEM = {
    {0, 0, T_BODY_TL}, {1, 0, T_BODY_TOP}, {2, 0, T_BODY_TR},
    {0, 1, T_BODY_L},  {1, 1, T_EYE_SLEEP}, {2, 1, T_BODY_R},
    {0, 2, T_BODY_BL}, {1, 2, T_MOUTH_SMILE}, {2, 2, T_BODY_BR},
    {3, 0, T_Z_MARK},  {4, -1, T_Z_MARK}};

const Cell kSick[] PROGMEM = {
    {0, 0, T_BODY_TL}, {1, 0, T_BODY_TOP}, {2, 0, T_BODY_TOP},
    {3, 0, T_BODY_TR}, {0, 1, T_BODY_L},   {1, 1, T_SICK_EYES},
    {3, 1, T_BODY_R},  {0, 2, T_BODY_L},   {2, 2, T_MOUTH_SAD},
    {3, 2, T_BODY_R},  {0, 3, T_BODY_BL},  {1, 3, T_BODY_BOTTOM},
    {2, 3, T_BODY_BOTTOM}, {3, 3, T_BODY_BR}};

const Cell kJoy[] PROGMEM = {
    {0, 0, T_BODY_TL},   {1, 0, T_BODY_TOP}, {2, 0, T_BODY_TOP},
    {3, 0, T_BODY_TR},   {0, 1, T_BODY_L},   {1, 1, T_EYE_WINK},
    {3, 1, T_BODY_R},    {0, 2, T_BODY_L},   {2, 2, T_MOUTH_OPEN},
    {3, 2, T_BODY_R},    {0, 3, T_BODY_BL},  {1, 3, T_BODY_BOTTOM},
    {2, 3, T_BODY_BOTTOM}, {3, 3, T_BODY_BR}, {4, 0, T_HEART}};

const Cell kSadFrame[] PROGMEM = {
    {0, 0, T_BODY_TL},   {1, 0, T_BODY_TOP}, {2, 0, T_BODY_TOP},
    {3, 0, T_BODY_TR},   {0, 1, T_BODY_L},   {1, 1, T_EYE_ANGRY},
    {3, 1, T_BODY_R},    {0, 2, T_BODY_L},   {2, 2, T_MOUTH_SAD},
    {3, 2, T_BODY_R},    {0, 3, T_BODY_BL},  {1, 3, T_BODY_BOTTOM},
    {2, 3, T_BODY_BOTTOM}, {3, 3, T_BODY_BR}};

const Cell kFriend[] PROGMEM = {
    {0, 0, T_BODY_TL},   {1, 0, T_BODY_TOP}, {2, 0, T_BODY_TOP},
    {3, 0, T_BODY_TR},   {0, 1, T_BODY_L},   {1, 1, T_EYE_PAIR},
    {3, 1, T_BODY_R},    {0, 2, T_BODY_L},   {2, 2, T_MOUTH_SMILE},
    {3, 2, T_BODY_R},    {0, 3, T_BODY_BL},  {1, 3, T_BODY_BOTTOM},
    {2, 3, T_BODY_BOTTOM}, {3, 3, T_BODY_BR}, {-1, 0, T_HEART},
    {4, 1, T_HEART}};

const Cell kAthlete[] PROGMEM = {
    {1, -1, T_ANT_L},    {3, -1, T_ANT_R},   {0, 0, T_BODY_TL},
    {1, 0, T_BODY_TOP},  {2, 0, T_BODY_TOP}, {3, 0, T_BODY_TOP},
    {4, 0, T_BODY_TR},   {0, 1, T_BODY_L},   {2, 1, T_EYE_PAIR},
    {4, 1, T_BODY_R},    {0, 2, T_BODY_L},   {2, 2, T_MOUTH_SMILE},
    {4, 2, T_BODY_R},    {0, 3, T_BODY_BL},  {1, 3, T_BODY_BOTTOM},
    {2, 3, T_BODY_BOTTOM}, {3, 3, T_BODY_BOTTOM}, {4, 3, T_BODY_BR},
    {0, 4, T_BALL},      {4, 4, T_FOOT_R}};

const Cell kScholar[] PROGMEM = {
    {1, -1, T_CROWN},    {0, 0, T_BODY_TL},  {1, 0, T_BODY_TOP},
    {2, 0, T_BODY_TOP},  {3, 0, T_BODY_TOP}, {4, 0, T_BODY_TR},
    {0, 1, T_BODY_L},    {2, 1, T_EYE_PAIR}, {4, 1, T_BODY_R},
    {0, 2, T_BODY_L},    {2, 2, T_MOUTH_SMILE}, {4, 2, T_BODY_R},
    {0, 3, T_BODY_BL},   {1, 3, T_BODY_BOTTOM}, {2, 3, T_BODY_BOTTOM},
    {3, 3, T_BODY_BOTTOM}, {4, 3, T_BODY_BR}, {-1, 2, T_BOOK},
    {4, 4, T_FOOT_R}};

const Cell kDream[] PROGMEM = {
    {1, -1, T_BOW},      {0, 0, T_BODY_TL},  {1, 0, T_BODY_TOP},
    {2, 0, T_BODY_TOP},  {3, 0, T_BODY_TOP}, {4, 0, T_BODY_TR},
    {0, 1, T_BODY_L},    {2, 1, T_EYE_PAIR}, {4, 1, T_BODY_R},
    {0, 2, T_BODY_L},    {2, 2, T_MOUTH_SMILE}, {4, 2, T_BODY_R},
    {0, 3, T_BODY_BL},   {1, 3, T_BODY_BOTTOM}, {2, 3, T_BODY_BOTTOM},
    {3, 3, T_BODY_BOTTOM}, {4, 3, T_BODY_BR}, {5, 1, T_SPARKLE},
    {0, 4, T_FOOT_L}};

const Cell kRascal[] PROGMEM = {
    {1, -1, T_SPIKE},    {3, -1, T_SPIKE},   {0, 0, T_BODY_TL},
    {1, 0, T_BODY_TOP},  {2, 0, T_BODY_TOP}, {3, 0, T_BODY_TOP},
    {4, 0, T_BODY_TR},   {0, 1, T_BODY_L},   {2, 1, T_EYE_ANGRY},
    {4, 1, T_BODY_R},    {0, 2, T_BODY_L},   {2, 2, T_MOUTH_OPEN},
    {4, 2, T_BODY_R},    {0, 3, T_BODY_BL},  {1, 3, T_BODY_BOTTOM},
    {2, 3, T_BODY_BOTTOM}, {3, 3, T_BODY_BOTTOM}, {4, 3, T_BODY_BR},
    {1, 4, T_FOOT_L},    {4, 4, T_FOOT_R}};

const ComposedFrame* connectFrame(SpriteFrame frame) {
  switch (frame) {
    case SpriteFrame::kChild0:
    case SpriteFrame::kTeen0:
    case SpriteFrame::kAdult0:
    case SpriteFrame::kElder0:
      return &kFrameStand0;
    case SpriteFrame::kChild1:
    case SpriteFrame::kTeen1:
    case SpriteFrame::kAdult1:
    case SpriteFrame::kElder1:
      return &kFrameStand1;
    case SpriteFrame::kSleep:
      return &kFrameSleepComposed;
    case SpriteFrame::kSick:
      return &kFrameSickComposed;
    case SpriteFrame::kJoy:
      return &kFrameHappy;
    case SpriteFrame::kSad:
      return &kFrameSadComposed;
    case SpriteFrame::kFriend:
      return &kFrameFriendComposed;
    case SpriteFrame::kAthlete:
      return &kFrameAthleteComposed;
    case SpriteFrame::kScholar:
      return &kFrameScholarComposed;
    case SpriteFrame::kDream:
      return &kFrameDreamComposed;
    case SpriteFrame::kRascal:
      return &kFrameRascalComposed;
    case SpriteFrame::kEgg0:
      return &kFrameEgg0Composed;
    case SpriteFrame::kEgg1:
      return &kFrameEgg1Composed;
    case SpriteFrame::kEggCrack0:
      return &kFrameEggCrack0Composed;
    case SpriteFrame::kEggCrack1:
      return &kFrameEggCrack1Composed;
    case SpriteFrame::kEggHatch:
      return &kFrameEggHatchComposed;
    case SpriteFrame::kBaby0:
      return &kFrameBaby0Composed;
    case SpriteFrame::kBaby1:
      return &kFrameBaby1Composed;
    case SpriteFrame::kEatMeal0:
      return &kFrameEatMeal0Composed;
    case SpriteFrame::kEatMeal1:
      return &kFrameEatMeal1Composed;
    case SpriteFrame::kEatSnack0:
      return &kFrameEatSnack0Composed;
    case SpriteFrame::kEatSnack1:
      return &kFrameEatSnack1Composed;
    case SpriteFrame::kFoodCrumbs:
      return &kFrameFoodCrumbsComposed;
    case SpriteFrame::kFoodRefuse:
      return &kFrameFoodRefuseComposed;
    case SpriteFrame::kFoodDone:
      return &kFrameFoodDoneComposed;
    case SpriteFrame::kToilet0:
      return &kFrameToilet0Composed;
    case SpriteFrame::kToilet1:
      return &kFrameToilet1Composed;
    case SpriteFrame::kToiletMess:
      return &kFrameToiletMessComposed;
    case SpriteFrame::kToiletCleanupWall0:
      return &kFrameToiletCleanupWall0Composed;
    case SpriteFrame::kToiletCleanupWall1:
      return &kFrameToiletCleanupWall1Composed;
    case SpriteFrame::kToiletDone:
      return &kFrameToiletDoneComposed;
    case SpriteFrame::kToiletNoMess:
      return &kFrameToiletNoMessComposed;
    case SpriteFrame::kMedicine0:
      return &kFrameMedicine0Composed;
    case SpriteFrame::kMedicine1:
      return &kFrameMedicine1Composed;
    case SpriteFrame::kMedicineSickSkull:
      return &kFrameMedicineSickSkullComposed;
    case SpriteFrame::kMedicineSickTooth:
      return &kFrameMedicineSickToothComposed;
    case SpriteFrame::kMedicineDose0:
      return &kFrameMedicineDose0Composed;
    case SpriteFrame::kMedicineDose1:
      return &kFrameMedicineDose1Composed;
    case SpriteFrame::kMedicineRecover:
      return &kFrameMedicineRecoverComposed;
    case SpriteFrame::kMedicineRefuse:
      return &kFrameMedicineRefuseComposed;
    case SpriteFrame::kLightsOn:
      return &kFrameLightsOnComposed;
    case SpriteFrame::kLightsOff:
      return &kFrameLightsOffComposed;
    case SpriteFrame::kLightsSelectorOn:
      return &kFrameLightsSelectorOnComposed;
    case SpriteFrame::kLightsSelectorOff:
      return &kFrameLightsSelectorOffComposed;
    case SpriteFrame::kLightsWake:
      return &kFrameLightsWakeComposed;
    case SpriteFrame::kLightsInvalid:
      return &kFrameLightsInvalidComposed;
    case SpriteFrame::kDisciplineTimeout:
      return &kFrameDisciplineTimeoutComposed;
    case SpriteFrame::kDisciplinePraise:
      return &kFrameDisciplinePraiseComposed;
    case SpriteFrame::kDisciplineInvalid:
      return &kFrameDisciplineInvalidComposed;
    case SpriteFrame::kAttentionCall:
      return &kFrameAttentionCallComposed;
    case SpriteFrame::kAttentionMissed:
      return &kFrameAttentionMissedComposed;
    case SpriteFrame::kItemPlay:
      return &kFrameItemPlayComposed;
    case SpriteFrame::kShopBooth:
      return &kFrameShopBoothComposed;
    case SpriteFrame::kShopkeeperIdle:
      return &kFrameShopkeeperIdleComposed;
    case SpriteFrame::kShopkeeperSurprise:
      return &kFrameShopkeeperSurpriseComposed;
    case SpriteFrame::kShopkeeperHappy:
      return &kFrameShopkeeperHappyComposed;
    case SpriteFrame::kShopItemPreview:
      return &kFrameShopItemPreviewComposed;
    case SpriteFrame::kShopBuyOk:
      return &kFrameShopBuyOkComposed;
    case SpriteFrame::kShopBuyNoMoney:
      return &kFrameShopBuyNoMoneyComposed;
    case SpriteFrame::kShopBuyFull:
      return &kFrameShopBuyFullComposed;
    case SpriteFrame::kShopSoldOut:
      return &kFrameShopSoldOutComposed;
    case SpriteFrame::kLinkSend:
      return &kFrameLinkSendComposed;
    case SpriteFrame::kLinkReceive:
      return &kFrameLinkReceiveComposed;
    case SpriteFrame::kLovePartner:
      return &kFrameLovePartnerComposed;
    case SpriteFrame::kLoveBaby:
      return &kFrameLoveBabyComposed;
    case SpriteFrame::kParentDepart:
      return &kFrameParentDepartComposed;
    case SpriteFrame::kGameWin:
      return &kFrameGameWinComposed;
    case SpriteFrame::kGameLose:
      return &kFrameGameLoseComposed;
    case SpriteFrame::kGameGetNote:
      return &kFrameGameGetNoteComposed;
    case SpriteFrame::kGameGetBad:
      return &kFrameGameGetBadComposed;
    case SpriteFrame::kGameGetCatch:
      return &kFrameGameGetCatchComposed;
    case SpriteFrame::kGameGetMiss:
      return &kFrameGameGetMissComposed;
    case SpriteFrame::kGameBumpMeter:
      return &kFrameGameBumpMeterComposed;
    case SpriteFrame::kGameBumpPush:
      return &kFrameGameBumpPushComposed;
    case SpriteFrame::kGameBumpFall:
      return &kFrameGameBumpFallComposed;
    case SpriteFrame::kGameFlagLeft:
      return &kFrameGameFlagLeftComposed;
    case SpriteFrame::kGameFlagRight:
      return &kFrameGameFlagRightComposed;
    case SpriteFrame::kGameFlagBoth:
      return &kFrameGameFlagBothComposed;
    case SpriteFrame::kGameFlagGood:
      return &kFrameGameFlagGoodComposed;
    case SpriteFrame::kGameFlagMiss:
      return &kFrameGameFlagMissComposed;
    case SpriteFrame::kGameHeadingBall:
      return &kFrameGameHeadingBallComposed;
    case SpriteFrame::kGameHeadingHit:
      return &kFrameGameHeadingHitComposed;
    case SpriteFrame::kGameHeadingMiss:
      return &kFrameGameHeadingMissComposed;
    case SpriteFrame::kGameMemoryReveal:
      return &kFrameGameMemoryRevealComposed;
    case SpriteFrame::kGameMemoryCursor:
      return &kFrameGameMemoryCursorComposed;
    case SpriteFrame::kGameMemoryGood:
      return &kFrameGameMemoryGoodComposed;
    case SpriteFrame::kGameMemoryWrong:
      return &kFrameGameMemoryWrongComposed;
    case SpriteFrame::kGameSprintRunner0:
      return &kFrameGameSprintRunner0Composed;
    case SpriteFrame::kGameSprintRunner1:
      return &kFrameGameSprintRunner1Composed;
    case SpriteFrame::kGameSprintFinish:
      return &kFrameGameSprintFinishComposed;
    case SpriteFrame::kGameHoopsHoop:
      return &kFrameGameHoopsHoopComposed;
    case SpriteFrame::kGameHoopsShoot:
      return &kFrameGameHoopsShootComposed;
    case SpriteFrame::kGameHoopsMade:
      return &kFrameGameHoopsMadeComposed;
    case SpriteFrame::kGameHoopsMiss:
      return &kFrameGameHoopsMissComposed;
    case SpriteFrame::kPassed:
      return &kFramePassedComposed;
  }
  return nullptr;
}

const Cell* frameCells(SpriteFrame frame, uint8_t& count) {
  switch (frame) {
    case SpriteFrame::kEgg0:
      count = sizeof(kEgg0) / sizeof(kEgg0[0]);
      return kEgg0;
    case SpriteFrame::kEgg1:
      count = sizeof(kEgg1) / sizeof(kEgg1[0]);
      return kEgg1;
    case SpriteFrame::kChild0:
      count = sizeof(kChild0) / sizeof(kChild0[0]);
      return kChild0;
    case SpriteFrame::kChild1:
      count = sizeof(kChild1) / sizeof(kChild1[0]);
      return kChild1;
    case SpriteFrame::kTeen0:
      count = sizeof(kTeen0) / sizeof(kTeen0[0]);
      return kTeen0;
    case SpriteFrame::kTeen1:
      count = sizeof(kTeen1) / sizeof(kTeen1[0]);
      return kTeen1;
    case SpriteFrame::kAdult0:
      count = sizeof(kAdult0) / sizeof(kAdult0[0]);
      return kAdult0;
    case SpriteFrame::kAdult1:
      count = sizeof(kAdult1) / sizeof(kAdult1[0]);
      return kAdult1;
    case SpriteFrame::kElder0:
      count = sizeof(kElder0) / sizeof(kElder0[0]);
      return kElder0;
    case SpriteFrame::kElder1:
      count = sizeof(kElder1) / sizeof(kElder1[0]);
      return kElder1;
    case SpriteFrame::kSleep:
      count = sizeof(kSleep) / sizeof(kSleep[0]);
      return kSleep;
    case SpriteFrame::kSick:
      count = sizeof(kSick) / sizeof(kSick[0]);
      return kSick;
    case SpriteFrame::kJoy:
      count = sizeof(kJoy) / sizeof(kJoy[0]);
      return kJoy;
    case SpriteFrame::kSad:
      count = sizeof(kSadFrame) / sizeof(kSadFrame[0]);
      return kSadFrame;
    case SpriteFrame::kFriend:
      count = sizeof(kFriend) / sizeof(kFriend[0]);
      return kFriend;
    case SpriteFrame::kAthlete:
      count = sizeof(kAthlete) / sizeof(kAthlete[0]);
      return kAthlete;
    case SpriteFrame::kScholar:
      count = sizeof(kScholar) / sizeof(kScholar[0]);
      return kScholar;
    case SpriteFrame::kDream:
      count = sizeof(kDream) / sizeof(kDream[0]);
      return kDream;
    case SpriteFrame::kRascal:
      count = sizeof(kRascal) / sizeof(kRascal[0]);
      return kRascal;
    case SpriteFrame::kEatMeal0:
    case SpriteFrame::kEatMeal1:
    case SpriteFrame::kEatSnack0:
    case SpriteFrame::kEatSnack1:
    case SpriteFrame::kToilet0:
    case SpriteFrame::kToilet1:
    case SpriteFrame::kMedicine0:
    case SpriteFrame::kMedicine1:
    case SpriteFrame::kLightsOff:
    case SpriteFrame::kDisciplineTimeout:
    case SpriteFrame::kDisciplinePraise:
    case SpriteFrame::kAttentionCall:
    case SpriteFrame::kItemPlay:
    case SpriteFrame::kLinkSend:
    case SpriteFrame::kLinkReceive:
    case SpriteFrame::kGameWin:
    case SpriteFrame::kGameLose:
    case SpriteFrame::kPassed:
      break;
    default:
      break;
  }
  count = 0;
  return nullptr;
}

}  // namespace

void drawTile(EchoPetDisplayDevice& display, uint8_t tile, int16_t x, int16_t y,
              uint8_t scale) {
  if (tile >= sizeof(kTiles) / sizeof(kTiles[0])) {
    return;
  }
  for (uint8_t row = 0; row < 8; row++) {
    const uint8_t bits = pgm_read_byte(&kTiles[tile][row]);
    for (uint8_t col = 0; col < 8; col++) {
      if (bits & (0x80 >> col)) {
        if (scale == 1) {
          display.drawPixel(x + col, y + row, EPD_BLACK);
        } else {
          display.fillRect(x + col * scale, y + row * scale, scale, scale,
                           EPD_BLACK);
        }
      }
    }
  }
}

void drawMessIcon(EchoPetDisplayDevice& display, int16_t x, int16_t y,
                  uint8_t scale) {
  drawTile(display, T_MESS, x, y, scale);
}

void drawBitmapAsset(EchoPetDisplayDevice& display, const BitmapAsset& bitmap,
                     int16_t x, int16_t y, uint8_t scale, uint8_t flags) {
  const bool flipX = (flags & kPartFlipX) != 0;
  for (uint8_t row = 0; row < bitmap.height; row++) {
    for (uint8_t col = 0; col < bitmap.width; col++) {
      const uint8_t sourceCol = flipX ? bitmap.width - 1 - col : col;
      const uint8_t bits = pgm_read_byte(
          bitmap.data + row * bitmap.stride + sourceCol / 8);
      if ((bits & (0x80 >> (sourceCol & 0x07))) == 0) {
        continue;
      }
      if (scale == 1) {
        display.drawPixel(x + col, y + row, EPD_BLACK);
      } else {
        display.fillRect(x + col * scale, y + row * scale, scale, scale,
                         EPD_BLACK);
      }
    }
  }
}

void drawComposedFrame(EchoPetDisplayDevice& display, const ComposedFrame& frame,
                       int16_t x, int16_t y, uint8_t scale) {
  for (uint8_t i = 0; i < frame.count; i++) {
    FramePart part;
    memcpy_P(&part, &frame.parts[i], sizeof(part));
    if (part.asset >= sizeof(kBitmapAssets) / sizeof(kBitmapAssets[0])) {
      continue;
    }
    drawBitmapAsset(display, kBitmapAssets[part.asset],
                    x + part.x * scale, y + part.y * scale, scale,
                    part.flags);
  }
}

void drawSpriteFrame(EchoPetDisplayDevice& display, SpriteFrame frame, int16_t x,
                     int16_t y, uint8_t scale) {
  const ComposedFrame* composed = connectFrame(frame);
  if (composed) {
    drawComposedFrame(display, *composed, x, y, scale);
    return;
  }

  uint8_t count = 0;
  const Cell* cells = frameCells(frame, count);
  if (!cells) {
    return;
  }
  const int16_t step = 8 * scale;
  for (uint8_t i = 0; i < count; i++) {
    Cell cell;
    memcpy_P(&cell, &cells[i], sizeof(cell));
    drawTile(display, cell.tile, x + cell.x * step, y + cell.y * step, scale);
  }
}

SpriteFrameInfo spriteFrameInfo(SpriteFrame frame) {
  const ComposedFrame* composed = connectFrame(frame);
  if (composed) {
    return {composed->width, composed->height};
  }

  uint8_t count = 0;
  const Cell* cells = frameCells(frame, count);
  if (!cells || count == 0) {
    return {0, 0};
  }

  int8_t minX = 127;
  int8_t minY = 127;
  int8_t maxX = -128;
  int8_t maxY = -128;
  for (uint8_t i = 0; i < count; i++) {
    Cell cell;
    memcpy_P(&cell, &cells[i], sizeof(cell));
    if (cell.x < minX) minX = cell.x;
    if (cell.y < minY) minY = cell.y;
    if (cell.x > maxX) maxX = cell.x;
    if (cell.y > maxY) maxY = cell.y;
  }

  return {static_cast<uint8_t>((maxX - minX + 1) * 8),
          static_cast<uint8_t>((maxY - minY + 1) * 8)};
}

SpriteFrame defaultStageFrame(Stage stage, bool alt) {
  switch (stage) {
    case Stage::kBaby:
      return alt ? SpriteFrame::kBaby1 : SpriteFrame::kBaby0;
    case Stage::kChild:
      return alt ? SpriteFrame::kChild1 : SpriteFrame::kChild0;
    case Stage::kTeen:
      return alt ? SpriteFrame::kTeen1 : SpriteFrame::kTeen0;
    case Stage::kAdult:
    case Stage::kParentCare:
      return alt ? SpriteFrame::kAdult1 : SpriteFrame::kAdult0;
    case Stage::kElder:
      return alt ? SpriteFrame::kElder1 : SpriteFrame::kElder0;
    case Stage::kEgg:
      return alt ? SpriteFrame::kEgg1 : SpriteFrame::kEgg0;
  }
  return SpriteFrame::kChild0;
}

SpriteFrame characterFrameFamilyFrame(const Snapshot& pet, bool alt) {
  if (pet.stage == Stage::kBaby) {
    return defaultStageFrame(pet.stage, alt);
  }
  if ((pet.memoryFlags & kMemoryFlagHohotchiCostume) != 0) {
    return alt ? SpriteFrame::kElder1 : SpriteFrame::kElder0;
  }
  if ((pet.memoryFlags & kMemoryFlagNyatchiCostume) != 0) {
    return SpriteFrame::kDream;
  }

  const CharacterCatalogEntry& character =
      characterCatalogEntry(pet.characterCatalogId);
  switch (character.frameFamily) {
    case 0:
    case 1:
      return alt ? SpriteFrame::kChild1 : SpriteFrame::kChild0;
    case 2:
      return SpriteFrame::kFriend;
    case 3:
      return alt ? SpriteFrame::kAdult1 : SpriteFrame::kAdult0;
    case 4:
      return SpriteFrame::kDream;
    case 5:
      return SpriteFrame::kAthlete;
    case 6:
      return SpriteFrame::kRascal;
    case 7:
      return SpriteFrame::kScholar;
    case 8:
      return alt ? SpriteFrame::kElder1 : SpriteFrame::kElder0;
    default:
      break;
  }
  return defaultStageFrame(pet.stage, alt);
}

SpriteFrame selectSpriteFrame(const Snapshot& pet, uint8_t phase) {
  if (pet.mood == Mood::kAsleep) {
    return SpriteFrame::kSleep;
  }
  if (pet.mood == Mood::kSick) {
    return SpriteFrame::kSick;
  }
  if (pet.mood == Mood::kPassed) {
    return SpriteFrame::kPassed;
  }

  switch (pet.notice) {
    case Notice::kMeal:
      switch (phase & 0x03) {
        case 1:
          return SpriteFrame::kEatMeal1;
        case 2:
          return SpriteFrame::kFoodCrumbs;
        case 3:
          return SpriteFrame::kFoodDone;
        default:
          return SpriteFrame::kEatMeal0;
      }
    case Notice::kSnack:
      switch (phase & 0x03) {
        case 1:
          return SpriteFrame::kEatSnack1;
        case 2:
          return SpriteFrame::kFoodCrumbs;
        case 3:
          return SpriteFrame::kFoodDone;
        default:
          return SpriteFrame::kEatSnack0;
      }
    case Notice::kFull:
      return SpriteFrame::kFoodRefuse;
    case Notice::kMedicine:
    case Notice::kNeedMoreMedicine:
      if (pet.notice == Notice::kMedicine && !(pet.sickness || pet.toothache)) {
        switch (phase & 0x03) {
          case 0:
            return SpriteFrame::kMedicineDose0;
          case 1:
            return SpriteFrame::kMedicineDose1;
          case 2:
            return SpriteFrame::kMedicineRecover;
          default:
            return SpriteFrame::kMedicine1;
        }
      }
      switch (phase & 0x03) {
        case 0:
          return pet.toothache ? SpriteFrame::kMedicineSickTooth
                               : SpriteFrame::kMedicineSickSkull;
        case 1:
          return SpriteFrame::kMedicineDose0;
        case 2:
          return SpriteFrame::kMedicineDose1;
        default:
          return pet.notice == Notice::kNeedMoreMedicine
                     ? SpriteFrame::kMedicine0
                     : SpriteFrame::kMedicineRecover;
      }
    case Notice::kNoMedicine:
      return SpriteFrame::kMedicineRefuse;
    case Notice::kLightsOff:
    case Notice::kSleeping:
      return SpriteFrame::kLightsOff;
    case Notice::kLightsOn:
      return SpriteFrame::kLightsOn;
    case Notice::kTrain:
      return SpriteFrame::kDisciplineTimeout;
    case Notice::kWrongDiscipline:
      return SpriteFrame::kDisciplineInvalid;
    case Notice::kPraise:
      return SpriteFrame::kDisciplinePraise;
    case Notice::kNeedsCare:
      return SpriteFrame::kAttentionCall;
    case Notice::kGameGood:
    case Notice::kGameWin:
      return SpriteFrame::kGameWin;
    case Notice::kItemUsed:
      return SpriteFrame::kItemPlay;
    case Notice::kBought:
      return SpriteFrame::kFoodDone;
    case Notice::kPartner:
      return SpriteFrame::kLovePartner;
    case Notice::kBaby:
      return SpriteFrame::kLoveBaby;
    case Notice::kParentLeft:
      return SpriteFrame::kParentDepart;
    case Notice::kFriendVisit:
    case Notice::kAnniversary:
    case Notice::kGift:
    case Notice::kFriendPresent:
      return SpriteFrame::kFriend;
    case Notice::kFriendReady:
    case Notice::kFriendGame:
      return (phase & 0x01) ? SpriteFrame::kLinkReceive
                            : SpriteFrame::kLinkSend;
    case Notice::kGameMiss:
    case Notice::kGameTired:
    case Notice::kLoveRejected:
    case Notice::kNoItem:
      return SpriteFrame::kGameLose;
    case Notice::kGameLose:
      return SpriteFrame::kGameBumpFall;
    case Notice::kPassed:
      return SpriteFrame::kPassed;
    default:
      break;
  }

  const bool alt = (phase & 0x01) != 0;
  switch (pet.stage) {
    case Stage::kEgg:
      if (pet.ageMinutes >= 4) {
        return (phase & 0x01) ? SpriteFrame::kEggHatch
                              : SpriteFrame::kEggCrack1;
      }
      if (pet.ageMinutes >= 2) {
        return (phase & 0x01) ? SpriteFrame::kEggCrack1
                              : SpriteFrame::kEggCrack0;
      }
      return alt ? SpriteFrame::kEgg1 : SpriteFrame::kEgg0;
    case Stage::kBaby:
    case Stage::kChild:
    case Stage::kTeen:
    case Stage::kAdult:
    case Stage::kParentCare:
    case Stage::kElder:
      return characterFrameFamilyFrame(pet, alt);
  }
  return SpriteFrame::kChild0;
}

}  // namespace echopet
