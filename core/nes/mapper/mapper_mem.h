// 各マッパーの必要なメモリを設定
union {

// NES_UNIFmapper2
struct {
	u8 wram[0x1000];
} NES_UNIFmapper2;

// NES_UNIFmapper3
struct {
	u8 regs[1];
} NES_UNIFmapper3;

// NES_UNIFmapper4
struct {
	u8 regs[5], dregs[8];
	int irq_counter, irq_latch;
	u8 irq_enabled;
} NES_UNIFmapper4;

// Mapper NSF
struct {
	u8 wram1[0x2000];
	u8 wram2[0x8000];
	u8 chip_type;
} MapperNSF;

// Mapper 255
struct {
	u8 regs[4];
} Mapper255;

// Mapper 254
struct {
	u8 regs[1];
	u32 irq_counter, irq_latch;
	u8 irq_enabled;
} Mapper254;

// Mapper 252
struct {
	u8 regs[1];
} Mapper252;

// Mapper 251
struct {
	u8 regs[11];
	u8 bregs[4];
} Mapper251;

// Mapper 248
struct {
	u8  regs[8];

	u32 prg0,prg1;
	u32 chr01,chr23,chr4,chr5,chr6,chr7;

	u8 irq_enabled; // IRQs enabled
	u8 irq_counter; // IRQ scanline counter, decreasing
	u8 irq_latch;   // IRQ scanline counter latch
} Mapper248;

// Mapper 245
struct {
	u8 regs[1];

	u32 irq_counter, irq_latch;
	u8 irq_enabled;
} Mapper245;

// Mapper 243
struct {
	u8 regs[4];
} Mapper243;

// Mapper 241
struct {
	u8 regs[1];
} Mapper241;

// Mapper 249
struct {
	u8 regs[1];
} Mapper249;

// Mapper 236
struct {
	u8 bank, mode;
} Mapper236;

// Mapper 237
struct {
	u8 *wram;
} Mapper237;

// Mapper 235
struct {
	u8 dummy[0x2000];
} Mapper235;

// Mapper 234
struct {
	u8 regs[3];
} Mapper234;

// Mapper 232
struct {
	u8 regs[2];
} Mapper232;

// Mapper 230
struct {
	u8 rom_switch;
} Mapper230;

// Mapper 226
struct {
	u8 regs[2];
} Mapper226;

// Mapper 189
struct {
	u8 regs[1];
	u8 irq_counter;
	u8 irq_latch;
	u8 irq_enabled;
} Mapper189;

// Mapper 188
struct {
	u8 dummy[0x2000];
} Mapper188;

// Mapper 187
struct {
	u8  regs[8];
	u8  bregs[8];
	u32 ex_bank_enabled,ex_bank_mode;
	u32 prg0,prg1,ex_prg0,ex_prg1;
	u32 chr01,chr23,chr4,chr5,chr6,chr7;
	u8 irq_enabled; // IRQs enabled
	u8 irq_counter; // IRQ scanline counter, decreasing
	u8 irq_latch;   // IRQ scanline counter latch
} Mapper187;

// Mapper 185
struct {
	u8 patch;
	u8 dummy_chr_rom[0x400];
} Mapper185;

// Mapper 183
struct {
	u8 regs[8];
	u8 irq_enabled;
	u32 irq_counter;
} Mapper183;

// Mapper 182
struct {
	u8 regs[1];
	u8 irq_enabled;
	u8 irq_counter;
} Mapper182;

// Mapper 160
struct {
	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;
	u8 refresh_type;
} Mapper160;

// Mapper 122
struct {
	int patch;
} Mapper122;

// Mapper 119
struct {
	u8 regs[8];
	u32 prg0,prg1;
	u32 chr01,chr23,chr4,chr5,chr6,chr7;
	u8 irq_enabled; // IRQs enabled
	u8 irq_counter; // IRQ scanline counter, decreasing
	u8 irq_latch;   // IRQ scanline counter latch
} Mapper119;

// Mapper 118
struct {
	u8  regs[8];
	u32 prg0,prg1;
	u32 chr0,chr1,chr2,chr3,chr4,chr5,chr6,chr7;
	u8 irq_enabled; // IRQs enabled
	u8 irq_counter; // IRQ scanline counter, decreasing
	u8 irq_latch;   // IRQ scanline counter latch
} Mapper118;

// Mapper 117
struct {
	u8 irq_line;
	u8 irq_enabled1;
	u8 irq_enabled2;
} Mapper117;

// Mapper 115
struct {
	u8 regs[1];

	u32 irq_counter, irq_latch;
	u8 irq_enabled;
} Mapper115;

// Mapper 114
struct {
	u8 regs[1];
} Mapper114;

// Mapper 112
struct {
	u8  regs[8];
	u32 prg0,prg1;
	u32 chr01,chr23,chr4,chr5,chr6,chr7;
	u8 irq_enabled; // IRQs enabled
	u8 irq_counter; // IRQ scanline counter, decreasing
	u8 irq_latch;   // IRQ scanline counter latch
} Mapper112;

// Mapper 105
struct {
	u8  write_count;
	u8  bits;
	u8  regs[4];

