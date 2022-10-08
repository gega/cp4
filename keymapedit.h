/* :ts=8							keymapedit.h
 *
 *    cp4 - Commodore C+4 emulator
 *    Copyright (C) 1998 Gáti Gergely
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software Foundation,
 *    Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *    e-mail: gatig@dragon.klte.hu
 */

#include "keymap.h"

#define GetString( g )      ((( struct StringInfo * )g->SpecialInfo )->Buffer  )
#define GetNumber( g )      ((( struct StringInfo * )g->SpecialInfo )->LongInt )

#define gettxtnum( b )	( b==0 ? 133 : b+68 )
#define getbutnum( t )	( t==133 ? 0 : t-68 )

#define GD_kkukac                              0
#define GD_kf3                                 1
#define GD_kf2                                 2
#define GD_kf1                                 3
#define GD_khelp                               4
#define GD_kfont                               5
#define GD_kreturn                             6
#define GD_kinstdel                            7
#define GD_kshift                              8
#define GD_ke                                  9
#define GD_ks                                  10
#define GD_kz                                  11
#define GD_k4                                  12
#define GD_ka                                  13
#define GD_kw                                  14
#define GD_k3                                  15
#define GD_kx                                  16
#define GD_kt                                  17
#define GD_kf                                  18
#define GD_kc                                  19
#define GD_k6                                  20
#define GD_kd                                  21
#define GD_kr                                  22
#define GD_k5                                  23
#define GD_kv                                  24
#define GD_ku                                  25
#define GD_kh                                  26
#define GD_kb                                  27
#define GD_k8                                  28
#define GD_kg                                  29
#define GD_ky                                  30
#define GD_k7                                  31
#define GD_kn                                  32
#define GD_ko                                  33
#define GD_kk                                  34
#define GD_km                                  35
#define GD_k0                                  36
#define GD_kj                                  37
#define GD_ki                                  38
#define GD_k9                                  39
#define GD_kvesszo                             40
#define GD_kminusz                             41
#define GD_kkettospont                         42
#define GD_kpont                               43
#define GD_kup                                 44
#define GD_kl                                  45
#define GD_kp                                  46
#define GD_kdown                               47
#define GD_kper                                48
#define GD_kplusz                              49
#define GD_kegyenlo                            50
#define GD_kescape                             51
#define GD_kright                              52
#define GD_kpontosvesszo                       53
#define GD_kcsillag                            54
#define GD_kleft                               55
#define GD_krunstop                            56
#define GD_kq                                  57
#define GD_kcommodore                          58
#define GD_kspace                              59
#define GD_k2                                  60
#define GD_kcontrol                            61
#define GD_kclearhome                          62
#define GD_k1                                  63

#define GD_b_open                              64
#define GD_b_save                              65
#define GD_b_cancel                            66
#define GD_b_status                            67
#define GD_b_new                               68

#define GD_tf3                                 69
#define GD_tf2                                 70
#define GD_tf1                                 71
#define GD_thelp                               72
#define GD_tfont                               73
#define GD_treturn                             74
#define GD_tinstdel                            75
#define GD_tshift                              76
#define GD_te                                  77
#define GD_ts                                  78
#define GD_tz                                  79
#define GD_t4                                  80
#define GD_ta                                  81
#define GD_tw                                  82
#define GD_t3                                  83
#define GD_tx                                  84
#define GD_tt                                  85
#define GD_tf                                  86
#define GD_tc                                  87
#define GD_t6                                  88
#define GD_td                                  89
#define GD_tr                                  90
#define GD_t5                                  91
#define GD_tv                                  92
#define GD_tu                                  93
#define GD_th                                  94
#define GD_tb                                  95
#define GD_t8                                  96
#define GD_tg                                  97
#define GD_ty                                  98
#define GD_t7                                  99
#define GD_tn                                  100
#define GD_to                                  101
#define GD_tk                                  102
#define GD_tm                                  103
#define GD_t0                                  104
#define GD_tj                                  105
#define GD_ti                                  106
#define GD_t9                                  107
#define GD_tvesszo                             108
#define GD_tminusz                             109
#define GD_tkettospont                         110
#define GD_tpont                               111
#define GD_tup                                 112
#define GD_tl                                  113
#define GD_tp                                  114
#define GD_tdown                               115
#define GD_tper                                116
#define GD_tplusz                              117
#define GD_tegyenlo                            118
#define GD_tescape                             119
#define GD_tright                              120
#define GD_tpontosvesszo                       121
#define GD_tcsillag                            122
#define GD_tleft                               123
#define GD_trunstop                            124
#define GD_tq                                  125
#define GD_tcommodore                          126
#define GD_tspace                              127
#define GD_t2                                  128
#define GD_tcontrol                            129
#define GD_tclearhome                          130
#define GD_t1                                  131

