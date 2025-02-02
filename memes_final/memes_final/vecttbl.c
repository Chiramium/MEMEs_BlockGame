/***********************************************************************/
/*                                                                     */
/*  FILE        :vecttbl.c                                             */
/*  DATE        :Fri, Jan 11, 2019                                     */
/*  DESCRIPTION :Initialize of Vector Table                            */
/*  CPU TYPE    :SH7085                                                */
/*                                                                     */
/*  This file is generated by Renesas Project Generator (Ver.4.19).    */
/*  NOTE:THIS IS A TYPICAL EXAMPLE.                                    */
/***********************************************************************/
                  


#include "vect.h"

#pragma section VECTTBL

void *RESET_Vectors[] = {
//;<<VECTOR DATA START (POWER ON RESET)>>
//;0 Power On Reset PC
PowerON_Reset_PC,                                                                                                                            
//;<<VECTOR DATA END (POWER ON RESET)>>
// 1 Power On Reset SP
    __secend("S"),
//;<<VECTOR DATA START (MANUAL RESET)>>
//;2 Manual Reset PC
Manual_Reset_PC,                                                                                                                             
//;<<VECTOR DATA END (MANUAL RESET)>>
// 3 Manual Reset SP
    __secend("S")

};
#pragma section INTTBL
void *INT_Vectors[] = {
// 4 Illegal code
    (void*) INT_Illegal_code,
// 5 Reserved
    (void*) Dummy,
// 6 Illegal slot
    (void*) INT_Illegal_slot,
// 7 Reserved
    (void*) Dummy,
// 8 Reserved
    (void*) Dummy,
// 9 CPU Address error
    (void*) INT_CPU_Address,
// 10 DMAC/DTC Address error
    (void*) INT_DTC_Address,
// 11 NMI
    (void*) INT_NMI,
// 12 User breakpoint trap
    (void*) INT_User_Break,
// 13 Reserved
    (void*) Dummy,
// 14 H-UDI
    (void*) Dummy,
// 15 Reserved
    (void*) Dummy,
// 16 Reserved
    (void*) Dummy,
// 17 Reserved
    (void*) Dummy,
// 18 Reserved
    (void*) Dummy,
// 19 Reserved
    (void*) Dummy,
// 20 Reserved
    (void*) Dummy,
// 21 Reserved
    (void*) Dummy,
// 22 Reserved
    (void*) Dummy,
// 23 Reserved
    (void*) Dummy,
// 24 Reserved
    (void*) Dummy,
// 25 Reserved
    (void*) Dummy,
// 26 Reserved
    (void*) Dummy,
// 27 Reserved
    (void*) Dummy,
// 28 Reserved
    (void*) Dummy,
// 29 Reserved
    (void*) Dummy,
// 30 Reserved
    (void*) Dummy,
// 31 Reserved
    (void*) Dummy,
// 32 TRAPA (User Vecter)
    (void*) INT_TRAPA32,
// 33 TRAPA (User Vecter)
    (void*) INT_TRAPA33,
// 34 TRAPA (User Vecter)
    (void*) INT_TRAPA34,
// 35 TRAPA (User Vecter)
    (void*) INT_TRAPA35,
// 36 TRAPA (User Vecter)
    (void*) INT_TRAPA36,
// 37 TRAPA (User Vecter)
    (void*) INT_TRAPA37,
// 38 TRAPA (User Vecter)
    (void*) INT_TRAPA38,
// 39 TRAPA (User Vecter)
    (void*) INT_TRAPA39,
// 40 TRAPA (User Vecter)
    (void*) INT_TRAPA40,
// 41 TRAPA (User Vecter)
    (void*) INT_TRAPA41,
// 42 TRAPA (User Vecter)
    (void*) INT_TRAPA42,
// 43 TRAPA (User Vecter)
    (void*) INT_TRAPA43,
// 44 TRAPA (User Vecter)
    (void*) INT_TRAPA44,
// 45 TRAPA (User Vecter)
    (void*) INT_TRAPA45,
// 46 TRAPA (User Vecter)
    (void*) INT_TRAPA46,
// 47 TRAPA (User Vecter)
    (void*) INT_TRAPA47,
// 48 TRAPA (User Vecter)
    (void*) INT_TRAPA48,
// 49 TRAPA (User Vecter)
    (void*) INT_TRAPA49,
// 50 TRAPA (User Vecter)
    (void*) INT_TRAPA50,
// 51 TRAPA (User Vecter)
    (void*) INT_TRAPA51,
// 52 TRAPA (User Vecter)
    (void*) INT_TRAPA52,
// 53 TRAPA (User Vecter)
    (void*) INT_TRAPA53,
// 54 TRAPA (User Vecter)
    (void*) INT_TRAPA54,
// 55 TRAPA (User Vecter)
    (void*) INT_TRAPA55,
// 56 TRAPA (User Vecter)
    (void*) INT_TRAPA56,
// 57 TRAPA (User Vecter)
    (void*) INT_TRAPA57,
// 58 TRAPA (User Vecter)
    (void*) INT_TRAPA58,
// 59 TRAPA (User Vecter)
    (void*) INT_TRAPA59,
// 60 TRAPA (User Vecter)
    (void*) INT_TRAPA60,
// 61 TRAPA (User Vecter)
    (void*) INT_TRAPA61,
// 62 TRAPA (User Vecter)
    (void*) INT_TRAPA62,
// 63 TRAPA (User Vecter)
    (void*) INT_TRAPA63,
// 64 Interrupt IRQ0
    (void*) INT_IRQ0,
// 65 Interrupt IRQ1
    (void*) INT_IRQ1,
// 66 Interrupt IRQ2
    (void*) INT_IRQ2,
// 67 Interrupt IRQ3
    (void*) INT_IRQ3,
// 68 Interrupt IRQ4
    (void*) INT_IRQ4,
// 69 Interrupt IRQ5
    (void*) INT_IRQ5,
// 70 Interrupt IRQ6
    (void*) INT_IRQ6,
// 71 Interrupt IRQ7
    (void*) INT_IRQ7,
// 72 DMAC0 DEI0
    (void*) INT_DMAC0_DEI0,
// 73 Reserved
    (void*) Dummy,
// 74 Reserved
    (void*) Dummy,
// 75 Reserved
    (void*) Dummy,
// 76 DMAC1 DEI1
    (void*) INT_DMAC1_DEI1,
// 77 Reserved
    (void*) Dummy,
// 78 Reserved
    (void*) Dummy,
// 79 Reserved
    (void*) Dummy,
// 80 DMAC2 DEI2
    (void*) INT_DMAC2_DEI2,
// 81 Reserved
    (void*) Dummy,
// 82 Reserved
    (void*) Dummy,
// 83 Reserved
    (void*) Dummy,
// 84 DMAC3 DEI3
    (void*) INT_DMAC3_DEI3,
// 85 Reserved
    (void*) Dummy,
// 86 Reserved
    (void*) Dummy,
// 87 Reserved
    (void*) Dummy,
// 88 MTU0 TGIA0
    (void*) INT_MTU0_TGIA0,
// 89 MTU0 TGIB0
    (void*) INT_MTU0_TGIB0,
// 90 MTU0 TGIC0
    (void*) INT_MTU0_TGIC0,
// 91 MTU0 TGID0
    (void*) INT_MTU0_TGID0,
// 92 MTU0 TCIV0
    (void*) INT_MTU0_TCIV0,
// 93 MTU0 TGIE0
    (void*) INT_MTU0_TGIE0,
// 94 MTU0 TGIF0
    (void*) INT_MTU0_TGIF0,
// 95 Reserved
    (void*) Dummy,
// 96 MTU1 TGIA1
    (void*) INT_MTU1_TGIA1,
// 97 MTU1 TGIB1
    (void*) INT_MTU1_TGIB1,
// 98 Reserved
    (void*) Dummy,
// 99 Reserved
    (void*) Dummy,
// 100 MTU1 TCIV1
    (void*) INT_MTU1_TCIV1,
// 101 MTU1 TCIU1
    (void*) INT_MTU1_TCIU1,
// 102 Reserved
    (void*) Dummy,
// 103 Reserved
    (void*) Dummy,
// 104 MTU2 TGIA2
    (void*) INT_MTU2_TGIA2,
// 105 MTU2 TGIB2
    (void*) INT_MTU2_TGIB2,
// 106 Reserved
    (void*) Dummy,
// 107 Reserved
    (void*) Dummy,
// 108 MTU2 TCIV2
    (void*) INT_MTU2_TCIV2,
// 109 MTU2 TCIU2
    (void*) INT_MTU2_TCIU2,
// 110 Reserved
    (void*) Dummy,
// 111 Reserved
    (void*) Dummy,
// 112 MTU3 TGIA3
    (void*) INT_MTU3_TGIA3,
// 113 MTU3 TGIB3
    (void*) INT_MTU3_TGIB3,
// 114 MTU3 TGIC3
    (void*) INT_MTU3_TGIC3,
// 115 MTU3 TGID3
    (void*) INT_MTU3_TGID3,
// 116 MTU3 TCIV3
    (void*) INT_MTU3_TCIV3,
// 117 Reserved
    (void*) Dummy,
// 118 Reserved
    (void*) Dummy,
// 119 Reserved
    (void*) Dummy,
// 120 MTU4 TGIA4
    (void*) INT_MTU4_TGIA4,
// 121 MTU4 TGIB4
    (void*) INT_MTU4_TGIB4,
// 122 MTU4 TGIC4
    (void*) INT_MTU4_TGIC4,
// 123 MTU4 TGID4
    (void*) INT_MTU4_TGID4,
// 124 MTU4 TCIV4
    (void*) INT_MTU4_TCIV4,
// 125 Reserved
    (void*) Dummy,
// 126 Reserved
    (void*) Dummy,
// 127 Reserved
    (void*) Dummy,
// 128 MTU5 TGIU5
    (void*) INT_MTU5_TGIU5,
// 129 MTU5 TGIV5
    (void*) INT_MTU5_TGIV5,
// 130 MTU5 TGIW5
    (void*) INT_MTU5_TGIW5,
// 131 Reserved
    (void*) Dummy,
// 132 POE OEI1
    (void*) INT_POE_OEI1,
// 133 POE OEI3
    (void*) INT_POE_OEI3,
// 134 Reserved
    (void*) Dummy,
// 135 Reserved
    (void*) Dummy,
// 136 Reserved
    (void*) Dummy,
// 137 Reserved
    (void*) Dummy,
// 138 Reserved
    (void*) Dummy,
// 139 Reserved
    (void*) Dummy,
// 140 Reserved
    (void*) Dummy,
// 141 Reserved
    (void*) Dummy,
// 142 Reserved
    (void*) Dummy,
// 143 Reserved
    (void*) Dummy,
// 144 Reserved
    (void*) Dummy,
// 145 Reserved
    (void*) Dummy,
// 146 Reserved
    (void*) Dummy,
// 147 Reserved
    (void*) Dummy,
// 148 Reserved
    (void*) Dummy,
// 149 Reserved
    (void*) Dummy,
// 150 Reserved
    (void*) Dummy,
// 151 Reserved
    (void*) Dummy,
// 152 Reserved
    (void*) Dummy,
// 153 Reserved
    (void*) Dummy,
// 154 Reserved
    (void*) Dummy,
// 155 Reserved
    (void*) Dummy,
// 156 IIC2 IINAKI
    (void*) INT_IIC2_IINAKI,
// 157 Reserved
    (void*) Dummy,
// 158 Reserved
    (void*) Dummy,
// 159 Reserved
    (void*) Dummy,
// 160 MTU2S_3 TGIA_3S
    (void*) INT_MTU2S_3_TGIA_3S,
// 161 MTU2S_3 TGIB_3S
    (void*) INT_MTU2S_3_TGIB_3S,
// 162 MTU2S_3 TGIC_3S
    (void*) INT_MTU2S_3_TGIC_3S,
// 163 MTU2S_3 TGID_3S
    (void*) INT_MTU2S_3_TGID_3S,
// 164 MTU2S_3 TCIV_3S
    (void*) INT_MTU2S_3_TCIV_3S,
// 165 Reserved
    (void*) Dummy,
// 166 Reserved
    (void*) Dummy,
// 167 Reserved
    (void*) Dummy,
// 168 MTU2S_4 TGIA_4S
    (void*) INT_MTU2S_4_TGIA_4S,
// 169 MTU2S_4 TGIB_4S
    (void*) INT_MTU2S_4_TGIB_4S,
// 170 MTU2S_4 TGIC_4S
    (void*) INT_MTU2S_4_TGIC_4S,
// 171 MTU2S_4 TGID_4S
    (void*) INT_MTU2S_4_TGID_4S,
// 172 MTU2S_4 TCIV_4S
    (void*) INT_MTU2S_4_TCIV_4S,
// 173 Reserved
    (void*) Dummy,
// 174 Reserved
    (void*) Dummy,
// 175 Reserved
    (void*) Dummy,
// 176 MTU2S_5 TGIU_5S
    (void*) INT_MTU2S_5_TGIU_5S,
// 177 MTU2S_5 TGIV_5S
    (void*) INT_MTU2S_5_TGIV_5S,
// 178 MTU2S_5 TGIW_5S
    (void*) INT_MTU2S_5_TGIW_5S,
// 179 Reserved
    (void*) Dummy,
// 180 POE OEI2
    (void*) INT_POE_OEI2,
// 181 Reserved
    (void*) Dummy,
// 182 Reserved
    (void*) Dummy,
// 183 Reserved
    (void*) Dummy,
// 184 CMT0 CMI0
    (void*) INT_CMT0_CMI0,
// 185 Reserved
    (void*) Dummy,
// 186 Reserved
    (void*) Dummy,
// 187 Reserved
    (void*) Dummy,
// 188 CMT1 CMI1
    (void*) INT_CMT1_CMI1,
// 189 Reserved
    (void*) Dummy,
// 190 Reserved
    (void*) Dummy,
// 191 Reserved
    (void*) Dummy,
// 192 BSC CMI
    (void*) INT_BSC_CMI,
// 193 Reserved
    (void*) Dummy,
// 194 Reserved
    (void*) Dummy,
// 195 Reserved
    (void*) Dummy,
// 196 WDT ITI
    (void*) INT_WDT_ITI,
// 197 Reserved
    (void*) Dummy,
// 198 Reserved
    (void*) Dummy,
// 199 Reserved
    (void*) Dummy,
// 200 AD0 ADI0
    (void*) INT_AD0_ADI0,
// 201 AD1 ADI1
    (void*) INT_AD1_ADI1,
// 202 Reserved
    (void*) Dummy,
// 203 Reserved
    (void*) Dummy,
// 204 AD2 ADI2
    (void*) INT_AD2_ADI2,
// 205 Reserved
    (void*) Dummy,
// 206 Reserved
    (void*) Dummy,
// 207 Reserved
    (void*) Dummy,
// 208 Reserved
    (void*) Dummy,
// 209 Reserved
    (void*) Dummy,
// 210 Reserved
    (void*) Dummy,
// 211 Reserved
    (void*) Dummy,
// 212 Reserved
    (void*) Dummy,
// 213 Reserved
    (void*) Dummy,
// 214 Reserved
    (void*) Dummy,
// 215 Reserved
    (void*) Dummy,
// 216 SCI0 ERI0
    (void*) INT_SCI0_ERI0,
// 217 SCI0 RXI0
    (void*) INT_SCI0_RXI0,
// 218 SCI0 TXI0
    (void*) INT_SCI0_TXI0,
// 219 SCI0 TEI0
    (void*) INT_SCI0_TEI0,
// 220 SCI1 ERI1
    (void*) INT_SCI1_ERI1,
// 221 SCI1 RXI1
    (void*) INT_SCI1_RXI1,
// 222 SCI1 TXI1
    (void*) INT_SCI1_TXI1,
// 223 SCI1 TEI1
    (void*) INT_SCI1_TEI1,
// 224 SCI2 ERI2
    (void*) INT_SCI2_ERI2,
// 225 SCI2 RXI2
    (void*) INT_SCI2_RXI2,
// 226 SCI2 TXI2
    (void*) INT_SCI2_TXI2,
// 227 SCI2 TEI2
    (void*) INT_SCI2_TEI2,
// 228 SCIF ERIF
    (void*) INT_SCIF_ERIF,
// 229 SCIF RXIF
    (void*) INT_SCIF_RXIF,
// 230 SCIF BRIF
    (void*) INT_SCIF_BRIF,
// 231 SCIF TXIF
    (void*) INT_SCIF_TXIF,
// 232 SSU SSERI
    (void*) INT_SSU_SSERI,
// 233 SSU SSRXI
    (void*) INT_SSU_SSRXI,
// 234 SSU SSTXI
    (void*) INT_SSU_SSTXI,
// 235 Reserved
    (void*) Dummy,
// 236 IIC2 IITEI
    (void*) INT_IIC2_IITEI,
// 237 IIC2 IISTPI
    (void*) INT_IIC2_IISTPI,
// 238 IIC2 IITXI
    (void*) INT_IIC2_IITXI,
// 239 IIC2 IIRXI
    (void*) INT_IIC2_IIRXI,
// 240 Reserved
    (void*) Dummy,
// 241 Reserved
    (void*) Dummy,
// 242 Reserved
    (void*) Dummy,
// 243 Reserved
    (void*) Dummy,
// 244 Reserved
    (void*) Dummy,
// 245 Reserved
    (void*) Dummy,
// 246 Reserved
    (void*) Dummy,
// 247 Reserved
    (void*) Dummy,
// 248 Reserved
    (void*) Dummy,
// 249 Reserved
    (void*) Dummy,
// 250 Reserved
    (void*) Dummy,
// 251 Reserved
    (void*) Dummy,
// 252 Reserved
    (void*) Dummy,
// 253 Reserved
    (void*) Dummy,
// 254 Reserved
    (void*) Dummy,
// 255 Reserved
    (void*) Dummy
};
