#include <pebble.h>

Window *my_window;
TextLayer *time_layer;
TextLayer *date_layer;
static Layer *s_circle_layer;

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

static void canvas_update_circle_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);  
      
  uint16_t circle_width = 9;
  
  graphics_context_set_stroke_color(ctx, COLOR_FALLBACK(GColorChromeYellow, GColorWhite));  
  graphics_context_set_stroke_width(ctx, circle_width);
  graphics_context_set_antialiased(ctx, true);
  
  uint16_t radius = (bounds.size.w / 2) - (circle_width * 1.25);
  
  #if defined(PBL_ROUND)
  GPoint center = grect_center_point(&bounds);
  graphics_draw_circle(ctx, center, radius);
  #else
  GRect rect = grect_inset(bounds, GEdgeInsets1(7));
  graphics_draw_round_rect(ctx, rect, radius / 5);
  #endif
}

void handle_init(void) {
  my_window = window_create(); 
    
  Layer *window_layer = window_get_root_layer(my_window);
  
  window_set_background_color(my_window, COLOR_FALLBACK(GColorDukeBlue, GColorBlack));
  
  GRect bounds = layer_get_bounds(window_layer);
  s_circle_layer = layer_create(bounds);
  
  layer_set_update_proc(s_circle_layer, canvas_update_circle_proc);
  layer_add_child(window_get_root_layer(my_window), s_circle_layer);
  
  window_stack_push(my_window, true);
  
  date_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(70, 50), 95, 75, 25));
  //text_layer_create(GRect(x, y, w, h))
  text_layer_set_background_color(date_layer, GColorClear);
  text_layer_set_text_color(date_layer, PBL_IF_COLOR_ELSE(GColorMelon, GColorWhite));
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  
  time_layer = text_layer_create(
    GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50)
  );
  
  text_layer_set_background_color(time_layer, GColorClear);
  text_layer_set_text_color(time_layer, GColorWhite);
  text_layer_set_font(time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(time_layer, GTextAlignmentCenter);
  
  layer_add_child(window_layer, text_layer_get_layer(date_layer));
  layer_add_child(window_layer, text_layer_get_layer(time_layer));
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

void handle_deinit(void) {
  text_layer_destroy(time_layer);
  window_destroy(my_window);
}

int main(void) {
  handle_init();
  app_event_loop();
  handle_deinit();
}
