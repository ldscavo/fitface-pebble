#include <pebble.h>

Window *my_window;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *steps_layer;
TextLayer *weather_layer;
TextLayer *battery_layer;

static Layer *s_circle_layer;

static int s_step_count = 0;
static int s_step_avg = 0;
static int s_stepgoal = 5000;

static char s_step_avg_scope[8];

static char s_tempunits[] = "F";

static bool s_bt_vibe = false;
static bool s_circle_rounded = true;
static bool s_show_step_avg = false;
static bool s_show_battery_level = false;

GColor getColor(const uint32_t Key, GColor default_color, GColor default_bw) {
  GColor color;
  if (persist_exists(Key)) {
    color = GColorFromHEX(persist_read_int(Key));
  } else {
    color = COLOR_FALLBACK(default_color, default_bw);
  }
  
  return color;
}

static void update_weather_and_settings() {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();
}

static void update_avg_steps() {
  HealthServiceTimeScope avg_scope;
    
  if (strcmp(s_step_avg_scope, "weekend") == 0) {
    avg_scope = HealthServiceTimeScopeDailyWeekdayOrWeekend;
  } else if (strcmp(s_step_avg_scope, "weekly") == 0) {
    avg_scope = HealthServiceTimeScopeWeekly;
  } else {
    avg_scope = HealthServiceTimeScopeDaily;
  }
  
  s_step_avg = (int)health_service_sum_averaged(
    HealthMetricStepCount, time_start_of_today(), time(NULL), avg_scope
  );
}

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  
  text_layer_set_text(time_layer, s_buffer);

  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %b %d", tick_time);
  text_layer_set_text(date_layer, date_buffer);
  
  if(tick_time->tm_min % 30 == 0) {
    update_weather_and_settings();
    if (s_show_step_avg) {
      update_avg_steps();
    }
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void update_steps() {
  HealthMetric steps = HealthMetricStepCount;
  time_t start = time_start_of_today();
  time_t end = time(NULL);

  HealthServiceAccessibilityMask mask = health_service_metric_accessible(steps, start, end);
  static char steps_buffer[16];
  
  if(mask & HealthServiceAccessibilityMaskAvailable) {
    s_step_count = (int)health_service_sum_today(steps);
        
    snprintf(steps_buffer, sizeof(steps_buffer), "%u Steps", s_step_count);
    
    text_layer_set_text(steps_layer, steps_buffer);
  }  
  
  // Mark the circle layer as dirty so it will be redrawn
  layer_mark_dirty(s_circle_layer);
}

static void steps_handler(HealthEventType event, void *context) {
  update_steps();
}

static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[9];
  if (!s_show_battery_level) {
    strcpy(battery_text, "");
  } else if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "charging");
  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

static void canvas_update_circle_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
  #if defined(PBL_HEALTH)
  const GRect inset_frame = grect_inset(inset, GEdgeInsets(PBL_IF_ROUND_ELSE(7, 3)));
      
  GColor frame_color = getColor(MESSAGE_KEY_COLOR_CIRCLE_SECONDARY, GColorLightGray, GColorWhite);
  graphics_context_set_fill_color(ctx, frame_color);
  graphics_context_set_antialiased(ctx, true);
  
  graphics_fill_radial(
    ctx, inset_frame, GOvalScaleModeFitCircle, 2, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360)
  );
  #endif
  
  GColor circle_color = getColor(MESSAGE_KEY_COLOR_CIRCLE_PRIMARY, GColorChromeYellow, GColorLightGray);  
  graphics_context_set_fill_color(ctx, circle_color);
  
  #if defined(PBL_HEALTH)
  int arc_angle = s_stepgoal > s_step_count ? 360 * s_step_count / s_stepgoal : 360;
  
  graphics_fill_radial(
    ctx, inset, GOvalScaleModeFitCircle, PBL_IF_ROUND_ELSE(15, 9), DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(arc_angle)
  );
  
  if (s_circle_rounded && s_step_count > 0) {
    GRect inset_edges = grect_inset(inset, GEdgeInsets(PBL_IF_ROUND_ELSE(7, 4)));
    
    GPoint startpoint = gpoint_from_polar(inset_edges, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(0));
    GPoint endpoint = gpoint_from_polar(inset_edges, GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(arc_angle));
    
    graphics_fill_circle(ctx, startpoint, PBL_IF_ROUND_ELSE(6, 4));
    graphics_fill_circle(ctx, endpoint, PBL_IF_ROUND_ELSE(6, 4));
  }
  
  if (s_show_step_avg) {
    GColor avg_color = getColor(MESSAGE_KEY_COLOR_AVG_LINE, GColorPastelYellow, GColorWhite);
    graphics_context_set_fill_color(ctx, avg_color);
    
    int avg_arc_angle = s_stepgoal > s_step_avg ? 360 * s_step_avg / s_stepgoal : 360;
    graphics_fill_radial(
      ctx, layer_get_bounds(layer), GOvalScaleModeFitCircle, PBL_IF_ROUND_ELSE(25, 17), DEG_TO_TRIGANGLE(avg_arc_angle), DEG_TO_TRIGANGLE(avg_arc_angle + 3)
    );
  }
  #else
  graphics_fill_radial(
    ctx, inset, GOvalScaleModeFitCircle, 7, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360)
  );
  #endif
}