	u8  irq_enabled;
	u32 irq_counter;
	u8  init_state;
} Mapper105;

// Mapper 100
struct {
	u8  regs[8];

	u32 prg0,prg1,prg2,prg3;
	u32 chr0,chr1,chr2,chr3,chr4,chr5,chr6,chr7;

	u8 irq_enabled; // IRQs enabled
	u8 irq_counter; // IRQ scanline counter, decreasing
	u8 irq_latch;   // IRQ scanline counter latch
} Mapper100;

// Mapper 96
struct {
	u8 vbank0,vbank1;
} Mapper96;

// Mapper 95
struct {
	u8  regs[1];
	u32 prg0,prg1;
	u32 chr01,chr23,chr4,chr5,chr6,chr7;
} Mapper95;

// Mapper 91
struct {
	u8 irq_counter;
	u8 irq_enabled;
} Mapper91;

// Mapper 90
struct {
	u8 prg_reg[4];
	u8 chr_low_reg[8];
	u8 chr_high_reg[8];
	u8 nam_low_reg[4];
	u8 nam_high_reg[4];

	u8 prg_bank_size;
	u8 prg_bank_6000;
	u8 prg_bank_e000;
	u8 chr_bank_size;
	u8 mirror_mode;
	u8 mirror_type;

	u32 value1;
	u32 value2;

	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;

	u8 mode;
} Mapper90;

// Mapper 88
struct {
	u8  regs[2];
} Mapper88;

// Mapper 85
struct {
	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;
	int patch;
} Mapper85;

// Mapper 83
struct {
	u8 regs[3];
	u32 irq_counter;
	u8 irq_enabled;
} Mapper83;

// Mapper 82
struct {
	u8 regs[1];
} Mapper82;

// Mapper 80
struct {
	u8 patch;
} Mapper80;

// Mapper 76
struct {
	u8 regs[1];
} Mapper76;

// Mapper 75
struct {
	u8 regs[2];
} Mapper75;

// Mapper 74
struct {
	u8 regs[1];
} Mapper74;

// Mapper 73
struct {
	u8 irq_enabled;
	u32 irq_counter;
} Mapper73;

// Mapper 70
struct {
	u8 patch;
} Mapper70;

// Mapper 69
struct {
	u8 patch;
	u8 regs[1];
	u8 irq_enabled;
	u32 irq_counter;
} Mapper69;

// Mapper 68
struct {
	u8 regs[4];
} Mapper68;

// Mapper 67
struct {
	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;
} Mapper67;

// Mapper 65
struct {
	u8 patch, patch2;
	u8 irq_enabled;
	u32 irq_counter;
	u32 irq_latch;
} Mapper65;

// Mapper 64
struct {
	u8 regs[3];
	u8 irq_latch;
	u8 irq_counter;
	u8 irq_enabled;
} Mapper64;

// Mapper 57
struct {
	u8 regs[1];
} Mapper57;

// Mapper 52
struct {
	u8  regs[8];
	u32 prg0,prg1;
	u32 chr01,chr23,chr4,chr5,chr6,chr7;
	u8 irq_enabled; // IRQs enabled
	u8 irq_counter; // IRQ scanline counter, decreasing
	u8 irq_latch;   // IRQ scanline counter latch
} Mapper52;

// Mapper 51
struct {
	u8 bank, mode;
} Mapper51;

// Mapper 50
struct {
	u8 irq_enabled;
} Mapper50;

// Mapper 49
struct {
	u8  regs[3];
	u32 prg0,prg1;
	u32 chr01,chr23,chr4,chr5,chr6,chr7;

	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;
} Mapper49;

// Mapper 48
struct {
	u8 regs[1];
	u8 irq_enabled;
	u8 irq_counter;
} Mapper48;

// Mapper 47
struct {
	u8  regs[8];
	u8  patch;
	u32 rom_bank;
	u32 prg0,prg1;
	u32 chr01,chr23,chr4,chr5,chr6,chr7;
	u8  irq_enabled; // IRQs enabled
	u8  irq_counter; // IRQ scanline counter, decreasing
	u8  irq_latch;   // IRQ scanline counter latch
} Mapper47;

// Mapper 46
struct {
	u8 regs[4];
} Mapper46;

// Mapper 45
struct {
	u8 patch;

	u8  regs[7];
	u32 p[4],prg0,prg1,prg2,prg3;
	u32 c[8],chr0,chr1,chr2,chr3,chr4,chr5,chr6,chr7;

	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;
} Mapper45;

// Mapper 44
struct {
	u8  regs[8];

