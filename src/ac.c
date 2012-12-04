#include <math.h>

#if 0
static const uint16_t integral_lut [] PROGMEM = {
  //x = acos(1 - 2*y) / PI
0x0000, // 0
0x0a36, // 1
0x0e73, // 2
0x11b6, // 3
0x1477, // 4
0x16e5, // 5
0x1919, // 6
0x1b20, // 7
0x1d05, // 8
0x1ecc, // 9
0x207d, // 10
0x2218, // 11
0x23a3, // 12
0x251e, // 13
0x268b, // 14
0x27ec, // 15
0x2943, // 16
0x2a8f, // 17
0x2bd3, // 18
0x2d0e, // 19
0x2e42, // 20
0x2f6f, // 21
0x3096, // 22
0x31b6, // 23
0x32d1, // 24
0x33e6, // 25
0x34f7, // 26
0x3603, // 27
0x370b, // 28
0x380f, // 29
0x390e, // 30
0x3a0b, // 31
0x3b03, // 32
0x3bf8, // 33
0x3ceb, // 34
0x3dda, // 35
0x3ec6, // 36
0x3fb0, // 37
0x4097, // 38
0x417b, // 39
0x425d, // 40
0x433d, // 41
0x441b, // 42
0x44f6, // 43
0x45d0, // 44
0x46a7, // 45
0x477d, // 46
0x4851, // 47
0x4923, // 48
0x49f3, // 49
0x4ac2, // 50
0x4b90, // 51
0x4c5b, // 52
0x4d26, // 53
0x4def, // 54
0x4eb6, // 55
0x4f7c, // 56
0x5041, // 57
0x5105, // 58
0x51c8, // 59
0x5289, // 60
0x5349, // 61
0x5409, // 62
0x54c7, // 63
0x5584, // 64
0x5640, // 65
0x56fb, // 66
0x57b6, // 67
0x586f, // 68
0x5928, // 69
0x59e0, // 70
0x5a96, // 71
0x5b4d, // 72
0x5c02, // 73
0x5cb7, // 74
0x5d6a, // 75
0x5e1e, // 76
0x5ed0, // 77
0x5f82, // 78
0x6033, // 79
0x60e4, // 80
0x6194, // 81
0x6243, // 82
0x62f2, // 83
0x63a0, // 84
0x644e, // 85
0x64fc, // 86
0x65a8, // 87
0x6655, // 88
0x6700, // 89
0x67ac, // 90
0x6857, // 91
0x6901, // 92
0x69ac, // 93
0x6a55, // 94
0x6aff, // 95
0x6ba8, // 96
0x6c50, // 97
0x6cf9, // 98
0x6da1, // 99
0x6e48, // 100
0x6ef0, // 101
0x6f97, // 102
0x703e, // 103
0x70e4, // 104
0x718b, // 105
0x7231, // 106
0x72d7, // 107
0x737c, // 108
0x7422, // 109
0x74c7, // 110
0x756c, // 111
0x7611, // 112
0x76b6, // 113
0x775b, // 114
0x77ff, // 115
0x78a3, // 116
0x7948, // 117
0x79ec, // 118
0x7a90, // 119
0x7b34, // 120
0x7bd8, // 121
0x7c7b, // 122
0x7d1f, // 123
0x7dc3, // 124
0x7e66, // 125
0x7f0a, // 126
0x7fae, // 127
0x8051, // 128
0x80f5, // 129
0x8199, // 130
0x823c, // 131
0x82e0, // 132
0x8384, // 133
0x8427, // 134
0x84cb, // 135
0x856f, // 136
0x8613, // 137
0x86b7, // 138
0x875c, // 139
0x8800, // 140
0x88a4, // 141
0x8949, // 142
0x89ee, // 143
0x8a93, // 144
0x8b38, // 145
0x8bdd, // 146
0x8c83, // 147
0x8d28, // 148
0x8dce, // 149
0x8e74, // 150
0x8f1b, // 151
0x8fc1, // 152
0x9068, // 153
0x910f, // 154
0x91b7, // 155
0x925e, // 156
0x9306, // 157
0x93af, // 158
0x9457, // 159
0x9500, // 160
0x95aa, // 161
0x9653, // 162
0x96fe, // 163
0x97a8, // 164
0x9853, // 165
0x98ff, // 166
0x99aa, // 167
0x9a57, // 168
0x9b03, // 169
0x9bb1, // 170
0x9c5f, // 171
0x9d0d, // 172
0x9dbc, // 173
0x9e6b, // 174
0x9f1b, // 175
0x9fcc, // 176
0xa07d, // 177
0xa12f, // 178
0xa1e1, // 179
0xa295, // 180
0xa348, // 181
0xa3fd, // 182
0xa4b2, // 183
0xa569, // 184
0xa61f, // 185
0xa6d7, // 186
0xa790, // 187
0xa849, // 188
0xa904, // 189
0xa9bf, // 190
0xaa7b, // 191
0xab38, // 192
0xabf6, // 193
0xacb6, // 194
0xad76, // 195
0xae37, // 196
0xaefa, // 197
0xafbe, // 198
0xb083, // 199
0xb149, // 200
0xb210, // 201
0xb2d9, // 202
0xb3a4, // 203
0xb46f, // 204
0xb53d, // 205
0xb60c, // 206
0xb6dc, // 207
0xb7ae, // 208
0xb882, // 209
0xb958, // 210
0xba2f, // 211
0xbb09, // 212
0xbbe4, // 213
0xbcc2, // 214
0xbda2, // 215
0xbe84, // 216
0xbf68, // 217
0xc04f, // 218
0xc139, // 219
0xc225, // 220
0xc314, // 221
0xc407, // 222
0xc4fc, // 223
0xc5f4, // 224
0xc6f1, // 225
0xc7f0, // 226
0xc8f4, // 227
0xc9fc, // 228
0xcb08, // 229
0xcc19, // 230
0xcd2e, // 231
0xce49, // 232
0xcf69, // 233
0xd090, // 234
0xd1bd, // 235
0xd2f1, // 236
0xd42c, // 237
0xd570, // 238
0xd6bc, // 239
0xd813, // 240
0xd974, // 241
0xdae1, // 242
0xdc5c, // 243
0xdde7, // 244
0xdf82, // 245
0xe133, // 246
0xe2fa, // 247
0xe4df, // 248
0xe6e6, // 249
0xe91a, // 250
0xeb88, // 251
0xee49, // 252
0xf18c, // 253
0xf5c9, // 254
//0x10000, // 255
};
#else
static const uint8_t integral_lut [] PROGMEM = {
0x00, // 0
0x14, // 1
0x1c, // 2
0x23, // 3
0x29, // 4
0x2e, // 5
0x32, // 6
0x36, // 7
0x3a, // 8
0x3e, // 9
0x42, // 10
0x45, // 11
0x48, // 12
0x4c, // 13
0x4f, // 14
0x52, // 15
0x55, // 16
0x58, // 17
0x5b, // 18
0x5d, // 19
0x60, // 20
0x63, // 21
0x66, // 22
0x68, // 23
0x6b, // 24
0x6e, // 25
0x70, // 26
0x73, // 27
0x75, // 28
0x78, // 29
0x7a, // 30
0x7d, // 31
0x80, // 32
0x82, // 33
0x85, // 34
0x87, // 35
0x8a, // 36
0x8c, // 37
0x8f, // 38
0x91, // 39
0x94, // 40
0x97, // 41
0x99, // 42
0x9c, // 43
0x9f, // 44
0xa2, // 45
0xa4, // 46
0xa7, // 47
0xaa, // 48
0xad, // 49
0xb0, // 50
0xb3, // 51
0xb7, // 52
0xba, // 53
0xbd, // 54
0xc1, // 55
0xc5, // 56
0xc9, // 57
0xcd, // 58
0xd1, // 59
0xd6, // 60
0xdc, // 61
0xe3, // 62
0xeb, // 63
//0x100, // 64
};
#endif

