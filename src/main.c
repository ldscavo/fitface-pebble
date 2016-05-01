#include <pebble.h>

Window *my_window;
TextLayer *time_layer;
TextLayer *date_layer;
TextLayer *steps_layer;
TextLayer *weather_layer;
static Layer *s_circle_layer;

#if defined(PBL_HEALTH)
static int s_step_count = 0;
#endif

#define KEY_TEMP 0

static void update_time() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
  text_layer_set_text(time_layer, s_buffer);
  
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %b %d", tick_time);
  text_layer_set_text(date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

#if defined(PBL_HEALTH)
static void update_steps() {
  HealthMetric steps = HealthMetricStepCount;
  time_t start = time_start_of_today();
  time_t end = time(NULL);

  HealthServiceAccessibilityMask mask = health_service_metric_accessible(steps, start, end);
  static char steps_buffer[16];
  if(mask & HealthServiceAccessibilityMaskAvailable) {
    // Data is available!
    s_step_count = (int)health_service_sum_today(steps);
    snprintf(steps_buffer, sizeof(steps_buffer), "%u steps", s_step_count);
  } else {
    // No data recorded yet today
    snprintf(steps_buffer, sizeof(steps_buffer), "No steps");
  }
  text_layer_set_text(steps_layer, steps_buffer);
  
  // Mark the circle layer as dirty so it will be redrawn
  layer_mark_dirty(s_circle_layer);
}

static void steps_handler(HealthEventType event, void *context) {
  update_steps();
}
#endif

static void canvas_update_circle_proc(Layer *layer, GContext *ctx) {
  const GRect inset = grect_inset(layer_get_bounds(layer), GEdgeInsets(2));
  #if defined(PBL_HEALTH)
  const GRect inset_frame = grect_inset(inset, GEdgeInsets(3));
  
  graphics_context_set_fill_color(ctx, GColorLightGray);
  graphics_context_set_antialiased(ctx, true);
  
  graphics_fill_radial(
    ctx, inset_frame, GOvalScaleModeFitCircle, 1, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360)
  );
  #endif
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(GColorChromeYellow, GColorWhite));
  
  #if defined(PBL_HEALTH)
  graphics_fill_radial(
    ctx, inset, GOvalScaleModeFitCircle, PBL_IF_ROUND_ELSE(9, 7), DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360 * s_step_count / 4000)
  );
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
  
  // If all data is available, use it
  if(temp_tuple) {
    snprintf(weather_buffer, sizeof(weather_buffer), "%d°", (int)temp_tuple->value->int32);
    text_layer_set_text(weather_layer, weather_buffer);
  }
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

void handle_init(void) {
  my_window = window_create(); 
    
  Layer *window_layer = window_get_root_layer(my_window);
  
  window_set_background_color(my_window, COLOR_FALLBACK(GColorBlue, GColorBlack));
  
  GRect bounds = layer_get_bounds(window_layer);
  s_circle_layer = layer_create(bounds);
  
  layer_set_update_proc(s_circle_layer, canvas_update_circle_proc);
  layer_add_child(window_get_root_layer(my_window), s_circle_layer);
  
  window_stack_push(my_window, true);
  
  date_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(100, 95), bounds.size.w, 25));
  //text_layer_create(GRect(x, y, w, h))
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, PBL_IF_COLOR_ELSE(GColorWhite, GColorWhite));
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  
  time_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50)
  );
  
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  
  steps_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(40, 30), PBL_IF_ROUND_ELSE(40, 35), 100, 25));
  
  text_layer_set_background_color(steps_layer, GColorClear);
  text_layer_set_text_color(steps_layer, PBL_IF_COLOR_ELSE(GColorMelon, GColorWhite));
  text_layer_set_font(steps_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(steps_layer, GTextAlignmentCenter);
  
  weather_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(125, 115), bounds.size.w, 25));
  
  text_layer_set_background_color(weather_layer, GColorClear);
  text_layer_set_text_color(weather_layer, PBL_IF_COLOR_ELSE(GColorMelon, GColorWhite));
  text_layer_set_font(weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(weather_layer, GTextAlignmentCenter);
  text_layer_set_text(weather_layer, "∞");
  
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  layer_add_child(window_layer, text_layer_get_layer(steps_layer));
  layer_add_child(window_layer, text_layer_get_layer(weather_layer));
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  #if defined(PBL_HEALTH)
  health_service_events_subscribe(steps_handler, NULL);
  #endif
  
  update_time();
  #if defined(PBL_HEALTH)
  update_steps();
  #endif
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

void handle_deinit(void) {
  text_layer_destroy(time_layer);
  text_layer_destroy(date_layer);
  text_layer_destroy(steps_layer);
  text_layer_destroy(weather_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
