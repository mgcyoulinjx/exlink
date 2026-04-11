#include <Arduino.h>
#include "ui.h"
#include "event.h"

int saved_scroll_y = 0;
int saved_focused_index = -1;

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
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
    ui_Image1 = lv_img_create(lv_scr_act());
    lv_img_set_src(ui_Image1, &ui_img_game3_png);
    lv_obj_set_width(ui_Image1, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Image1, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_Image1, 45, 50);
    lv_obj_add_flag(ui_Image1, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_Image1, LV_OBJ_FLAG_SCROLLABLE);

    // 初始化动画
    lv_anim_t anim1;
    lv_anim_init(&anim1);                                // 初始化动画结构体
    lv_anim_set_exec_cb(&anim1, anim_cb1);               // 设置动画回调函数
    lv_anim_set_var(&anim1, ui_Image1);                  // 设置动画作用的对象
    lv_anim_set_time(&anim1, 450);                       // 设置动画时间
    lv_anim_set_values(&anim1, 12, 60);                  // 设置运动轨迹
    lv_anim_set_path_cb(&anim1, lv_anim_path_overshoot); // 使用“overshoot”路径效果，使动画更加生动

    lv_obj_t *line = lv_line_create(lv_scr_act());

    // 设置线条的起始和结束点
    static lv_point_t points[] = {{LINE_X1, LINE_Y1}, {LINE_X2, LINE_Y2}};
    lv_line_set_points(line, points, 2);

    // 设置线条的样式
    lv_obj_set_style_line_width(line, 10, 0);
    lv_obj_set_style_line_color(line, lv_color_hex(0xFF0000), 0); // 红色
    lv_obj_set_style_line_rounded(line, true, LV_PART_MAIN);
    lv_anim_t anim2;
    lv_anim_init(&anim2);                                      // 初始化动画结构体
    lv_anim_set_var(&anim2, line);                             // 设置动画作用的对象
    lv_anim_set_exec_cb(&anim2, (lv_anim_exec_xcb_t)anim_cb2); // 设置动画回调函数
    lv_anim_set_values(&anim2, -100, 15);                      // 设置运动轨迹
    lv_anim_set_time(&anim2, 450);                             // 设置动画时间
    lv_anim_set_path_cb(&anim2, lv_anim_path_overshoot);       // 动画路径效果

    ui_Image2 = lv_img_create(lv_scr_act());
    lv_img_set_src(ui_Image2, &Exlink_png);
    lv_obj_set_width(ui_Image2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Image2, LV_SIZE_CONTENT);
    lv_obj_set_pos(ui_Image2, 35, 150);
    lv_obj_add_flag(ui_Image2, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_Image2, LV_OBJ_FLAG_SCROLLABLE);
    lv_anim_t anim3;
    lv_anim_init(&anim3);                                      // 初始化动画结构体
    lv_anim_set_var(&anim3, ui_Image2);                        // 设置动画作用的对象
    lv_anim_set_exec_cb(&anim3, (lv_anim_exec_xcb_t)anim_cb2); // 设置动画回调函数
    lv_anim_set_values(&anim3, 240, 150);                      // 设置运动轨迹
    lv_anim_set_time(&anim3, 450);                             // 动画时间
    lv_anim_set_path_cb(&anim3, lv_anim_path_overshoot);       // 动画路径效果

    lv_obj_t *label = lv_label_create(lv_scr_act()); /* 创建标签 */
    lv_label_set_text(label, "");                    /* 设置文本内容 */
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);    /* 设置位置 */
    lv_anim_t anim4;
    lv_anim_init(&anim4);           // 初始化动画结构体
    lv_anim_set_var(&anim4, label); // 设置动画作用的对象
    lv_anim_set_ready_cb(&anim4, anim_end_callback);
    lv_anim_set_exec_cb(&anim4, (lv_anim_exec_xcb_t)anim_cb2); // 设置动画回调函数
    lv_anim_set_values(&anim4, 30, 30);                        // 设置动画轨迹
    lv_anim_set_time(&anim4, 2000);                            // 设置动画时间为 2000 毫秒
    lv_anim_set_path_cb(&anim4, lv_anim_path_overshoot);       // 动画路径效果
    // lv_anim_start(&anim2); // 启动动画

    // 设置动画时间线
    lv_anim_timeline_t *anim_timeline = lv_anim_timeline_create();
    lv_anim_timeline_add(anim_timeline, 0, &anim1);
    lv_anim_timeline_add(anim_timeline, 150, &anim2);
    lv_anim_timeline_add(anim_timeline, 200, &anim3);
    lv_anim_timeline_add(anim_timeline, 200, &anim4);
    // 启动动画时间线
    lv_anim_timeline_start(anim_timeline);
}

