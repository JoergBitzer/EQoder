#pragma once

const int g_minGuiSize_x(660);
const int g_maxGuiSize_x(1200);
const int g_minGuiSize_y(400);
const float g_guiratio = float(g_minGuiSize_y)/g_minGuiSize_x;

// global graphics constants for minimal size of buttons slider combos etc. 

#define GLOBAL_MIN_LABEL_WIDTH 60
#define GLOBAL_MIN_LABEL_FONTSIZE 15

#define GLOBAL_MIN_ROTARY_WIDTH 60
#define GLOBAL_MIN_ROTARY_TB_WIDTH 60
#define GLOBAL_MIN_ROTARY_TB_HEIGHT 20

#define GLOBAL_MIN_DISTANCE 5
#define GLOBAL_MIN_LABEL_HEIGHT 20

// parameter blocks
#define EQODER_MIN_XPOS 10
#define EQODER_MIN_YPOS 40
#define EQODER_MIN_WIDTH g_minGuiSize_x - 2*EQODER_MIN_XPOS
#define EQODER_MIN_HEIGHT g_minGuiSize_y-60-EQODER_MIN_YPOS-100

//
#define ENVELOPE1_MIN_XPOS 10
#define ENVELOPE1_MIN_YPOS EQODER_MIN_YPOS+EQODER_MIN_HEIGHT
#define ENVELOPE1_MIN_WIDTH g_minGuiSize_x-2*ENVELOPE1_MIN_XPOS // 260 for ADSR //
#define ENVELOPE1_MIN_HEIGHT 85

// 
#define DISPLAY_MIN_XPOS g_minGuiSize_x-52
#define DISPLAY_MIN_YPOS 140
#define DISPLAY_MIN_WIDTH 42
#define DISPLAY_MIN_HEIGHT g_minGuiSize_y - 2*(DISPLAY_MIN_YPOS)-23

#define EQDISP_XPOS 5
#define EQDISP_YPOS 100
#define EQDISP_WIDTH g_minGuiSize_x-2*EQDISP_XPOS - 57
#define EQDISP_HEIGHT EQODER_MIN_HEIGHT-EQDISP_YPOS-GLOBAL_MIN_DISTANCE