#define GD_b_functions                         132

#define GD_tkukac                              133

#define GDX_kkukac                             0
#define GDX_kf3                                1
#define GDX_khelp                              2
#define GDX_kf2                                3
#define GDX_kf1                                4
#define GDX_kfont                              5
#define GDX_kreturn                            6
#define GDX_kinstdel                           7
#define GDX_kshift                             8
#define GDX_ke                                 9
#define GDX_ks                                 10
#define GDX_kz                                 11
#define GDX_k4                                 12
#define GDX_ka                                 13
#define GDX_kw                                 14
#define GDX_k3                                 15
#define GDX_kx                                 16
#define GDX_kt                                 17
#define GDX_kf                                 18
#define GDX_kc                                 19
#define GDX_k6                                 20
#define GDX_kd                                 21
#define GDX_kr                                 22
#define GDX_k5                                 23
#define GDX_kv                                 24
#define GDX_ku                                 25
#define GDX_kh                                 26
#define GDX_kb                                 27
#define GDX_k8                                 28
#define GDX_kg                                 29
#define GDX_ky                                 30
#define GDX_k7                                 31
#define GDX_kn                                 32
#define GDX_ko                                 33
#define GDX_kk                                 34
#define GDX_km                                 35
#define GDX_k0                                 36
#define GDX_kj                                 37
#define GDX_ki                                 38
#define GDX_k9                                 39
#define GDX_kvesszo                            40
#define GDX_kminusz                            41
#define GDX_kkettospont                        42
#define GDX_kpont                              43
#define GDX_kup                                44
#define GDX_kl                                 45
#define GDX_kp                                 46
#define GDX_kdown                              47
#define GDX_kper                               48
#define GDX_kplusz                             49
#define GDX_kegyenlo                           50
#define GDX_kescape                            51
#define GDX_kright                             52
#define GDX_kpontosvesszo                      53
#define GDX_kcsillag                           54
#define GDX_kleft                              55
#define GDX_krunstop                           56
#define GDX_kq                                 57
#define GDX_kcommodore                         58
#define GDX_kspace                             59
#define GDX_k2                                 60
#define GDX_kcontrol                           61
#define GDX_kclearhome                         62
#define GDX_k1                                 63
#define GDX_b_open                             64
#define GDX_b_save                             65
#define GDX_b_cancel                           66
#define GDX_b_status                           67
#define GDX_b_new                              68
#define GDX_tf3                                69
#define GDX_thelp                              70
#define GDX_tf2                                71
#define GDX_tf1                                72
#define GDX_tfont                              73
#define GDX_treturn                            74
#define GDX_tinstdel                           75
#define GDX_tshift                             76
#define GDX_te                                 77
#define GDX_ts                                 78
#define GDX_tz                                 79
#define GDX_t4                                 80
#define GDX_ta                                 81
#define GDX_tw                                 82
#define GDX_t3                                 83
#define GDX_tx                                 84
#define GDX_tt                                 85
#define GDX_tf                                 86
#define GDX_tc                                 87
#define GDX_t6                                 88
#define GDX_td                                 89
#define GDX_tr                                 90
#define GDX_t5                                 91
#define GDX_tv                                 92
#define GDX_tu                                 93
#define GDX_th                                 94
#define GDX_tb                                 95
#define GDX_t8                                 96
#define GDX_tg                                 97
#define GDX_ty                                 98
#define GDX_t7                                 99
#define GDX_tn                                 100
#define GDX_to                                 101
#define GDX_tk                                 102
#define GDX_tm                                 103
#define GDX_t0                                 104
#define GDX_tj                                 105
#define GDX_ti                                 106
#define GDX_t9                                 107
#define GDX_tvesszo                            108
#define GDX_tminusz                            109
#define GDX_tkettospont                        110
#define GDX_tpont                              111
#define GDX_tup                                112
#define GDX_tl                                 113
#define GDX_tp                                 114
#define GDX_tdown                              115
#define GDX_tper                               116
#define GDX_tplusz                             117
#define GDX_tegyenlo                           118
#define GDX_tescape                            119
#define GDX_tright                             120
#define GDX_tpontosvesszo                      121
#define GDX_tcsillag                           122
#define GDX_tleft                              123
#define GDX_trunstop                           124
#define GDX_tq                                 125
#define GDX_tcommodore                         126
#define GDX_tspace                             127
#define GDX_t2                                 128
#define GDX_tcontrol                           129
#define GDX_tclearhome                         130
#define GDX_t1                                 131
#define GDX_b_functions                        132
#define GDX_tkukac                             133