static void process_config_for_battery_level(DictionaryIterator *iterator) {
  Tuple *battery_level_tuple = dict_find(iterator, MESSAGE_KEY_BATTERY_LEVEL);
  if (battery_level_tuple) {
    s_show_battery_level = battery_level_tuple->value->int32 == 1;
    persist_write_bool(MESSAGE_KEY_BATTERY_LEVEL, s_show_battery_level);
    handle_battery(battery_state_service_peek());
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char weather_buffer[8];
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, MESSAGE_KEY_KEY_TEMP);
  Tuple *stepgoal_tuple = dict_find(iterator, MESSAGE_KEY_STEPGOAL);
  Tuple *tempunits_tuple = dict_find(iterator, MESSAGE_KEY_TEMP_UNITS);
  Tuple *color_bg_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_BG);
  Tuple *color_circle_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_CIRCLE_PRIMARY);
  Tuple *color_text_primary_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_TEXT_PRIMARY);
  Tuple *color_text_secondary_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_TEXT_SECONDARY);
  Tuple *color_circle_secondary_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_CIRCLE_SECONDARY);
  Tuple *color_avg_line_tuple = dict_find(iterator, MESSAGE_KEY_COLOR_AVG_LINE);
  Tuple *bt_vibe_tuple = dict_find(iterator, MESSAGE_KEY_BT_VIBE);
  Tuple *circle_rounded_tuple = dict_find(iterator, MESSAGE_KEY_CIRCLE_ROUNDED);
  Tuple *step_avg_tuple = dict_find(iterator, MESSAGE_KEY_STEP_AVG);
  Tuple *step_avg_scope_tuple = dict_find(iterator, MESSAGE_KEY_STEP_AVG_SCOPE);
  
  if (tempunits_tuple) {
    snprintf(s_tempunits, sizeof(s_tempunits), "%s", tempunits_tuple->value->cstring);
    persist_write_string(MESSAGE_KEY_TEMP_UNITS, s_tempunits);
    
    update_weather_and_settings();
  }
  
  if(temp_tuple) {
    int temp = (int)temp_tuple->value->int32;
    
    if (strcmp(s_tempunits, "C") == 0) {
      temp = (temp - 32) / 1.8;
    }
    snprintf(weather_buffer, sizeof(weather_buffer), "%d°%s", temp, s_tempunits);
    text_layer_set_text(weather_layer, weather_buffer);
  }
  if (stepgoal_tuple) {
    s_stepgoal = stepgoal_tuple->value->uint32;
    persist_write_int(MESSAGE_KEY_STEPGOAL, s_stepgoal);
    update_steps();
  }
  if (color_bg_tuple) {
    persist_write_int(MESSAGE_KEY_COLOR_BG, (int)color_bg_tuple->value->uint32);
    window_set_background_color(my_window, GColorFromHEX(color_bg_tuple->value->uint32));
  }
  if (color_circle_tuple) {
    persist_write_int(MESSAGE_KEY_COLOR_CIRCLE_PRIMARY, (int)color_circle_tuple->value->uint32);
    layer_mark_dirty(s_circle_layer);
  }
  if (color_text_primary_tuple) {
    persist_write_int(MESSAGE_KEY_COLOR_TEXT_PRIMARY, (int)color_text_primary_tuple->value->uint32);
    GColor color = GColorFromHEX((int)color_text_primary_tuple->value->uint32);
    text_layer_set_text_color(time_layer, color);
    text_layer_set_text_color(date_layer, color);
  }
  if (color_text_secondary_tuple) {
    persist_write_int(MESSAGE_KEY_COLOR_TEXT_SECONDARY, (int)color_text_secondary_tuple->value->uint32);
    GColor color = GColorFromHEX((int)color_text_secondary_tuple->value->uint32);
    text_layer_set_text_color(steps_layer, color);
    text_layer_set_text_color(weather_layer, color);
  }
  if (color_circle_secondary_tuple) {
    persist_write_int(MESSAGE_KEY_COLOR_CIRCLE_SECONDARY, (int)color_circle_secondary_tuple->value->uint32);
    layer_mark_dirty(s_circle_layer);
  }
  if (color_avg_line_tuple) {
    persist_write_int(MESSAGE_KEY_COLOR_AVG_LINE, (int)color_avg_line_tuple->value->uint32);
    layer_mark_dirty(s_circle_layer);
  }
  if (bt_vibe_tuple) {
    persist_write_bool(MESSAGE_KEY_BT_VIBE, (bool)bt_vibe_tuple->value->uint32);
  }
  if (circle_rounded_tuple) {
    s_circle_rounded = (bool)circle_rounded_tuple->value->uint32;
    persist_write_bool(MESSAGE_KEY_CIRCLE_ROUNDED, s_circle_rounded);
    layer_mark_dirty(s_circle_layer);
  }
  if (step_avg_tuple) {
    s_show_step_avg = (bool)step_avg_tuple->value->uint32;
    if (s_show_step_avg) {
      update_avg_steps();
    }
    persist_write_bool(MESSAGE_KEY_STEP_AVG, s_show_step_avg);
    layer_mark_dirty(s_circle_layer);
  }
  if (step_avg_scope_tuple) {
    snprintf(s_step_avg_scope, sizeof(s_step_avg_scope), "%s", step_avg_scope_tuple->value->cstring);
    persist_write_string(MESSAGE_KEY_STEP_AVG_SCOPE, s_step_avg_scope);
    update_avg_steps();
    layer_mark_dirty(s_circle_layer);
  }

  process_config_for_battery_level(iterator);
}

