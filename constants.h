/* For generation of constants.h use https://jlus.de/destbox */

#define DEVICE_ID 1
#define EEPROM_VERSION 1


// reset, when value not equal to EEPROM_VERSION
#define ROM_RESET 0
// ID for the next OVRD-K
#define ROM_NEXT_OVRD 1
// Auth1 password set, when value = 255
#define ROM_AUTH1_SET 2
// 4 byte integer representing the AUTH1 pwd
// with 0 <= a < 4
#define ROM_AUTH1(a) (3+a)
// Auth2 key and counter set, when value = 255
#define ROM_AUTH2_SET 7
// the 20 byte shared secret for auth2
// with 0 <= a < 20
#define ROM_AUTH2_KEY(a) (8+a)
// the 4 byte big endian counter for auth2
// with 0 <= a < 4)
#define ROM_AUTH2_COUNTER(a) (28+a)
// Auth1 and Auth2 failed attempts
#define ROM_FAILED_COUNTER 32



const uint32_t ovrdKeyShas[10][5] = {
  // Sample value for Key = "6875327784"
  {0x5d6e1a86, 0xa66354e7, 0xe30bd313, 0x33b5d10c, 0x17493fba},
  {} // more coming here
};
