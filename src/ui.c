#include <Arduino.h>
#include "ui.h"
#include "event.h"

int saved_scroll_y = 0;
int saved_focused_index = -1;

static lv_obj_t *setting_touch_debug_dot = NULL;
static lv_obj_t *setting_touch_debug_label = NULL;
static lv_timer_t *setting_touch_debug_timer = NULL;

typedef enum
{
    TOUCH_CAL_STEP_INTRO = 0,
    TOUCH_CAL_STEP_0,
    TOUCH_CAL_STEP_1,
    TOUCH_CAL_STEP_2,
    TOUCH_CAL_STEP_3,
    TOUCH_CAL_STEP_4,
    TOUCH_CAL_STEP_5,
    TOUCH_CAL_STEP_6,
    TOUCH_CAL_STEP_7,
    TOUCH_CAL_STEP_8,
    TOUCH_CAL_STEP_9,
    TOUCH_CAL_STEP_10,
    TOUCH_CAL_STEP_11,
    TOUCH_CAL_STEP_12,
    TOUCH_CAL_STEP_13,
    TOUCH_CAL_STEP_14,
    TOUCH_CAL_STEP_15,
    TOUCH_CAL_STEP_16,
    TOUCH_CAL_STEP_17,
    TOUCH_CAL_STEP_18,
    TOUCH_CAL_STEP_19,
    TOUCH_CAL_STEP_20,
    TOUCH_CAL_STEP_21,
    TOUCH_CAL_STEP_22,
    TOUCH_CAL_STEP_23,
    TOUCH_CAL_STEP_24,
    TOUCH_CAL_STEP_VERIFY,
} touch_calibration_step_t;

static const touch_calibration_step_t kTouchCalibrationCaptureSteps[] = {
    TOUCH_CAL_STEP_0, TOUCH_CAL_STEP_1, TOUCH_CAL_STEP_2, TOUCH_CAL_STEP_3, TOUCH_CAL_STEP_4,
    TOUCH_CAL_STEP_5, TOUCH_CAL_STEP_6, TOUCH_CAL_STEP_7, TOUCH_CAL_STEP_8, TOUCH_CAL_STEP_9,
    TOUCH_CAL_STEP_10, TOUCH_CAL_STEP_11, TOUCH_CAL_STEP_12, TOUCH_CAL_STEP_13, TOUCH_CAL_STEP_14,
    TOUCH_CAL_STEP_15, TOUCH_CAL_STEP_16, TOUCH_CAL_STEP_17, TOUCH_CAL_STEP_18, TOUCH_CAL_STEP_19,
    TOUCH_CAL_STEP_20, TOUCH_CAL_STEP_21, TOUCH_CAL_STEP_22, TOUCH_CAL_STEP_23, TOUCH_CAL_STEP_24,
};

static const char *const kTouchCalibrationStepLabels[25] = {
    "P0(0,0)", "P1(1,0)", "P2(2,0)", "P3(3,0)", "P4(4,0)",
    "P5(0,1)", "P6(1,1)", "P7(2,1)", "P8(3,1)", "P9(4,1)",
    "P10(0,2)", "P11(1,2)", "P12(2,2)", "P13(3,2)", "P14(4,2)",
    "P15(0,3)", "P16(1,3)", "P17(2,3)", "P18(3,3)", "P19(4,3)",
    "P20(0,4)", "P21(1,4)", "P22(2,4)", "P23(3,4)", "P24(4,4)",
};

static touch_calibration_step_t g_touch_calibration_step = TOUCH_CAL_STEP_INTRO;
static lv_timer_t *touch_calibration_timer = NULL;
static lv_obj_t *touch_calibration_target = NULL;
static lv_obj_t *touch_calibration_hint_label = NULL;
static lv_obj_t *touch_calibration_status_label = NULL;
static lv_obj_t *touch_calibration_debug_label = NULL;
static lv_obj_t *touch_calibration_dot = NULL;
static lv_obj_t *touch_calibration_primary_btn = NULL;
static lv_obj_t *touch_calibration_secondary_btn = NULL;
static lv_obj_t *touch_calibration_tertiary_btn = NULL;
static lv_obj_t *touch_calibration_quaternary_btn = NULL;
static lv_obj_t *touch_calibration_quinary_btn = NULL;
static bool touch_calibration_waiting_release = false;
static uint32_t touch_calibration_step_enter_ms = 0;
static int32_t touch_calibration_sum_x[25];
static int32_t touch_calibration_sum_y[25];
static uint8_t touch_calibration_sample_count[25];
static lv_coord_t touch_calibration_screen_w = 240;
static lv_coord_t touch_calibration_screen_h = 284;
static const uint8_t kTouchCalibrationPointCount = 25;
static const uint8_t kTouchCalibrationSamplesPerPoint = 6;
static const uint32_t kTouchCalibrationStepDelayMs = 350;

static int32_t get_saved_slider_value(void) {
    if (saved_focused_index >= 1 && saved_focused_index <= 11) {
        return 12 - saved_focused_index;
    }
    return 11;
}

// 第二阶段恢复：在焦点设置后再次确保滚动位置
static void restore_scroll_phase2(void *param) {
    (void)param;
    if (panel) {
        lv_obj_update_layout(panel);
        lv_obj_scroll_to_y(panel, saved_scroll_y, LV_ANIM_OFF);
        // 恢复滚动方向和滚动到焦点功能
        lv_obj_set_scroll_dir(panel, LV_DIR_VER);
        lv_obj_add_flag(panel, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    }
}

// 延迟恢复主屏幕状态（在下丢帧执行）
static void delayed_restore_state(void *param) {
    (void)param; // 未使用参数
    if (slider) {
        lv_slider_set_value(slider, get_saved_slider_value(), LV_ANIM_OFF);
    }

    // 先恢复滚动位置
    if (panel) {
        lv_obj_update_layout(panel);
        lv_obj_scroll_to_y(panel, saved_scroll_y, LV_ANIM_OFF);
    }

    // 设置焦点
    if (group && saved_focused_index >= 1 && saved_focused_index <= 11) {
        lv_obj_t *btn = NULL;
        switch (saved_focused_index) {
            case 1: btn = btn1; break;
            case 2: btn = btn2; break;
            case 3: btn = btn3; break;
            case 4: btn = btn4; break;
            case 5: btn = btn5; break;
            case 6: btn = btn6; break;
            case 7: btn = btn7; break;
            case 8: btn = btn8; break;
            case 9: btn = btn9; break;
            case 10: btn = btn10; break;
            case 11: btn = btn11; break;
        }
        if (btn) {
            // 临时禁用 panel 滚动和滚动到焦点功能，防止焦点设置时自动滚动
            if (panel) {
                lv_obj_set_scroll_dir(panel, LV_DIR_NONE);
                lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
            }
            lv_group_focus_obj(btn);
        }
    }

    // 再次异步调用确保滚动位置（在焦点设置生效后）
    lv_async_call(restore_scroll_phase2, NULL);
}

// 定义线条的起始和结束位置
#define LINE_X1 40
#define LINE_X2 40
#define LINE_Y1 45
#define LINE_Y2 115

// 创建样式对象
lv_style_t style_rect;

extern double v, a, w;

ui_page_id_t g_current_page = UI_PAGE_BOOT;

typedef struct
{
    uint32_t bg;
    uint32_t panel;
    uint32_t card;
    uint32_t primary;
    uint32_t primary_soft;
    uint32_t secondary;
    uint32_t secondary_soft;
    uint32_t accent;
    uint32_t accent_soft;
    uint32_t border;
    uint32_t text;
    uint32_t muted;
    uint32_t text_warm;
    uint32_t text_cool;
    uint32_t text_blue;
    uint32_t text_green;
    uint32_t text_red;
} ui_palette_t;

static const ui_palette_t kLightPalette = {
    .bg = 0xFFF7E8,
    .panel = 0xFFFDF6,
    .card = 0xFFFFFF,
    .primary = 0xFF7A00,
    .primary_soft = 0xFFD39A,
    .secondary = 0x16C47F,
    .secondary_soft = 0xB8F5D1,
    .accent = 0x2F89FC,
    .accent_soft = 0xB9DDFF,
    .border = 0xFFB347,
    .text = 0x2D2A26,
    .muted = 0x7B6F66,
    .text_warm = 0xA84B00,
    .text_cool = 0x006D77,
    .text_blue = 0x1D4ED8,
    .text_green = 0x17803D,
    .text_red = 0xC62828,
};

static const ui_palette_t kDarkPalette = {
    .bg = 0x121212,
    .panel = 0x1B1B1B,
    .card = 0x232323,
    .primary = 0xFF9A3D,
    .primary_soft = 0x5A3A18,
    .secondary = 0x35D39A,
    .secondary_soft = 0x1E4A39,
    .accent = 0x63A8FF,
    .accent_soft = 0x1F3554,
    .border = 0x4C3B2A,
    .text = 0xF5F1EB,
    .muted = 0xB5AEA5,
    .text_warm = 0xFFB566,
    .text_cool = 0x72D2DF,
    .text_blue = 0x7DB2FF,
    .text_green = 0x52D88F,
    .text_red = 0xFF8A80,
};

static const ui_palette_t *ui_get_palette(void)
{
    return g_ui_theme_mode == UI_THEME_DARK ? &kDarkPalette : &kLightPalette;
}

#define UI_COLOR_BG (ui_get_palette()->bg)
#define UI_COLOR_PANEL (ui_get_palette()->panel)
#define UI_COLOR_CARD (ui_get_palette()->card)
#define UI_COLOR_PRIMARY (ui_get_palette()->primary)
#define UI_COLOR_PRIMARY_SOFT (ui_get_palette()->primary_soft)
#define UI_COLOR_SECONDARY (ui_get_palette()->secondary)
#define UI_COLOR_SECONDARY_SOFT (ui_get_palette()->secondary_soft)
#define UI_COLOR_ACCENT (ui_get_palette()->accent)
#define UI_COLOR_ACCENT_SOFT (ui_get_palette()->accent_soft)
#define UI_COLOR_BORDER (ui_get_palette()->border)
#define UI_COLOR_TEXT (ui_get_palette()->text)
#define UI_COLOR_MUTED (ui_get_palette()->muted)
#define UI_COLOR_TEXT_WARM (ui_get_palette()->text_warm)
#define UI_COLOR_TEXT_COOL (ui_get_palette()->text_cool)
#define UI_COLOR_TEXT_BLUE (ui_get_palette()->text_blue)
#define UI_COLOR_TEXT_GREEN (ui_get_palette()->text_green)
#define UI_COLOR_TEXT_RED (ui_get_palette()->text_red)

void ui_set_current_page(ui_page_id_t page)
{
    g_current_page = page;
}

static void apply_page_bg(void)
{
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(UI_COLOR_BG), 0);
    lv_obj_set_style_text_color(lv_scr_act(), lv_color_hex(UI_COLOR_TEXT), 0);
}