	u32 rom_bank;
	u32 prg0,prg1;
	u32 chr01,chr23,chr4,chr5,chr6,chr7;
	u8 irq_enabled; // IRQs enabled
	u8 irq_counter; // IRQ scanline counter, decreasing
	u8 irq_latch;   // IRQ scanline counter latch
} Mapper44;

// Mapper 43
struct {
	u8 irq_enabled;
	u32 irq_counter;
} Mapper43;

// Mapper 42
struct {
	u8 irq_counter;
	u8 irq_enabled;
} Mapper42;

// Mapper 41
struct {
	u8 regs[1];
} Mapper41;

// Mapper 40 (smb2j)
struct {
	u8 irq_enabled;
	u32 lines_to_irq;
} Mapper40;

// Mapper 33
struct {
	u8 patch;
	u8 patch2;
	u8 irq_enabled;
	u8 irq_counter;
} Mapper33;

// Mapper 32
struct {
	u8 patch;
	u8 regs[1];
} Mapper32;

// Mapper 26
struct {
	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;
} Mapper26;

// Mapper 25
struct {
	u8 patch;
	u8 regs[11];
	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;
} Mapper25;

// Mapper 24
struct {
	u8 patch;
	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;
} Mapper24;

// Mapper 23
struct {
	u8 regs[9];

	u32 patch;
	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;
} Mapper23;

// Mapper 21
struct {
	u8 regs[9];
	u8 irq_enabled;
	u8 irq_counter;
	u8 irq_latch;
} Mapper21;

// Mapper 20
struct {
	u8 bios[0x2000];
	u8 *wram;
	u8 disk[0x40000];

	u8 irq_enabled;
	u32 irq_counter;
	u32 irq_latch;
	u8 irq_wait;

	u8 disk_enabled;
	u32 head_position;
	u8 write_skip;
	u8 disk_status;
	u8 write_reg;
	u8 current_side;

	u8 access_flag;
	u8 last_side;
	u8 insert_wait;

	u8 patch;
} Mapper20;

// Mapper 19
struct {
	u8 patch;

	u8 regs[3];
	u8 irq_enabled;
	u32 irq_counter;
	u32 irq_sn;
} Mapper19;

// Mapper 18
struct {
	u8 patch;
	u8 regs[11];
	u8 irq_enabled;
	u32 irq_latch;
	u32 irq_counter;
} Mapper18;

// Mapper 17
struct {
	u8 irq_enabled;
	u32 irq_counter;
	u32 irq_latch;
} Mapper17;

// Mapper 16
struct {
	u8 patch, patch2;
	u8 regs[3];

	u8 serial_out[0x2000];

	u8 eeprom_cmd[4];
	u8 eeprom_status;
	u8 eeprom_mode;
	u8 eeprom_pset;
	u8 eeprom_addr, eeprom_data;
	u16 eeprom_wbit, eeprom_rbit;

	u8 barcode[256];
	u8 barcode_status;
	u8 barcode_pt;
	u8 barcode_pt_max;
	u8 barcode_phase;
	u32 barcode_wait;
	u8 barcode_enabled;

	u8 irq_enabled;
	u32 irq_counter;
	u32 irq_latch;
} Mapper16;

// Mapper 13
struct {
	u8 prg_bank;
	u8 chr_bank;
} Mapper13;

// Mapper 10
struct {
	u8 regs[6];
	u8 latch_0000;
	u8 latch_1000;
} Mapper10;

// Mapper 9
struct {
	u8 regs[6];
	u8 latch_0000;
	u8 latch_1000;
} Mapper9;

// Mapper 6
struct {
	u8 irq_enabled;
	u32 irq_counter;
	u8 chr_ram[4*0x2000];
} Mapper6;

// Mapper 5
struct {
	u32 wb[8];
	u8 *wram;
	u8 wram_size;

	u8 chr_reg[8][2];

	u8 irq_enabled;
	u8 irq_status;
	u32 irq_line;

	u32 value0;
	u32 value1;

	u8 wram_protect0;
	u8 wram_protect1;
	u8 prg_size;
	u8 chr_size;
	u8 gfx_mode;
	u8 split_control;
	u8 split_bank;
} Mapper5;

// Mapper 4
struct {
	u8  patch;
	u8  regs[8];

	u32 prg0,prg1;
	u32 chr01,chr23,chr4,chr5,chr6,chr7;
	u8 irq_enabled; // IRQs enabled
	u8 irq_counter; // IRQ scanline counter, decreasing
	u8 irq_latch;   // IRQ scanline counter latch

	u8 vs_index; // VS Atari RBI Baseball and VS TKO Boxing
} Mapper4;

// Mapper 1
struct {
	u32 write_count;
	u8  bits;
	u8  regs[4];
	u32 last_write_addr;

	// Best Play - Pro Yakyuu Special
	u8 patch;
	u8 wram_bank, wram_flag, wram_count;
//	u8 wram[0x4000];
	u8 *wram;

	MMC1_Size_t MMC1_Size;
	u32 MMC1_256K_base;
	u32 MMC1_swap;

	// these are the 4 ROM banks currently selected
	u32 MMC1_bank1;
	u32 MMC1_bank2;
	u32 MMC1_bank3;
	u32 MMC1_bank4;

	u32 MMC1_HI1;
	u32 MMC1_HI2;
} Mapper1;

}; // union
