/***********************************************************************/
/*  FILE        :memes_final.c                                         */
/***********************************************************************/
#include <stdlib.h>
#include "iodefine.h"
#include "typedefine.h"
#include "machine.h"
#include "ctype.h"
 
#define printf  ((int (*)(const char *,...))0x00007c7c)
#define	scanf	((int (*)(const char *,...))0x00007cb8)
 
#define SW6     (PD.DR.BIT.B18)
#define SW5     (PD.DR.BIT.B17)
#define SW4     (PD.DR.BIT.B16)
 
#define LED6    (PE.DR.BIT.B11)
#define LED_ON  (0)
#define LED_OFF (1)
 
#define DIG1    (PE.DR.BIT.B3)
#define DIG2    (PE.DR.BIT.B2)
#define DIG3    (PE.DR.BIT.B1)
 
#define SPK (PE.DR.BIT.B0)
 
#define LCD_RS      (PA.DR.BIT.B22)
#define LCD_E       (PA.DR.BIT.B23)
#define LCD_RW      (PD.DR.BIT.B23)
#define LCD_DATA    (PD.DR.BYTE.HH)
 
#define NMROF_BLOCKS 110

#define	SD_CD		(PE.DR.BIT.B11)
#define	SD_CS		(PE.DR.BIT.B9)
#define TFTDATA  (*(volatile unsigned short*)0x08000000)
#define TFTCTRL  (*(volatile unsigned short*)0x08000002)
#define _COL_WHITE    (0xFFFF)
#define _COL_RED      (0xF800)
#define _COL_GREEN    (0x07E0)
#define _COL_BLUE     (0x001F)
#define _COL_BLACK    (0x0000)

volatile	_UWORD FrameBuf[320*240];
 
enum STS{TITLE, STOP, RUN, PAUSE1, PAUSE2, PAUSE3, CLEAR, FAIL};

int score = 0;	//	�X�R�A���i�[����ϐ�
int status;		// �Q�[���̏�ʂ��i�[����ϐ�

int speed = 5;	// �v���C���[�̃X�s�[�h	
int life = 3;	// �v���C���[�̎c�@

int blocks = 0;	// ��ʏ�̃u���b�N��

/* �摜���i�[����z�� */
_UWORD img_A[64];
_UWORD img_B[192];	
_UWORD img_C[192];
_UWORD img_D[192];

/* �u���b�N�ƃv���C���[�̏����i�[����\���� */
struct position {
  int x;	//
  int y;	// LCD��̍��W
  int active;	// �u���b�N��\��...1 / ��\��...0 / �A�C�e��...2
};

/* �{�[���̏����i�[����\���� */
struct ball {
  int x;	//
  int y;	// LCD��̍��W
  int dx;   //
  int dy;	// �x�N�g������
};

void wait_us(_UINT);
void init_CMT0();
void init_SCI2();
unsigned char SPI_tx_rx(unsigned char ch);
unsigned int card_exist();
unsigned char calc_CRC7(unsigned char *data, int len);
unsigned char SD_send_cmd(unsigned char cmd, int arg);
unsigned int Enter_SPI_mode();
void read_sector(unsigned int sect, unsigned char *dt);
void print_sector(unsigned int sect, unsigned char *dt);
void TFT_draw_screen(void);
void TFT_clear(void);
void TFT_On(void);
void init_CS2(void);
void TFT_draw_point(_UWORD x_pix, _UWORD y_pix, _UWORD p_color);
void TFT_draw_rect(_UWORD x, _UWORD y, _UWORD w, _UWORD h, _UWORD p_color);
void TFT_put_img(_UWORD x, _UWORD y, _UWORD w, _UWORD h, _UWORD *img);
void LCD_inst(_SBYTE);
void LCD_data(_SBYTE);
void LCD_cursor(_UINT, _UINT);
void LCD_putch(_SBYTE);
void LCD_putstr(_SBYTE *);
void LCD_cls(void);
void LCD_init(void);
 
// --------------------
// -- �g�p����֐��Q --
// --------------------
void wait_us(_UINT us) {
    _UINT val;
 
    val = us * 10 / 16;
    if (val >= 0xffff)
        val = 0xffff;
 
    CMT0.CMCOR = val;
    CMT0.CMCSR.BIT.CMF &= 0;
    CMT.CMSTR.BIT.STR0 = 1;
	
    while (!CMT0.CMCSR.BIT.CMF);
	
    CMT0.CMCSR.BIT.CMF = 0;
    CMT.CMSTR.BIT.STR0 = 0;
}

void init_CMT0()
{
	STB.CR4.BIT._CMT = 0;

	CMT0.CMCNT = 0;
	CMT0.CMCSR.BIT.CKS = 1;		// CKS�ݒ�(0:1/8, 1:1/32, 2:1/128, 3:1/512)
}