static void style_input_surface(lv_obj_t *obj)
{
    if (!obj) return;
    lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_color(obj, lv_color_hex(UI_COLOR_TEXT), LV_PART_ITEMS);
    lv_obj_set_style_border_color(obj, lv_color_hex(UI_COLOR_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 12, LV_PART_MAIN);
}

static void style_chart_surface(lv_obj_t *obj)
{
    if (!obj) return;
    lv_obj_set_style_bg_color(obj, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_line_color(obj, lv_color_hex(UI_COLOR_PRIMARY_SOFT), LV_PART_MAIN);
    lv_obj_set_style_border_color(obj, lv_color_hex(UI_COLOR_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(obj, 18, LV_PART_MAIN);
}

static void style_menu_icon(lv_obj_t *obj, uint32_t color)
{
    if (!obj) return;
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(obj, 0, LV_PART_MAIN);
    lv_obj_set_style_img_recolor(obj, lv_color_hex(color), LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(obj, LV_OPA_90, LV_PART_MAIN);
}

static lv_obj_t *create_menu_icon_badge(lv_obj_t *parent, uint32_t color, lv_coord_t x_ofs)
{
    (void)x_ofs;
    lv_obj_t *badge = lv_obj_create(parent);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, 52, 52);
    lv_obj_set_style_radius(badge, 18, LV_PART_MAIN);
    lv_obj_set_style_bg_color(badge, lv_color_hex(color), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(badge, LV_OPA_20, LV_PART_MAIN);
    lv_obj_set_style_border_width(badge, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(badge, 0, LV_PART_MAIN);
    lv_obj_align(badge, LV_ALIGN_RIGHT_MID, -6, 0);
    lv_obj_move_background(badge);
    return badge;
}

static void style_menu_entry_label(lv_obj_t *label, const lv_font_t *font)
{
    if (!label) return;
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 4, 0);
    lv_obj_set_width(label, 114);
    lv_obj_set_style_text_color(label, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
}

void ui_rebuild_current_page(void)
{
    switch (g_current_page)
    {
    case UI_PAGE_MAIN:
        ui_Screen1_screen_init();
        break;
    case UI_PAGE_PINMAP:
        pinmap_init();
        break;
    case UI_PAGE_POWER:
        power_init();
        break;
    case UI_PAGE_PWM:
        pwm_init();
        break;
    case UI_PAGE_UART:
        uarthelper_init();
        break;
    case UI_PAGE_I2C:
        i2c_init();
        break;
    case UI_PAGE_VOLTMETER:
        voltmeter_init();
        break;
    case UI_PAGE_DSO:
        DSO_init();
        break;
    case UI_PAGE_WIRELESS_UART:
        wirelessuart_init();
        break;
    case UI_PAGE_FRECOUNT:
        FREcount_init();
        break;
    case UI_PAGE_README:
        readme_init();
        break;
    case UI_PAGE_SETTING:
        setting_init();
        break;
    case UI_PAGE_TOUCH_CALIBRATION:
        touch_calibration_init();
        break;
    case UI_PAGE_BOOT:
    default:
        ui_Screen1_screen_init();
        break;
    }
}

void ui_apply_theme_mode(ui_theme_mode_t mode, bool persist)
{
    g_ui_theme_mode = mode == UI_THEME_DARK ? UI_THEME_DARK : UI_THEME_LIGHT;

    lv_theme_default_init(
        NULL,
        lv_color_hex(UI_COLOR_BG),
        lv_color_hex(UI_COLOR_PRIMARY),
        g_ui_theme_mode == UI_THEME_DARK,
        NULL);

    if (persist)
    {
        persist_theme_mode((int)g_ui_theme_mode);
    }

    if (g_current_page != UI_PAGE_BOOT)
    {
        ui_rebuild_current_page();
    }
}

void update_label_timer1(lv_timer_t *timer)
{
    lv_obj_t *voltmeter_label1 = (lv_obj_t *)timer->user_data;
    lv_label_set_text(voltmeter_label1, voltageStr);
}

void update_label_timer2(lv_timer_t *timer)
{
    lv_obj_t *voltmeter_label2 = (lv_obj_t *)timer->user_data;
    lv_label_set_text(voltmeter_label2, currentStr);
}

void update_label_timer3(lv_timer_t *timer)
{
    lv_obj_t *voltmeter_label3 = (lv_obj_t *)timer->user_data;
    lv_label_set_text(voltmeter_label3, powerStr);
}

void DSO_update_maxValue_timer(lv_timer_t *timer)
{
    lv_obj_t *maxValue_label = (lv_obj_t *)timer->user_data;
    lv_label_set_text(maxValue_label, maxValueStr);
}

void DSO_update_minValue_timer(lv_timer_t *timer)
{
    lv_obj_t *minValue_label = (lv_obj_t *)timer->user_data;
    lv_label_set_text(minValue_label, minValueStr);
}

void DSO_update_peakToPeakValue_timer(lv_timer_t *timer)
{
    lv_obj_t *peakToPeakValue_label = (lv_obj_t *)timer->user_data;
    lv_label_set_text(peakToPeakValue_label, peakToPeakValueStr);
}

void update_slider_timer(lv_timer_t *timer)
{
    static float filtered_bat = 0.0f;
    static float prev_filtered_bat = 0.0f;
    static float rise_ref_voltage = 0.0f;
    static uint32_t rise_ref_time = 0;
    static bool battery_filter_initialized = false;
    static bool charge_icon_active = false;
    static uint32_t charge_icon_hold_until = 0;

    if (lv_obj_has_state(btn1, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 11, LV_ANIM_OFF);
    }
    if (lv_obj_has_state(btn2, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 10, LV_ANIM_OFF);
    }
    if (lv_obj_has_state(btn3, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 9, LV_ANIM_OFF);
    }
    if (lv_obj_has_state(btn4, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 8, LV_ANIM_OFF);
    }
    if (lv_obj_has_state(btn5, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 7, LV_ANIM_OFF);
    }
    if (lv_obj_has_state(btn6, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 6, LV_ANIM_OFF);
    }
    if (lv_obj_has_state(btn7, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 5, LV_ANIM_OFF);
    }
    if (lv_obj_has_state(btn8, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 4, LV_ANIM_OFF);
    }
    if (lv_obj_has_state(btn9, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 3, LV_ANIM_OFF);
    }
    if (lv_obj_has_state(btn10, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 2, LV_ANIM_OFF);
    }
    if (lv_obj_has_state(btn11, LV_STATE_FOCUSED))
    {
        lv_slider_set_value(slider, 1, LV_ANIM_OFF);
    }

    float volt = analogRead(2);
    float bat = volt / 4095 * 6.6f;

    if (!battery_filter_initialized)
    {
        filtered_bat = bat;
        prev_filtered_bat = bat;
        rise_ref_voltage = bat;
        rise_ref_time = millis();
        battery_filter_initialized = true;
    }
    else
    {
        filtered_bat = filtered_bat * 0.85f + bat * 0.15f;
    }

    uint32_t now = millis();
    float rise_delta = filtered_bat - prev_filtered_bat;
    prev_filtered_bat = filtered_bat;

    // 充电识别1：单周期快速升压触发
    if (rise_delta >= 0.010f) // 每 300ms 上升约 10mV
    {
        charge_icon_active = true;
        charge_icon_hold_until = now + 8000; // 至少保持 8 秒，避免闪烁
    }

    // 充电识别2：短时间累计升压触发（适合慢慢抬升的场景）
    if (filtered_bat < rise_ref_voltage - 0.01f)
    {
        rise_ref_voltage = filtered_bat;
        rise_ref_time = now;
    }
    if ((int32_t)(now - rise_ref_time) > 5000)
    {
        rise_ref_voltage = filtered_bat;
        rise_ref_time = now;
    }
    if ((filtered_bat - rise_ref_voltage) >= 0.08f)
    {
        charge_icon_active = true;
        charge_icon_hold_until = now + 12000; // 累计升压触发后保持更久
        rise_ref_voltage = filtered_bat;
        rise_ref_time = now;
    }

    if (filtered_bat >= 4.18f)
    {
        charge_icon_active = true;
    }

    // 过了保持时间后再按电压退出充电图标
    if (charge_icon_active && (int32_t)(now - charge_icon_hold_until) >= 0)
    {
        if (filtered_bat <= 4.05f)
        {
            charge_icon_active = false;
        }
    }

    // 10挡电量显示：根据电压映射到 0~10 格
    float level_voltage = filtered_bat;
    if (charge_icon_active && level_voltage > 4.20f)
    {
        level_voltage = 4.20f;
    }
    if (level_voltage < 3.30f)
    {
        level_voltage = 3.30f;
    }
    if (level_voltage > 4.20f)
    {
        level_voltage = 4.20f;
    }

    int level = (int)(((level_voltage - 3.30f) / (4.20f - 3.30f)) * 10.0f + 0.5f);
    if (level < 0)
    {
        level = 0;
    }
    if (level > 10)
    {
        level = 10;
    }

    if (charge_icon_active)
    {
        lv_obj_set_pos(bat_label, 250, 196);
        lv_label_set_text(bat_label, LV_SYMBOL_CHARGE);
    }
    else if (level >= 8)
    {
        lv_obj_set_pos(bat_label, 241, 196);
        lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_FULL);
    }
    else if (level >= 5)
    {
        lv_obj_set_pos(bat_label, 241, 196);
        lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_3);
    }
    else if (level >= 3)
    {
        lv_obj_set_pos(bat_label, 241, 196);
        lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_2);
    }
    else if (level >= 1)
    {
        lv_obj_set_pos(bat_label, 241, 196);
        lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_1);
    }
    else
    {
        lv_obj_set_pos(bat_label, 241, 196);
        lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_EMPTY);
    }

    snprintf(batStr, sizeof(batStr), "%.2fV", filtered_bat);
    lv_label_set_text(bat_voltage_label, batStr);
    lv_obj_align_to(bat_voltage_label, bat_label, LV_ALIGN_OUT_TOP_MID, 0, -2);
}

void FRE_label_update(lv_timer_t *timer)
{
    lv_label_set_text(FRE_label, freqencyStr);
}

void add_data(lv_timer_t *timer)
{

    lv_chart_set_next_value(volt_chart, timer->user_data, v);
    // lv_label_set_text(label1, voltageStr);
}

void add_data2(lv_timer_t *timer)
{

    lv_chart_set_next_value(cur_chart, timer->user_data, a * 100);
    // lv_label_set_text(label1, voltageStr);
}

// 创建启动动画
void create_boot_animation(void)
{
    apply_page_bg();
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    const lv_coord_t screen_w = lv_obj_get_width(lv_scr_act());
    const lv_coord_t screen_h = lv_obj_get_height(lv_scr_act());
    const lv_coord_t card_w = 220;
    const lv_coord_t card_h = 156;
    const lv_coord_t card_x = (screen_w - card_w) / 2;
    const lv_coord_t startup_group_h = 220;
    const lv_coord_t card_y = (screen_h - startup_group_h) / 2;
    const lv_coord_t line_y = card_y + card_h + 14;
    const lv_coord_t loading_y = line_y + 20;

    ui_Image1 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(ui_Image1, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(ui_Image1, card_w, card_h);
    lv_obj_set_pos(ui_Image1, card_x, card_y);
    lv_obj_set_style_radius(ui_Image1, 28, LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_Image1, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_color(ui_Image1, lv_color_hex(UI_COLOR_PRIMARY_SOFT), LV_PART_MAIN);
    lv_obj_set_style_border_width(ui_Image1, 3, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(ui_Image1, 0, LV_PART_MAIN);
    lv_obj_clear_flag(ui_Image1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(ui_Image1, LV_DIR_NONE);

    lv_obj_t *accent_bar = lv_obj_create(ui_Image1);
    lv_obj_remove_style_all(accent_bar);
    lv_obj_set_size(accent_bar, 146, 6);
    lv_obj_align(accent_bar, LV_ALIGN_TOP_MID, 0, 16);
    lv_obj_set_style_radius(accent_bar, 6, LV_PART_MAIN);
    lv_obj_set_style_bg_color(accent_bar, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(accent_bar, LV_OPA_100, LV_PART_MAIN);

    lv_obj_t *hero_header = lv_obj_create(ui_Image1);
    lv_obj_remove_style_all(hero_header);
    lv_obj_set_size(hero_header, 184, 56);
    lv_obj_align(hero_header, LV_ALIGN_TOP_MID, 0, 34);

    lv_obj_t *hero_badge = lv_obj_create(hero_header);
    lv_obj_remove_style_all(hero_badge);
    lv_obj_set_size(hero_badge, 58, 58);
    lv_obj_align(hero_badge, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_radius(hero_badge, 18, LV_PART_MAIN);
    lv_obj_set_style_bg_color(hero_badge, lv_color_hex(0x5B6CFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(hero_badge, LV_OPA_30, LV_PART_MAIN);

    lv_obj_t *hero_badge_icon = lv_img_create(hero_badge);
    lv_img_set_src(hero_badge_icon, &readme_png);
    lv_img_set_zoom(hero_badge_icon, 205);
    lv_obj_center(hero_badge_icon);
    style_menu_icon(hero_badge_icon, 0x5B6CFF);

    lv_obj_t *hero_title_wrap = lv_obj_create(hero_header);
    lv_obj_remove_style_all(hero_title_wrap);
    lv_obj_set_size(hero_title_wrap, 118, 42);
    lv_obj_align_to(hero_title_wrap, hero_badge, LV_ALIGN_OUT_RIGHT_MID, 12, 0);

    lv_obj_t *hero_title = lv_label_create(hero_title_wrap);
    lv_label_set_text(hero_title, "Exlink Tool");
    lv_obj_set_style_text_color(hero_title, lv_color_hex(UI_COLOR_TEXT_RED), 0);
    lv_obj_set_style_text_font(hero_title, &lv_font_montserrat_20, 0);
    lv_obj_align(hero_title, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *hero_sub = lv_label_create(hero_title_wrap);
    lv_label_set_text(hero_sub, "Bright Edition");
    lv_obj_set_style_text_color(hero_sub, lv_color_hex(UI_COLOR_TEXT_BLUE), 0);
    lv_obj_set_style_text_font(hero_sub, &lv_font_montserrat_14, 0);
    lv_obj_align(hero_sub, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t *boot_info = lv_obj_create(ui_Image1);
    lv_obj_remove_style(boot_info, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(boot_info, 184, 44);
    lv_obj_align(boot_info, LV_ALIGN_BOTTOM_MID, 0, -18);
    lv_obj_set_style_radius(boot_info, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(boot_info, lv_color_hex(UI_COLOR_BG), LV_PART_MAIN);
    lv_obj_set_style_border_color(boot_info, lv_color_hex(UI_COLOR_ACCENT_SOFT), LV_PART_MAIN);
    lv_obj_set_style_border_width(boot_info, 2, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(boot_info, 0, LV_PART_MAIN);
    lv_obj_clear_flag(boot_info, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(boot_info, LV_DIR_NONE);

    lv_obj_t *boot_info_text = lv_label_create(boot_info);
    lv_label_set_text(boot_info_text, "Portable bright toolkit\nfor Exlink 0603");
    lv_obj_set_style_text_color(boot_info_text, lv_color_hex(UI_COLOR_MUTED), 0);
    lv_obj_set_style_text_font(boot_info_text, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(boot_info_text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(boot_info_text);

    lv_obj_t *line = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(line);
    lv_obj_set_size(line, 156, 8);
    lv_obj_set_pos(line, (screen_w - 156) / 2, line_y);
    lv_obj_set_style_radius(line, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(line, lv_color_hex(UI_COLOR_PRIMARY_SOFT), LV_PART_MAIN);

    lv_obj_t *line_fill = lv_obj_create(line);
    lv_obj_remove_style_all(line_fill);
    lv_obj_set_size(line_fill, 52, 8);
    lv_obj_align(line_fill, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_radius(line_fill, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(line_fill, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_MAIN);

    ui_Image2 = lv_label_create(lv_scr_act());
    lv_label_set_text(ui_Image2, "Loading workspace...");
    lv_obj_set_style_text_color(ui_Image2, lv_color_hex(UI_COLOR_TEXT_COOL), 0);
    lv_obj_set_style_text_font(ui_Image2, &lv_font_montserrat_16, 0);
    lv_obj_clear_flag(ui_Image2, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align_to(ui_Image2, line, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    lv_obj_t *done_anchor = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(done_anchor);
    lv_obj_set_size(done_anchor, 1, 1);
    lv_obj_set_pos(done_anchor, 0, 0);

    lv_anim_t anim1;
    lv_anim_init(&anim1);
    lv_anim_set_exec_cb(&anim1, anim_cb2);
    lv_anim_set_var(&anim1, ui_Image1);
    lv_anim_set_time(&anim1, 450);
    lv_anim_set_values(&anim1, -180, card_y);
    lv_anim_set_path_cb(&anim1, lv_anim_path_overshoot);

    lv_anim_t anim2;
    lv_anim_init(&anim2);
    lv_anim_set_exec_cb(&anim2, anim_cb1);
    lv_anim_set_var(&anim2, accent_bar);
    lv_anim_set_time(&anim2, 380);
    lv_anim_set_values(&anim1, 220, 30);
    lv_anim_set_path_cb(&anim2, lv_anim_path_overshoot);

    lv_anim_t anim3;
    lv_anim_init(&anim3);
    lv_anim_set_exec_cb(&anim3, anim_cb1);
    lv_anim_set_var(&anim3, hero_badge);
    lv_anim_set_time(&anim3, 380);
    lv_anim_set_values(&anim3, -80, 0);
    lv_anim_set_path_cb(&anim3, lv_anim_path_overshoot);

    lv_anim_t anim4;
    lv_anim_init(&anim4);
    lv_anim_set_exec_cb(&anim4, anim_cb1);
    lv_anim_set_var(&anim4, hero_title_wrap);
    lv_anim_set_time(&anim4, 380);
    lv_anim_set_values(&anim4, 180, 64);
    lv_anim_set_path_cb(&anim4, lv_anim_path_overshoot);

    lv_anim_t anim6;
    lv_anim_init(&anim6);
    lv_anim_set_exec_cb(&anim6, anim_cb2);
    lv_anim_set_var(&anim6, boot_info);
    lv_anim_set_time(&anim6, 420);
    lv_anim_set_values(&anim6, 160, 94);
    lv_anim_set_path_cb(&anim6, lv_anim_path_overshoot);

    lv_anim_t anim7;
    lv_anim_init(&anim7);
    lv_anim_set_exec_cb(&anim7, anim_cb2);
    lv_anim_set_var(&anim7, line);
    lv_anim_set_time(&anim7, 360);
    lv_anim_set_values(&anim7, screen_h + 24, line_y);
    lv_anim_set_path_cb(&anim7, lv_anim_path_overshoot);

    lv_anim_t anim8;
    lv_anim_init(&anim8);
    lv_anim_set_exec_cb(&anim8, anim_cb2);
    lv_anim_set_var(&anim8, ui_Image2);
    lv_anim_set_time(&anim8, 360);
    lv_anim_set_values(&anim8, screen_h + 48, loading_y);
    lv_anim_set_path_cb(&anim8, lv_anim_path_overshoot);

    lv_anim_t anim9;
    lv_anim_init(&anim9);
    lv_anim_set_var(&anim9, done_anchor);
    lv_anim_set_ready_cb(&anim9, anim_end_callback);
    lv_anim_set_exec_cb(&anim9, anim_cb2);
    lv_anim_set_time(&anim9, 2000);
    lv_anim_set_values(&anim9, 0, 0);

    lv_anim_timeline_t *anim_timeline = lv_anim_timeline_create();
    lv_anim_timeline_add(anim_timeline, 0, &anim1);
    lv_anim_timeline_add(anim_timeline, 90, &anim2);
    lv_anim_timeline_add(anim_timeline, 130, &anim3);
    lv_anim_timeline_add(anim_timeline, 150, &anim4);
    lv_anim_timeline_add(anim_timeline, 180, &anim6);
    lv_anim_timeline_add(anim_timeline, 230, &anim7);
    lv_anim_timeline_add(anim_timeline, 270, &anim8);
    lv_anim_timeline_add(anim_timeline, 310, &anim9);
    lv_anim_timeline_start(anim_timeline);
}

void ui_Screen1_screen_init(void) // 创建主界面
{
    ui_set_current_page(UI_PAGE_MAIN);
    lv_obj_clean(lv_scr_act());
    set_swipe_back_enabled(true);
    apply_page_bg();
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

    panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(panel, 230, 240);
    lv_obj_set_pos(panel, 5, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(UI_COLOR_PANEL), 0);
    lv_obj_set_style_text_color(panel, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_anim_time(panel, 0, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_remove_style(panel, 0, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN); // 垂直排列子对象
    lv_obj_set_scroll_dir(panel, LV_DIR_VER);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_style_init(&style_rect);
    // 设置边框的颜色和粗细
    lv_style_set_border_color(&style_rect, lv_color_hex(UI_COLOR_BORDER));
    lv_style_set_border_width(&style_rect, 3);                      // 5 像素粗细
    lv_style_set_bg_color(&style_rect, lv_color_hex(UI_COLOR_CARD));
    lv_style_set_text_color(&style_rect, lv_color_hex(UI_COLOR_TEXT));
    lv_style_set_radius(&style_rect, 20);                           // 圆角半径

    // 设置聚焦状的边框
    static lv_style_t focused_style;
    lv_style_init(&focused_style);
    lv_style_set_border_color(&focused_style, lv_color_hex(UI_COLOR_PRIMARY));
    lv_style_set_border_width(&focused_style, 5);

    btn1 = lv_btn_create(panel);
    lv_obj_set_size(btn1, 215, 80);
    lv_obj_align(btn1, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn1, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn1, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn1, btn1_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn1_label = lv_label_create(btn1);
    lv_label_set_text(btn1_label, "Pin Map");
    style_menu_entry_label(btn1_label, &lv_font_montserrat_20);
    lv_obj_t *badge1 = create_menu_icon_badge(btn1, UI_COLOR_ACCENT, 126);
    lv_obj_t *img1 = lv_img_create(badge1);
    lv_img_set_src(img1, &pinmap_png);                       // Replace with your image variable or path
    lv_obj_set_size(img1, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_img_set_zoom(img1, 205);
    lv_obj_center(img1);
    style_menu_icon(img1, UI_COLOR_ACCENT);

    btn2 = lv_btn_create(panel);
    lv_obj_set_size(btn2, 215, 80);
    lv_obj_align(btn2, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn2, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn2, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn2, btn2_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn2_label = lv_label_create(btn2);
    lv_label_set_text(btn2_label, "DC POWER");
    style_menu_entry_label(btn2_label, &lv_font_montserrat_20);
    lv_obj_t *badge2 = create_menu_icon_badge(btn2, UI_COLOR_PRIMARY, 126);
    lv_obj_t *img2 = lv_img_create(badge2);
    lv_img_set_src(img2, &power_png);                        // Replace with your image variable or path
    lv_obj_set_size(img2, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_img_set_zoom(img2, 205);
    lv_obj_center(img2);
    style_menu_icon(img2, UI_COLOR_PRIMARY);

    btn3 = lv_btn_create(panel);
    lv_obj_set_size(btn3, 215, 80);
    lv_obj_align(btn3, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn3, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_event_cb(btn3, btn3_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_style(btn3, &focused_style, LV_STATE_FOCUSED);
    lv_obj_t *btn3_label = lv_label_create(btn3);
    lv_label_set_text(btn3_label, "PWM OUT");
    style_menu_entry_label(btn3_label, &lv_font_montserrat_20);
    lv_obj_t *badge3 = create_menu_icon_badge(btn3, 0xFF5C8A, 126);
    lv_obj_t *img3 = lv_img_create(badge3);
    lv_img_set_src(img3, &pwm_png);                          // Replace with your image variable or path
    lv_obj_set_size(img3, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_img_set_zoom(img3, 205);
    lv_obj_center(img3);
    style_menu_icon(img3, 0xFF5C8A);

    btn4 = lv_btn_create(panel);
    lv_obj_set_size(btn4, 215, 80);
    lv_obj_align(btn4, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn4, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn4, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn4, btn4_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn4_label = lv_label_create(btn4);
    lv_label_set_text(btn4_label, "UART HELPER");
    style_menu_entry_label(btn4_label, &lv_font_montserrat_16);
    lv_obj_t *badge4 = create_menu_icon_badge(btn4, 0x7C4DFF, 126);
    lv_obj_t *img4 = lv_img_create(badge4);
    lv_img_set_src(img4, &usarthelper_png);                  // Replace with your image variable or path
    lv_obj_set_size(img4, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_img_set_zoom(img4, 205);
    lv_obj_center(img4);
    style_menu_icon(img4, 0x7C4DFF);

    btn5 = lv_btn_create(panel);
    lv_obj_set_size(btn5, 215, 80);
    lv_obj_align(btn5, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn5, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn5, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn5, btn5_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn5_label = lv_label_create(btn5);
    lv_label_set_text(btn5_label, "I2C SCAN");
    style_menu_entry_label(btn5_label, &lv_font_montserrat_20);
    lv_obj_t *badge5 = create_menu_icon_badge(btn5, UI_COLOR_TEXT_COOL, 126);
    lv_obj_t *img5 = lv_img_create(badge5);
    lv_img_set_src(img5, &i2c_png);                          // Replace with your image variable or path
    lv_obj_set_size(img5, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_img_set_zoom(img5, 205);
    lv_obj_center(img5);
    style_menu_icon(img5, UI_COLOR_TEXT_COOL);

    btn6 = lv_btn_create(panel);
    lv_obj_set_size(btn6, 215, 80);
    lv_obj_align(btn6, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn6, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn6, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn6, btn6_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn6, btn6_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn6_label = lv_label_create(btn6);
    lv_label_set_text(btn6_label, "Voltmeter");
    style_menu_entry_label(btn6_label, &lv_font_montserrat_20);
    lv_obj_t *badge6 = create_menu_icon_badge(btn6, UI_COLOR_TEXT_GREEN, 126);
    lv_obj_t *img6 = lv_img_create(badge6);
    lv_img_set_src(img6, &voltmeter_png);                    // Replace with your image variable or path
    lv_obj_set_size(img6, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_img_set_zoom(img6, 205);
    lv_obj_center(img6);
    style_menu_icon(img6, UI_COLOR_TEXT_GREEN);

    btn7 = lv_btn_create(panel);
    lv_obj_set_size(btn7, 215, 80);
    lv_obj_align(btn7, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn7, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn7, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn7, btn7_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn7_label = lv_label_create(btn7);
    lv_label_set_text(btn7_label, "Simple DSO");
    style_menu_entry_label(btn7_label, &lv_font_montserrat_20);
    lv_obj_t *badge7 = create_menu_icon_badge(btn7, 0xE64980, 126);
    lv_obj_t *img7 = lv_img_create(badge7);
    lv_img_set_src(img7, &DSO_png);                          // Replace with your image variable or path
    lv_obj_set_size(img7, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_img_set_zoom(img7, 205);
    lv_obj_center(img7);
    style_menu_icon(img7, 0xE64980);

    btn8 = lv_btn_create(panel);
    lv_obj_set_size(btn8, 215, 80);
    lv_obj_align(btn8, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn8, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn8, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn8, btn8_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn8_label = lv_label_create(btn8);
    lv_label_set_text(btn8_label, "BLE UART");
    style_menu_entry_label(btn8_label, &lv_font_montserrat_20);
    lv_obj_t *badge8 = create_menu_icon_badge(btn8, 0x00A6A6, 126);
    lv_obj_t *img8 = lv_img_create(badge8);
    lv_img_set_src(img8, &wireless_png);                     // Replace with your image variable or path
    lv_obj_set_size(img8, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_img_set_zoom(img8, 205);
    lv_obj_center(img8);
    style_menu_icon(img8, 0x00A6A6);

    btn9 = lv_btn_create(panel);
    lv_obj_set_size(btn9, 215, 80);
    lv_obj_align(btn9, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn9, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn9, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn9, btn9_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn9_label = lv_label_create(btn9);
    lv_label_set_text(btn9_label, "FRE Count");
    style_menu_entry_label(btn9_label, &lv_font_montserrat_20);
    lv_obj_t *badge9 = create_menu_icon_badge(btn9, 0xF59E0B, 126);
    lv_obj_t *img9 = lv_img_create(badge9);
    lv_img_set_src(img9, &FREcounter_png);                   // Replace with your image variable or path
    lv_obj_set_size(img9, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_img_set_zoom(img9, 205);
    lv_obj_center(img9);
    style_menu_icon(img9, 0xF59E0B);

    btn10 = lv_btn_create(panel);
    lv_obj_set_size(btn10, 215, 80);
    lv_obj_align(btn10, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn10, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn10, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn10, btn10_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn10_label = lv_label_create(btn10);
    lv_label_set_text(btn10_label, "Device INFO");
    style_menu_entry_label(btn10_label, &lv_font_montserrat_20);
    lv_obj_t *badge10 = create_menu_icon_badge(btn10, 0x5B6CFF, 126);
    lv_obj_t *img10 = lv_img_create(badge10);
    lv_img_set_src(img10, &readme_png);                       // Replace with your image variable or path
    lv_obj_set_size(img10, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_img_set_zoom(img10, 205);
    lv_obj_center(img10);
    style_menu_icon(img10, 0x5B6CFF);

    btn11 = lv_btn_create(panel);
    lv_obj_set_size(btn11, 215, 80);
    lv_obj_align(btn11, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn11, &style_rect, 0);
    lv_obj_add_style(btn11, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn11, btn11_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn11_label = lv_label_create(btn11);
    lv_label_set_text(btn11_label, "Setting");
    style_menu_entry_label(btn11_label, &lv_font_montserrat_20);
    lv_obj_t *badge11 = create_menu_icon_badge(btn11, UI_COLOR_TEXT_RED, 126);
    lv_obj_t *btn11_icon = lv_img_create(badge11);
    lv_img_set_src(btn11_icon, &setting_png);
    lv_obj_set_size(btn11_icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_img_set_zoom(btn11_icon, 205);
    lv_obj_center(btn11_icon);
    style_menu_icon(btn11_icon, UI_COLOR_TEXT_RED);

    lv_style_init(&style_rect);
    // 设置边框的颜色和粗细
    lv_style_set_border_color(&style_rect, lv_color_hex(UI_COLOR_BORDER));
    lv_style_set_border_width(&style_rect, 3);                      // 5 像素粗细
    lv_style_set_bg_color(&style_rect, lv_color_hex(UI_COLOR_CARD));
    lv_style_set_radius(&style_rect, 20);                           // 圆角半径

    slider = lv_slider_create(lv_scr_act());
    lv_obj_set_size(slider, 15, 90);
    lv_obj_set_pos(slider, 250, 30);
    lv_slider_set_range(slider, 1, 11);
    lv_slider_set_value(slider, get_saved_slider_value(), LV_ANIM_OFF);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(slider, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider, lv_color_hex(UI_COLOR_PRIMARY_SOFT), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(UI_COLOR_SECONDARY_SOFT), LV_PART_INDICATOR);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    slider_update_timer = lv_timer_create(update_slider_timer, 300, NULL);

    /*
        lv_obj_t *bat_volt_label = lv_label_create(lv_scr_act());
        lv_label_set_text(bat_volt_label, "100");
        lv_obj_set_pos(bat_volt_label, 251, 220);
        lv_obj_set_style_text_color(bat_volt_label, lv_color_hex(UI_COLOR_TEXT), 0);
        lv_obj_set_style_text_font(bat_volt_label, &lv_font_montserrat_10, 0);

        lv_obj_t *bat_volt_label2 = lv_label_create(lv_scr_act());
        lv_obj_set_pos(bat_volt_label2, 275, 220);
        lv_label_set_text(bat_volt_label2, "%");
        lv_obj_set_style_text_color(bat_volt_label2, lv_color_hex(UI_COLOR_TEXT), 0);
        lv_obj_set_style_text_font(bat_volt_label2, &lv_font_montserrat_14, 0);
       */
    bat_label = lv_label_create(lv_scr_act());
    lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_EMPTY);
    lv_obj_set_pos(bat_label, 241, 196);
    lv_obj_set_style_text_color(bat_label, lv_color_hex(0x32CD32), 0);
    lv_obj_set_style_text_font(bat_label, &lv_font_montserrat_26, 0);

    bat_voltage_label = lv_label_create(lv_scr_act());
    lv_label_set_text(bat_voltage_label, "0.00V");
    lv_obj_set_style_text_color(bat_voltage_label, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(bat_voltage_label, &lv_font_montserrat_14, 0);
    lv_obj_align_to(bat_voltage_label, bat_label, LV_ALIGN_OUT_TOP_MID, 0, -2);

    group = lv_group_create();
    lv_indev_set_group(indev_keypad, group);
    lv_group_add_obj(group, btn1);
    lv_group_add_obj(group, btn2);
    lv_group_add_obj(group, btn3);
    lv_group_add_obj(group, btn4);
    lv_group_add_obj(group, btn5);
    lv_group_add_obj(group, btn6);
    lv_group_add_obj(group, btn7);
    lv_group_add_obj(group, btn8);
    lv_group_add_obj(group, btn9);
    lv_group_add_obj(group, btn10);
    lv_group_add_obj(group, btn11);

    // 使用异步调用延迟恢复状，确保界面完全创建
    configure_swipe_back_for_current_screen(true);
    lv_async_call(delayed_restore_state, NULL);
}

void pinmap_init(void)
{

    ui_set_current_page(UI_PAGE_PINMAP);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *name_label1 = lv_label_create(lv_scr_act());         // 将文本标签添加到圆角矩形中
    lv_label_set_text(name_label1, "#A84B00 ROW1:MCU and power#"); // 设置文本内容
    lv_obj_set_style_text_font(name_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(name_label1, 10, 26);
    lv_label_set_recolor(name_label1, true);

    lv_obj_t *name_label2 = lv_label_create(lv_scr_act());           // 将文本标签添加到圆角矩形中
    lv_label_set_text(name_label2, "#006D77 ROW2:DLA and DAPlink#"); // 设置文本内容
    lv_obj_set_style_text_font(name_label2, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(name_label2, 10, 60);
    lv_label_set_recolor(name_label2, true);

    lv_obj_t *rounded_rect1 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect1, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect1, 21, 65);
    lv_obj_set_pos(rounded_rect1, 6, 110);
    lv_obj_set_style_radius(rounded_rect1, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect1, lv_color_hex(0x696969), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect1_label1 = lv_label_create(rounded_rect1);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label1, "GD");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect1_label1, LV_ALIGN_TOP_MID, 0, 0);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect1_label2 = lv_label_create(rounded_rect1); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label2, "1");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect1_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect2 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect2, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect2, 21, 65);
    lv_obj_set_pos(rounded_rect2, 27, 110);
    lv_obj_set_style_radius(rounded_rect2, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect2, lv_color_hex(0xA52A2A), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect2_label1 = lv_label_create(rounded_rect2);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label1, "IO");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect2_label1, LV_ALIGN_TOP_MID, 0, 0);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect2_label2 = lv_label_create(rounded_rect2); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label2, "2");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect2_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect3 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect3, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect3, 21, 65);
    lv_obj_set_pos(rounded_rect3, 48, 110);
    lv_obj_set_style_radius(rounded_rect3, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect3, lv_color_hex(0xFF0000), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect3_label1 = lv_label_create(rounded_rect3);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect3_label1, "3V");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect3_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect3_label1, LV_ALIGN_TOP_MID, 0, 0);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect3_label2 = lv_label_create(rounded_rect3); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect3_label2, "3");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect3_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect3_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect4 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect4, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect4, 21, 65);
    lv_obj_set_pos(rounded_rect4, 69, 110);
    lv_obj_set_style_radius(rounded_rect4, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect4, lv_color_hex(0xFF0000), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect4_label1 = lv_label_create(rounded_rect4);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect4_label1, "5V");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect4_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect4_label1, LV_ALIGN_TOP_MID, 0, 0);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect4_label2 = lv_label_create(rounded_rect4); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect4_label2, "4");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect4_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect4_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect5 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect5, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect5, 21, 65);
    lv_obj_set_pos(rounded_rect5, 90, 110);
    lv_obj_set_style_radius(rounded_rect5, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect5, lv_color_hex(0x008000), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect5_label1 = lv_label_create(rounded_rect5);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect5_label1, "CO");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect5_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect5_label1, LV_ALIGN_TOP_MID, 0, 0);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect5_label2 = lv_label_create(rounded_rect5); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect5_label2, "5");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect5_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect5_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect6 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect6, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect6, 21, 65);
    lv_obj_set_pos(rounded_rect6, 111, 110);
    lv_obj_set_style_radius(rounded_rect6, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect6, lv_color_hex(0x008000), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect6_label1 = lv_label_create(rounded_rect6);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect6_label1, "PW");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect6_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect6_label1, LV_ALIGN_TOP_MID, 0, 0);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect6_label2 = lv_label_create(rounded_rect6); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect6_label2, "6");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect6_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect6_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect7 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect7, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect7, 21, 65);
    lv_obj_set_pos(rounded_rect7, 132, 110);
    lv_obj_set_style_radius(rounded_rect7, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect7, lv_color_hex(0x0000FF), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect7_label1 = lv_label_create(rounded_rect7);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect7_label1, "SL");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect7_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect7_label1, LV_ALIGN_TOP_MID, 0, 0);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect7_label2 = lv_label_create(rounded_rect7); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect7_label2, "7");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect7_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect7_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect8 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect8, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect8, 21, 65);
    lv_obj_set_pos(rounded_rect8, 153, 110);
    lv_obj_set_style_radius(rounded_rect8, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect8, lv_color_hex(0x0000FF), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect8_label1 = lv_label_create(rounded_rect8);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect8_label1, "SA");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect8_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect8_label1, LV_ALIGN_TOP_MID, 0, 0);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect8_label2 = lv_label_create(rounded_rect8); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect8_label2, "8");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect8_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect8_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect9 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect9, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect9, 21, 65);
    lv_obj_set_pos(rounded_rect9, 174, 110);
    lv_obj_set_style_radius(rounded_rect9, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect9, lv_color_hex(0xC71585), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect9_label1 = lv_label_create(rounded_rect9);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect9_label1, "DI");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect9_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect9_label1, LV_ALIGN_TOP_MID, 0, 0);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect9_label2 = lv_label_create(rounded_rect9); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect9_label2, "9");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect9_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect9_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect10 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect10, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect10, 21, 65);
    lv_obj_set_pos(rounded_rect10, 195, 110);
    lv_obj_set_style_radius(rounded_rect10, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect10, lv_color_hex(0xC71585), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect10_label1 = lv_label_create(rounded_rect10);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect10_label1, "CL");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect10_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect10_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect10_label2 = lv_label_create(rounded_rect10); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect10_label2, "10");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect10_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect10_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect11 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect11, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect11, 21, 65);
    lv_obj_set_pos(rounded_rect11, 216, 110);
    lv_obj_set_style_radius(rounded_rect11, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect11, lv_color_hex(0xC71585), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect11_label1 = lv_label_create(rounded_rect11);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect11_label1, "RS");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect11_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect11_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect11_label2 = lv_label_create(rounded_rect11); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect11_label2, "11");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect11_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect11_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect12 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect12, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect12, 21, 65);
    lv_obj_set_pos(rounded_rect12, 237, 110);
    lv_obj_set_style_radius(rounded_rect12, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect12, lv_color_hex(0xFF8C00), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect12_label1 = lv_label_create(rounded_rect12);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect12_label1, "RX");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect12_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect12_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect12_label2 = lv_label_create(rounded_rect12); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect12_label2, "12");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect12_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect12_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect13 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect13, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect13, 21, 65);
    lv_obj_set_pos(rounded_rect13, 258, 110);
    lv_obj_set_style_radius(rounded_rect13, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect13, lv_color_hex(0xFF8C00), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect13_label1 = lv_label_create(rounded_rect13);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect13_label1, "TX");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect13_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect13_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect13_label2 = lv_label_create(rounded_rect13); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect13_label2, "13");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect13_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect13_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect14 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect14, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect14, 21, 65);
    lv_obj_set_pos(rounded_rect14, 6, 175);
    lv_obj_set_style_radius(rounded_rect14, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect14, lv_color_hex(0x696969), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect14_label1 = lv_label_create(rounded_rect14);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect14_label1, "GD");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect14_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect14_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect14_label2 = lv_label_create(rounded_rect14); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect14_label2, "14");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect14_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect14_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect15 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect15, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect15, 21, 65);
    lv_obj_set_pos(rounded_rect15, 27, 175);
    lv_obj_set_style_radius(rounded_rect15, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect15, lv_color_hex(0x20B2AA), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect15_label1 = lv_label_create(rounded_rect15);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect15_label1, "C0");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect15_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect15_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect15_label2 = lv_label_create(rounded_rect15); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect15_label2, "15");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect15_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect15_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect16 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect16, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect16, 21, 65);
    lv_obj_set_pos(rounded_rect16, 48, 175);
    lv_obj_set_style_radius(rounded_rect16, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect16, lv_color_hex(0x20B2AA), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect16_label1 = lv_label_create(rounded_rect16);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect16_label1, "C1");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect16_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect16_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect16_label2 = lv_label_create(rounded_rect16); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect16_label2, "16");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect16_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect16_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect17 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect17, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect17, 21, 65);
    lv_obj_set_pos(rounded_rect17, 69, 175);
    lv_obj_set_style_radius(rounded_rect17, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect17, lv_color_hex(0x20B2AA), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect17_label1 = lv_label_create(rounded_rect17);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect17_label1, "C2");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect17_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect17_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect17_label2 = lv_label_create(rounded_rect17); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect17_label2, "17");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect17_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect17_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect18 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect18, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect18, 21, 65);
    lv_obj_set_pos(rounded_rect18, 90, 175);
    lv_obj_set_style_radius(rounded_rect18, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect18, lv_color_hex(0x20B2AA), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect18_label1 = lv_label_create(rounded_rect18);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect18_label1, "C3");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect18_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect18_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect18_label2 = lv_label_create(rounded_rect18); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect18_label2, "18");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect18_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect18_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect19 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect19, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect19, 21, 65);
    lv_obj_set_pos(rounded_rect19, 111, 175);
    lv_obj_set_style_radius(rounded_rect19, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect19, lv_color_hex(0x20B2AA), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect19_label1 = lv_label_create(rounded_rect19);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect19_label1, "C4");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect19_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect19_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect19_label2 = lv_label_create(rounded_rect19); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect19_label2, "19");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect19_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect19_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect20 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect20, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect20, 21, 65);
    lv_obj_set_pos(rounded_rect20, 132, 175);
    lv_obj_set_style_radius(rounded_rect20, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect20, lv_color_hex(0x20B2AA), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect20_label1 = lv_label_create(rounded_rect20);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect20_label1, "C5");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect20_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect20_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect20_label2 = lv_label_create(rounded_rect20); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect20_label2, "20");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect20_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect20_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect21 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect21, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect21, 21, 65);
    lv_obj_set_pos(rounded_rect21, 153, 175);
    lv_obj_set_style_radius(rounded_rect21, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect21, lv_color_hex(0x20B2AA), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect21_label1 = lv_label_create(rounded_rect21);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect21_label1, "C6");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect21_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect21_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect21_label2 = lv_label_create(rounded_rect21); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect21_label2, "21");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect21_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect21_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect22 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect22, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect22, 21, 65);
    lv_obj_set_pos(rounded_rect22, 174, 175);
    lv_obj_set_style_radius(rounded_rect22, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect22, lv_color_hex(0x20B2AA), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect22_label1 = lv_label_create(rounded_rect22);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect22_label1, "C7");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect22_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect22_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect22_label2 = lv_label_create(rounded_rect22); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect22_label2, "22");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect22_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect22_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect23 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect23, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect23, 21, 65);
    lv_obj_set_pos(rounded_rect23, 195, 175);
    lv_obj_set_style_radius(rounded_rect23, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect23, lv_color_hex(0x006400), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect23_label1 = lv_label_create(rounded_rect23);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect23_label1, "DI");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect23_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect23_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect23_label2 = lv_label_create(rounded_rect23); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect23_label2, "23");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect23_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect23_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect24 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect24, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect24, 21, 65);
    lv_obj_set_pos(rounded_rect24, 216, 175);
    lv_obj_set_style_radius(rounded_rect24, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect24, lv_color_hex(0x006400), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect24_label1 = lv_label_create(rounded_rect24);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect24_label1, "CL");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect24_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect24_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect24_label2 = lv_label_create(rounded_rect24); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect24_label2, "24");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect24_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect24_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect25 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect25, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect25, 21, 65);
    lv_obj_set_pos(rounded_rect25, 257, 175);
    lv_obj_set_style_radius(rounded_rect25, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect25, lv_color_hex(0xFF1493), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect25_label1 = lv_label_create(rounded_rect25);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect25_label1, "RX");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect25_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect25_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect25_label2 = lv_label_create(rounded_rect25); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect25_label2, "25");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect25_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect25_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形

    lv_obj_t *rounded_rect26 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect26, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect26, 21, 65);
    lv_obj_set_pos(rounded_rect26, 258, 175);
    lv_obj_set_style_radius(rounded_rect26, 5, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rounded_rect26, lv_color_hex(0xFF1493), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect26_label1 = lv_label_create(rounded_rect26);               // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect26_label1, "TX");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect26_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect26_label1, LV_ALIGN_TOP_MID, 0, 0);       // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect26_label2 = lv_label_create(rounded_rect26); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect26_label2, "26");                    // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect26_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect26_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
}