const uint8_t ac_level_max = sizeof(integral_lut) / sizeof(integral_lut[0]);

static volatile uint8_t ac_level = 0x0;
static uint16_t ne2cross = 0x180;
static uint16_t period; //     = TIMER_MS(10);
static uint16_t cross_next;// = TIMER_MS(10);

void ac_init()
{
  port_set_0(0, 0x02);
  TCCR1A = (0x2 << COM1A0) | (0x0 << COM1B0) | (0x0 << WGM10);
  //TIMSK1 |= /*(1 << ICIE1) |*/ (1 << OCIE1A);
  TIMSK1 |= (1 << ICIE1) | (1 << OCIE1A);
}

bool in_range16(uint16_t s, uint16_t val, uint16_t e)
{
  bool if0 = val >= s;
  bool if1 = val <  e;
  return s < e ? if0 && if1 :
                 if0 || if1;
}

DBG_ISR(TIMER1_CAPT_vect,)
{
  uint16_t curr = ICR1;
  if (TCCR1B & (1 << ICES1)) { // posedge
    TCCR1B &= ~(1 << ICES1);
    //timer_cancel()...
  } else {
    TCCR1B |=  (1 << ICES1);
    static uint16_t prev;
    period = (curr - prev) / 2;
    prev   = curr;
    uint16_t cross_new = curr + ne2cross;
    if (OCR1A == cross_next) {
      const uint16_t margin = 100 / PRESCALER + 1;
      if (in_range16(curr, TCNT1 - margin, cross_new)) {
        OCR1A = cross_new;
        assert(in_range16(curr, TCNT1, cross_new));
      }
    }
    cross_next = cross_new;
  }
}

DBG_ISR(TIMER1_COMPA_vect,)
{
  //period = (period & 0xfff) + 0x13;
  while (1) {
    static uint16_t cross_last;
    if (!in_range16(cross_last - 1, TCNT1, cross_next)) {
      cross_last  = cross_next;
      cross_next += period;
    }

    uint16_t new = cross_next;
    if (TCCR1A & (1 << COM1A0)) {
      if (ac_level < ac_level_max) TCCR1A &= ~(1 << COM1A0);
    } else {
      if (ac_level) {
        TCCR1A |=  (1 << COM1A0);
        uint16_t level;
        if (ac_level <= ac_level_max / 2) level = PGM_GET(integral_lut[ac_level]);
        else                              level = (1ul << (sizeof(integral_lut[0]) * 8)) - 1 - PGM_GET(integral_lut[ac_level_max - ac_level]);
        DBG_VAR(level_optimized, level);
        DBG_VAR(level_normal,    (uint16_t)PGM_GET(integral_lut[ac_level]));
        new -= (uint16_t)((uint32_t)PGM_GET(integral_lut[ac_level]) * period / (1ul << (sizeof(integral_lut[0]) * 8))); // remove cast?
      }
    }
    
    OCR1A = new;
    TIFR1 = (1 << OCIE1A);
    if (!in_range16(cross_last, TCNT1, new)) {
      TCCR1C = (1 << FOC1A);
      DBG_MAX(max_OCR1A_late, TCNT1 - new);
    } else {
      break;
    }
  }
}