void init_SCI2()
{
	STB.CR3.BIT._SCI2 = 0;
	SCI2.SCSCR.BYTE= 0x00;
	SCI2.SCSMR.BIT.CA = 1;		// �N���b�N������
	SCI2.SCBRR = 12;			// 384kbps
	SCI2.SCSDCR.BIT.DIR = 1;	// MSB first
	wait_us(1);
	PFC.PECRL3.BIT.PE10MD = 2;	// PE10 .. TxD
	PFC.PECRL3.BIT.PE8MD = 2;	// PE8 .. SCK
	PFC.PECRL2.BIT.PE7MD = 2;	// PE7 .. RxD
	PFC.PEIORL.BIT.B12 = 0;		// PE12 .. WP����
	PFC.PEIORL.BIT.B11 = 0;		// PE11 .. CD����
	PFC.PEIORL.BIT.B9 = 1;		// PE9 .. CS�o��
	PE.DR.BIT.B9 = 1;			// CS�����l
	SCI2.SCSCR.BYTE |= 0x30;	// TE, RE = 1
}

unsigned char SPI_tx_rx(unsigned char ch)
{
	while (!SCI2.SCSSR.BIT.TDRE)
		;
	SCI2.SCTDR = ch;
	SCI2.SCSSR.BIT.TDRE = 0;

	while (!SCI2.SCSSR.BIT.RDRF)
		;
	ch = SCI2.SCRDR;
	SCI2.SCSSR.BIT.RDRF = 0;
	return (ch);
}

unsigned int card_exist()
{
	return (PE.DR.BIT.B11 ? 0 : 1);
}

unsigned char calc_CRC7(unsigned char *data, int len)
{
	int i, j;
	char crc, dt;

	crc = 0;
	for (i = 0; i < len; i++) {
		dt = *data++;
		for (j = 0; j < 8; j++) {
			crc <<=1;
			if ((crc & 0x80) ^ (dt & 0x80))
				crc ^= 0x09;
			dt <<= 1;
		}
	}
	return (crc & 0x7f);
}

unsigned char SD_send_cmd(unsigned char cmd, int arg)
{
	int i;
	unsigned char cmd_token[6], ret;

	while (1) {
		if (SPI_tx_rx(0xff) == 0xff)
			break;
	}

	cmd_token[0] = cmd;
	cmd_token[1] = (arg >> 24) & 0xff;
	cmd_token[2] = (arg >> 16) & 0xff;
	cmd_token[3] = (arg >> 8) & 0xff;
	cmd_token[4] = arg & 0xff;
	cmd_token[5] = (calc_CRC7(cmd_token, 5) << 1) | 0x01;

	for (i = 0; i < 6; i++)
		SPI_tx_rx(cmd_token[i]);

	do {
		ret = SPI_tx_rx(0xff);
	} while (ret & 0x80);

	return (ret);
}

unsigned int Enter_SPI_mode()
{
	int i;
	unsigned char ret;

	SD_CS = 1;				// CS negate

	for (i = 0; i < 10; i++)
		SPI_tx_rx(0xff);

	SD_CS = 0;				// CS assert

	ret = SD_send_cmd(0x40, 0);
	if (ret != 0x01)
		return (-1);

	do {
		ret = SD_send_cmd(0x41, 0);
	} while (ret != 0x00);

	return (1);
}

void read_sector(unsigned int sect, unsigned char *dt)
{
	unsigned char ch;
	int i;

	ch = SD_send_cmd(0x51, sect * 512);
	while (ch != 0xfe)
		ch = SPI_tx_rx(0xff);

	for (i = 0; i < 512; i++)
		*dt++ = SPI_tx_rx(0xff);
	SPI_tx_rx(0xff);				// CRC��
	SPI_tx_rx(0xff);				// CRC��
}

void print_sector(unsigned int sect, unsigned char *dt)
{
	int i, j;
	unsigned int adr;

	adr = sect * 512;

	printf("           ");
	for (i = 0; i < 16; i++) {
		printf("+%x ", i);
		if (i == 7)
			printf(" ");
	}
	printf("\n");

	for (i = 0; i < 32; i++) {
		printf("%08x : ", adr);
		for (j = 0; j < 16; j++) {
			printf("%02x ", dt[i * 16 + j]);
			if (j == 7)
				printf(" ");
		}
		printf(" ");
		for (j = 0; j < 16; j++) {
			if (isprint(dt[i * 16 + j]))
				printf("%c", dt[i * 16 + j]);
			else
				printf(".");
			if (j == 7)
				printf(" ");
		}
		printf("\n");
		adr += 16;
	}

}

// FrameBuf[]�̃f�[�^��TFT�֓]������
void TFT_draw_screen(void) {
  _SINT i;

  TFTCTRL = 0x4001;
  for (i = 0; i < (320*240); i++ ) {
    TFTDATA = FrameBuf[i];
  }
}

void TFT_clear(void) {
  _SINT i;

  TFTCTRL = 0x4001;
  for (i = 0; i < (320*240); i++ ) {
    TFTDATA = _COL_WHITE;
    FrameBuf[i] = _COL_WHITE;
  }
}

