#include <pebble.h>

Window *my_window;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *steps_layer;
TextLayer *weather_layer;

static Layer *s_circle_layer;

#define KEY_TEMP 0
#define STEPGOAL 101
#define TEMP_UNITS 102
#define BT_VIBE 103
#define CIRCLE_ROUNDED 104

#define COLOR_BG 301
#define COLOR_CIRCLE_PRIMARY 302
#define COLOR_TEXT_PRIMARY 303
#define COLOR_TEXT_SECONDARY 304
#define COLOR_CIRCLE_SECONDARY 305


static int s_step_count = 0;
static int s_stepgoal = 5000;

static char s_tempunits[] = "F";

static bool s_bt_vibe = false;
static bool s_circle_rounded = true;

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
    s_step_count = 3200;//(int)health_service_sum_today(steps);
    snprintf(steps_buffer, sizeof(steps_buffer), "%u Steps", s_step_count);
    
    text_layer_set_text(steps_layer, steps_buffer);
  }  
  
  // Mark the circle layer as dirty so it will be redrawn
  layer_mark_dirty(s_circle_layer);
}

static void steps_handler(HealthEventType event, void *context) {
  update_steps();
}

static void canvas_update_circle_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
  #if defined(PBL_HEALTH)
  const GRect inset_frame = grect_inset(inset, GEdgeInsets(PBL_IF_ROUND_ELSE(7, 3)));
  
  APP_LOG(APP_LOG_LEVEL_INFO, "Step Goal is %d", s_stepgoal);
    
  GColor frame_color = getColor(COLOR_CIRCLE_SECONDARY, GColorLightGray, GColorWhite);
  graphics_context_set_fill_color(ctx, frame_color);
  graphics_context_set_antialiased(ctx, true);
  
  graphics_fill_radial(
    ctx, inset_frame, GOvalScaleModeFitCircle, 2, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360)
  );
  #endif
  
  GColor circle_color = getColor(COLOR_CIRCLE_PRIMARY, GColorChromeYellow, GColorWhite);  
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
  #else
  graphics_fill_radial(
    ctx, inset, GOvalScaleModeFitCircle, 7, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360)
  );
  #endif
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  static char weather_buffer[8];
  
  // Read tuples for data
  Tuple *temp_tuple = dict_find(iterator, KEY_TEMP);
  Tuple *stepgoal_tuple = dict_find(iterator, STEPGOAL);
  Tuple *tempunits_tuple = dict_find(iterator, TEMP_UNITS);
  Tuple *color_bg_tuple = dict_find(iterator, COLOR_BG);
  Tuple *color_circle_tuple = dict_find(iterator, COLOR_CIRCLE_PRIMARY);
  Tuple *color_text_primary_tuple = dict_find(iterator, COLOR_TEXT_PRIMARY);
  Tuple *color_text_secondary_tuple = dict_find(iterator, COLOR_TEXT_SECONDARY);
  Tuple *color_circle_secondary_tuple = dict_find(iterator, COLOR_CIRCLE_SECONDARY);
  Tuple *bt_vibe_tuple = dict_find(iterator, BT_VIBE);
  Tuple *circle_rounded_tuple = dict_find(iterator, CIRCLE_ROUNDED);
  
  if (tempunits_tuple) {
    snprintf(s_tempunits, sizeof(s_tempunits), "%s", tempunits_tuple->value->cstring);
    persist_write_string(TEMP_UNITS, s_tempunits);
    APP_LOG(APP_LOG_LEVEL_INFO, "Temp unit is %s", tempunits_tuple->value->cstring);
    
    update_weather_and_settings();
  }
  
  if(temp_tuple) {
    int temp = (int)temp_tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "Temperture is %d", temp);
    
    if (strcmp(s_tempunits, "C") == 0) {
      temp = (temp - 32) / 1.8;
    }
    snprintf(weather_buffer, sizeof(weather_buffer), "%d°%s", temp, s_tempunits);
    text_layer_set_text(weather_layer, weather_buffer);
  }
  if (stepgoal_tuple) {
    s_stepgoal = stepgoal_tuple->value->uint32;
    persist_write_int(STEPGOAL, s_stepgoal);
    update_steps();
    APP_LOG(APP_LOG_LEVEL_INFO, "Step Goal Submitted!");
  }
  if (color_bg_tuple) {
    persist_write_int(COLOR_BG, (int)color_bg_tuple->value->uint32);
    window_set_background_color(my_window, GColorFromHEX(color_bg_tuple->value->uint32));
    APP_LOG(APP_LOG_LEVEL_INFO, "Background color set to %x", (int)color_bg_tuple->value->uint32);
  }
  if (color_circle_tuple) {
    persist_write_int(COLOR_CIRCLE_PRIMARY, (int)color_circle_tuple->value->uint32);
    layer_mark_dirty(s_circle_layer);
  }
  if (color_text_primary_tuple) {
    persist_write_int(COLOR_TEXT_PRIMARY, (int)color_text_primary_tuple->value->uint32);
    GColor color = GColorFromHEX((int)color_text_primary_tuple->value->uint32);
    text_layer_set_text_color(time_layer, color);
    text_layer_set_text_color(date_layer, color);
  }
  if (color_text_secondary_tuple) {
    persist_write_int(COLOR_TEXT_SECONDARY, (int)color_text_secondary_tuple->value->uint32);
    GColor color = GColorFromHEX((int)color_text_secondary_tuple->value->uint32);
    text_layer_set_text_color(steps_layer, color);
    text_layer_set_text_color(weather_layer, color);
  }
  if (color_circle_secondary_tuple) {
    persist_write_int(COLOR_CIRCLE_SECONDARY, (int)color_circle_secondary_tuple->value->uint32);
    layer_mark_dirty(s_circle_layer);
  }  
  if (bt_vibe_tuple) {
    //APP_LOG(APP_LOG_LEVEL_INFO, "Bluetooth connection: %d", (int)bt_vibe_tuple->value->uint32);
    persist_write_bool(BT_VIBE, (bool)bt_vibe_tuple->value->uint32);
  }
  if (circle_rounded_tuple) {
    s_circle_rounded = (bool)circle_rounded_tuple->value->uint32;
    persist_write_bool(CIRCLE_ROUNDED, s_circle_rounded);
    layer_mark_dirty(s_circle_layer);
  }
}