void power_init(void)
{
    ui_set_current_page(UI_PAGE_POWER);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();

    ina266_flag = 1;
    digitalWrite(1, LOW);

    lv_obj_t *volt = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(volt, lv_color_hex(UI_COLOR_TEXT_RED), 0);
    lv_obj_set_style_text_font(volt, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(volt, 20, 10);

    lv_obj_t *cur = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(cur, lv_color_hex(UI_COLOR_TEXT_GREEN), 0);
    lv_obj_set_style_text_font(cur, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(cur, 20, 35);

    lv_obj_t *watt = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(watt, lv_color_hex(UI_COLOR_TEXT_COOL), 0);
    lv_obj_set_style_text_font(watt, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(watt, 20, 60);

    updatelabel_timer1 = lv_timer_create(update_label_timer1, 100, volt);
    updatelabel_timer2 = lv_timer_create(update_label_timer2, 100, cur);
    updatelabel_timer3 = lv_timer_create(update_label_timer3, 100, watt);

    lv_obj_t *V = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(V, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(V, 106, 10);
    lv_label_set_text(V, "#C62828 V#"); // 设置文本内容
    lv_label_set_recolor(V, true);

    lv_obj_t *A = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(A, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(A, 106, 35);
    lv_label_set_text(A, "#17803D A#"); // 设置文本内容
    lv_label_set_recolor(A, true);

    lv_obj_t *W = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(W, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(W, 101, 60);
    lv_label_set_text(W, "#006D77 W#"); // 设置文本内容
    lv_label_set_recolor(W, true);

    volt_chart = lv_chart_create(lv_scr_act());
    style_chart_surface(volt_chart);
    lv_obj_set_size(volt_chart, 260, 50);
    lv_obj_set_pos(volt_chart, 10, 100);
    lv_chart_set_point_count(volt_chart, 15);
    lv_chart_set_axis_tick(volt_chart, LV_CHART_AXIS_PRIMARY_Y, 4, 2, 3, 2, false, 30);
    lv_chart_set_range(volt_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 12);
    lv_obj_set_style_width(volt_chart, 0, LV_PART_INDICATOR);
    lv_obj_set_style_height(volt_chart, 0, LV_PART_INDICATOR);
    lv_chart_set_update_mode(volt_chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_series_t *ser1 = lv_chart_add_series(volt_chart, lv_color_hex(0xFF0000), LV_CHART_AXIS_PRIMARY_Y);
    adddata_timer = lv_timer_create(add_data, 100, ser1);
    lv_chart_refresh(volt_chart);

    cur_chart = lv_chart_create(lv_scr_act());
    style_chart_surface(cur_chart);
    lv_obj_set_size(cur_chart, 260, 50);
    lv_obj_set_pos(cur_chart, 10, 153);
    lv_chart_set_point_count(cur_chart, 15);
    lv_chart_set_axis_tick(cur_chart, LV_CHART_AXIS_PRIMARY_Y, 4, 2, 3, 2, false, 30);
    lv_chart_set_range(cur_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
    lv_obj_set_style_width(cur_chart, 0, LV_PART_INDICATOR);
    lv_obj_set_style_height(cur_chart, 0, LV_PART_INDICATOR);
    lv_chart_set_update_mode(cur_chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_series_t *ser2 = lv_chart_add_series(cur_chart, lv_color_hex(0x00FF7F), LV_CHART_AXIS_PRIMARY_Y);
    adddata_timer2 = lv_timer_create(add_data2, 100, ser2);
    lv_chart_refresh(cur_chart);

    // 设置聚焦状的边框
    static lv_style_t focused_style;
    lv_style_init(&focused_style);
    lv_style_set_border_color(&focused_style, lv_color_hex(UI_COLOR_PRIMARY));
    lv_style_set_border_width(&focused_style, 3);

    static lv_style_t powerbtn_style;
    lv_style_init(&powerbtn_style);
    lv_style_set_bg_color(&powerbtn_style, lv_color_hex(UI_COLOR_CARD));
    lv_style_set_border_color(&powerbtn_style, lv_color_hex(UI_COLOR_BORDER));
    lv_style_set_border_width(&powerbtn_style, 2);
    lv_style_set_text_color(&powerbtn_style, lv_color_hex(UI_COLOR_TEXT));

    lv_obj_t *poweron = lv_btn_create(lv_scr_act());
    lv_obj_set_size(poweron, 41, 83);
    lv_obj_set_pos(poweron, 229, 10);
    lv_obj_add_style(poweron, &powerbtn_style, 0); // 将样式应用到矩形中
    lv_obj_add_style(poweron, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(poweron, poweronbtn_event_cb, LV_EVENT_CLICKED, NULL);
    poweron_label = lv_label_create(poweron);
    lv_obj_set_style_text_color(poweron_label, lv_color_hex(UI_COLOR_PRIMARY), 0);
    lv_obj_align(poweron_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(poweron_label, LV_SYMBOL_POWER);
    lv_obj_set_style_text_font(poweron_label, &lv_font_montserrat_28, 0);

    lv_obj_t *VUP = lv_btn_create(lv_scr_act());
    lv_obj_set_size(VUP, 40, 40);
    lv_obj_set_pos(VUP, 183, 10);
    lv_obj_add_style(VUP, &powerbtn_style, 0); // 将样式应用到矩形中
    lv_obj_add_style(VUP, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(VUP, VUPbtn_event_cb, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_style(poweron, &focused_style, LV_STATE_FOCUSED);
    // lv_obj_add_event_cb(poweron, btn1_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *VUP_label = lv_label_create(VUP);
    lv_obj_set_style_text_color(VUP_label, lv_color_hex(UI_COLOR_TEXT_BLUE), 0);
    lv_obj_align(VUP_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(VUP_label, LV_SYMBOL_PLUS);
    lv_obj_set_style_text_font(VUP_label, &lv_font_montserrat_24, 0);

    lv_obj_t *VDOWN = lv_btn_create(lv_scr_act());
    lv_obj_set_size(VDOWN, 40, 40);
    lv_obj_set_pos(VDOWN, 183, 51);
    lv_obj_add_style(VDOWN, &powerbtn_style, 0); // 将样式应用到矩形中
    lv_obj_add_style(VDOWN, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(VDOWN, VDOWNbtn_event_cb, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_style(poweron, &focused_style, LV_STATE_FOCUSED);
    // lv_obj_add_event_cb(poweron, btn1_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *VDOWN_label = lv_label_create(VDOWN);
    lv_obj_set_style_text_color(VDOWN_label, lv_color_hex(UI_COLOR_TEXT_BLUE), 0);
    lv_obj_align(VDOWN_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(VDOWN_label, LV_SYMBOL_MINUS);
    lv_obj_set_style_text_font(VDOWN_label, &lv_font_montserrat_24, 0);

    lv_obj_t *V11 = lv_btn_create(lv_scr_act());
    lv_obj_set_size(V11, 42, 26);
    lv_obj_set_pos(V11, 135, 10);
    lv_obj_add_style(V11, &powerbtn_style, 0); // 将样式应用到矩形中
    lv_obj_add_style(V11, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(V11, V11btn_event_cb, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_style(poweron, &focused_style, LV_STATE_FOCUSED);
    // lv_obj_add_event_cb(poweron, btn1_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *V11_label = lv_label_create(V11);
    lv_obj_set_style_text_color(V11_label, lv_color_hex(UI_COLOR_TEXT_WARM), 0);
    lv_obj_align(V11_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(V11_label, "11V");
    lv_obj_set_style_text_font(V11_label, &lv_font_montserrat_20, 0);

    lv_obj_t *V5 = lv_btn_create(lv_scr_act());
    lv_obj_set_size(V5, 42, 26);
    lv_obj_set_pos(V5, 135, 37);
    lv_obj_add_style(V5, &powerbtn_style, 0); // 将样式应用到矩形中
    lv_obj_add_style(V5, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(V5, V5btn_event_cb, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_style(poweron, &focused_style, LV_STATE_FOCUSED);
    // lv_obj_add_event_cb(poweron, btn1_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *V5_label = lv_label_create(V5);
    lv_obj_set_style_text_color(V5_label, lv_color_hex(UI_COLOR_TEXT_WARM), 0);
    lv_obj_align(V5_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(V5_label, "5V");
    lv_obj_set_style_text_font(V5_label, &lv_font_montserrat_20, 0);

    lv_obj_t *V3 = lv_btn_create(lv_scr_act());
    lv_obj_set_size(V3, 42, 26);
    lv_obj_set_pos(V3, 135, 64);
    lv_obj_add_style(V3, &powerbtn_style, 0); // 将样式应用到矩形中
    lv_obj_add_style(V3, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(V3, V3btn_event_cb, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_style(poweron, &focused_style, LV_STATE_FOCUSED);
    // lv_obj_add_event_cb(poweron, btn1_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *V3_label = lv_label_create(V3);
    lv_obj_set_style_text_color(V3_label, lv_color_hex(UI_COLOR_TEXT_WARM), 0);
    lv_obj_align(V3_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(V3_label, "3V3");
    lv_obj_set_style_text_font(V3_label, &lv_font_montserrat_18, 0);

    group = lv_group_create();
    lv_indev_set_group(indev_keypad, group);
    lv_group_add_obj(group, VUP);
    lv_group_add_obj(group, VDOWN);
    lv_group_add_obj(group, V11);
    lv_group_add_obj(group, V5);
    lv_group_add_obj(group, V3);
    lv_group_focus_obj(V3);

    lv_obj_t *rounded_rect1 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect1, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect1, 50, 35);
    lv_obj_set_pos(rounded_rect1, 6, 205);
    lv_obj_set_style_radius(rounded_rect1, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect1, lv_color_hex(0x696969), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect1_label1 = lv_label_create(rounded_rect1);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label1, "1");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect1_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect1_label2 = lv_label_create(rounded_rect1); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label2, "GND");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect1_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect2 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect2, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect2, 50, 35);
    lv_obj_set_pos(rounded_rect2, 56, 205);
    lv_obj_set_style_radius(rounded_rect2, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect2, lv_color_hex(0x8B0000), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect2_label1 = lv_label_create(rounded_rect2);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label1, "2");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect2_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect2_label2 = lv_label_create(rounded_rect2); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label2, "OUT");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect2_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *volt_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label1, "#A84B00 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    configure_swipe_back_for_current_screen(true);
}

void pwm_init(void)
{

    ui_set_current_page(UI_PAGE_PWM);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *pwm_wave_card = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(pwm_wave_card, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(pwm_wave_card, 250, 92);
    lv_obj_set_pos(pwm_wave_card, 24, 108);
    lv_obj_set_style_radius(pwm_wave_card, 20, LV_PART_MAIN);
    lv_obj_set_style_bg_color(pwm_wave_card, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_color(pwm_wave_card, lv_color_hex(UI_COLOR_ACCENT_SOFT), LV_PART_MAIN);
    lv_obj_set_style_border_width(pwm_wave_card, 2, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(pwm_wave_card, 0, LV_PART_MAIN);

    lv_obj_t *pwm_wave_glow = lv_obj_create(pwm_wave_card);
    lv_obj_remove_style_all(pwm_wave_glow);
    lv_obj_set_size(pwm_wave_glow, 220, 56);
    lv_obj_align(pwm_wave_glow, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(pwm_wave_glow, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(pwm_wave_glow, lv_color_hex(0xFFE2EF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(pwm_wave_glow, LV_OPA_60, LV_PART_MAIN);


    lv_obj_t *pwm = lv_img_create(lv_scr_act());
    lv_img_set_src(pwm, &pwmint_png);                       // Replace with your image variable or path
    lv_obj_set_size(pwm, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_set_pos(pwm, 40, 116);
    lv_obj_set_style_img_recolor(pwm, lv_color_hex(0xFF5C8A), LV_PART_MAIN);
    lv_obj_set_style_img_recolor_opa(pwm, LV_OPA_90, LV_PART_MAIN);

    lv_obj_t *rounded_rect1 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect1, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect1, 50, 35);
    lv_obj_set_pos(rounded_rect1, 6, 205);
    lv_obj_set_style_radius(rounded_rect1, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect1, lv_color_hex(0x696969), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect1_label1 = lv_label_create(rounded_rect1);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label1, "1");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect1_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect1_label2 = lv_label_create(rounded_rect1); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label2, "GND");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect1_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect2 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect2, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect2, 50, 35);
    lv_obj_set_pos(rounded_rect2, 56, 205);
    lv_obj_set_style_radius(rounded_rect2, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect2, lv_color_hex(0x8B0000), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect2_label1 = lv_label_create(rounded_rect2);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label1, "6");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect2_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect2_label2 = lv_label_create(rounded_rect2); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label2, "PWM");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect2_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *volt_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label1, "#A84B00 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    fre = lv_textarea_create(lv_scr_act()); /* 创建文本框*/
    lv_textarea_set_placeholder_text(fre, "0-100K");
    lv_obj_set_style_text_font(fre, &lv_font_montserrat_20, LV_PART_MAIN); /* 设置字体 */
    lv_textarea_set_one_line(fre, true);                                   /* 设置单行模式 */
    lv_obj_set_size(fre, 100, 45);
    lv_obj_set_pos(fre, 72, 10);
    style_input_surface(fre);

    duty = lv_textarea_create(lv_scr_act()); /* 创建文本框*/
    lv_textarea_set_placeholder_text(duty, "0-100");
    lv_obj_set_style_text_font(duty, &lv_font_montserrat_20, LV_PART_MAIN); /* 设置字体 */
    lv_textarea_set_one_line(duty, true);                                   /* 设置单行模式 */
    lv_obj_set_size(duty, 100, 45);
    lv_obj_set_pos(duty, 72, 60);
    style_input_surface(duty);

    lv_obj_t *fre_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(fre_label1, "#A84B00 FRE#");        // 设置文本内容
    lv_obj_set_style_text_font(fre_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(fre_label1, 10, 20);
    lv_label_set_recolor(fre_label1, true);

    lv_obj_t *duty_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(duty_label1, "#1D4ED8 DUTY#");       // 设置文本内容
    lv_obj_set_style_text_font(duty_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(duty_label1, 10, 70);
    lv_label_set_recolor(duty_label1, true);

    lv_obj_t *fre_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(fre_label2, "#A84B00 Hz#");         // 设置文本内容
    lv_obj_set_style_text_font(fre_label2, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(fre_label2, 180, 20);
    lv_label_set_recolor(fre_label2, true);

    lv_obj_t *duty_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(duty_label2, "#1D4ED8 %#");          // 设置文本内容
    lv_obj_set_style_text_font(duty_label2, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(duty_label2, 180, 65);
    lv_label_set_recolor(duty_label2, true);

    fre_keyboard = lv_keyboard_create(lv_scr_act());
    duty_keyboard = lv_keyboard_create(lv_scr_act());
    lv_keyboard_set_mode(fre_keyboard, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_mode(duty_keyboard, LV_KEYBOARD_MODE_NUMBER);
    lv_keyboard_set_textarea(fre_keyboard, fre);
    lv_keyboard_set_textarea(duty_keyboard, duty);
    lv_obj_add_flag(fre_keyboard, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(duty_keyboard, LV_OBJ_FLAG_HIDDEN);

    lv_obj_add_event_cb(fre, fretext_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(duty, dutytext_event_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(fre_keyboard, clear_keyboard_event_handler, LV_EVENT_VALUE_CHANGED, NULL);  /* 设置键盘事件回调 */
    lv_obj_add_event_cb(duty_keyboard, clear_keyboard_event_handler, LV_EVENT_VALUE_CHANGED, NULL); /* 设置键盘事件回调 */

    static lv_style_t pwm_style;
    lv_style_init(&pwm_style);
    lv_style_set_bg_color(&pwm_style, lv_color_hex(UI_COLOR_PRIMARY));
    lv_style_set_border_color(&pwm_style, lv_color_hex(UI_COLOR_BORDER));
    lv_style_set_border_width(&pwm_style, 2);
    lv_style_set_text_color(&pwm_style, lv_color_hex(0xFFFFFF));

    pwm_btn = lv_btn_create(lv_scr_act());
    lv_obj_add_style(pwm_btn, &pwm_style, 0); // 将样式应用到矩形中
    lv_obj_set_size(pwm_btn, 69, 50);
    lv_obj_set_pos(pwm_btn, 210, 30);
    lv_obj_add_event_cb(pwm_btn, pwm_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *pwm_btn_label = lv_label_create(pwm_btn);
    lv_obj_set_style_text_color(pwm_btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(pwm_btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(pwm_btn_label, "OPEN");
    lv_obj_set_style_text_font(pwm_btn_label, &lv_font_montserrat_20, 0);

    configure_swipe_back_for_current_screen(true);
}

void uarthelper_init(void)
{

    ui_set_current_page(UI_PAGE_UART);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);


    lv_obj_t *rounded_rect1 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect1, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect1, 50, 35);
    lv_obj_set_pos(rounded_rect1, 6, 205);
    lv_obj_set_style_radius(rounded_rect1, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect1, lv_color_hex(0x696969), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect1_label1 = lv_label_create(rounded_rect1);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label1, "1");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect1_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect1_label2 = lv_label_create(rounded_rect1); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label2, "GND");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect1_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect2 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect2, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect2, 50, 35);
    lv_obj_set_pos(rounded_rect2, 56, 205);
    lv_obj_set_style_radius(rounded_rect2, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect2, lv_color_hex(0xFFA500), LV_PART_MAIN); // 背景色为橙色
    lv_obj_t *rounded_rect2_label1 = lv_label_create(rounded_rect2);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label1, "12");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect2_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect2_label2 = lv_label_create(rounded_rect2); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label2, "RX");                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect2_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect3 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect3, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect3, 50, 35);
    lv_obj_set_pos(rounded_rect3, 106, 205);
    lv_obj_set_style_radius(rounded_rect3, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect3, lv_color_hex(0xFFA500), LV_PART_MAIN); // 背景色为橙色
    lv_obj_t *rounded_rect3_label1 = lv_label_create(rounded_rect3);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect3_label1, "13");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect3_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect3_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect3_label2 = lv_label_create(rounded_rect3); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect3_label2, "TX");                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect3_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect3_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *volt_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label1, "#A84B00 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    lv_obj_t *volt_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label2, "#17803D UART#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label2, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(volt_label2, 15, 32);
    lv_label_set_recolor(volt_label2, true);

    lv_obj_t *volt_label3 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label3, "#006D77 baudRate#");   // 设置文本内容
    lv_obj_set_style_text_font(volt_label3, &lv_font_montserrat_18, 0);
    lv_obj_set_pos(volt_label3, 170, 5);
    lv_label_set_recolor(volt_label3, true);

    uart_extarea = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(uart_extarea, 269, 126);
    lv_obj_set_pos(uart_extarea, 6, 75);
    lv_textarea_set_max_length(uart_extarea, 1024);
    lv_textarea_set_placeholder_text(uart_extarea, "USART monitor output");
    style_input_surface(uart_extarea);

    uart_list = lv_dropdown_create(lv_scr_act());
    lv_obj_set_size(uart_list, 105, 40);
    lv_obj_set_pos(uart_list, 170, 30);
    style_input_surface(uart_list);
    lv_dropdown_set_options(uart_list, "");
    lv_dropdown_add_option(uart_list, "1200", 0);
    lv_dropdown_add_option(uart_list, "2400", 1);
    lv_dropdown_add_option(uart_list, "4800", 2);
    lv_dropdown_add_option(uart_list, "9600", 3);
    lv_dropdown_add_option(uart_list, "19200", 4);
    lv_dropdown_add_option(uart_list, "43000", 5);
    lv_dropdown_add_option(uart_list, "76800", 6);
    lv_dropdown_add_option(uart_list, "115200", 7);
    lv_dropdown_add_option(uart_list, "128000", 8);
    lv_dropdown_add_option(uart_list, "230400", 9);
    lv_dropdown_add_option(uart_list, "256000", 10);
    lv_dropdown_add_option(uart_list, "460800", 11);
    lv_dropdown_add_option(uart_list, "921600", 12);
    lv_dropdown_set_selected(uart_list, 7);

    static lv_style_t uart_style;
    lv_style_init(&uart_style);
    lv_style_set_bg_color(&uart_style, lv_color_hex(UI_COLOR_PRIMARY));
    lv_style_set_border_color(&uart_style, lv_color_hex(UI_COLOR_BORDER));
    lv_style_set_border_width(&uart_style, 2);
    lv_style_set_text_color(&uart_style, lv_color_hex(0xFFFFFF));

    uart_btn = lv_btn_create(lv_scr_act());
    lv_obj_add_style(uart_btn, &uart_style, 0); // 将样式应用到矩形中
    lv_obj_set_size(uart_btn, 65, 52);
    lv_obj_set_pos(uart_btn, 100, 20);
    lv_obj_add_event_cb(uart_btn, uart_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *uart_btn_label = lv_label_create(uart_btn);
    lv_obj_set_style_text_color(uart_btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(uart_btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(uart_btn_label, "OPEN");
    lv_obj_set_style_text_font(uart_btn_label, &lv_font_montserrat_20, 0);

    configure_swipe_back_for_current_screen(true);
}

void i2c_init(void)
{

    ui_set_current_page(UI_PAGE_I2C);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);


    lv_obj_t *rounded_rect1 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect1, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect1, 50, 35);
    lv_obj_set_pos(rounded_rect1, 6, 205);
    lv_obj_set_style_radius(rounded_rect1, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect1, lv_color_hex(0x696969), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect1_label1 = lv_label_create(rounded_rect1);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label1, "1");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect1_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect1_label2 = lv_label_create(rounded_rect1); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label2, "GND");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect1_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect2 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect2, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect2, 50, 35);
    lv_obj_set_pos(rounded_rect2, 56, 205);
    lv_obj_set_style_radius(rounded_rect2, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect2, lv_color_hex(0x1E90FF), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect2_label1 = lv_label_create(rounded_rect2);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label1, "7");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect2_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect2_label2 = lv_label_create(rounded_rect2); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label2, "SCL");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect2_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect3 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect3, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect3, 50, 35);
    lv_obj_set_pos(rounded_rect3, 106, 205);
    lv_obj_set_style_radius(rounded_rect3, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect3, lv_color_hex(0x1E90FF), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect3_label1 = lv_label_create(rounded_rect3);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect3_label1, "8");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect3_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect3_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect3_label2 = lv_label_create(rounded_rect3); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect3_label2, "SDA");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect3_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect3_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *volt_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label1, "#A84B00 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    lv_obj_t *volt_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label2, "#006D77 I2C Device#"); // 设置文本内容
    lv_obj_set_style_text_font(volt_label2, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(volt_label2, 15, 26);
    lv_label_set_recolor(volt_label2, true);

    i2c_extarea = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(i2c_extarea, 269, 120);
    lv_obj_set_pos(i2c_extarea, 6, 75);
    style_input_surface(i2c_extarea);

    static lv_style_t i2con_style;
    lv_style_init(&i2con_style);
    lv_style_set_bg_color(&i2con_style, lv_color_hex(UI_COLOR_PRIMARY));
    lv_style_set_border_color(&i2con_style, lv_color_hex(UI_COLOR_BORDER));
    lv_style_set_border_width(&i2con_style, 2);
    lv_style_set_text_color(&i2con_style, lv_color_hex(0xFFFFFF));

    i2con = lv_btn_create(lv_scr_act());
    lv_obj_add_style(i2con, &i2con_style, 0); // 将样式应用到矩形中
    lv_obj_set_size(i2con, 90, 45);
    lv_obj_set_pos(i2con, 180, 20);
    lv_obj_add_event_cb(i2con, i2conbtn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *i2con_label = lv_label_create(i2con);
    lv_obj_set_style_text_color(i2con_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(i2con_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(i2con_label, "SCAN");
    lv_obj_set_style_text_font(i2con_label, &lv_font_montserrat_24, 0);

    configure_swipe_back_for_current_screen(true);
}

void voltmeter_init(void)
{

    ui_set_current_page(UI_PAGE_VOLTMETER);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    ina266_flag = 1;
    digitalWrite(1, LOW);
    lv_obj_t *label1 = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(label1, 95, 20);
    updatelabel_timer1 = lv_timer_create(update_label_timer1, 100, label1);
    lv_obj_t *label2 = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(label2, 200, 20);
    lv_label_set_text(label2, "#C62828 V#"); // 设置文本内容
    lv_label_set_recolor(label2, true);

    volt_chart = lv_chart_create(lv_scr_act());
    style_chart_surface(volt_chart);
    lv_obj_set_size(volt_chart, 260, 140);
    lv_obj_set_pos(volt_chart, 35, 60);
    lv_chart_set_point_count(volt_chart, 15);
    lv_chart_set_axis_tick(volt_chart, LV_CHART_AXIS_PRIMARY_Y, 4, 2, 6, 2, true, 30);
    lv_chart_set_range(volt_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 20);
    lv_obj_set_style_width(volt_chart, 0, LV_PART_INDICATOR);
    lv_obj_set_style_height(volt_chart, 0, LV_PART_INDICATOR);
    lv_chart_set_update_mode(volt_chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_series_t *ser1 = lv_chart_add_series(volt_chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
    adddata_timer = lv_timer_create(add_data, 100, ser1);
    lv_chart_refresh(volt_chart);

    lv_obj_t *rounded_rect1 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect1, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect1, 50, 35);
    lv_obj_set_pos(rounded_rect1, 6, 205);
    lv_obj_set_style_radius(rounded_rect1, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect1, lv_color_hex(0x696969), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect1_label1 = lv_label_create(rounded_rect1);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label1, "1");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect1_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect1_label2 = lv_label_create(rounded_rect1); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label2, "GND");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect1_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect2 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect2, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect2, 50, 35);
    lv_obj_set_pos(rounded_rect2, 56, 205);
    lv_obj_set_style_radius(rounded_rect2, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect2, lv_color_hex(0x8B0000), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect2_label1 = lv_label_create(rounded_rect2);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label1, "2");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect2_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect2_label2 = lv_label_create(rounded_rect2); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label2, "VIN");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect2_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *volt_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label1, "#A84B00 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    configure_swipe_back_for_current_screen(true);
}

void DSO_init(void)
{

    ui_set_current_page(UI_PAGE_DSO);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);


    DSO_flag = 1;

    DSO_chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(DSO_chart, 269, 165);
    lv_obj_set_pos(DSO_chart, 6, 30);
    style_chart_surface(DSO_chart);
    lv_chart_set_point_count(DSO_chart, 64);
    lv_chart_set_div_line_count(DSO_chart, 7, 11);
    lv_chart_set_axis_tick(DSO_chart, LV_CHART_AXIS_PRIMARY_Y, 4, 2, 7, 2, false, 30);
    lv_chart_set_range(DSO_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 4095);
    lv_obj_set_style_width(DSO_chart, 0, LV_PART_INDICATOR);
    lv_obj_set_style_height(DSO_chart, 0, LV_PART_INDICATOR);
    lv_chart_set_update_mode(DSO_chart, LV_CHART_UPDATE_MODE_SHIFT);
    DSO_ser = lv_chart_add_series(DSO_chart, lv_color_hex(0xFFFF00), LV_CHART_AXIS_PRIMARY_Y);
    // DSO_adddata_timer = lv_timer_create(DSO_add_data, 10, DSO_ser);
    lv_chart_refresh(DSO_chart);

    lv_obj_t *max_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(max_label, lv_color_hex(UI_COLOR_TEXT_RED), 0);
    lv_obj_set_style_text_font(max_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(max_label, 10, 10);
    lv_label_set_text(max_label, "max"); // 设置文本内容
    lv_obj_t *maxValue_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(maxValue_label, lv_color_hex(UI_COLOR_TEXT_RED), 0);
    lv_obj_set_style_text_font(maxValue_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(maxValue_label, 55, 10);

    lv_obj_t *min_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(min_label, lv_color_hex(UI_COLOR_TEXT_GREEN), 0);
    lv_obj_set_style_text_font(min_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(min_label, 98, 10);
    lv_label_set_text(min_label, "min"); // 设置文本内容
    lv_obj_t *minValue_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(minValue_label, lv_color_hex(UI_COLOR_TEXT_GREEN), 0);
    lv_obj_set_style_text_font(minValue_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(minValue_label, 140, 10);

    lv_obj_t *vpp_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(vpp_label, lv_color_hex(UI_COLOR_TEXT_COOL), 0);
    lv_obj_set_style_text_font(vpp_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(vpp_label, 185, 10);
    lv_label_set_text(vpp_label, "vpp"); // 设置文本内容
    lv_obj_t *peakToPeakValue_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(peakToPeakValue_label, lv_color_hex(UI_COLOR_TEXT_COOL), 0);
    lv_obj_set_style_text_font(peakToPeakValue_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(peakToPeakValue_label, 225, 10);

    DSO_update_timer1 = lv_timer_create(DSO_update_maxValue_timer, 50, maxValue_label);
    DSO_update_timer2 = lv_timer_create(DSO_update_minValue_timer, 50, minValue_label);
    DSO_update_timer3 = lv_timer_create(DSO_update_peakToPeakValue_timer, 50, peakToPeakValue_label);

    lv_obj_t *rounded_rect1 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect1, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect1, 50, 35);
    lv_obj_set_pos(rounded_rect1, 6, 205);
    lv_obj_set_style_radius(rounded_rect1, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect1, lv_color_hex(0x696969), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect1_label1 = lv_label_create(rounded_rect1);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label1, "1");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect1_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect1_label2 = lv_label_create(rounded_rect1); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label2, "GND");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect1_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect2 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect2, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect2, 50, 35);
    lv_obj_set_pos(rounded_rect2, 56, 205);
    lv_obj_set_style_radius(rounded_rect2, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect2, lv_color_hex(0xFF8C00), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect2_label1 = lv_label_create(rounded_rect2);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label1, "5");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect2_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect2_label2 = lv_label_create(rounded_rect2); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label2, "IN");                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect2_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *volt_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label1, "#A84B00 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 115, 210);
    lv_label_set_recolor(volt_label1, true);

    lv_obj_t *volt_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label2, "#A84B00 0.55v/Div#");  // 设置文本内容
    lv_obj_set_style_text_font(volt_label2, &lv_font_montserrat_18, 0);
    lv_obj_set_pos(volt_label2, 190, 211);
    lv_label_set_recolor(volt_label2, true);

    configure_swipe_back_for_current_screen(true);
}

static void wireless_uart_clear_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    if (wireless_uart_extarea)
    {
        lv_textarea_set_text(wireless_uart_extarea, "");
    }
}

void wirelessuart_init(void)
{

    ui_set_current_page(UI_PAGE_WIRELESS_UART);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);


    lv_obj_t *rounded_rect1 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect1, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect1, 50, 35);
    lv_obj_set_pos(rounded_rect1, 6, 205);
    lv_obj_set_style_radius(rounded_rect1, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect1, lv_color_hex(0x696969), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect1_label1 = lv_label_create(rounded_rect1);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label1, "1");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect1_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect1_label2 = lv_label_create(rounded_rect1); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label2, "GND");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect1_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect2 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect2, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect2, 50, 35);
    lv_obj_set_pos(rounded_rect2, 56, 205);
    lv_obj_set_style_radius(rounded_rect2, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect2, lv_color_hex(0xFFA500), LV_PART_MAIN); // 背景色为橙色
    lv_obj_t *rounded_rect2_label1 = lv_label_create(rounded_rect2);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label1, "12");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect2_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect2_label2 = lv_label_create(rounded_rect2); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label2, "RX");                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect2_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect3 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect3, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect3, 50, 35);
    lv_obj_set_pos(rounded_rect3, 106, 205);
    lv_obj_set_style_radius(rounded_rect3, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect3, lv_color_hex(0xFFA500), LV_PART_MAIN); // 背景色为橙色
    lv_obj_t *rounded_rect3_label1 = lv_label_create(rounded_rect3);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect3_label1, "13");                                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect3_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect3_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect3_label2 = lv_label_create(rounded_rect3); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect3_label2, "TX");                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect3_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect3_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *volt_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label1, "#A84B00 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    lv_obj_t *volt_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_obj_set_style_text_color(volt_label2, lv_color_hex(UI_COLOR_TEXT_BLUE), 0);
    lv_label_set_text(volt_label2, LV_SYMBOL_BLUETOOTH "BLE"); // 设置文本内容
    lv_obj_set_style_text_font(volt_label2, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(volt_label2, 6, 32);
    lv_label_set_recolor(volt_label2, true);

    lv_obj_t *volt_label3 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label3, "#006D77 baudRate#");   // 设置文本内容
    lv_obj_set_style_text_font(volt_label3, &lv_font_montserrat_18, 0);
    lv_obj_set_pos(volt_label3, 170, 5);
    lv_label_set_recolor(volt_label3, true);

    wireless_uart_list = lv_dropdown_create(lv_scr_act());
    lv_obj_set_size(wireless_uart_list, 105, 40);
    lv_obj_set_pos(wireless_uart_list, 170, 30);
    style_input_surface(wireless_uart_list);
    lv_dropdown_set_options(wireless_uart_list, "");
    lv_dropdown_add_option(wireless_uart_list, "1200", 0);
    lv_dropdown_add_option(wireless_uart_list, "2400", 1);
    lv_dropdown_add_option(wireless_uart_list, "4800", 2);
    lv_dropdown_add_option(wireless_uart_list, "9600", 3);
    lv_dropdown_add_option(wireless_uart_list, "19200", 4);
    lv_dropdown_add_option(wireless_uart_list, "43000", 5);
    lv_dropdown_add_option(wireless_uart_list, "76800", 6);
    lv_dropdown_add_option(wireless_uart_list, "115200", 7);
    lv_dropdown_add_option(wireless_uart_list, "128000", 8);
    lv_dropdown_add_option(wireless_uart_list, "230400", 9);
    lv_dropdown_add_option(wireless_uart_list, "256000", 10);
    lv_dropdown_add_option(wireless_uart_list, "460800", 11);
    lv_dropdown_add_option(wireless_uart_list, "921600", 12);
    lv_dropdown_set_selected(wireless_uart_list, 7);

    wireless_uart_extarea = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(wireless_uart_extarea, 269, 120);
    lv_obj_set_pos(wireless_uart_extarea, 6, 75);
    lv_textarea_set_max_length(wireless_uart_extarea, 1024);
    lv_textarea_set_placeholder_text(wireless_uart_extarea, "BLE passthrough monitor");
    style_input_surface(wireless_uart_extarea);

    static lv_style_t wireless_uart_style;
    lv_style_init(&wireless_uart_style);
    lv_style_set_bg_color(&wireless_uart_style, lv_color_hex(UI_COLOR_PRIMARY));
    lv_style_set_border_color(&wireless_uart_style, lv_color_hex(UI_COLOR_BORDER));
    lv_style_set_border_width(&wireless_uart_style, 2);
    lv_style_set_text_color(&wireless_uart_style, lv_color_hex(0xFFFFFF));

    wireless_uart_btn = lv_btn_create(lv_scr_act());
    lv_obj_add_style(wireless_uart_btn, &wireless_uart_style, 0); // 将样式应用到矩形中
    lv_obj_set_size(wireless_uart_btn, 65, 52);
    lv_obj_set_pos(wireless_uart_btn, 100, 20);
    lv_obj_add_event_cb(wireless_uart_btn, wireless_uart_btn_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *wireless_uart_btn_label = lv_label_create(wireless_uart_btn);
    lv_obj_set_style_text_color(wireless_uart_btn_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(wireless_uart_btn_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(wireless_uart_btn_label, "OPEN");
    lv_obj_set_style_text_font(wireless_uart_btn_label, &lv_font_montserrat_20, 0);

    lv_obj_t *wireless_uart_clear_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(wireless_uart_clear_btn, 65, 52);
    lv_obj_set_pos(wireless_uart_clear_btn, 30, 20);
    lv_obj_set_style_bg_color(wireless_uart_clear_btn, lv_color_hex(UI_COLOR_ACCENT), 0);
    lv_obj_set_style_border_color(wireless_uart_clear_btn, lv_color_hex(UI_COLOR_BORDER), 0);
    lv_obj_set_style_border_width(wireless_uart_clear_btn, 2, 0);
    lv_obj_add_event_cb(wireless_uart_clear_btn, wireless_uart_clear_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *wireless_uart_clear_label = lv_label_create(wireless_uart_clear_btn);
    lv_obj_set_style_text_color(wireless_uart_clear_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(wireless_uart_clear_label, LV_ALIGN_CENTER, 0, 0);
    lv_label_set_text(wireless_uart_clear_label, "CLEAR");
    lv_obj_set_style_text_font(wireless_uart_clear_label, &lv_font_montserrat_16, 0);

    configure_swipe_back_for_current_screen(true);
}

void FREcount_init(void)
{

    ui_set_current_page(UI_PAGE_FRECOUNT);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);


    FREcount_flag = 1;

    lv_obj_t *rounded_rect1 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect1, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect1, 50, 35);
    lv_obj_set_pos(rounded_rect1, 6, 205);
    lv_obj_set_style_radius(rounded_rect1, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect1, lv_color_hex(0x696969), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect1_label1 = lv_label_create(rounded_rect1);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label1, "1");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect1_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect1_label2 = lv_label_create(rounded_rect1); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect1_label2, "GND");                  // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect1_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect1_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *rounded_rect2 = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(rounded_rect2, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(rounded_rect2, 50, 35);
    lv_obj_set_pos(rounded_rect2, 56, 205);
    lv_obj_set_style_radius(rounded_rect2, 5, LV_PART_MAIN);                        // 圆角半径20
    lv_obj_set_style_bg_color(rounded_rect2, lv_color_hex(0x8B0000), LV_PART_MAIN); // 背景色为绿色
    lv_obj_t *rounded_rect2_label1 = lv_label_create(rounded_rect2);                // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label1, "6");                                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label1, &lv_font_montserrat_12, 0);
    lv_obj_align(rounded_rect2_label1, LV_ALIGN_TOP_MID, 0, 2);      // 将文本居中对齐到圆角矩形
    lv_obj_t *rounded_rect2_label2 = lv_label_create(rounded_rect2); // 将文本标签添加到圆角矩形中
    lv_label_set_text(rounded_rect2_label2, "IN");                   // 设置文本内容
    lv_obj_set_style_text_font(rounded_rect2_label2, &lv_font_montserrat_16, 0);
    lv_obj_align(rounded_rect2_label2, LV_ALIGN_BOTTOM_MID, 0, 0); // 将文本居中对齐到圆角矩形
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *FREcount_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(FREcount_label1, "#A84B00 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(FREcount_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(FREcount_label1, 180, 210);
    lv_label_set_recolor(FREcount_label1, true);

    lv_obj_t *FREcount_label2 = lv_label_create(lv_scr_act());        // 将文本标签添加到圆角矩形中
    lv_label_set_text(FREcount_label2, "#006D77 Frequency Counter#"); // 设置文本内容
    lv_obj_set_style_text_font(FREcount_label2, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(FREcount_label2, 15, 26);
    lv_label_set_recolor(FREcount_label2, true);

    FRE_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(FRE_label, &lv_font_montserrat_42, 0);
    lv_obj_set_pos(FRE_label, 30, 100);
    lv_label_set_text(FRE_label, "0");
    FRE_label_update_timer = lv_timer_create(FRE_label_update, 1000, NULL);

    lv_obj_t *Hz_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(Hz_label, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(Hz_label, 236, 110);
    lv_label_set_text(Hz_label, "#C62828 Hz#"); // 设置文本内容
    lv_label_set_recolor(Hz_label, true);

    configure_swipe_back_for_current_screen(true);
}

void readme_init(void)
{
    ui_set_current_page(UI_PAGE_README);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *halo = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(halo);
    lv_obj_set_size(halo, 250, 250);
    lv_obj_align(halo, LV_ALIGN_CENTER, 0, 4);
    lv_obj_set_style_radius(halo, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(halo, lv_color_hex(UI_COLOR_PRIMARY_SOFT), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(halo, LV_OPA_20, LV_PART_MAIN);
    lv_obj_clear_flag(halo, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(halo, LV_DIR_NONE);

    lv_obj_t *card = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(card, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(card, 286, 214);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_radius(card, 28, LV_PART_MAIN);
    lv_obj_set_style_bg_color(card, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(UI_COLOR_PRIMARY_SOFT), LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 3, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(card, 0, LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(card, LV_DIR_NONE);

    lv_obj_t *accent_bar = lv_obj_create(card);
    lv_obj_remove_style_all(accent_bar);
    lv_obj_set_size(accent_bar, 240, 8);
    lv_obj_align(accent_bar, LV_ALIGN_TOP_MID, 0, 14);
    lv_obj_set_style_radius(accent_bar, 8, LV_PART_MAIN);
    lv_obj_set_style_bg_color(accent_bar, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(accent_bar, LV_OPA_80, LV_PART_MAIN);

    lv_obj_t *header = lv_obj_create(card);
    lv_obj_remove_style_all(header);
    lv_obj_set_size(header, 236, 60);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 34);

    lv_obj_t *badge = lv_obj_create(header);
    lv_obj_remove_style_all(badge);
    lv_obj_set_size(badge, 50, 50);
    lv_obj_align(badge, LV_ALIGN_LEFT_MID, 2, 0);
    lv_obj_set_style_radius(badge, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(badge, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(badge, LV_OPA_20, LV_PART_MAIN);

    lv_obj_t *badge_text = lv_label_create(badge);
    lv_label_set_text(badge_text, "EX");
    lv_obj_set_style_text_color(badge_text, lv_color_hex(UI_COLOR_PRIMARY), 0);
    lv_obj_set_style_text_font(badge_text, &lv_font_montserrat_22, 0);
    lv_obj_center(badge_text);

    lv_obj_t *title_wrap = lv_obj_create(header);
    lv_obj_remove_style_all(title_wrap);
    lv_obj_set_size(title_wrap, 128, 40);
    lv_obj_align(title_wrap, LV_ALIGN_CENTER, 2, 4);

    lv_obj_t *title = lv_label_create(title_wrap);
    lv_label_set_text(title, "Exlink Tool");
    lv_obj_set_style_text_color(title, lv_color_hex(UI_COLOR_TEXT_RED), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_18, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 0);

    lv_obj_t *sub = lv_label_create(title_wrap);
    lv_label_set_text(sub, "Device Information");
    lv_obj_set_style_text_color(sub, lv_color_hex(UI_COLOR_TEXT_BLUE), 0);
    lv_obj_set_style_text_font(sub, &lv_font_montserrat_12, 0);
    lv_obj_align(sub, LV_ALIGN_BOTTOM_MID, 0, 0);

    lv_obj_t *chip = lv_obj_create(header);
    lv_obj_remove_style(chip, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(chip, 42, 20);
    lv_obj_align(chip, LV_ALIGN_TOP_RIGHT, -2, 2);
    lv_obj_set_style_radius(chip, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(chip, lv_color_hex(UI_COLOR_SECONDARY_SOFT), LV_PART_MAIN);
    lv_obj_set_style_border_width(chip, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(chip, 0, LV_PART_MAIN);
    lv_obj_t *chip_label = lv_label_create(chip);
    lv_label_set_text(chip_label, "v2.0");
    lv_obj_set_style_text_color(chip_label, lv_color_hex(UI_COLOR_TEXT_GREEN), 0);
    lv_obj_set_style_text_font(chip_label, &lv_font_montserrat_10, 0);
    lv_obj_center(chip_label);

    lv_obj_t *info_box = lv_obj_create(card);
    lv_obj_remove_style(info_box, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(info_box, 236, 78);
    lv_obj_align_to(info_box, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    lv_obj_set_style_radius(info_box, 18, LV_PART_MAIN);
    lv_obj_set_style_bg_color(info_box, lv_color_hex(UI_COLOR_BG), LV_PART_MAIN);
    lv_obj_set_style_border_color(info_box, lv_color_hex(UI_COLOR_ACCENT_SOFT), LV_PART_MAIN);
    lv_obj_set_style_border_width(info_box, 2, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(info_box, 0, LV_PART_MAIN);
    lv_obj_clear_flag(info_box, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scroll_dir(info_box, LV_DIR_NONE);

    lv_obj_t *info_right = lv_label_create(info_box);
    lv_label_set_text_fmt(info_right, "MCU: ESP32-S3\nFlash: 16 MB\nDisplay: GC9307 240x284\nWireless: WiFi + BLE\nBuzzer: L%d",
                          buzzer_volume_level);
    lv_obj_set_style_text_color(info_right, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(info_right, &lv_font_montserrat_10, 0);
    lv_obj_set_width(info_right, 208);
    lv_label_set_long_mode(info_right, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(info_right, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_center(info_right);

    configure_swipe_back_for_current_screen(false);
}

static void setting_volume_option_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    lv_obj_t *target = lv_event_get_target(e);
    int value = (int)(intptr_t)lv_event_get_user_data(e);
    if (value < 1)
    {
        value = 1;
    }
    if (value > 4)
    {
        value = 4;
    }

    buzzer_volume_level = value;
    persist_buzzer_volume_level(value);

    lv_obj_t *value_label = (lv_obj_t *)lv_obj_get_user_data(target);
    if (value_label)
    {
        lv_label_set_text_fmt(value_label, "L%d", value);
    }

    lv_obj_t *row = lv_obj_get_parent(target);
    if (!row)
    {
        return;
    }

    uint32_t row_child_count = lv_obj_get_child_cnt(row);
    for (uint32_t idx = 0; idx < row_child_count; idx++)
    {
        lv_obj_t *option_btn = lv_obj_get_child(row, idx);
        if (!option_btn || lv_obj_get_user_data(option_btn) != value_label)
        {
            continue;
        }

        bool is_active = option_btn == target;
        lv_obj_set_style_bg_color(option_btn, lv_color_hex(is_active ? UI_COLOR_PRIMARY : UI_COLOR_CARD), LV_PART_MAIN);
        lv_obj_set_style_border_color(option_btn, lv_color_hex(is_active ? UI_COLOR_PRIMARY : UI_COLOR_BORDER), LV_PART_MAIN);
        lv_obj_set_style_border_width(option_btn, is_active ? 3 : 2, LV_PART_MAIN);

        lv_obj_t *option_label = lv_obj_get_child(option_btn, 0);
        if (option_label)
        {
            lv_obj_set_style_text_color(option_label, lv_color_hex(is_active ? UI_COLOR_CARD : UI_COLOR_TEXT), 0);
        }
    }
}

static void setting_theme_switch_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_VALUE_CHANGED)
    {
        return;
    }

    lv_obj_t *theme_switch = lv_event_get_target(e);
    bool dark_enabled = lv_obj_has_state(theme_switch, LV_STATE_CHECKED);
    ui_apply_theme_mode(dark_enabled ? UI_THEME_DARK : UI_THEME_LIGHT, true);
}

static void setting_touch_debug_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (g_current_page != UI_PAGE_SETTING)
    {
        return;
    }

    if (!setting_touch_debug_dot || !lv_obj_is_valid(setting_touch_debug_dot) ||
        !setting_touch_debug_label || !lv_obj_is_valid(setting_touch_debug_label))
    {
        return;
    }

    if (!g_touch_debug_pressed)
    {
        lv_obj_add_flag(setting_touch_debug_dot, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(setting_touch_debug_label, "Touch: released");
        return;
    }

    lv_obj_clear_flag(setting_touch_debug_dot, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_pos(setting_touch_debug_dot, g_touch_debug_x - 6, g_touch_debug_y - 6);
    lv_label_set_text_fmt(setting_touch_debug_label, "Touch: %d,%d raw:%d,%d",
                          g_touch_debug_x,
                          g_touch_debug_y,
                          g_touch_debug_raw_x,
                          g_touch_debug_raw_y);
}

static lv_obj_t *create_settings_card(lv_obj_t *parent, lv_coord_t height)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_remove_style(card, 0, LV_PART_SCROLLBAR);
    lv_obj_set_width(card, LV_PCT(100));
    lv_obj_set_height(card, height);
    lv_obj_set_style_bg_color(card, lv_color_hex(UI_COLOR_CARD), LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(UI_COLOR_BORDER), LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 2, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 18, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(card, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(card, 14, LV_PART_MAIN);
    lv_obj_set_scroll_dir(card, LV_DIR_NONE);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    return card;
}

static lv_obj_t *create_action_button(lv_obj_t *parent, const char *text, lv_coord_t width, lv_coord_t height)
{
    lv_obj_t *button = lv_btn_create(parent);
    lv_obj_set_size(button, width, height);
    lv_obj_set_style_radius(button, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_border_color(button, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 2, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_color(label, lv_color_hex(UI_COLOR_CARD), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, 0);
    lv_obj_center(label);
    return button;
}

static void touch_calibration_open_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    touch_calibration_init();
}

static lv_coord_t touch_calibration_clamp_coord(lv_coord_t value, lv_coord_t min_value, lv_coord_t max_value)
{
    if (value < min_value)
    {
        return min_value;
    }
    if (value > max_value)
    {
        return max_value;
    }
    return value;
}

static int touch_calibration_step_index(touch_calibration_step_t step);
static touch_calibration_step_t touch_calibration_step_from_index(int index);

static lv_point_t touch_calibration_target_point(touch_calibration_step_t step)
{
    lv_coord_t width = touch_calibration_screen_w;
    lv_coord_t height = touch_calibration_screen_h;

    int index = touch_calibration_step_index(step);
    if (index < 0 || index >= 25) {
        return (lv_point_t){width / 2, height / 2};
    }

    int col = index % 5;
    int row = index / 5;

    // Use a small margin for the outer edge points to avoid physical bezel issues
    // but keep them as close to the edge as possible for proper calibration
    lv_coord_t margin_x = width > 36 ? 8 : 4;
    lv_coord_t margin_y = height > 36 ? 8 : 4;

    lv_coord_t x_coords[5] = {
        margin_x,
        width / 4,
        width / 2,
        (width * 3) / 4,
        width - 1 - margin_x
    };

    lv_coord_t y_coords[5] = {
        margin_y,
        height / 4,
        height / 2,
        (height * 3) / 4,
        height - 1 - margin_y
    };

    return (lv_point_t){x_coords[col], y_coords[row]};
}

static lv_point_t touch_calibration_target_output_point(int index)
{
    lv_coord_t width = touch_calibration_screen_w;
    lv_coord_t height = touch_calibration_screen_h;

    if (index < 0 || index >= 25) {
        return (lv_point_t){width / 2, height / 2};
    }

    int col = index % 5;
    int row = index / 5;

    // The mathematical target output should ideally be the exact 0, 25, 50, 75, 100% of the screen
    lv_coord_t x_coords[5] = {
        0,
        width / 4,
        width / 2,
        (width * 3) / 4,
        width - 1
    };

    lv_coord_t y_coords[5] = {
        0,
        height / 4,
        height / 2,
        (height * 3) / 4,
        height - 1
    };

    return (lv_point_t){x_coords[col], y_coords[row]};
}

static int touch_calibration_step_index(touch_calibration_step_t step)
{
    for (int idx = 0; idx < kTouchCalibrationPointCount; idx++)
    {
        if (kTouchCalibrationCaptureSteps[idx] == step)
        {
            return idx;
        }
    }
    return -1;
}

static touch_calibration_step_t touch_calibration_step_from_index(int index)
{
    if (index < 0 || index >= kTouchCalibrationPointCount)
    {
        return TOUCH_CAL_STEP_VERIFY;
    }
    return kTouchCalibrationCaptureSteps[index];
}

static void touch_calibration_reset_samples(void)
{
    for (int idx = 0; idx < kTouchCalibrationPointCount; idx++)
    {
        touch_calibration_sum_x[idx] = 0;
        touch_calibration_sum_y[idx] = 0;
        touch_calibration_sample_count[idx] = 0;
    }
    touch_calibration_waiting_release = false;
}

static void touch_calibration_cleanup_timer(void)
{
    if (touch_calibration_timer)
    {
        lv_timer_del(touch_calibration_timer);
        touch_calibration_timer = NULL;
    }
}

static void touch_calibration_enter_step(touch_calibration_step_t step)
{
    g_touch_calibration_step = step;
    touch_calibration_waiting_release = false;
    touch_calibration_step_enter_ms = lv_tick_get();

    if (touch_calibration_primary_btn)
    {
        lv_obj_add_flag(touch_calibration_primary_btn, LV_OBJ_FLAG_HIDDEN);
    }
    if (touch_calibration_secondary_btn)
    {
        lv_obj_add_flag(touch_calibration_secondary_btn, LV_OBJ_FLAG_HIDDEN);
    }
    if (touch_calibration_tertiary_btn)
    {
        lv_obj_add_flag(touch_calibration_tertiary_btn, LV_OBJ_FLAG_HIDDEN);
    }
    if (touch_calibration_quaternary_btn)
    {
        lv_obj_add_flag(touch_calibration_quaternary_btn, LV_OBJ_FLAG_HIDDEN);
    }
    if (touch_calibration_quinary_btn)
    {
        lv_obj_add_flag(touch_calibration_quinary_btn, LV_OBJ_FLAG_HIDDEN);
    }

    if (touch_calibration_target)
    {
        if (step == TOUCH_CAL_STEP_INTRO || step == TOUCH_CAL_STEP_VERIFY)
        {
            lv_obj_add_flag(touch_calibration_target, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_point_t point = touch_calibration_target_point(step);
            lv_obj_clear_flag(touch_calibration_target, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(touch_calibration_target, point.x - 14, point.y - 14);
            lv_obj_move_foreground(touch_calibration_target);
        }
    }

    if (touch_calibration_hint_label)
    {
        if (step == TOUCH_CAL_STEP_INTRO)
        {
            lv_label_set_text(touch_calibration_hint_label,
                              "Touch the edge cross center at each step.\nRelease after every sample.");
        }
        else if (step == TOUCH_CAL_STEP_VERIFY)
        {
            lv_label_set_text(touch_calibration_hint_label,
                              "Calibration saved automatically. Check the red dot, or retry/reset.");
        }
        else
        {
            int step_index = touch_calibration_step_index(step);
            if (step_index >= 0 && step_index < kTouchCalibrationPointCount)
            {
                lv_label_set_text_fmt(touch_calibration_hint_label,
                                      "Step %d/%d: touch %s",
                                      step_index + 1,
                                      kTouchCalibrationPointCount,
                                      kTouchCalibrationStepLabels[step_index]);
            }
            else
            {
                lv_label_set_text(touch_calibration_hint_label, "Status: waiting touch");
            }
        }
    }

    if (touch_calibration_status_label)
    {
        switch (step)
        {
        case TOUCH_CAL_STEP_INTRO:
            lv_label_set_text(touch_calibration_status_label, "Status: ready");
            break;
        case TOUCH_CAL_STEP_VERIFY:
            lv_label_set_text(touch_calibration_status_label, "Status: verification");
            break;
        default:
            lv_label_set_text(touch_calibration_status_label, "Status: waiting touch");
            break;
        }
    }

    if (step == TOUCH_CAL_STEP_INTRO)
    {
        lv_coord_t intro_y = touch_calibration_screen_h - 44;
        if (touch_calibration_primary_btn)
        {
            lv_obj_set_pos(touch_calibration_primary_btn, 16, intro_y);
            lv_obj_clear_flag(touch_calibration_primary_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if (touch_calibration_secondary_btn)
        {
            lv_obj_set_pos(touch_calibration_secondary_btn, touch_calibration_screen_w - 16 - 86, intro_y);
            lv_obj_clear_flag(touch_calibration_secondary_btn, LV_OBJ_FLAG_HIDDEN);
        }
    }
    else if (step == TOUCH_CAL_STEP_VERIFY)
    {
        lv_coord_t bottom_y = touch_calibration_screen_h - 40;
        lv_coord_t action_y = touch_calibration_screen_h - 82;
        lv_coord_t action_x = (touch_calibration_screen_w - (74 * 3 + 8 * 2)) / 2;
        if (touch_calibration_secondary_btn)
        {
            lv_obj_set_pos(touch_calibration_secondary_btn, (touch_calibration_screen_w - 86) / 2, bottom_y);
            lv_obj_clear_flag(touch_calibration_secondary_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if (touch_calibration_tertiary_btn)
        {
            lv_obj_set_pos(touch_calibration_tertiary_btn, action_x, action_y);
            lv_obj_clear_flag(touch_calibration_tertiary_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if (touch_calibration_quaternary_btn)
        {
            lv_obj_set_pos(touch_calibration_quaternary_btn, action_x + 82, action_y);
            lv_obj_clear_flag(touch_calibration_quaternary_btn, LV_OBJ_FLAG_HIDDEN);
        }
        if (touch_calibration_quinary_btn)
        {
            lv_obj_set_pos(touch_calibration_quinary_btn, action_x + 164, action_y);
            lv_obj_clear_flag(touch_calibration_quinary_btn, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void touch_calibration_start_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    touch_calibration_reset_samples();
    touch_calibration_enter_step(touch_calibration_step_from_index(0));
}

static void touch_calibration_back_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    setting_init();
}

static void touch_calibration_retry_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    touch_calibration_reset_samples();
    touch_calibration_enter_step(touch_calibration_step_from_index(0));
}

static void touch_calibration_reset_default_event_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    clear_touch_calibration();
    touch_calibration_reset_samples();
    touch_calibration_enter_step(TOUCH_CAL_STEP_VERIFY);
    if (touch_calibration_status_label)
    {
        lv_label_set_text(touch_calibration_status_label, "Status: default mapping restored");
    }
}

static void touch_calibration_save_event_cb(lv_event_t *e)
{
    if (e && lv_event_get_code(e) != LV_EVENT_CLICKED)
    {
        return;
    }

    lv_point_t raw_points[kTouchCalibrationPointCount];
    lv_point_t target_points[kTouchCalibrationPointCount];

    for (int idx = 0; idx < kTouchCalibrationPointCount; idx++)
    {
        int sample_count = touch_calibration_sample_count[idx];
        if (sample_count == 0)
        {
            if (touch_calibration_status_label)
            {
                lv_label_set_text(touch_calibration_status_label, "Status: calibration data incomplete");
            }
            return;
        }

        raw_points[idx].x = touch_calibration_sum_x[idx] / sample_count;
        raw_points[idx].y = touch_calibration_sum_y[idx] / sample_count;
        target_points[idx] = touch_calibration_target_output_point(idx);
    }

    if (!persist_touch_calibration_points(raw_points, target_points, kTouchCalibrationPointCount))
    {
        if (touch_calibration_status_label)
        {
            lv_label_set_text(touch_calibration_status_label, "Status: invalid calibration, retry");
        }
        return;
    }

    if (touch_calibration_status_label)
    {
        lv_label_set_text(touch_calibration_status_label, "Status: calibration saved");
    }
}

static void touch_calibration_timer_cb(lv_timer_t *timer)
{
    (void)timer;
    if (g_current_page != UI_PAGE_TOUCH_CALIBRATION)
    {
        return;
    }

    if (touch_calibration_debug_label)
    {
        if (!g_touch_debug_pressed)
        {
            lv_label_set_text(touch_calibration_debug_label, "raw:-,- rot:-,- out:-,-");
        }
        else
        {
            lv_label_set_text_fmt(touch_calibration_debug_label,
                                  "raw:%d,%d rot:%d,%d out:%d,%d",
                                  g_touch_debug_raw_x,
                                  g_touch_debug_raw_y,
                                  g_touch_debug_rotated_x,
                                  g_touch_debug_rotated_y,
                                  g_touch_debug_x,
                                  g_touch_debug_y);
        }
    }

    if (touch_calibration_dot)
    {
        if (!g_touch_debug_pressed)
        {
            lv_obj_add_flag(touch_calibration_dot, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            lv_obj_clear_flag(touch_calibration_dot, LV_OBJ_FLAG_HIDDEN);
            lv_obj_set_pos(touch_calibration_dot, g_touch_debug_x - 6, g_touch_debug_y - 6);
            lv_obj_move_foreground(touch_calibration_dot);
        }
    }

    if (g_touch_calibration_step == TOUCH_CAL_STEP_INTRO || g_touch_calibration_step == TOUCH_CAL_STEP_VERIFY)
    {
        return;
    }

    if (lv_tick_elaps(touch_calibration_step_enter_ms) < kTouchCalibrationStepDelayMs)
    {
        return;
    }

    if (!g_touch_debug_pressed)
    {
        touch_calibration_waiting_release = false;
        return;
    }

    int step_index = touch_calibration_step_index(g_touch_calibration_step);
    if (step_index < 0 || step_index >= kTouchCalibrationPointCount)
    {
        return;
    }

    if (touch_calibration_waiting_release)
    {
        return;
    }

    touch_calibration_sum_x[step_index] += g_touch_debug_rotated_x;
    touch_calibration_sum_y[step_index] += g_touch_debug_rotated_y;
    touch_calibration_sample_count[step_index]++;
    touch_calibration_waiting_release = true;

    if (touch_calibration_status_label)
    {
        lv_label_set_text_fmt(touch_calibration_status_label,
                              "Status: point %d sample %d/%d",
                              step_index + 1,
                              touch_calibration_sample_count[step_index],
                              kTouchCalibrationSamplesPerPoint);
    }

    if (touch_calibration_sample_count[step_index] < kTouchCalibrationSamplesPerPoint)
    {
        return;
    }

    int next_step_index = step_index + 1;
    if (next_step_index < kTouchCalibrationPointCount)
    {
        touch_calibration_enter_step(touch_calibration_step_from_index(next_step_index));
        return;
    }

    touch_calibration_enter_step(TOUCH_CAL_STEP_VERIFY);
    touch_calibration_save_event_cb(NULL);
}

void touch_calibration_init(void)
{
    clear_touch_calibration();
    touch_calibration_reset_samples();
    ui_set_current_page(UI_PAGE_TOUCH_CALIBRATION);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    configure_swipe_back_for_current_screen(false);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);
    touch_calibration_cleanup_timer();
    touch_calibration_target = NULL;
    touch_calibration_hint_label = NULL;
    touch_calibration_status_label = NULL;
    touch_calibration_debug_label = NULL;
    touch_calibration_dot = NULL;
    touch_calibration_primary_btn = NULL;
    touch_calibration_secondary_btn = NULL;
    touch_calibration_tertiary_btn = NULL;
    touch_calibration_quaternary_btn = NULL;
    touch_calibration_quinary_btn = NULL;

    lv_disp_t *disp = lv_disp_get_default();
    if (disp)
    {
        touch_calibration_screen_w = lv_disp_get_hor_res(disp);
        touch_calibration_screen_h = lv_disp_get_ver_res(disp);
    }

    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "Touch Calibration");
    lv_obj_set_style_text_color(title, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(title, 14, 12);

    touch_calibration_hint_label = lv_label_create(lv_scr_act());
    lv_obj_set_width(touch_calibration_hint_label, touch_calibration_screen_w - 32);
    lv_obj_set_pos(touch_calibration_hint_label, 14, 44);
    lv_obj_set_style_text_color(touch_calibration_hint_label, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(touch_calibration_hint_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(touch_calibration_hint_label, LV_TEXT_ALIGN_LEFT, 0);
    lv_label_set_long_mode(touch_calibration_hint_label, LV_LABEL_LONG_WRAP);

    touch_calibration_status_label = lv_label_create(lv_scr_act());
    lv_obj_set_pos(touch_calibration_status_label, 14, 88);
    lv_obj_set_style_text_color(touch_calibration_status_label, lv_color_hex(UI_COLOR_MUTED), 0);
    lv_obj_set_style_text_font(touch_calibration_status_label, &lv_font_montserrat_12, 0);

    touch_calibration_debug_label = lv_label_create(lv_scr_act());
    lv_obj_set_width(touch_calibration_debug_label, touch_calibration_screen_w - 28);
    lv_obj_set_pos(touch_calibration_debug_label, 14, 108);
    lv_obj_set_style_text_color(touch_calibration_debug_label, lv_color_hex(UI_COLOR_MUTED), 0);
    lv_obj_set_style_text_font(touch_calibration_debug_label, &lv_font_montserrat_10, 0);
    lv_label_set_long_mode(touch_calibration_debug_label, LV_LABEL_LONG_WRAP);

    lv_obj_t *target_container = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(target_container);
    lv_obj_set_size(target_container, touch_calibration_screen_w, touch_calibration_screen_h);
    lv_obj_set_pos(target_container, 0, 0);
    lv_obj_set_style_bg_opa(target_container, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_clear_flag(target_container, LV_OBJ_FLAG_SCROLLABLE);

    touch_calibration_target = lv_obj_create(target_container);
    lv_obj_remove_style_all(touch_calibration_target);
    lv_obj_set_size(touch_calibration_target, 28, 28);
    lv_obj_set_style_radius(touch_calibration_target, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_border_color(touch_calibration_target, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_set_style_border_width(touch_calibration_target, 3, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(touch_calibration_target, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_add_flag(touch_calibration_target, LV_OBJ_FLAG_HIDDEN);

    lv_obj_t *cross_h = lv_obj_create(touch_calibration_target);
    lv_obj_remove_style_all(cross_h);
    lv_obj_set_size(cross_h, 18, 2);
    lv_obj_set_style_bg_color(cross_h, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_center(cross_h);

    lv_obj_t *cross_v = lv_obj_create(touch_calibration_target);
    lv_obj_remove_style_all(cross_v);
    lv_obj_set_size(cross_v, 2, 18);
    lv_obj_set_style_bg_color(cross_v, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_MAIN);
    lv_obj_center(cross_v);

    touch_calibration_dot = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(touch_calibration_dot);
    lv_obj_set_size(touch_calibration_dot, 12, 12);
    lv_obj_set_style_radius(touch_calibration_dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(touch_calibration_dot, lv_color_hex(0xFF3B30), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(touch_calibration_dot, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_border_color(touch_calibration_dot, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_width(touch_calibration_dot, 2, LV_PART_MAIN);
    lv_obj_add_flag(touch_calibration_dot, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_IGNORE_LAYOUT);

    lv_coord_t intro_y = touch_calibration_screen_h - 44;
    lv_coord_t action_y = touch_calibration_screen_h - 82;
    lv_coord_t action_x = (touch_calibration_screen_w - (74 * 3 + 8 * 2)) / 2;

    touch_calibration_primary_btn = create_action_button(lv_scr_act(), "Start", 86, 34);
    lv_obj_set_pos(touch_calibration_primary_btn, 16, intro_y);
    lv_obj_add_event_cb(touch_calibration_primary_btn, touch_calibration_start_event_cb, LV_EVENT_CLICKED, NULL);

    touch_calibration_secondary_btn = create_action_button(lv_scr_act(), "Back", 86, 34);
    lv_obj_set_pos(touch_calibration_secondary_btn, touch_calibration_screen_w - 16 - 86, intro_y);
    lv_obj_add_event_cb(touch_calibration_secondary_btn, touch_calibration_back_event_cb, LV_EVENT_CLICKED, NULL);

    touch_calibration_tertiary_btn = create_action_button(lv_scr_act(), "Retry", 74, 34);
    lv_obj_set_pos(touch_calibration_tertiary_btn, action_x, action_y);
    lv_obj_add_event_cb(touch_calibration_tertiary_btn, touch_calibration_retry_event_cb, LV_EVENT_CLICKED, NULL);

    touch_calibration_quaternary_btn = create_action_button(lv_scr_act(), "Reset", 74, 34);
    lv_obj_set_pos(touch_calibration_quaternary_btn, action_x + 82, action_y);
    lv_obj_add_event_cb(touch_calibration_quaternary_btn, touch_calibration_reset_default_event_cb, LV_EVENT_CLICKED, NULL);

    touch_calibration_quinary_btn = create_action_button(lv_scr_act(), "Save", 74, 34);
    lv_obj_set_pos(touch_calibration_quinary_btn, action_x + 164, action_y);
    lv_obj_add_event_cb(touch_calibration_quinary_btn, touch_calibration_save_event_cb, LV_EVENT_CLICKED, NULL);

    touch_calibration_reset_samples();
    touch_calibration_enter_step(TOUCH_CAL_STEP_INTRO);
    touch_calibration_timer = lv_timer_create(touch_calibration_timer_cb, 33, NULL);
}

void setting_init(void)
{
    ui_set_current_page(UI_PAGE_SETTING);
    lv_obj_clean(lv_scr_act());
    apply_page_bg();
    configure_swipe_back_for_current_screen(false);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

    if (setting_touch_debug_timer)
    {
        lv_timer_del(setting_touch_debug_timer);
        setting_touch_debug_timer = NULL;
    }
    setting_touch_debug_dot = NULL;
    setting_touch_debug_label = NULL;

    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "Settings");
    lv_obj_set_style_text_color(title, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(title, 14, 12);

    setting_touch_debug_label = lv_label_create(lv_scr_act());
    lv_label_set_text(setting_touch_debug_label, "Touch: released");
    lv_obj_set_style_text_color(setting_touch_debug_label, lv_color_hex(UI_COLOR_MUTED), 0);
    lv_obj_set_style_text_font(setting_touch_debug_label, &lv_font_montserrat_10, 0);
    lv_obj_set_pos(setting_touch_debug_label, 14, 34);

    setting_touch_debug_dot = lv_obj_create(lv_scr_act());
    lv_obj_remove_style_all(setting_touch_debug_dot);
    lv_obj_set_size(setting_touch_debug_dot, 12, 12);
    lv_obj_set_style_radius(setting_touch_debug_dot, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_bg_color(setting_touch_debug_dot, lv_color_hex(0xFF3B30), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(setting_touch_debug_dot, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_border_color(setting_touch_debug_dot, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_border_width(setting_touch_debug_dot, 2, LV_PART_MAIN);
    lv_obj_add_flag(setting_touch_debug_dot, LV_OBJ_FLAG_HIDDEN | LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_move_foreground(setting_touch_debug_dot);

    setting_touch_debug_timer = lv_timer_create(setting_touch_debug_timer_cb, 33, NULL);

    lv_obj_t *content = lv_obj_create(lv_scr_act());
    lv_obj_remove_style(content, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(content, 268, 224);
    lv_obj_set_pos(content, 10, 50);
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(content, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(content, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(content, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(content, 8, LV_PART_MAIN);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);
    lv_obj_add_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *theme_card = create_settings_card(content, 54);
    lv_obj_set_layout(theme_card, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(theme_card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(theme_card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *theme_left = lv_obj_create(theme_card);
    lv_obj_remove_style_all(theme_left);
    lv_obj_set_size(theme_left, 154, 26);
    lv_obj_set_style_bg_opa(theme_left, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(theme_left, 0, LV_PART_MAIN);
    lv_obj_set_layout(theme_left, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(theme_left, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(theme_left, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(theme_left, 10, LV_PART_MAIN);
    lv_obj_clear_flag(theme_left, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *theme_title = lv_label_create(theme_left);
    lv_label_set_text(theme_title, "Theme");
    lv_obj_set_style_text_color(theme_title, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(theme_title, &lv_font_montserrat_18, 0);

    lv_obj_t *theme_status_chip = lv_obj_create(theme_left);
    lv_obj_remove_style(theme_status_chip, 0, LV_PART_SCROLLBAR);
    lv_obj_set_size(theme_status_chip, 74, 24);
    lv_obj_set_style_radius(theme_status_chip, 12, LV_PART_MAIN);
    lv_obj_set_style_bg_color(theme_status_chip, lv_color_hex(UI_COLOR_PRIMARY_SOFT), LV_PART_MAIN);
    lv_obj_set_style_border_width(theme_status_chip, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(theme_status_chip, 0, LV_PART_MAIN);
    lv_obj_clear_flag(theme_status_chip, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *theme_status_label = lv_label_create(theme_status_chip);
    lv_label_set_text(theme_status_label, g_ui_theme_mode == UI_THEME_DARK ? "Dark" : "Light");
    lv_obj_set_style_text_color(theme_status_label, lv_color_hex(UI_COLOR_PRIMARY), 0);
    lv_obj_set_style_text_font(theme_status_label, &lv_font_montserrat_14, 0);
    lv_obj_center(theme_status_label);

    lv_obj_t *theme_switch = lv_switch_create(theme_card);
    lv_obj_set_style_bg_color(theme_switch, lv_color_hex(UI_COLOR_PRIMARY_SOFT), LV_PART_MAIN);
    lv_obj_set_style_bg_color(theme_switch, lv_color_hex(UI_COLOR_PRIMARY), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(theme_switch, lv_color_hex(UI_COLOR_CARD), LV_PART_KNOB);
    if (g_ui_theme_mode == UI_THEME_DARK)
    {
        lv_obj_add_state(theme_switch, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(theme_switch, setting_theme_switch_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *volume_card = create_settings_card(content, 96);

    lv_obj_t *volume_header = lv_obj_create(volume_card);
    lv_obj_remove_style_all(volume_header);
    lv_obj_set_size(volume_header, LV_PCT(100), 26);
    lv_obj_set_pos(volume_header, 0, -6);
    lv_obj_set_style_bg_opa(volume_header, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(volume_header, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(volume_header, 0, LV_PART_MAIN);
    lv_obj_set_layout(volume_header, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(volume_header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(volume_header, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *volume_title = lv_label_create(volume_header);
    lv_label_set_text(volume_title, "Buzzer Volume");
    lv_obj_set_style_text_color(volume_title, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(volume_title, &lv_font_montserrat_18, 0);

    lv_obj_t *value_label = lv_label_create(volume_header);
    lv_label_set_text_fmt(value_label, "L%d", buzzer_volume_level);
    lv_obj_set_style_text_color(value_label, lv_color_hex(UI_COLOR_PRIMARY), 0);
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_20, 0);

    static lv_style_t setting_volume_btn_style;
    static lv_style_t setting_volume_btn_focus_style;
    static bool setting_volume_btn_style_ready = false;
    if (!setting_volume_btn_style_ready)
    {
        lv_style_init(&setting_volume_btn_style);
        lv_style_set_bg_color(&setting_volume_btn_style, lv_color_hex(UI_COLOR_CARD));
        lv_style_set_border_color(&setting_volume_btn_style, lv_color_hex(UI_COLOR_BORDER));
        lv_style_set_border_width(&setting_volume_btn_style, 2);
        lv_style_set_radius(&setting_volume_btn_style, 10);
        lv_style_set_shadow_width(&setting_volume_btn_style, 0);

        lv_style_init(&setting_volume_btn_focus_style);
        lv_style_set_border_color(&setting_volume_btn_focus_style, lv_color_hex(UI_COLOR_PRIMARY));
        lv_style_set_border_width(&setting_volume_btn_focus_style, 3);

        setting_volume_btn_style_ready = true;
    }
    else
    {
        lv_style_set_bg_color(&setting_volume_btn_style, lv_color_hex(UI_COLOR_CARD));
        lv_style_set_border_color(&setting_volume_btn_style, lv_color_hex(UI_COLOR_BORDER));
        lv_style_set_border_width(&setting_volume_btn_style, 2);
        lv_style_set_radius(&setting_volume_btn_style, 10);

        lv_style_set_border_color(&setting_volume_btn_focus_style, lv_color_hex(UI_COLOR_PRIMARY));
        lv_style_set_border_width(&setting_volume_btn_focus_style, 3);
    }

    lv_obj_t *option_row = lv_obj_create(volume_card);
    lv_obj_remove_style_all(option_row);
    lv_obj_set_size(option_row, 232, 36);
    lv_obj_align_to(option_row, volume_header, LV_ALIGN_OUT_BOTTOM_MID, 0, 8);
    lv_obj_set_style_bg_opa(option_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(option_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(option_row, LV_OBJ_FLAG_SCROLLABLE);

    for (int idx = 0; idx < 4; idx++)
    {
        int level = idx + 1;
        bool is_active = level == buzzer_volume_level;

        lv_obj_t *segment_btn = lv_btn_create(option_row);
        lv_obj_set_size(segment_btn, 52, 36);
        lv_obj_set_pos(segment_btn, idx * 60, 0);
        lv_obj_add_style(segment_btn, &setting_volume_btn_style, 0);
        lv_obj_add_style(segment_btn, &setting_volume_btn_focus_style, LV_STATE_FOCUSED);
        lv_obj_set_style_bg_color(segment_btn, lv_color_hex(is_active ? UI_COLOR_PRIMARY : UI_COLOR_CARD), LV_PART_MAIN);
        lv_obj_set_style_border_color(segment_btn, lv_color_hex(is_active ? UI_COLOR_PRIMARY : UI_COLOR_BORDER), LV_PART_MAIN);
        lv_obj_set_style_border_width(segment_btn, is_active ? 3 : 2, LV_PART_MAIN);
        lv_obj_set_ext_click_area(segment_btn, 8);
        lv_obj_clear_flag(segment_btn, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_user_data(segment_btn, value_label);
        lv_obj_add_event_cb(segment_btn, setting_volume_option_event_cb, LV_EVENT_CLICKED, (void *)(intptr_t)level);

        lv_obj_t *segment_label = lv_label_create(segment_btn);
        lv_label_set_text_fmt(segment_label, "L%d", level);
        lv_obj_set_style_text_color(segment_label, lv_color_hex(is_active ? UI_COLOR_CARD : UI_COLOR_TEXT), 0);
        lv_obj_set_style_text_font(segment_label, &lv_font_montserrat_16, 0);
        lv_obj_center(segment_label);
    }

    lv_obj_t *touch_calibration_card = create_settings_card(content, 68);

    lv_obj_t *touch_calibration_header = lv_obj_create(touch_calibration_card);
    lv_obj_remove_style_all(touch_calibration_header);
    lv_obj_set_size(touch_calibration_header, 154, 44);
    lv_obj_set_pos(touch_calibration_header, 0, 0);
    lv_obj_set_style_bg_opa(touch_calibration_header, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(touch_calibration_header, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(touch_calibration_header, 0, LV_PART_MAIN);
    lv_obj_set_layout(touch_calibration_header, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(touch_calibration_header, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(touch_calibration_header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(touch_calibration_header, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *touch_calibration_title = lv_label_create(touch_calibration_header);
    lv_label_set_text(touch_calibration_title, "Touch Calibration");
    lv_obj_set_style_text_color(touch_calibration_title, lv_color_hex(UI_COLOR_TEXT), 0);
    lv_obj_set_style_text_font(touch_calibration_title, &lv_font_montserrat_18, 0);

    lv_obj_t *touch_calibration_desc = lv_label_create(touch_calibration_header);
    lv_label_set_text(touch_calibration_desc, "Fix touch accuracy");
    lv_obj_set_style_text_color(touch_calibration_desc, lv_color_hex(UI_COLOR_MUTED), 0);
    lv_obj_set_style_text_font(touch_calibration_desc, &lv_font_montserrat_12, 0);

    lv_obj_t *touch_calibration_start_btn = create_action_button(touch_calibration_card, "Start", 78, 36);
    lv_obj_align(touch_calibration_start_btn, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_ext_click_area(touch_calibration_start_btn, 8);
    lv_obj_add_event_cb(touch_calibration_start_btn, touch_calibration_open_event_cb, LV_EVENT_CLICKED, NULL);

    configure_swipe_back_for_current_screen(false);
}

void ui_init(void)
{
    // 设置显示主题
    lv_theme_default_init(NULL, lv_color_hex(UI_COLOR_BG), lv_color_hex(UI_COLOR_PRIMARY), g_ui_theme_mode == UI_THEME_DARK, NULL);
    create_boot_animation();

    // lv_obj_set_style_border_color(lv_scr_act(), lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_width(lv_scr_act(), 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_opa(lv_scr_act(), LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);

    // lv_obj_t* obj = lv_obj_create(lv_scr_act());
    // lv_obj_set_style_border_color(obj, lv_color_white(), LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_width(obj, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_style_border_opa(obj, LV_OPA_100, LV_PART_MAIN | LV_STATE_DEFAULT);
    // lv_obj_set_size(obj, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));

    // lv_obj_t *p_label = lv_label_create(obj);
    // lv_obj_align(p_label, LV_ALIGN_CENTER, 0, 0);
    // lv_label_set_text(p_label, "Hello World");
}