void TFT_On(void) {
  TFTCTRL = 0x4000;
}

void init_CS2(void) {
  BSC.CS2BCR.LONG = 0x12490400;
  BSC.CS2WCR      = 0x000302C0;
  PFC.PACRL4.BIT.PA15MD = 1;
  PFC.PACRL2.BIT.PA6MD  = 2;
}

void TFT_draw_point(_UWORD x_pix, _UWORD y_pix, _UWORD p_color) {
  FrameBuf[y_pix*320+x_pix] = p_color;
}

void TFT_draw_rect(_UWORD x, _UWORD y, _UWORD w, _UWORD h, _UWORD p_color)
{
	int i, j;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			TFT_draw_point(x+i, y+j, p_color);
		}
	}
}

void TFT_put_img(_UWORD x, _UWORD y, _UWORD w, _UWORD h, _UWORD *img) {
	int i, j;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			TFT_draw_point(x+i, y+j, *(img+(j*w+i)));
		}
	}
}
 
void LCD_inst(_SBYTE inst) {
    LCD_E = 0;
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_E = 1;
    LCD_DATA = inst;
    wait_us(1);
    LCD_E = 0;
    wait_us(40);
}
 
void LCD_data(_SBYTE data) {
    LCD_E = 0;
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_E = 1;
    LCD_DATA = data;
    wait_us(1);
    LCD_E = 0;
    wait_us(40);
}
 
void LCD_cursor(_UINT x, _UINT y) {
    if (x > 15)
        x = 15;
    if (y > 1)
        y = 1;
    LCD_inst(0x80 | x | y << 6);
}
 
void LCD_putch(_SBYTE ch) {
    LCD_data(ch);
}
 
void LCD_putstr(_SBYTE *str) {
    _SBYTE ch;
 
    while (ch = *str++)
        LCD_putch(ch);
}
 
void LCD_cls(void) {
    LCD_inst(0x01);
    wait_us(1640);
}
 
void LCD_init(void) {
    wait_us(45000);
    LCD_inst(0x30);
    wait_us(4100);
    LCD_inst(0x30);
    wait_us(100);
    LCD_inst(0x30);
     
    LCD_inst(0x38);
    LCD_inst(0x08);
    LCD_inst(0x01);
    wait_us(1640);
    LCD_inst(0x06);
    LCD_inst(0x0c);
}
 
// --------------------------------------------
// -- �Q�[���p�̊֐��Q --
 
/* �v���C���[���ړ� */
void move_me(struct position *me)
{
    struct position old_position;	// �ړ��O�̍��W
 
    old_position.x = me->x;
    old_position.y = me->y;
	
	if (AD0.ADDR1 < 0x4000) {
        // -- �W���C�X�e�B�b�N�E --
        if (me->x <= 288 - speed) {
			me->x += speed;
		}
		else {
			me->x = 288;
		}
    } else if (AD0.ADDR1 > 0xc000) {
        // -- �W���C�X�e�B�b�N�� --
        if (me->x >= 0 + speed) {
			me->x -= speed;
		}
		else {
			me->x = 0;
		}
    }
	TFT_draw_rect(old_position.x, old_position.y, 32, 6, _COL_WHITE);
	TFT_put_img(me->x, me->y, 32, 6, img_B);
}

/* int�^������char�^������z��֕ϊ� */
char *itoa(int val, char *a, int radix)
{
	char *p = a;
	unsigned int v = val;
	int n = 1;
	
	// �����̎Z�o
	while (v >= radix) {
		v /= radix;
		n++;
	}
	
	p = a + n;
	v = val;
	*p = '\0';	// EOF
	
	do {
		--p;
		*p = v % radix + '0';	// �ꌅ���ϊ�
		// �\�i���ŕ\���ł��Ȃ��ꍇ
		if (*p > '9') {
			*p = v % radix - 10 + 'A';	// �A���t�@�x�b�g�ɕϊ�
		}
		v /= radix;
	} while (p != a);
	
	return a;
}

/* �X�R�A��\�� */
void disp_score(int score)
{
    int i;
	char num[8];
	
	itoa(score,num,10);		// �X�R�A�𕶎���ɕϊ�
	
	LCD_cursor(0, 0);
    LCD_putstr("SCORE:");
	LCD_cursor(8, 0);
	LCD_putstr(num);
}

/* �c�@��\�� */
void disp_life(int life)
{
    int i;
	
	LCD_cursor(0, 1);
    LCD_putstr("LIFE:");
	
	if (life > 5) {
		life = 5;
	}
	
	// ���C�t�̐���\��
	for (i = 0; i < life; i++) {
		LCD_cursor(15-(2*i), 1);
		LCD_putch('*');
	}
	for (i; i < 5; i++) {
		LCD_cursor(15-(2*i), 1);
		LCD_putch(' ');
	}
}