void ui_Screen1_screen_init(void) // 创建主界面
{
    lv_obj_clean(lv_scr_act());
    set_swipe_back_enabled(true);
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
    lv_obj_clear_flag(lv_scr_act(), LV_OBJ_FLAG_SCROLLABLE);

    panel = lv_obj_create(lv_scr_act());
    lv_obj_set_size(panel, 230, 240);
    lv_obj_set_pos(panel, 5, 0);
    lv_obj_set_style_bg_color(panel, lv_color_hex(0x000000), 0);
    lv_obj_set_style_anim_time(panel, 0, 0);
    lv_obj_set_style_border_width(panel, 0, 0);
    lv_obj_remove_style(panel, 0, LV_PART_SCROLLBAR);
    lv_obj_set_flex_flow(panel, LV_FLEX_FLOW_COLUMN); // 垂直排列子对象
    lv_obj_set_scroll_dir(panel, LV_DIR_VER);
    lv_obj_clear_flag(panel, LV_OBJ_FLAG_SCROLL_ON_FOCUS);

    lv_style_init(&style_rect);
    // 设置边框的颜色和粗细
    lv_style_set_border_color(&style_rect, lv_color_hex(0xFF0000)); // 红色边框
    lv_style_set_border_width(&style_rect, 3);                      // 5 像素粗细
    lv_style_set_bg_color(&style_rect, lv_color_hex(0x000000));     // 白色背景
    lv_style_set_radius(&style_rect, 20);                           // 圆角半径

    // 设置聚焦状的边框
    static lv_style_t focused_style;
    lv_style_init(&focused_style);
    lv_style_set_border_color(&focused_style, lv_color_hex(0xFFD700));
    lv_style_set_border_width(&focused_style, 5);

    btn1 = lv_btn_create(panel);
    lv_obj_set_size(btn1, 215, 80);
    lv_obj_align(btn1, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn1, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn1, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn1, btn1_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn1_label = lv_label_create(btn1);
    lv_obj_align(btn1_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn1_label, "Pin Map");
    lv_obj_set_style_text_font(btn1_label, &lv_font_montserrat_20, 0);
    lv_obj_t *img1 = lv_img_create(btn1);
    lv_img_set_src(img1, &pinmap_png);                       // Replace with your image variable or path
    lv_obj_set_size(img1, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img1, LV_ALIGN_OUT_RIGHT_MID, 126, 0);      // Center the image within the button

    btn2 = lv_btn_create(panel);
    lv_obj_set_size(btn2, 215, 80);
    lv_obj_align(btn2, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn2, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn2, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn2, btn2_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn2_label = lv_label_create(btn2);
    lv_obj_align(btn2_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn2_label, "DC POWER");
    lv_obj_set_style_text_font(btn2_label, &lv_font_montserrat_20, 0);
    lv_obj_t *img2 = lv_img_create(btn2);
    lv_img_set_src(img2, &power_png);                        // Replace with your image variable or path
    lv_obj_set_size(img2, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img2, LV_ALIGN_OUT_RIGHT_MID, 126, 0);      // Center the image within the button

    btn3 = lv_btn_create(panel);
    lv_obj_set_size(btn3, 215, 80);
    lv_obj_align(btn3, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn3, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_event_cb(btn3, btn3_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_style(btn3, &focused_style, LV_STATE_FOCUSED);
    lv_obj_t *btn3_label = lv_label_create(btn3);
    lv_obj_align(btn3_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn3_label, "PWM OUT");
    lv_obj_set_style_text_font(btn3_label, &lv_font_montserrat_20, 0);
    lv_obj_t *img3 = lv_img_create(btn3);
    lv_img_set_src(img3, &pwm_png);                          // Replace with your image variable or path
    lv_obj_set_size(img3, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img3, LV_ALIGN_OUT_RIGHT_MID, 126, 0);      // Center the image within the button

    btn4 = lv_btn_create(panel);
    lv_obj_set_size(btn4, 215, 80);
    lv_obj_align(btn4, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn4, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn4, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn4, btn4_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn4_label = lv_label_create(btn4);
    lv_obj_align(btn4_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn4_label, "UART HELPER");
    lv_obj_set_style_text_font(btn4_label, &lv_font_montserrat_16, 0);
    lv_obj_t *img4 = lv_img_create(btn4);
    lv_img_set_src(img4, &usarthelper_png);                  // Replace with your image variable or path
    lv_obj_set_size(img4, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img4, LV_ALIGN_OUT_RIGHT_MID, 126, 0);      // Center the image within the button

    btn5 = lv_btn_create(panel);
    lv_obj_set_size(btn5, 215, 80);
    lv_obj_align(btn5, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn5, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn5, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn5, btn5_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn5_label = lv_label_create(btn5);
    lv_obj_align(btn5_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn5_label, "I2C SCAN");
    lv_obj_set_style_text_font(btn5_label, &lv_font_montserrat_20, 0);
    lv_obj_t *img5 = lv_img_create(btn5);
    lv_img_set_src(img5, &i2c_png);                          // Replace with your image variable or path
    lv_obj_set_size(img5, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img5, LV_ALIGN_OUT_RIGHT_MID, 126, 0);      // Center the image within the button

    btn6 = lv_btn_create(panel);
    lv_obj_set_size(btn6, 215, 80);
    lv_obj_align(btn6, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn6, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn6, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn6, btn6_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn6, btn6_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn6_label = lv_label_create(btn6);
    lv_obj_align(btn6_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn6_label, "Voltmeter");
    lv_obj_set_style_text_font(btn6_label, &lv_font_montserrat_20, 0);
    lv_obj_t *img6 = lv_img_create(btn6);
    lv_img_set_src(img6, &voltmeter_png);                    // Replace with your image variable or path
    lv_obj_set_size(img6, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img6, LV_ALIGN_OUT_RIGHT_MID, 126, 0);      // Center the image within the button

    btn7 = lv_btn_create(panel);
    lv_obj_set_size(btn7, 215, 80);
    lv_obj_align(btn7, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn7, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn7, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn7, btn7_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn7_label = lv_label_create(btn7);
    lv_obj_align(btn7_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn7_label, "Simple DSO");
    lv_obj_set_style_text_font(btn7_label, &lv_font_montserrat_20, 0);
    lv_obj_t *img7 = lv_img_create(btn7);
    lv_img_set_src(img7, &DSO_png);                          // Replace with your image variable or path
    lv_obj_set_size(img7, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img7, LV_ALIGN_OUT_RIGHT_MID, 126, 0);      // Center the image within the button

    btn8 = lv_btn_create(panel);
    lv_obj_set_size(btn8, 215, 80);
    lv_obj_align(btn8, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn8, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn8, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn8, btn8_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn8_label = lv_label_create(btn8);
    lv_obj_align(btn8_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn8_label, "BLE UART");
    lv_obj_set_style_text_font(btn8_label, &lv_font_montserrat_20, 0);
    lv_obj_t *img8 = lv_img_create(btn8);
    lv_img_set_src(img8, &wireless_png);                     // Replace with your image variable or path
    lv_obj_set_size(img8, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img8, LV_ALIGN_OUT_RIGHT_MID, 126, 0);      // Center the image within the button

    btn9 = lv_btn_create(panel);
    lv_obj_set_size(btn9, 215, 80);
    lv_obj_align(btn9, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn9, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn9, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn9, btn9_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn9_label = lv_label_create(btn9);
    lv_obj_align(btn9_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn9_label, "FRE Count");
    lv_obj_set_style_text_font(btn9_label, &lv_font_montserrat_20, 0);
    lv_obj_t *img9 = lv_img_create(btn9);
    lv_img_set_src(img9, &FREcounter_png);                   // Replace with your image variable or path
    lv_obj_set_size(img9, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img9, LV_ALIGN_OUT_RIGHT_MID, 126, 0);      // Center the image within the button

    btn10 = lv_btn_create(panel);
    lv_obj_set_size(btn10, 215, 80);
    lv_obj_align(btn10, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn10, &style_rect, 0); // 将样式应用到矩形中
    lv_obj_add_style(btn10, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn10, btn10_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn10_label = lv_label_create(btn10);
    lv_obj_align(btn10_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn10_label, "Device INFO");
    lv_obj_set_style_text_font(btn10_label, &lv_font_montserrat_20, 0);
    lv_obj_t *img10 = lv_img_create(btn10);
    lv_img_set_src(img10, &readme_png);                       // Replace with your image variable or path
    lv_obj_set_size(img10, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img10, LV_ALIGN_OUT_RIGHT_MID, 126, 0);      // Center the image within the button

    btn11 = lv_btn_create(panel);
    lv_obj_set_size(btn11, 215, 80);
    lv_obj_align(btn11, LV_ALIGN_OUT_RIGHT_TOP, 0, 0);
    lv_obj_add_style(btn11, &style_rect, 0);
    lv_obj_add_style(btn11, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(btn11, btn11_event_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn11_label = lv_label_create(btn11);
    lv_obj_align(btn11_label, LV_ALIGN_OUT_RIGHT_MID, 0, 18);
    lv_label_set_text(btn11_label, "Setting");
    lv_obj_set_style_text_font(btn11_label, &lv_font_montserrat_20, 0);
    lv_obj_t *btn11_icon = lv_img_create(btn11);
    lv_img_set_src(btn11_icon, &setting_png);
    lv_obj_set_size(btn11_icon, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_align(btn11_icon, LV_ALIGN_OUT_RIGHT_MID, 126, -3);

    lv_style_init(&style_rect);
    // 设置边框的颜色和粗细
    lv_style_set_border_color(&style_rect, lv_color_hex(0xFF0000)); // 红色边框
    lv_style_set_border_width(&style_rect, 3);                      // 5 像素粗细
    lv_style_set_bg_color(&style_rect, lv_color_hex(0x000000));     // 白色背景
    lv_style_set_radius(&style_rect, 20);                           // 圆角半径

    slider = lv_slider_create(lv_scr_act());
    lv_obj_set_size(slider, 15, 90);
    lv_obj_set_pos(slider, 250, 30);
    lv_slider_set_range(slider, 1, 11);
    lv_slider_set_value(slider, get_saved_slider_value(), LV_ANIM_OFF);
    lv_obj_set_style_bg_opa(slider, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0xFF0000), LV_PART_KNOB);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x333333), LV_PART_MAIN);
    lv_obj_set_style_bg_color(slider, lv_color_hex(0x333333), LV_PART_INDICATOR);
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    slider_update_timer = lv_timer_create(update_slider_timer, 300, NULL);

    /*
        lv_obj_t *bat_volt_label = lv_label_create(lv_scr_act());
        lv_label_set_text(bat_volt_label, "100");
        lv_obj_set_pos(bat_volt_label, 251, 220);
        lv_obj_set_style_text_color(bat_volt_label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(bat_volt_label, &lv_font_montserrat_10, 0);

        lv_obj_t *bat_volt_label2 = lv_label_create(lv_scr_act());
        lv_obj_set_pos(bat_volt_label2, 275, 220);
        lv_label_set_text(bat_volt_label2, "%");
        lv_obj_set_style_text_color(bat_volt_label2, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(bat_volt_label2, &lv_font_montserrat_14, 0);
       */
    bat_label = lv_label_create(lv_scr_act());
    lv_label_set_text(bat_label, LV_SYMBOL_BATTERY_EMPTY);
    lv_obj_set_pos(bat_label, 241, 196);
    lv_obj_set_style_text_color(bat_label, lv_color_hex(0x32CD32), 0);
    lv_obj_set_style_text_font(bat_label, &lv_font_montserrat_26, 0);

    bat_voltage_label = lv_label_create(lv_scr_act());
    lv_label_set_text(bat_voltage_label, "0.00V");
    lv_obj_set_style_text_color(bat_voltage_label, lv_color_hex(0xFFFFFF), 0);
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

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);

    lv_obj_t *name_label1 = lv_label_create(lv_scr_act());         // 将文本标签添加到圆角矩形中
    lv_label_set_text(name_label1, "#FFD700 ROW1:MCU and power#"); // 设置文本内容
    lv_obj_set_style_text_font(name_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(name_label1, 10, 26);
    lv_label_set_recolor(name_label1, true);

    lv_obj_t *name_label2 = lv_label_create(lv_scr_act());           // 将文本标签添加到圆角矩形中
    lv_label_set_text(name_label2, "#00FFFF ROW2:DLA and DAPlink#"); // 设置文本内容
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
    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);

    ina266_flag = 1;
    digitalWrite(1, LOW);

    lv_obj_t *volt = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(volt, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_font(volt, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(volt, 20, 10);

    lv_obj_t *cur = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(cur, lv_color_hex(0x00FF7F), 0);
    lv_obj_set_style_text_font(cur, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(cur, 20, 35);

    lv_obj_t *watt = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(watt, lv_color_hex(0x00FFFF), 0);
    lv_obj_set_style_text_font(watt, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(watt, 20, 60);

    updatelabel_timer1 = lv_timer_create(update_label_timer1, 100, volt);
    updatelabel_timer2 = lv_timer_create(update_label_timer2, 100, cur);
    updatelabel_timer3 = lv_timer_create(update_label_timer3, 100, watt);

    lv_obj_t *V = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(V, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(V, 106, 10);
    lv_label_set_text(V, "#FF0000 V#"); // 设置文本内容
    lv_label_set_recolor(V, true);

    lv_obj_t *A = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(A, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(A, 106, 35);
    lv_label_set_text(A, "#00FF7F A#"); // 设置文本内容
    lv_label_set_recolor(A, true);

    lv_obj_t *W = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(W, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(W, 101, 60);
    lv_label_set_text(W, "#00FFFF W#"); // 设置文本内容
    lv_label_set_recolor(W, true);

    volt_chart = lv_chart_create(lv_scr_act());
    lv_obj_set_style_bg_color(volt_chart, lv_color_hex(0x303030), LV_PART_MAIN);
    lv_obj_set_style_line_color(volt_chart, lv_color_hex(0x696969), LV_PART_MAIN);
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
    lv_obj_set_style_bg_color(cur_chart, lv_color_hex(0x303030), LV_PART_MAIN);
    lv_obj_set_style_line_color(cur_chart, lv_color_hex(0x696969), LV_PART_MAIN);
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
    lv_style_set_border_color(&focused_style, lv_color_hex(0xFFD700));
    lv_style_set_border_width(&focused_style, 3);

    static lv_style_t powerbtn_style;
    lv_style_init(&powerbtn_style);
    lv_style_set_bg_color(&powerbtn_style, lv_color_hex(0x000000));
    lv_style_set_border_color(&powerbtn_style, lv_color_hex(0x808080));
    lv_style_set_border_width(&powerbtn_style, 2);

    lv_obj_t *poweron = lv_btn_create(lv_scr_act());
    lv_obj_set_size(poweron, 41, 83);
    lv_obj_set_pos(poweron, 229, 10);
    lv_obj_add_style(poweron, &powerbtn_style, 0); // 将样式应用到矩形中
    lv_obj_add_style(poweron, &focused_style, LV_STATE_FOCUSED);
    lv_obj_add_event_cb(poweron, poweronbtn_event_cb, LV_EVENT_CLICKED, NULL);
    poweron_label = lv_label_create(poweron);
    lv_obj_set_style_text_color(poweron_label, lv_color_hex(0xFF0000), 0);
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
    lv_obj_set_style_text_color(VUP_label, lv_color_hex(0x87CEEB), 0);
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
    lv_obj_set_style_text_color(VDOWN_label, lv_color_hex(0x87CEEB), 0);
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
    lv_obj_set_style_text_color(V11_label, lv_color_hex(0xFFD700), 0);
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
    lv_obj_set_style_text_color(V5_label, lv_color_hex(0xFFD700), 0);
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
    lv_obj_set_style_text_color(V3_label, lv_color_hex(0xFFD700), 0);
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
    lv_label_set_text(volt_label1, "#FFD700 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    configure_swipe_back_for_current_screen(true);
}

void pwm_init(void)
{

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);


    lv_obj_t *pwm = lv_img_create(lv_scr_act());
    lv_img_set_src(pwm, &pwmint_png);                       // Replace with your image variable or path
    lv_obj_set_size(pwm, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_set_pos(pwm, 40, 110);

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
    lv_label_set_text(volt_label1, "#FFD700 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    fre = lv_textarea_create(lv_scr_act()); /* 创建文本框*/
    lv_textarea_set_placeholder_text(fre, "0-100K");
    lv_obj_set_style_text_font(fre, &lv_font_montserrat_20, LV_PART_MAIN); /* 设置字体 */
    lv_textarea_set_one_line(fre, true);                                   /* 设置单行模式 */
    lv_obj_set_size(fre, 100, 45);
    lv_obj_set_pos(fre, 72, 10);

    duty = lv_textarea_create(lv_scr_act()); /* 创建文本框*/
    lv_textarea_set_placeholder_text(duty, "0-100");
    lv_obj_set_style_text_font(duty, &lv_font_montserrat_20, LV_PART_MAIN); /* 设置字体 */
    lv_textarea_set_one_line(duty, true);                                   /* 设置单行模式 */
    lv_obj_set_size(duty, 100, 45);
    lv_obj_set_pos(duty, 72, 60);

    lv_obj_t *fre_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(fre_label1, "#FFD700 FRE#");        // 设置文本内容
    lv_obj_set_style_text_font(fre_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(fre_label1, 10, 20);
    lv_label_set_recolor(fre_label1, true);

    lv_obj_t *duty_label1 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(duty_label1, "#87CEFA DUTY#");       // 设置文本内容
    lv_obj_set_style_text_font(duty_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(duty_label1, 10, 70);
    lv_label_set_recolor(duty_label1, true);

    lv_obj_t *fre_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(fre_label2, "#FFD700 Hz#");         // 设置文本内容
    lv_obj_set_style_text_font(fre_label2, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(fre_label2, 180, 20);
    lv_label_set_recolor(fre_label2, true);

    lv_obj_t *duty_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(duty_label2, "#87CEFA %#");          // 设置文本内容
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
    lv_style_set_bg_color(&pwm_style, lv_color_hex(0xFF0000));
    lv_style_set_border_color(&pwm_style, lv_color_hex(0x000000));
    lv_style_set_border_width(&pwm_style, 2);

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

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
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
    lv_label_set_text(volt_label1, "#FFD700 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    lv_obj_t *volt_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label2, "#00FF7F UART#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label2, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(volt_label2, 15, 32);
    lv_label_set_recolor(volt_label2, true);

    lv_obj_t *volt_label3 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label3, "#00FFFF baudRate#");   // 设置文本内容
    lv_obj_set_style_text_font(volt_label3, &lv_font_montserrat_18, 0);
    lv_obj_set_pos(volt_label3, 170, 5);
    lv_label_set_recolor(volt_label3, true);

    uart_extarea = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(uart_extarea, 269, 126);
    lv_obj_set_pos(uart_extarea, 6, 75);
    lv_textarea_set_max_length(uart_extarea, 1024);
    lv_textarea_set_placeholder_text(uart_extarea, "USART monitor output");

    uart_list = lv_dropdown_create(lv_scr_act());
    lv_obj_set_size(uart_list, 105, 40);
    lv_obj_set_pos(uart_list, 170, 30);
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
    lv_style_set_bg_color(&uart_style, lv_color_hex(0xFF0000));
    lv_style_set_border_color(&uart_style, lv_color_hex(0x000000));
    lv_style_set_border_width(&uart_style, 2);

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

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
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
    lv_label_set_text(volt_label1, "#FFD700 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    lv_obj_t *volt_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label2, "#00FFFF I2C Device#"); // 设置文本内容
    lv_obj_set_style_text_font(volt_label2, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(volt_label2, 15, 26);
    lv_label_set_recolor(volt_label2, true);

    i2c_extarea = lv_textarea_create(lv_scr_act());
    lv_obj_set_size(i2c_extarea, 269, 120);
    lv_obj_set_pos(i2c_extarea, 6, 75);

    static lv_style_t i2con_style;
    lv_style_init(&i2con_style);
    lv_style_set_bg_color(&i2con_style, lv_color_hex(0xFF0000));
    lv_style_set_border_color(&i2con_style, lv_color_hex(0x000000));
    lv_style_set_border_width(&i2con_style, 2);

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

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
    ina266_flag = 1;
    digitalWrite(1, LOW);
    lv_obj_t *label1 = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label1, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(label1, 95, 20);
    updatelabel_timer1 = lv_timer_create(update_label_timer1, 100, label1);
    lv_obj_t *label2 = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(label2, 200, 20);
    lv_label_set_text(label2, "#FF0000 V#"); // 设置文本内容
    lv_label_set_recolor(label2, true);

    volt_chart = lv_chart_create(lv_scr_act());
    lv_obj_set_style_bg_color(volt_chart, lv_color_hex(0x303030), LV_PART_MAIN);
    lv_obj_set_style_line_color(volt_chart, lv_color_hex(0x696969), LV_PART_MAIN);
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
    lv_label_set_text(volt_label1, "#FFD700 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    configure_swipe_back_for_current_screen(true);
}

void DSO_init(void)
{

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
    lv_obj_add_event_cb(lv_scr_act(), event_handler_back, LV_EVENT_ALL, NULL);


    DSO_flag = 1;

    DSO_chart = lv_chart_create(lv_scr_act());
    lv_obj_set_size(DSO_chart, 269, 165);
    lv_obj_set_pos(DSO_chart, 6, 30);
    lv_obj_set_style_bg_color(DSO_chart, lv_color_hex(0x303030), LV_PART_MAIN);
    lv_obj_set_style_line_color(DSO_chart, lv_color_hex(0x696969), LV_PART_MAIN);
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
    lv_obj_set_style_text_color(max_label, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_font(max_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(max_label, 10, 10);
    lv_label_set_text(max_label, "max"); // 设置文本内容
    lv_obj_t *maxValue_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(maxValue_label, lv_color_hex(0xFF0000), 0);
    lv_obj_set_style_text_font(maxValue_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(maxValue_label, 55, 10);

    lv_obj_t *min_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(min_label, lv_color_hex(0x00FF7F), 0);
    lv_obj_set_style_text_font(min_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(min_label, 98, 10);
    lv_label_set_text(min_label, "min"); // 设置文本内容
    lv_obj_t *minValue_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(minValue_label, lv_color_hex(0x00FF7F), 0);
    lv_obj_set_style_text_font(minValue_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(minValue_label, 140, 10);

    lv_obj_t *vpp_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(vpp_label, lv_color_hex(0x00FFFF), 0);
    lv_obj_set_style_text_font(vpp_label, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(vpp_label, 185, 10);
    lv_label_set_text(vpp_label, "vpp"); // 设置文本内容
    lv_obj_t *peakToPeakValue_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(peakToPeakValue_label, lv_color_hex(0x00FFFF), 0);
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
    lv_label_set_text(volt_label1, "#FFD700 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 115, 210);
    lv_label_set_recolor(volt_label1, true);

    lv_obj_t *volt_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label2, "#FFD700 0.55v/Div#");  // 设置文本内容
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

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
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
    lv_label_set_text(volt_label1, "#FFD700 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(volt_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(volt_label1, 180, 210);
    lv_label_set_recolor(volt_label1, true);

    lv_obj_t *volt_label2 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_obj_set_style_text_color(volt_label2, lv_color_hex(0x1E90FF), 0);
    lv_label_set_text(volt_label2, LV_SYMBOL_BLUETOOTH "BLE"); // 设置文本内容
    lv_obj_set_style_text_font(volt_label2, &lv_font_montserrat_28, 0);
    lv_obj_set_pos(volt_label2, 6, 32);
    lv_label_set_recolor(volt_label2, true);

    lv_obj_t *volt_label3 = lv_label_create(lv_scr_act()); // 将文本标签添加到圆角矩形中
    lv_label_set_text(volt_label3, "#00FFFF baudRate#");   // 设置文本内容
    lv_obj_set_style_text_font(volt_label3, &lv_font_montserrat_18, 0);
    lv_obj_set_pos(volt_label3, 170, 5);
    lv_label_set_recolor(volt_label3, true);

    wireless_uart_list = lv_dropdown_create(lv_scr_act());
    lv_obj_set_size(wireless_uart_list, 105, 40);
    lv_obj_set_pos(wireless_uart_list, 170, 30);
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

    static lv_style_t wireless_uart_style;
    lv_style_init(&wireless_uart_style);
    lv_style_set_bg_color(&wireless_uart_style, lv_color_hex(0xFF0000));
    lv_style_set_border_color(&wireless_uart_style, lv_color_hex(0x000000));
    lv_style_set_border_width(&wireless_uart_style, 2);

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
    lv_obj_set_style_bg_color(wireless_uart_clear_btn, lv_color_hex(0x696969), 0);
    lv_obj_set_style_border_color(wireless_uart_clear_btn, lv_color_hex(0x000000), 0);
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

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
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
    lv_label_set_text(FREcount_label1, "#FFD700 ROW1#");       // 设置文本内容
    lv_obj_set_style_text_font(FREcount_label1, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(FREcount_label1, 180, 210);
    lv_label_set_recolor(FREcount_label1, true);

    lv_obj_t *FREcount_label2 = lv_label_create(lv_scr_act());        // 将文本标签添加到圆角矩形中
    lv_label_set_text(FREcount_label2, "#00FFFF Frequency Counter#"); // 设置文本内容
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
    lv_label_set_text(Hz_label, "#FF0000 Hz#"); // 设置文本内容
    lv_label_set_recolor(Hz_label, true);

    configure_swipe_back_for_current_screen(true);
}

void readme_init(void)
{

    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);

    lv_obj_t *img1 = lv_img_create(lv_scr_act());
    lv_img_set_src(img1, &kobe_png);                         // Replace with your image variable or path
    lv_obj_set_size(img1, LV_SIZE_CONTENT, LV_SIZE_CONTENT); // Set image size
    lv_obj_align(img1, LV_ALIGN_OUT_RIGHT_MID, 0, 0);        // Center the image within the button

    configure_swipe_back_for_current_screen(true);
}

static void setting_volume_slider_event_cb(lv_event_t *e)
{
    lv_obj_t *target = lv_event_get_target(e);
    int value = lv_slider_get_value(target);
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

    lv_obj_t *value_label = (lv_obj_t *)lv_event_get_user_data(e);
    if (value_label)
    {
        lv_label_set_text_fmt(value_label, "L%d", value);
    }
}

void setting_init(void)
{
    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x000000), 0);
    configure_swipe_back_for_current_screen(false);

    lv_obj_t *title = lv_label_create(lv_scr_act());
    lv_label_set_text(title, "#00FFFF setting#");
    lv_label_set_recolor(title, true);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_pos(title, 12, 16);

    lv_obj_t *desc = lv_label_create(lv_scr_act());
    lv_label_set_text(desc, "Buzzer Volume");
    lv_obj_set_style_text_font(desc, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(desc, 12, 62);

    lv_obj_t *volume_slider = lv_slider_create(lv_scr_act());
    lv_obj_set_size(volume_slider, 230, 16);
    lv_obj_set_pos(volume_slider, 20, 116);
    lv_slider_set_range(volume_slider, 1, 4);
    lv_slider_set_value(volume_slider, buzzer_volume_level, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0x3A3A3A), LV_PART_MAIN);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0xFF0000), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(volume_slider, lv_color_hex(0xFFFFFF), LV_PART_KNOB);

    lv_obj_t *value_label = lv_label_create(lv_scr_act());
    lv_label_set_text_fmt(value_label, "L%d", buzzer_volume_level);
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_22, 0);
    lv_obj_set_pos(value_label, 258, 106);

    lv_obj_add_event_cb(volume_slider, setting_volume_slider_event_cb, LV_EVENT_VALUE_CHANGED, value_label);

    const int slider_x = 20;
    const int slider_w = 230;
    const int scale_y = 145;
    const int step = slider_w / 3;
    for (int idx = 0; idx < 4; idx++)
    {
        lv_obj_t *tick_label = lv_label_create(lv_scr_act());
        lv_label_set_text_fmt(tick_label, "%d", idx + 1);
        lv_obj_set_style_text_font(tick_label, &lv_font_montserrat_16, 0);
        lv_obj_set_pos(tick_label, slider_x + idx * step - 4, scale_y);
    }

    lv_obj_t *tip_label = lv_label_create(lv_scr_act());
    lv_label_set_text(tip_label, "Double click PUSH to back");
    lv_obj_set_style_text_font(tip_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(tip_label, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_pos(tip_label, 12, 205);

    configure_swipe_back_for_current_screen(false);
}

void ui_init(void)
{
    // 设置显示主题
    lv_theme_default_init(NULL, lv_color_hex(0x000000), lv_color_hex(0xFF0000), LV_THEME_DEFAULT_DARK, NULL);
    lv_obj_add_event_cb(lv_scr_act(), longpress_event_handler_back, LV_EVENT_LONG_PRESSED, NULL);
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