#define GD_ak_yet                              0
#define GD_ak_new                              1
#define GD_ak_add                              2
#define GD_ak_replace                          3
#define GD_ak_cancel                           4

#define GDX_ak_yet                             0
#define GDX_ak_new                             1
#define GDX_ak_add                             2
#define GDX_ak_replace                         3
#define GDX_ak_cancel                          4

#define GD_kf_flist                            0
#define GD_kf_yet                              1
#define GD_kf_new                              2
#define GD_kf_assign                           3
#define GD_kf_cancel                           4

#define GDX_kf_flist                           0
#define GDX_kf_yet                             1
#define GDX_kf_new                             2
#define GDX_kf_assign                          3
#define GDX_kf_cancel                          4

#define kmapedit_CNT 134
#define kmapassign_CNT 5
#define kmapfunctions_CNT 5

extern struct IntuitionBase *IntuitionBase;
extern struct Library       *GadToolsBase;

//struct IntuiMessage   kmapeditMsg;
//struct IntuiMessage   kmapassignMsg;
//struct IntuiMessage   kmapfunctionsMsg;
extern struct MinList        kf_flist2List;

static int kkukacClicked( void );
static int kf3Clicked( void );
static int khelpClicked( void );
static int kf2Clicked( void );
static int kf1Clicked( void );
static int kfontClicked( void );
static int kreturnClicked( void );
static int kinstdelClicked( void );
static int kshiftClicked( void );
static int keClicked( void );
static int ksClicked( void );
static int kzClicked( void );
static int k4Clicked( void );
static int kaClicked( void );
static int kwClicked( void );
static int k3Clicked( void );
static int kxClicked( void );
static int ktClicked( void );
static int kfClicked( void );
static int kcClicked( void );
static int k6Clicked( void );
static int kdClicked( void );
static int krClicked( void );
static int k5Clicked( void );
static int kvClicked( void );
static int kuClicked( void );
static int khClicked( void );
static int kbClicked( void );
static int k8Clicked( void );
static int kgClicked( void );
static int kyClicked( void );
static int k7Clicked( void );
static int knClicked( void );
static int koClicked( void );
static int kkClicked( void );
static int kmClicked( void );
static int k0Clicked( void );
static int kjClicked( void );
static int kiClicked( void );
static int k9Clicked( void );
static int kvesszoClicked( void );
static int kminuszClicked( void );
static int kkettospontClicked( void );
static int kpontClicked( void );
static int kupClicked( void );
static int klClicked( void );
static int kpClicked( void );
static int kdownClicked( void );
static int kperClicked( void );
static int kpluszClicked( void );
static int kegyenloClicked( void );
static int kescapeClicked( void );
static int krightClicked( void );
static int kpontosvesszoClicked( void );
static int kcsillagClicked( void );
static int kleftClicked( void );
static int krunstopClicked( void );
static int kqClicked( void );
static int kcommodoreClicked( void );
static int kspaceClicked( void );
static int k2Clicked( void );
static int kcontrolClicked( void );
static int kclearhomeClicked( void );
static int k1Clicked( void );
static int b_openClicked( void );
static int b_saveClicked( void );
static int b_cancelClicked( void );
static int b_newClicked( void );
static int b_functionsClicked( void );
static int ak_addClicked( void );
static int ak_replaceClicked( void );
static int ak_cancelClicked( void );
static int kf_flistClicked( void );
static int kf_assignClicked( void );
static int kf_cancelClicked( void );

static int kmapeditCloseWindow();
static int kmapeditRawKey(struct IntuiMessage *imsg);
static int kmapeditGadgetHelp(struct IntuiMessage *imsg);
static int HandlekmapassignIDCMP( void );
static int kmapassignCloseWindow();
static int kmapassignRawKey(struct IntuiMessage *imsg);
static int OpenkmapassignWindow( void );
static void ClosekmapassignWindow( void );
static int HandlekmapfunctionsIDCMP( void );
static int kmapfunctionsCloseWindow();
static int kmapfunctionsRawKey(struct IntuiMessage *imsg);
static int OpenkmapfunctionsWindow( void );
static void ClosekmapfunctionsWindow( void );