/* �{�[���̈ړ�, �u���b�N�̏��� */
void move_ball(struct ball *ball, struct position *me, struct position *block)
{
	struct ball old_position;	// �ړ��O�̍��W
	struct position col_block;  // ���������u���b�N�̍��W
	int i;
	int flag_x = 0, flag_y = 0;
 
    old_position.x = ball->x;
    old_position.y = ball->y;
	
	ball->x += ball->dx;
	// x�����̒��˕Ԃ�
	if ((ball->x >= 320-8) || (ball->x <= 0) || (ball->x >= me->x-8 && ball->x <= me->x+32 && ball->y >= me->y-6 && ball->y <= me->y+6)) {
		ball->dx = -ball->dx;
	}
	if (ball->x < 0) {
		ball->x = 0;
	}
	if (ball->x > 320-8) {
		ball->x = 320-8;
	}
	
	if (ball->x > me->x-8 && ball->x < me->x+12 && ball->y > me->y-6 && ball->y < me->y+6) {
		ball->x = me->x-8;
	}
	if (ball->x > me->x+11 && ball->x < me->x+32 &&ball->y > me->y-6 && ball->y < me->y+6) {
		ball->x = me->x+32;
	}
	
	ball->y += ball->dy;
	// y�����̒��˕Ԃ�
	if (ball->y <= 0) {
		ball->dy = -ball->dy;
	}
	if (ball->y >= me->y-6 && ball->y <= me->y+6 && ball->x >= me->x-8 && ball->x <= me->x+32) {
		if (ball->dx < 0 && (ball->x >= me->x+13 && ball->x <= me->x+32)) {
			ball->dx = -2;
		}
		else if (ball->dx > 0 && (ball->x >= me->x && ball->x <= me->x+12)) {
			ball->dx = 2;
		}
		else if (ball->dx == 0 && (ball->x >= me->x+13 && ball->x <= me->x+32)) {
			ball->dx = 3;
		}
		else if (ball->dx == 0 && (ball->x >= me->x && ball->x <= me->x+12)) {
			ball->dx = -3;
		}
		ball->dy = -ball->dy;
	}
	
	if (ball->y < 0) {
		ball->y = 0;
	}
	if (ball->y > 240-8) {
		ball->y = 240-8;
	}if (ball->y > me->y-6 && ball->y < me->y+6 && ball->x > me->x-8 && ball->x < me->x+32) {
		ball->y = me->y-8;
	}
	
	// ��ʉ��Ƀ{�[�����������ꍇ
	if (ball->y >= 240-8) {
		if (life > 0) {	// �c�@�����炵�čăX�^�[�g
			life--;
			disp_life(life);
			TFT_draw_rect(me->x, me->y, 32, 6, _COL_WHITE);
			me->x = 144;
			ball->x = me->x+12;
			ball->y = me->y-8;
			status = STOP;
		}
		else {	// �Q�[���I�[�o�[
			status = FAIL;
		}
	}
	
	// �u���b�N���Ƃ̏Փ˔���
	for (i = 0; i < NMROF_BLOCKS; i++) {
		if (block->active == 1 || block->active == 2 || block->active == 3) {
			if ((ball->x >= block->x-8 && ball->x <= block->x+24) && (ball->y >= block->y-8 && ball->y <= block->y+8)) {
				block->active -= 1;
				if (block->active == 0) {	// �u���b�N�������ăX�R�A���Z
					score += 10;
					blocks -= 1;
					disp_score(score);
					TFT_draw_rect(block->x, block->y, 24, 8, _COL_WHITE);
				}
				else if (block->active == 1) {	//�u���b�N�����������ăX�R�A���Z
					score += 5;
					blocks -= 1;
					disp_score(score);
					col_block.x = block->x;
					col_block.y = block->y;
					col_block.active = block->active;
				}
				
				// �u���b�N�ƃ{�[���̒��S���W�̍���x, y�Ŕ�r��,���˕Ԃ�����𔻒f
				if (abs((ball->y+4) - (block->y+4)) >= abs((ball->x+4) - (block->x+12))-8) {
					if (flag_x != 1) {
						flag_y = 1;
					}
				}
				if (abs((ball->y+4) - (block->y+4)) < abs((ball->x+4) - (block->x+12))-8) {
					if (flag_y != 1) {
						flag_x = 1;
					}
				}
				//printf("%d, %d\n", abs((ball->x+4) - (block->x+12)), abs((ball->y+4) - (block->y+4)));
			}
		}
		block++;
	}
	
	// ���ׂẴu���b�N���������Ƃ�
	if (blocks <= 0) {
		score += 100;
		status = CLEAR;
	}
	
	// ���˕Ԃ菈��
	if (flag_x == 1) {
		ball->dx = -ball->dx;
	}
	if (flag_y == 1) {
		ball->dy = -ball->dy;
	}
	
	// �{�[���̈ʒu���X�V
	TFT_draw_rect(old_position.x, old_position.y, 8, 8, _COL_WHITE);
	TFT_put_img(ball->x, ball->y, 8, 8, img_A);
	
	// �u���b�N��������
	if (col_block.active == 1) {
		TFT_put_img(col_block.x, col_block.y, 24, 8, img_C);
	}
}

 
// --------------------------------------------
// -- ���C���֐� --
void main()
{
    struct position block[NMROF_BLOCKS];  // �u���b�N�̍��W
	struct position player;				  // �v���C���[�̍��W
	struct ball ball;					  // �{�[���̍��W
    int ad, i;
    int stop_sw, run_sw, pause_sw;
	
	struct position *block_p;
	
	int j;
	// �摜�ǂݍ��ݗp�ϐ�
	unsigned int SECT_NR;
	unsigned int First_sect_LBA, First_RDE_sect, First_FAT_sect, First_Data_sect;
	unsigned int BPB_BytesPerSec, BPB_SecPerClus, BPB_RsvdSecCnt;
	unsigned int BPB_NumFATs, BPB_RootEntCnt, BPB_FATSz16;
	unsigned int File_year, File_month, File_date, File_hour, File_min, File_sec;
	unsigned int FstClusLO, FileSize;
	unsigned int cluster;
	unsigned char dt[512], fat[512];
	unsigned int readsize = 0;
	unsigned int imgNum = 0;
	unsigned int img_firstFlag = 0;
	
	block_p = block;
	
	// TFT
	init_CMT0();
	init_SCI2();
	init_CS2();
  	TFT_On();
  	TFT_clear();
 
    STB.CR4.BIT._AD0 = 0;
    STB.CR4.BIT._MTU2 = 0;
	
	PFC.PEIORL.BIT.B0 = 1;
 
    // MTU2 ch0
    MTU20.TCR.BIT.TPSC = 3;         // 1/64�I��
    MTU20.TCR.BIT.CCLR = 1;         // TGRA�̃R���y�A�}�b�`�ŃN���A
    MTU20.TGRA = 31250 - 1;         // 100ms
    MTU20.TIER.BIT.TTGE = 1;        // A/D�ϊ��J�n�v��������
 
    // AD0
    AD0.ADCSR.BIT.ADM = 3;          // 2�`���l���X�L�������[�h
    AD0.ADCSR.BIT.CH = 1;           // AN1
    AD0.ADCSR.BIT.TRGE = 1;         // MTU2����̃g���K�L��
    AD0.ADTSR.BIT.TRG0S = 1;        // TGRA�R���y�A�}�b�`�Ńg���K
 
    // MTU2 ch1
    MTU21.TCR.BIT.TPSC = 3;         // 1/64�I��
    MTU21.TCR.BIT.CCLR = 1;         // TGRA�̃R���y�A�}�b�`�ŃN���A
    MTU21.TGRA = 25000 - 1;         // 80ms
 
    LCD_init();
 
    MTU2.TSTR.BIT.CST0 = 1;         // MTU2 CH0�X�^�[�g
    MTU2.TSTR.BIT.CST1 = 1;         // MTU2 CH1�X�^�[�g
	
	PFC.PEIORL.BIT.B11 = 1;
	
	if (!card_exist()) {
		printf("No card found\n");
	} else {
		if (Enter_SPI_mode() < 0) {
			printf("SPI mode Err\n");
		} else {
			//printf("SPI mode\n");
			SD_send_cmd(0x50, 512);		// �u���b�N�T�C�Y=512

			// MBR
			read_sector(0, dt);
			First_sect_LBA = (dt[457] << 24) | (dt[456] << 16) | (dt[455] << 8) | dt[454];
			// BPB
			read_sector(0x19, dt);	
			// BPB_BytesPerSec
			BPB_BytesPerSec	= (dt[12] << 8) | dt[11];
			// BPB_SecPerClus
			BPB_SecPerClus	= dt[13];
			// BPB_RsvdSecCnt
			BPB_RsvdSecCnt = (dt[15] << 2) | dt[14];
			// BPB_NumFATs
			BPB_NumFATs = dt[16];
			// BPB_RootEntCnt
			BPB_RootEntCnt = (dt[18] << 8) | dt[17];
			// BPB_FATSz16
			BPB_FATSz16 = (dt[23] << 8) | dt[22];
			// First_FAT_sect
			First_FAT_sect = First_sect_LBA + BPB_RsvdSecCnt;
			// First_RDE_sect
			First_RDE_sect = First_sect_LBA + BPB_RsvdSecCnt + (BPB_NumFATs * BPB_FATSz16);
			// First_Data_sect
			First_Data_sect = First_RDE_sect + (32*BPB_RootEntCnt) / BPB_BytesPerSec;
			
			/* �{�[���摜�ǂݍ��� */
			readsize = 0;
			imgNum = 0;
			img_firstFlag = 0;
			
			//RDE
			read_sector(First_RDE_sect, dt);
			
			i = 10;
			
			// FstClusLO
			FstClusLO = (dt[i * 32 + 27] << 8) | dt[i * 32 + 26];
			// FileSize
			FileSize = (dt[i * 32 + 31] << 24) | (dt[i * 32 + 30] << 16) | (dt[i * 32 + 29] << 8) | dt[i * 32 + 28];
			//FAT
			read_sector(First_FAT_sect, fat);
			cluster = FstClusLO;
			while (cluster != 0xffff) {
				for (i = 0; i < BPB_SecPerClus; i ++) {
					read_sector((cluster - 2) * BPB_SecPerClus + First_Data_sect + i, dt);
					for (j = 0; j < 512; j+=2) {
						readsize+=2;
						if (readsize > FileSize) {
							break;
						}
						if (img_firstFlag == 0) {
							if (j == 4) {
								img_firstFlag = 1;
							}
						}
						if (img_firstFlag == 1){
							img_A[imgNum] = (dt[j] << 8) | dt[j+1];
							imgNum++;
						}
					}
				}
				cluster = (fat[cluster * 2 + 1] << 8) | fat[cluster * 2];
			}
			
			/* �v���C���[�摜�ǂݍ��� */
			readsize = 0;
			imgNum = 0;
			img_firstFlag = 0;
			
			//RDE
			read_sector(First_RDE_sect, dt);
			
			i = 11;
						
			// FstClusLO
			FstClusLO = (dt[i * 32 + 27] << 8) | dt[i * 32 + 26];
			// FileSize
			FileSize = (dt[i * 32 + 31] << 24) | (dt[i * 32 + 30] << 16) | (dt[i * 32 + 29] << 8) | dt[i * 32 + 28];
			//FAT
			read_sector(First_FAT_sect, fat);
			cluster = FstClusLO;
			while (cluster != 0xffff) {
				for (i = 0; i < BPB_SecPerClus; i ++) {
					read_sector((cluster - 2) * BPB_SecPerClus + First_Data_sect + i, dt);
					for (j = 0; j < 512; j+=2) {
						readsize+=2;
						if (readsize > FileSize) {
							break;
						}
						if (img_firstFlag == 0) {
							if (j == 4) {
								img_firstFlag = 1;
							}
						}
						if (img_firstFlag == 1){
							img_B[imgNum] = (dt[j] << 8) | dt[j+1];
							imgNum++;
						}
					}
				}
				cluster = (fat[cluster * 2 + 1] << 8) | fat[cluster * 2];
			}
			
			/* �u���b�N�摜�ǂݍ��� */
			readsize = 0;
			imgNum = 0;
			img_firstFlag = 0;
			
			//RDE
			read_sector(First_RDE_sect, dt);
			
			i = 12;
			
			// FstClusLO
			FstClusLO = (dt[i * 32 + 27] << 8) | dt[i * 32 + 26];
			// FileSize
			FileSize = (dt[i * 32 + 31] << 24) | (dt[i * 32 + 30] << 16) | (dt[i * 32 + 29] << 8) | dt[i * 32 + 28];
			//FAT
			read_sector(First_FAT_sect, fat);
			cluster = FstClusLO;
			while (cluster != 0xffff) {
				for (i = 0; i < BPB_SecPerClus; i ++) {
					read_sector((cluster - 2) * BPB_SecPerClus + First_Data_sect + i, dt);
					for (j = 0; j < 512; j+=2) {
						readsize+=2;
						if (readsize > FileSize) {
							break;
						}
						if (img_firstFlag == 0) {
							if (j == 4) {
								img_firstFlag = 1;
							}
						}
						if (img_firstFlag == 1){
							img_C[imgNum] = (dt[j] << 8) | dt[j+1];
							imgNum++;
						}
					}
				}
				cluster = (fat[cluster * 2 + 1] << 8) | fat[cluster * 2];
			}
			
			/* ���F�u���b�N�摜�ǂݍ��� */
			readsize = 0;
			imgNum = 0;
			img_firstFlag = 0;
			
			//RDE
			read_sector(First_RDE_sect, dt);
			i = 13;
			
			// FstClusLO
			FstClusLO = (dt[i * 32 + 27] << 8) | dt[i * 32 + 26];
			// FileSize
			FileSize = (dt[i * 32 + 31] << 24) | (dt[i * 32 + 30] << 16) | (dt[i * 32 + 29] << 8) | dt[i * 32 + 28];
			//FAT
			read_sector(First_FAT_sect, fat);
			cluster = FstClusLO;
			while (cluster != 0xffff) {
				for (i = 0; i < BPB_SecPerClus; i ++) {
					read_sector((cluster - 2) * BPB_SecPerClus + First_Data_sect + i, dt);
					for (j = 0; j < 512; j+=2) {
						readsize+=2;
						if (readsize > FileSize) {
							break;
						}
						if (img_firstFlag == 0) {
							if (j == 4) {
								img_firstFlag = 1;
							}
						}
						if (img_firstFlag == 1){
							img_D[imgNum] = (dt[j] << 8) | dt[j+1];
							imgNum++;
						}
					}
				}
				cluster = (fat[cluster * 2 + 1] << 8) | fat[cluster * 2];
			}
			
			//printf("import done\n");
			
			// �v���C���[�̍��W��������
			player.x = 144;
			player.y = 224;
			
			// �{�[���̍��W, �x�N�g����������
			ball.x = 156;
			ball.dx = -3;
			ball.y = 216;
			ball.dy = -3;
				
			/* �摜��`�� */
			TFT_put_img(ball.x, ball.y, 8, 8, img_A);
			TFT_put_img(player.x, player.y, 32, 6, img_B);
			for (i = 0; i < 11; i++) {
				for (j = 0; j < 10; j++) {
					block_p->x = 40+(24*j);
					block_p->y = 24+(8*i);
					if (j % 2 == 0) {
						block_p->active = 1;
						blocks++;
						TFT_put_img(block_p->x, block_p->y, 24, 8, img_C);
					}
					else if (j % 2 != 0) {
						block_p->active = 2;
						blocks +=2 ;
						TFT_put_img(block_p->x, block_p->y, 24, 8, img_D);
					}
					else {
					}
					block_p++;
				}
			}
  			TFT_draw_screen();
			/* ----------------------------------------- */
			
			block_p = block;
			
			LCD_init();	//	LCD��������
			score = 0;	// �X�R�A��������
			disp_life(life);	// �c�@�\��
			disp_score(score);	// �X�R�A�\��
			
			status = STOP;	// �ҋ@��ʂɈڍs
			
			//printf("start\n");
			// ----------------
			while (1) {
				if (MTU21.TSR.BIT.TGFA) { // �Q�[���p�̃^�C�}�̃t���O
		            // MTU2 ch1 �R���y�A�}�b�`����(80ms��)
		            MTU21.TSR.BIT.TGFA = 0; // �t���O�N���A
					
		            // 80ms��1��X�C�b�`��ǂ�
		            stop_sw = SW4;
		            pause_sw = SW5;
		            run_sw = SW6;
					
					// �Q�[�����
					if (status == RUN) {
						move_ball(&ball, &player, block_p);	// �{�[���ړ�
						move_me(&player);					// �v���C���[�ړ�
					
						TFT_draw_screen();
						if (pause_sw) {	// �|�[�Y��ʂֈڍs
							status = PAUSE1;
						}
						if (stop_sw) {	//	stop�X�C�b�`�������ꂽ�ꍇ
							LCD_init();	//	LCD��������
							score = 0;	//
							life = 3;	// �c�@, �X�R�A��������
							disp_life(life);
							disp_score(score);
							
							// �v���C���[�̍��W��������
							player.x = 144;
							player.y = 224;
				
							// �{�[���̍��W, �x�N�g����������
							ball.x = 156;
							ball.dx = -3;
							ball.y = 216;
							ball.dy = -3;
							
							// TFT�̕\����������
							TFT_clear();
							
							/* �摜�`�� */
							TFT_put_img(ball.x, ball.y, 8, 8, img_A);
							TFT_put_img(player.x, player.y, 32, 6, img_B);
							for (i = 0; i < 11; i++) {
								for (j = 0; j < 10; j++) {
									block_p->x = 40+(24*j);
									block_p->y = 24+(8*i);
									if (j % 2 == 0) {
										block_p->active = 1;
										blocks++;
										TFT_put_img(block_p->x, block_p->y, 24, 8, img_C);
									}
									else if (j % 2 != 0) {
										block_p->active = 2;
										blocks +=2 ;
										TFT_put_img(block_p->x, block_p->y, 24, 8, img_D);
									}
									else {
										block_p->active = 0;
									}
									block_p++;
								}
							}
			  				TFT_draw_screen();
							block_p = block;
							
							status = STOP;	// �ҋ@��ʂɈڍs
						}
					}
					else if (status == PAUSE1) {
		                if (!pause_sw) // pause_sw �� OFF �Ȃ� 
		                	status = PAUSE2; // status �� PAUSE2 ��
		            } else if (status == PAUSE2) {
						LCD_init();	//	LCD��������
						LCD_cursor(5, 0);
						LCD_putstr("PAUSE");	//	PAUSE����ʂɕ\��
						if (stop_sw) {	//	stop�X�C�b�`�������ꂽ��
							LCD_init();	//	LCD��������
							score = 0;	//
							life = 3;	// �c�@, �X�R�A��������
							disp_life(life);
							disp_score(score);
							
							// �v���C���[�̍��W��������
							player.x = 144;
							player.y = 224;
				
							// �{�[���̍��W��������
							ball.x = 156;
							ball.dx = -3;
							ball.y = 216;
							ball.dy = -3;
							
							// TFT�\�����e������
							TFT_clear();
							
							/* �摜�`�� */
							TFT_put_img(ball.x, ball.y, 8, 8, img_A);
							TFT_put_img(player.x, player.y, 32, 6, img_B);
							for (i = 0; i < 11; i++) {
								for (j = 0; j < 10; j++) {
									block_p->x = 40+(24*j);
									block_p->y = 24+(8*i);
									if (j % 2 == 0) {
										block_p->active = 1;
										blocks++;
										TFT_put_img(block_p->x, block_p->y, 24, 8, img_C);
									}
									else if (j % 2 != 0) {
										block_p->active = 2;
										blocks +=2 ;
										TFT_put_img(block_p->x, block_p->y, 24, 8, img_D);
									}
									else {
										block_p->active = 0;
									}
									block_p++;
								}
							}
			  				TFT_draw_screen();
							block_p = block;
							
							status = STOP;	//	�ҋ@��ʂɈڍs
						}
		                if (pause_sw) { // pause_sw �� ON �Ȃ�
							LCD_init();	//	LCD��������
							disp_life(life);
							disp_score(score);
		                    status = PAUSE3; // status �� PAUSE3 ��
						}
		            } else if (status == PAUSE3) {
		                if (!pause_sw) // pause_sw �� OFF �Ȃ�
		                    status = RUN; // �Q�[����ʂɈڍs
					} else if (status == CLEAR) {	//	�N���A���
						LCD_init();	//	LCD��������
						disp_score(score);
						LCD_cursor(5, 1);
						LCD_putstr("CLEAR!!");
				
						if (stop_sw) {	//	stop�X�C�b�`�������ꂽ�ꍇ
							LCD_init();	//	LCD��������
							score = 0;	//
							life = 3;	// �c�@, �X�R�A��������
							disp_life(life);
							disp_score(score);
							
							// �v���C���[�̍��W��������
							player.x = 144;
							player.y = 224;
				
							// �{�[���̍��W, �x�N�g����������
							ball.x = 156;
							ball.dx = -3;
							ball.y = 216;
							ball.dy = -3;
							
							// TFT�\�����e������
							TFT_clear();
							
							/* �摜�`�� */
							TFT_put_img(ball.x, ball.y, 8, 8, img_A);
							TFT_put_img(player.x, player.y, 32, 6, img_B);
							for (i = 0; i < 11; i++) {
								for (j = 0; j < 10; j++) {
									block_p->x = 40+(24*j);
									block_p->y = 24+(8*i);
									if (j % 2 == 0) {
										block_p->active = 1;
										blocks++;
										TFT_put_img(block_p->x, block_p->y, 24, 8, img_C);
									}
									else if (j % 2 != 0) {
										block_p->active = 2;
										blocks +=2 ;
										TFT_put_img(block_p->x, block_p->y, 24, 8, img_D);
									}
									else {
										block_p->active = 0;
									}
									block_p++;
								}
							}
			  				TFT_draw_screen();
							block_p = block;
							
							status = STOP;	//	�ҋ@��ʂɈڍs
						}
		            } else if (status == FAIL) {	// �Q�[���I�[�o�[���
						LCD_init();	//	LCD��������
						disp_score(score);
						LCD_cursor(4, 1);
						LCD_putstr("GAME OVER");
						
						if (stop_sw) {	//	stop�X�C�b�`�������ꂽ�ꍇ
							LCD_init();	//	LCD��������
							score = 0;	//
							life = 3;	// �c�@, �X�R�A��������
							disp_life(life);
							disp_score(score);
							
							// �v���C���[�̍��W��������
							player.x = 144;
							player.y = 224;
				
							// �{�[���̍��W, �x�N�g����������
							ball.x = 156;
							ball.dx = -3;
							ball.y = 216;
							ball.dy = -3;
							
							// TFT�\�����e������
							TFT_clear();
							
							/* �摜�`�� */
							TFT_put_img(ball.x, ball.y, 8, 8, img_A);
							TFT_put_img(player.x, player.y, 32, 6, img_B);
							for (i = 0; i < 11; i++) {
								for (j = 0; j < 10; j++) {
									block_p->x = 40+(24*j);
									block_p->y = 24+(8*i);
									if (j % 2 == 0) {
										block_p->active = 1;
										blocks++;
										TFT_put_img(block_p->x, block_p->y, 24, 8, img_C);
									}
									else if (j % 2 != 0) {
										block_p->active = 2;
										blocks +=2 ;
										TFT_put_img(block_p->x, block_p->y, 24, 8, img_D);
									}
									else {
										block_p->active = 0;
									}
									block_p++;
								}
							}
			  				TFT_draw_screen();
							block_p = block;
							
							status = STOP;	//	�ҋ@��ʂɈڍs
						}
		   			} else if (status == STOP) { // �ҋ@���
		                // ��~��
		                if (run_sw) {	//	�ڑ҃��[�h�i�搶�p�j
							if (AD0.ADDR0 < 0x4000) {	//	�W���C�X�e�B�b�N����ɓ|������ԂŃX�^�[�g������
								blocks = 1;			// �c��u���b�N����ύX
								score = 9999;		// �X�R�A��ύX
								disp_score(score);
							}
		                    status= RUN;	// �Q�[����ʂɈڍs
						}
		            }
				}
			}
		}
	}
	SD_CS = 1;				// CS negate
}