static void bluetooth_disconnect(bool connected) {  
  if (persist_exists(MESSAGE_KEY_BT_VIBE)) {
    s_bt_vibe = persist_read_bool(MESSAGE_KEY_BT_VIBE);
  }
  
  if ((!connected) && s_bt_vibe) {
    vibes_double_pulse();
  }
}

static void init_battery_level(Layer *window_layer, GColor color) {
  battery_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(25, 22), layer_get_bounds(window_layer).size.w, 18));
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_text_color(battery_layer, color);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(battery_layer));

  battery_state_service_subscribe(handle_battery);
  if (persist_exists(MESSAGE_KEY_BATTERY_LEVEL)) {
    s_show_battery_level = persist_read_bool(MESSAGE_KEY_BATTERY_LEVEL);
  }
  handle_battery(battery_state_service_peek());
}

void handle_init(void) {
  my_window = window_create(); 
  
  Layer *window_layer = window_get_root_layer(my_window);
  
  GColor bg_color = getColor(MESSAGE_KEY_COLOR_BG, GColorBlue, GColorBlack);
  
  window_set_background_color(my_window, bg_color);
  
  GRect bounds = layer_get_bounds(window_layer);
  s_circle_layer = layer_create(bounds);
  
  if (persist_exists(MESSAGE_KEY_CIRCLE_ROUNDED)) {
    s_circle_rounded = persist_read_bool(MESSAGE_KEY_CIRCLE_ROUNDED);
  }
  
  if (persist_exists(MESSAGE_KEY_STEP_AVG_SCOPE)) {
    persist_read_string(MESSAGE_KEY_STEP_AVG_SCOPE, s_step_avg_scope, sizeof(s_step_avg_scope));
  } else {
    snprintf(s_step_avg_scope, sizeof(s_step_avg_scope), "daily");
  }
  
  if (persist_exists(MESSAGE_KEY_STEP_AVG)) {
    s_show_step_avg = persist_read_bool(MESSAGE_KEY_STEP_AVG);
  }
  
  if (s_show_step_avg) {
    update_avg_steps();
  }
  
  layer_set_update_proc(s_circle_layer, canvas_update_circle_proc);
  layer_add_child(window_get_root_layer(my_window), s_circle_layer);
  
  window_stack_push(my_window, true);
  
  GColor time_color = getColor(MESSAGE_KEY_COLOR_TEXT_PRIMARY, GColorWhite, GColorWhite);
  GColor step_color = getColor(MESSAGE_KEY_COLOR_TEXT_SECONDARY, GColorMelon, GColorWhite);
  
  date_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(100, 95), bounds.size.w, 25));
  
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, time_color);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  
  time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));
  
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, time_color);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  
  steps_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(40, 35), bounds.size.w, 25));
  
  text_layer_set_background_color(steps_layer, GColorClear);
  text_layer_set_text_color(steps_layer, step_color);
  text_layer_set_font(steps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(steps_layer, GTextAlignmentCenter);
  
  weather_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(125, 115), bounds.size.w, 25)
  );
  
  text_layer_set_background_color(weather_layer, GColorClear);
  text_layer_set_text_color(weather_layer, step_color);
  text_layer_set_font(weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(weather_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_layer, text_layer_get_layer(steps_layer));
  layer_add_child(window_layer, text_layer_get_layer(weather_layer));
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  health_service_events_subscribe(steps_handler, NULL);
  
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_disconnect
  });
  
  init_battery_level(window_layer, step_color);
  
  update_time();
  bluetooth_disconnect(connection_service_peek_pebble_app_connection());
  
  if (persist_exists(MESSAGE_KEY_STEPGOAL)) {
    s_stepgoal = persist_read_int(MESSAGE_KEY_STEPGOAL);
  }
  update_steps();
  
  if (persist_exists(MESSAGE_KEY_TEMP_UNITS)) {
    persist_read_string(MESSAGE_KEY_TEMP_UNITS, s_tempunits, sizeof(s_tempunits));
  }
    
  app_message_register_inbox_received(inbox_received_callback);
  const int inbox_size = 192;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

void handle_deinit(void) {
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(steps_layer);
  text_layer_destroy(weather_layer);
  text_layer_destroy(battery_layer);
  layer_destroy(s_circle_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
