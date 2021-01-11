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
 
#define NMROF_ROCKS 7

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
 
enum STS{STOP, RUN, PAUSE1, PAUSE2, PAUSE3, CLEAR};

int score = 0;	//	�X�R�A���i�[����ϐ�
int spk_switch = 0;	//	�X�s�[�J�[�̃X�C�b�`�؂�ւ� 0 ... �I�� / 1 ... �I�t
int counter = 0;	//	�X�s�[�J�[�I�����Ԃ��J�E���g����ϐ�
int sound = 0;	//	�o���������߂�ϐ�

int speed = 5;

_UWORD img_A[64];
		
_UWORD img_B[192];
	
_UWORD img_C[192];
 
struct position {
  int x;	//
  int y;	// LCD��̍��W
  int active;	// ���\��...1 / ��\��...0 / �A�C�e��...2
};

struct ball {
  int x;	//
  int y;	// LCD��̍��W
  int dx;
  int dy;
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
void TFT_put_img16(_UWORD x, _UWORD y, _UWORD *img);
void TFT_put_img(_UWORD x, _UWORD y, _UWORD w, _UWORD h, _UWORD *img);
void LCD_inst(_SBYTE);
void LCD_data(_SBYTE);
void LCD_cursor(_UINT, _UINT);
void LCD_putch(_SBYTE);
void LCD_putstr(_SBYTE *);
void LCD_cls(void);
void LCD_init(void);
void disp_led(_SINT digit);
 
 
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

void TFT_put_img16(_UWORD x, _UWORD y, _UWORD *img) {
	int i, j;
	for (j = 0; j < 16; j++) {
		for (i = 0; i < 16; i++) {
			TFT_draw_point(x+i, y+j, *(img+(j*16+i)));
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

//	LED�̕\��
void disp_led(_SINT digit) {
	_UINT i;
	_UINT tm = 1000;
	DIG1 = DIG2 = DIG3 = 0;
	PA.DR.BYTE.HL &= 0xf0;
	if (digit == 0) {
		PA.DR.BYTE.HL |= score % 10;
		DIG1 = 1;
	} else if (digit == 1) {
		PA.DR.BYTE.HL |= (score / 10) % 10;
		DIG2 = 1;
	} else {
		PA.DR.BYTE.HL |= (score / 100) % 10;
		DIG3 = 1;
	}
}
 
// --------------------------------------------
// -- �Q�[���p�̊֐��Q --
 
// -- �������ړ� --
void move_me(struct position *me)
{
    struct position old_position;
 
    old_position.x = me->x;
    old_position.y = me->y;
 
 	/*if (AD0.ADDR0 < 0x4000) {
        // -- �W���C�X�e�B�b�N�� --
		if (me->y >= 0 + speed) {
        	me->y -= speed;
		}
		else {
			me->y = 0;
		}
    } else if (AD0.ADDR0 > 0xc000) {
        // -- �W���C�X�e�B�b�N�� --
		if (me->y <= 234 - speed) {
        	me->y += speed;
		}
		else {
			me->y = 234;
		}
    }
	*/
	
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
    //TFT_clear();
}


// -- �Փ˔��� --/
void isCollided(struct position *me, struct ball *ball)
{
	int i;
	int j;
	
	
}
 
// -- ����ړ� --
void move_rock(struct position rock[])
{
    int i;
 
    for (i = 0; i < NMROF_ROCKS; i++) {
        if (rock[i].active) {
            // ��ʏ�Ɋ₪���݂���
            LCD_cursor(rock[i].x, rock[i].y);
            LCD_putch(' ');
            if (rock[i].x == 0) {
                // ����
                rock[i].active = 0;
				score++;
				//spk_switch = 1;
            } else {
                rock[i].x--;
                LCD_cursor(rock[i].x, rock[i].y);
				if (rock[i].active == 2) {	//	�A�C�e���̏ꍇ�̈ړ�
					LCD_putch('@');
				}	
                else{	//	��̏ꍇ�̈ړ�
					LCD_putch('*');
				}
            }
        }
    }
}
 
 
// -- �V���������� --
void new_rock(struct position rock[])
{
    int i;
 
    for (i = 0; i < NMROF_ROCKS; i++) {
        if (rock[i].active == 0) {
            // -- �V������ --
			if (rand() % 5 == 0) {	//	20%�̊m���ŃA�C�e���𐶐�
				rock[i].active = 2;	//	�A�C�e����active:2�Ƃ���
			}
			else {
            	rock[i].active = 1;	//	���active:1�Ƃ���
			}
            rock[i].x = 15;
            rock[i].y = rand() % 2;
            LCD_cursor(rock[i].x, rock[i].y);
			if (rock[i].active == 2) {	//	active��2�Ȃ���S��������A�C�e���i�{���j��\��
            	LCD_putch('@');
			}
			else {	//	1�Ȃ���\��
				LCD_putch('*');
			}
            break;
        }
    }
}


// -- ���S���� --
void erase_rock(struct position rock[])
{
	int i;
	for (i = 0; i < NMROF_ROCKS; i++) {	//	������ׂĂ͂��߂̏ꏊ�ɏ��������Ĕ�\���ɂ���
		rock[i].active = 0;
		rock[i].x = 15;
		rock[i].y =1;
	}
}

void move_ball(struct ball *ball, struct position *me)
{
	struct ball old_position;
 
    old_position.x = ball->x;
    old_position.y = ball->y;
	
	ball->x += ball->dx;
	if (ball->x >= 320-8 || ball->x <= 0 || (ball->x >= me->x-8 && ball->x <= me->x+32 && ball->y >= me->y-6 && ball->y <= me->y+6)) {
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
	if (ball->y >= 240-8 || ball->y <= 0 || (ball->y >= me->y-6 && ball->y <= me->y+6 && ball->x >= me->x-8 && ball->x <= me->x+32)) {
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
	
	TFT_draw_rect(old_position.x, old_position.y, 8, 8, _COL_WHITE);
	TFT_put_img(ball->x, ball->y, 8, 8, img_A);
}

 
// --------------------------------------------
// -- ���C���֐� --
void main()
{
    struct position me;                 // �����̎Ԃ̍��W
    struct position rock[NMROF_ROCKS];  // ��̍��W
	struct position player;
	struct ball ball;
    int move_timing, new_timing;
    int ad, i, digit = 0;
    int stop_sw, run_sw, pause_sw;
    int status;
	char result[3] = {' ', ' ', ' '};
	
	int j;
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
	
	init_CMT0();
	init_SCI2();
	init_CS2();
  	TFT_On();
  	TFT_clear();
 
    STB.CR4.BIT._AD0 = 0;
    STB.CR4.BIT._CMT = 0;
    STB.CR4.BIT._MTU2 = 0;
	
	PFC.PEIORL.BIT.B0 = 1;
 
    CMT0.CMCSR.BIT.CKS = 1;
 
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
    MTU21.TGRA = 3125 - 1;         // 100ms
	
	// MTU2 ch2
    MTU22.TCR.BIT.TPSC = 3;         // 1/64�I��
    MTU22.TCR.BIT.CCLR = 1;         // TGRA�̃R���y�A�}�b�`�ŃN���A
    MTU22.TGRA = 1563 - 1;          // 5ms
	
	// MTU2 ch3
    MTU23.TCR.BIT.TPSC = 0;         // 1/64�I��
    MTU23.TCR.BIT.CCLR = 1;         // TGRA�̃R���y�A�}�b�`�ŃN���A
    MTU23.TGRA = 9555 - 1;         // C
	
	SPK = 0;
 
    LCD_init();
 
    MTU2.TSTR.BIT.CST0 = 1;         // MTU2 CH0�X�^�[�g
    MTU2.TSTR.BIT.CST1 = 1;         // MTU2 CH1�X�^�[�g
	MTU2.TSTR.BIT.CST2 = 1;         // MTU2 CH2�X�^�[�g
	MTU2.TSTR.BIT.CST3 = 1;         // MTU2 CH3�X�^�[�g
	
	PFC.PEIORL.BIT.B11 = 1;
	
	//LED6 = LED_OFF;
 
	/*
    me.x = me.y = 0;
    for (i = 0; i < NMROF_ROCKS; i++)
        rock[i].active = 0;
 
    status = STOP;
    move_timing = new_timing = 0;
    while (1) {
        if (MTU21.TSR.BIT.TGFA) { // �Q�[���p�̃^�C�}�̃t���O
            // MTU2 ch1 �R���y�A�}�b�`����(100ms��)
            MTU21.TSR.BIT.TGFA = 0; // �t���O�N���A
 
            // 100ms��1��A�X�C�b�`��ǂ�
            stop_sw = SW4;
            pause_sw = SW5;
            run_sw = SW6;
 
            if (status == RUN) {
                // �Q�[����
				MTU23.TGRA = 10725 - 1;	//	����ς���
				isCollided(&me, rock);	//	�Փ˔���
				if (score >= 1000) {	//	score��1000�𒴂�����Q�[���N���A
					status = CLEAR;	//	�X�e�[�^�X��CLEAR�ɐݒ�
					LCD_init();	//	LCD��������
				}
				
				if (spk_switch == 1) {	//	�X�s�[�J�[�̃X�C�b�`��1�̏ꍇ
					if (counter >= 1) {	//	counter��1�𒴂�����
						spk_switch = 0;	//	�X�s�[�J�[�̃X�C�b�`��؂�
						counter = 0;	//	counter��������
					}
					else {
					counter++;	//	counter���C���N�������g
					}	
				}
				
                move_me(&me);           // �����ړ�
                if (move_timing++ >= 2) {
                    move_timing = 0;
					MTU23.TGRA = 14317 - 1;	//	����ς���
                    move_rock(rock);    // ����ړ�
                    if (new_timing-- <= 0) {
                        new_timing = rand() * 5 / (RAND_MAX + 1);
                        new_rock(rock);     // �V�����₪�o��
                    }
                }
                if (pause_sw) {
					spk_switch = 0;
                    status = PAUSE1;
				}
					
				if (stop_sw) {
					LCD_init();
					spk_switch = 0;
					status = STOP;
				}
            } else if (status == PAUSE1) {
                if (!pause_sw) // pause_sw �� OFF �Ȃ� 
                	status = PAUSE2; // status �� PAUSE2 ��
            } else if (status == PAUSE2) {
				LCD_cursor(10, 0);
				LCD_putstr("PAUSE");	//	PAUSE����ʂɕ\��
				if (stop_sw) {	//	stop�X�C�b�`�������ꂽ��
					LCD_init();	//	LCD��������
					status = STOP;	//	status��STOP�ɕύX
				}
                if (pause_sw) { // pause_sw �� ON �Ȃ�
					LCD_cursor(10, 0);
					LCD_putstr("     ");
                    status = PAUSE3; // status �� PAUSE3 ��
				}
            } else if (status == PAUSE3) {
                if (!pause_sw) // pause_sw �� OFF �Ȃ�
                    status = RUN; // sutatus �� RUN ��
			} else if (status == CLEAR) {	//	CLEAR���
				spk_switch = 1;	//	�X�s�[�J�[���I���ɂ���
				
				switch (sound) {	//	���K�����Ԃɏo��
					case 0:
						MTU23.TGRA = 10725 - 1;
						sound++;
						break;
					case 1:
						MTU23.TGRA = 8513 - 1;
						sound++;
						break;
					case 2:
						MTU23.TGRA = 7158 - 1;
						sound++;
						break;
					case 3:
						MTU23.TGRA = 5362 - 1;
						sound++;
						break;
					default:
						sound = 10;
						spk_switch = 0;	//	���K���Đ�������~�߂�
						break;
				}
				
				//	�N���A���
				LCD_cursor(5, 0);
				LCD_putstr("CLEAR!!");
				LCD_cursor(1, 1);
				LCD_putstr("YOU ARE GENIUS!");
				
				if (stop_sw) {	//	stop�X�C�b�`�������ꂽ�ꍇ
					status = STOP;	//	�X�e�[�^�X��STOP�ɐݒ�
					LCD_init();	//	LCD��������
				}
            } else if (status == STOP) {
                // ��~��
				//	�ϐ���������
				score = 0;
				spk_switch = 0;
				sound = 0;
				counter = 0;
				
				//	�^�C�g�����
				LCD_cursor(3, 0);
				LCD_putstr("I WA YO KE");
				LCD_cursor(4, 1);
				LCD_putstr("PUSH SW6");
				
                if (run_sw) {	//	�ڑ҃��[�h�i�搶�p�j
					if (AD0.ADDR0 < 0x4000) {	//	�W���C�X�e�B�b�N����ɓ|������ԂŃX�^�[�g������
						score = 995;	//	�X�R�A��995�_�ɐݒ�
					}
					//	�ϐ���������
					me.x = 0;
					me.y = 0;
					erase_rock(rock);
					LCD_init();
                    status= RUN;
				}
            }
        }
		// if (@@@@@) �Ƃ��� 7�Z�O�p�̃^�C�}�t���O ������悤�ɂ���Ƃ悢
		//	MTU2��p����7�Z�O�����gLED�̕\��
		if (MTU22.TSR.BIT.TGFA) {
			MTU22.TSR.BIT.TGFA = 0; // �t���O�N���A
			
			//DIG1 = DIG2 = DIG3 = 0;
			disp_led(digit);
			
			digit++;
			if (digit > 2)
				digit = 0; 
		}		
		//	MTU2��p�����X�s�[�J�̉����Đ�
		if (MTU23.TSR.BIT.TGFA) {
			if (spk_switch == 1) {
				MTU23.TSR.BIT.TGFA = 0;	//	�t���O�N���A
				SPK ^= 1;
			}
		}
    }
	*/
	
	
	if (!card_exist())
		printf("No card found\n");
	else {
		if (Enter_SPI_mode() < 0)
			printf("SPI mode Err\n");
		else {
			printf("SPI mode\n");

			SD_send_cmd(0x50, 512);		// �u���b�N�T�C�Y=512

			// MBR
			read_sector(0, dt);
			First_sect_LBA = (dt[457] << 24) | (dt[456] << 16) | (dt[455] << 8) | dt[454];
			printf("1st_sector(LBA) = 0x%x\n\n", First_sect_LBA); 

			// -- �����ɒǉ� --
			// BPB
			read_sector(0x19, dt);
			
			// BPB_BytesPerSec���v�Z
			BPB_BytesPerSec	= (dt[12] << 8) | dt[11];
			printf("BPB_BytesPerSec = %d\n", BPB_BytesPerSec);
			
			// BPB_SecPerClus���v�Z
			BPB_SecPerClus	= dt[13];
			printf("BPB_SecPerClus = %d\n", BPB_SecPerClus);
			
			// BPB_RsvdSecCnt���v�Z
			BPB_RsvdSecCnt = (dt[15] << 2) | dt[14];
			printf("BPB_RsvdSecCnt = %d\n", BPB_RsvdSecCnt);
			
			// BPB_NumFATs���v�Z
			BPB_NumFATs = dt[16];
			printf("BPB_NumFATs = %d\n", BPB_NumFATs);
			
			// BPB_RootEntCnt���v�Z
			BPB_RootEntCnt = (dt[18] << 8) | dt[17];
			printf("BPB_RootEntCnt = %d\n", BPB_RootEntCnt);
			
			// BPB_FATSz16���v�Z
			BPB_FATSz16 = (dt[23] << 8) | dt[22];
			printf("BPB_FATSz16 = %d\n", BPB_FATSz16);
			
			// First_FAT_sect
			First_FAT_sect = First_sect_LBA + BPB_RsvdSecCnt;
			printf("First_FAT_sect = 0x%x\n", First_FAT_sect);
			
			// First_RDE_sect
			First_RDE_sect = First_sect_LBA + BPB_RsvdSecCnt + (BPB_NumFATs * BPB_FATSz16);
			printf("FirstRDEsector = 0x%x\n", First_RDE_sect); 
			
			// First_Data_sect
			First_Data_sect = First_RDE_sect + (32*BPB_RootEntCnt) / BPB_BytesPerSec;
			printf("First_Data_sect = 0x%x\n\n", First_Data_sect);
				
			/* BALL */
			readsize = 0;
			imgNum = 0;
			img_firstFlag = 0;
			
			//RDE
			read_sector(First_RDE_sect, dt);
			
			i = 10;
				
				// Name
				printf("Name : ");
				for (j = 0; j < 8; j++) {
					if (isprint(dt[i * 32 + j]))
						printf("%c", dt[i * 32 + j]);
					else
						printf(".");	
				}
				printf("\n");
				
				// Ext
				printf("Ext : .");
				for (j = 0; j < 3; j++) {
					if (isprint(dt[i * 32 + j + 8]))
						printf("%c", dt[i * 32 + j + 8]);
					else
						printf(".");
				}
				printf("\n");
				
				// Attr
				if (dt[i * 32 + 11] == 0x10) {
					printf("Attr : DIR\n");
				}
				else {
					printf("Attr : FILE\n");
				}
				// CrtDate
				// File_year, File_month, File_date, File_hour, File_min, File_sec;
				File_year = (dt[i * 32 + 25] >> 1) + 1980;
				File_month = ((dt[i * 32 + 25] & 0x01) << 3) | (dt[i * 32 + 24] >> 5);
				File_date = dt[i * 32 + 24] & 0x1f;
				File_hour = dt[i * 32 + 23] >> 3;
				File_min = ((dt[i * 32 + 23] & 0x07) << 3) | dt[i * 32 + 22] >> 5;
				File_sec = dt[i * 32 + 22] & 0x1f;
				
				printf("Date : %d/%d/%d %d:%d:%d\n", File_year, File_month, File_date, File_hour, File_min, File_sec);
				
				// FstClusLO
				FstClusLO = (dt[i * 32 + 27] << 8) | dt[i * 32 + 26];
				printf("FstClusLO = %d\n", FstClusLO);
				
				// FileSize
				FileSize = (dt[i * 32 + 31] << 24) | (dt[i * 32 + 30] << 16) | (dt[i * 32 + 29] << 8) | dt[i * 32 + 28];
				printf("FileSize = %d Byte\n\n", FileSize);
				
				//FAT
				read_sector(First_FAT_sect, fat);
				cluster = FstClusLO;
				printf("%d ", cluster);
				while (cluster != 0xffff) {
					for (i = 0; i < BPB_SecPerClus; i ++) {
						read_sector((cluster - 2) * BPB_SecPerClus + First_Data_sect + i, dt);
						//printf("%d ", cluster);
						
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
							//printf("%c", dt[j]);
							if (img_firstFlag == 1){
								img_A[imgNum] = (dt[j] << 8) | dt[j+1];
								imgNum++;
							}
						}
					}
					cluster = (fat[cluster * 2 + 1] << 8) | fat[cluster * 2];
					printf("%d ", cluster);
				}
			
			/* PLAYER */
			readsize = 0;
			imgNum = 0;
			img_firstFlag = 0;
			
			//RDE
			read_sector(First_RDE_sect, dt);
			
			i = 11;
				
				// Name
				printf("Name : ");
				for (j = 0; j < 8; j++) {
					if (isprint(dt[i * 32 + j]))
						printf("%c", dt[i * 32 + j]);
					else
						printf(".");	
				}
				printf("\n");
				
				// Ext
				printf("Ext : .");
				for (j = 0; j < 3; j++) {
					if (isprint(dt[i * 32 + j + 8]))
						printf("%c", dt[i * 32 + j + 8]);
					else
						printf(".");
				}
				printf("\n");
				
				// Attr
				if (dt[i * 32 + 11] == 0x10) {
					printf("Attr : DIR\n");
				}
				else {
					printf("Attr : FILE\n");
				}
				// CrtDate
				// File_year, File_month, File_date, File_hour, File_min, File_sec;
				File_year = (dt[i * 32 + 25] >> 1) + 1980;
				File_month = ((dt[i * 32 + 25] & 0x01) << 3) | (dt[i * 32 + 24] >> 5);
				File_date = dt[i * 32 + 24] & 0x1f;
				File_hour = dt[i * 32 + 23] >> 3;
				File_min = ((dt[i * 32 + 23] & 0x07) << 3) | dt[i * 32 + 22] >> 5;
				File_sec = dt[i * 32 + 22] & 0x1f;
				
				printf("Date : %d/%d/%d %d:%d:%d\n", File_year, File_month, File_date, File_hour, File_min, File_sec);
				
				// FstClusLO
				FstClusLO = (dt[i * 32 + 27] << 8) | dt[i * 32 + 26];
				printf("FstClusLO = %d\n", FstClusLO);
				
				// FileSize
				FileSize = (dt[i * 32 + 31] << 24) | (dt[i * 32 + 30] << 16) | (dt[i * 32 + 29] << 8) | dt[i * 32 + 28];
				printf("FileSize = %d Byte\n\n", FileSize);
				
				//FAT
				read_sector(First_FAT_sect, fat);
				cluster = FstClusLO;
				printf("%d ", cluster);
				while (cluster != 0xffff) {
					for (i = 0; i < BPB_SecPerClus; i ++) {
						read_sector((cluster - 2) * BPB_SecPerClus + First_Data_sect + i, dt);
						//printf("%d ", cluster);
						
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
							//printf("%c", dt[j]);
							if (img_firstFlag == 1){
								img_B[imgNum] = (dt[j] << 8) | dt[j+1];
								imgNum++;
							}
						}
					}
					cluster = (fat[cluster * 2 + 1] << 8) | fat[cluster * 2];
					printf("%d ", cluster);
				}
				
			/* BLOCK */
			readsize = 0;
			imgNum = 0;
			img_firstFlag = 0;
			
			//RDE
			read_sector(First_RDE_sect, dt);
			
			i = 12;
				
				// Name
				printf("Name : ");
				for (j = 0; j < 8; j++) {
					if (isprint(dt[i * 32 + j]))
						printf("%c", dt[i * 32 + j]);
					else
						printf(".");	
				}
				printf("\n");
				
				// Ext
				printf("Ext : .");
				for (j = 0; j < 3; j++) {
					if (isprint(dt[i * 32 + j + 8]))
						printf("%c", dt[i * 32 + j + 8]);
					else
						printf(".");
				}
				printf("\n");
				
				// Attr
				if (dt[i * 32 + 11] == 0x10) {
					printf("Attr : DIR\n");
				}
				else {
					printf("Attr : FILE\n");
				}
				// CrtDate
				// File_year, File_month, File_date, File_hour, File_min, File_sec;
				File_year = (dt[i * 32 + 25] >> 1) + 1980;
				File_month = ((dt[i * 32 + 25] & 0x01) << 3) | (dt[i * 32 + 24] >> 5);
				File_date = dt[i * 32 + 24] & 0x1f;
				File_hour = dt[i * 32 + 23] >> 3;
				File_min = ((dt[i * 32 + 23] & 0x07) << 3) | dt[i * 32 + 22] >> 5;
				File_sec = dt[i * 32 + 22] & 0x1f;
				
				printf("Date : %d/%d/%d %d:%d:%d\n", File_year, File_month, File_date, File_hour, File_min, File_sec);
				
				// FstClusLO
				FstClusLO = (dt[i * 32 + 27] << 8) | dt[i * 32 + 26];
				printf("FstClusLO = %d\n", FstClusLO);
				
				// FileSize
				FileSize = (dt[i * 32 + 31] << 24) | (dt[i * 32 + 30] << 16) | (dt[i * 32 + 29] << 8) | dt[i * 32 + 28];
				printf("FileSize = %d Byte\n\n", FileSize);
				
				//FAT
				read_sector(First_FAT_sect, fat);
				cluster = FstClusLO;
				printf("%d ", cluster);
				while (cluster != 0xffff) {
					for (i = 0; i < BPB_SecPerClus; i ++) {
						read_sector((cluster - 2) * BPB_SecPerClus + First_Data_sect + i, dt);
						//printf("%d ", cluster);
						
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
							//printf("%c", dt[j]);
							if (img_firstFlag == 1){
								img_C[imgNum] = (dt[j] << 8) | dt[j+1];
								imgNum++;
							}
						}
					}
					cluster = (fat[cluster * 2 + 1] << 8) | fat[cluster * 2];
					printf("%d ", cluster);
				}
				
				player.x = 144;
				player.y = 224;
				
				ball.x = 156;
				ball.dx = 3;
				ball.y = 216;
				ball.dy = 3;
				
				/* Draw images */
				//TFT_put_img(156, 216, 8, 8, img_A);
				//TFT_put_img(player.x, player.y, 32, 6, img_B);
				for (i = 0; i < 11; i++) {
					for (j = 0; j < 10; j++) {
						TFT_put_img(40+(24*j), 24+(8*i), 24, 8, img_C);
					}
				}
  				TFT_draw_screen();

			// ----------------
			while (1) {
				/*printf("\nSector number = 0x");
				scanf("%x", &SECT_NR);
				read_sector(SECT_NR, dt);
				print_sector(SECT_NR, dt);*/
				//if (MTU21.TSR.BIT.TGFA) { // �Q�[���p�̃^�C�}�̃t���O
		            // MTU2 ch1 �R���y�A�}�b�`����(100ms��)
		            MTU21.TSR.BIT.TGFA = 0; // �t���O�N���A
 
		            // 100ms��1��A�X�C�b�`��ǂ�
		            stop_sw = SW4;
		            pause_sw = SW5;
		            run_sw = SW6;
					move_ball(&ball, &player);
					move_me(&player);
					//printf("%d, %d\n", player.x, player.y);
					//TFT_clear();
					//TFT_put_img(156, 216, 8, 8, img_A);
					/*for (i = 0; i < 11; i++) {
						for (j = 0; j < 10; j++) {
							TFT_put_img(40+(24*j), 24+(8*i), 24, 8, img_C);
						}
					}*/
					//TFT_draw_screen_partly(player.x, player.y, 32, 6);
					TFT_draw_screen();
					if (SW6) {
						printf("%d, %d\n", player.x, player.y);
					}
				//}
			}
		}
	}
	SD_CS = 1;				// CS negate
}