static void bluetooth_disconnect(bool connected) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Bluetooth connection: %d", (int)connected);
  
  if (persist_exists(BT_VIBE)) {
    s_bt_vibe = persist_read_bool(BT_VIBE);
  }
  
  if ((!connected) && s_bt_vibe) {
    vibes_double_pulse();
  }
}

void handle_init(void) {
  my_window = window_create(); 
  
  Layer *window_layer = window_get_root_layer(my_window);
  
  GColor bg_color = getColor(COLOR_BG, GColorBlue, GColorBlack);
  
  window_set_background_color(my_window, bg_color);
  
  GRect bounds = layer_get_bounds(window_layer);
  s_circle_layer = layer_create(bounds);
  
  if (persist_exists(CIRCLE_ROUNDED)) {
    s_circle_rounded = persist_read_bool(CIRCLE_ROUNDED);
  }
  
  layer_set_update_proc(s_circle_layer, canvas_update_circle_proc);
  layer_add_child(window_get_root_layer(my_window), s_circle_layer);
  
  window_stack_push(my_window, true);
  
  GColor time_color = getColor(COLOR_TEXT_PRIMARY, GColorWhite, GColorWhite);
  GColor step_color = getColor(COLOR_TEXT_SECONDARY, GColorMelon, GColorWhite);
  
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
  
  update_time();
  bluetooth_disconnect(connection_service_peek_pebble_app_connection());
  
  if (persist_exists(STEPGOAL)) {
    s_stepgoal = persist_read_int(STEPGOAL);
  }
  update_steps();
  
  if (persist_exists(TEMP_UNITS)) {
    persist_read_string(TEMP_UNITS, s_tempunits, sizeof(s_tempunits));
  }
    
  app_message_register_inbox_received(inbox_received_callback);
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

void handle_deinit(void) {
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(steps_layer);
  text_layer_destroy(weather_layer);
  layer_destroy(s_circle_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
