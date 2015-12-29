#include <pebble.h>

#define COLORS_FLAG PBL_IF_COLOR_ELSE(true, false)

#define KEY_TICK_MARK 0
#define KEY_NB_MARK 1
#define KEY_DAY_FRAME 2
   
static bool s_tick_mark = false;
static bool s_day_frame = false;
static int s_nb_mark = 4;

static char s_day[] = "00";
typedef struct {
   int hours;
   int minutes;
} Ctime;
static Ctime s_current_time;

static GPoint s_center;
static Window *s_main_window;
static Layer *s_canvas_layer;
static int s_radius = 0;



static void canvas_update_proc(Layer *this_layer, GContext *ctx) {
   
   GRect bounds = layer_get_bounds(this_layer);
   int day_frame_rect = 30;
   int fontsize = 24;
   
   // Reduce the flag vertical size if it's not the Round Pebble
   #if defined(PBL_ROUND)
      int reduce_vflag = 1;
      int pos_hflag = 0;
   #else
      int reduce_vflag = 9;
      int pos_hflag = 3;
   #endif
   
   // Day frame
   GRect day_bounds = GRect(bounds.size.w/1.25 - (day_frame_rect/2) -pos_hflag , bounds.size.h/2 - (day_frame_rect/2), day_frame_rect, day_frame_rect - day_frame_rect/5);
   GRect day_in_bounds = grect_inset(day_bounds, GEdgeInsets(1));
   GRect day_bound_text = GRect(day_bounds.origin.x, day_bounds.origin.y - (day_frame_rect - fontsize - 2), day_bounds.size.w, day_bounds.size.h);
   
   // Blue and red flags parts
   GRect flag_blue_part = GRect(bounds.origin.x, bounds.origin.y + (bounds.size.h/reduce_vflag), bounds.size.w/3, bounds.size.h - (bounds.size.h/reduce_vflag *2));
   GRect flag_red_part = GRect(bounds.origin.x + (bounds.size.w/3) * 2, (bounds.size.h/reduce_vflag), bounds.size.w/3, bounds.size.h - (bounds.size.h/reduce_vflag *2));
   #if (PBL_RECT)
      GRect white_flag_part = GRect(bounds.origin.x + (bounds.size.w/3), (bounds.size.h/reduce_vflag), bounds.size.w/3, bounds.size.h - (bounds.size.h/reduce_vflag *2));
   #endif
   
   // Draw the flag.  French Flag -> https://en.wikipedia.org/wiki/Flag_of_France
   if (COLORS_FLAG) {
      #if (PBL_RECT)
         // Background black.
         graphics_context_set_fill_color(ctx, GColorFromHEX(0x000000));
         graphics_fill_rect(ctx, bounds, 0, GCornerNone);
         graphics_context_set_fill_color(ctx, GColorFromHEX(0xffffff));
         graphics_fill_rect(ctx, white_flag_part, 0, GCornerNone);
      #endif
      graphics_context_set_fill_color(ctx, GColorFromHEX(0x022395));
      graphics_fill_rect(ctx, flag_blue_part, 0, GCornerNone);
      graphics_context_set_fill_color(ctx, GColorFromHEX(0xED2939));
      graphics_fill_rect(ctx, flag_red_part, 0, GCornerNone);
   }
   
   if (s_day_frame){
      // Draw the day frame
      graphics_context_set_fill_color(ctx, GColorBlack);
      graphics_fill_rect(ctx, day_bounds, 2, GCornersAll);
      graphics_context_set_fill_color(ctx, GColorWhite);
      graphics_fill_rect(ctx, day_in_bounds, 2, GCornersAll);
      
      // Draw the current day inside the frame
      graphics_context_set_text_color(ctx, GColorBlack);
      graphics_draw_text(ctx, s_day, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), day_bound_text, GTextOverflowModeFill, GTextAlignmentCenter, NULL);
   }
   
   // Calculate the hands angles
   float minute_angle = TRIG_MAX_ANGLE * s_current_time.minutes / 60;
   float hour_angle = TRIG_MAX_ANGLE * s_current_time.hours / 12;
   hour_angle += (minute_angle / TRIG_MAX_ANGLE) * (TRIG_MAX_ANGLE / 12);
   
   GPoint hour_hand = (GPoint) {
      .x = (int16_t)(sin_lookup(hour_angle) * (int32_t) (s_radius * 0.8) / TRIG_MAX_RATIO) + s_center.x,
      .y = (int16_t)(-cos_lookup(hour_angle) * (int32_t)(s_radius * 0.8) / TRIG_MAX_RATIO) + s_center.y
   };
   GPoint minute_hand = (GPoint) {
      .x = (int16_t)(sin_lookup(TRIG_MAX_ANGLE * s_current_time.minutes / 60) *  (s_radius) / TRIG_MAX_RATIO) + s_center.x,
      .y = (int16_t)(-cos_lookup(TRIG_MAX_ANGLE * s_current_time.minutes / 60) * (s_radius) / TRIG_MAX_RATIO) + s_center.y
   };
   
   // Draw hands
   graphics_context_set_stroke_color(ctx, GColorBlack);
   graphics_context_set_stroke_width(ctx, 10);
   graphics_draw_line(ctx, s_center, hour_hand);
   graphics_draw_line(ctx, s_center, minute_hand);
   
   // Hours marks are dots. Black on white and white on colors.
   if (s_tick_mark) {
      for(int i = 0; i < s_nb_mark; i++) {
         int hour_dot = (i * 360) / s_nb_mark;
         if (i == 0 || i == (s_nb_mark/2)){
            graphics_context_set_fill_color(ctx, GColorBlack);
         }else{
            graphics_context_set_fill_color(ctx, GColorWhite);
         }
         GPoint pos = gpoint_from_polar(grect_inset(bounds, GEdgeInsets(12)), GOvalScaleModeFitCircle, DEG_TO_TRIGANGLE(hour_dot));
         graphics_fill_circle(ctx, pos, 3);
      }
   }
   
}

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
   Tuple *tick_mark_t = dict_find(iter, KEY_TICK_MARK);
   Tuple *nb_mark_t = dict_find(iter, KEY_NB_MARK);
   Tuple *day_frame_t = dict_find(iter, KEY_DAY_FRAME);
   
   if (tick_mark_t) {
      s_tick_mark = tick_mark_t->value->int8;
      persist_write_int(KEY_TICK_MARK, s_tick_mark);
   }
   if (nb_mark_t) {
      s_nb_mark = nb_mark_t->value->int8;
      persist_write_int(KEY_NB_MARK, s_nb_mark);
   }
   if (day_frame_t) {
      s_day_frame = day_frame_t->value->int8;
      persist_write_int(KEY_DAY_FRAME, s_day_frame);
   }
   if (tick_mark_t || nb_mark_t || day_frame_t){
      vibes_double_pulse();
      layer_mark_dirty(s_canvas_layer);
   }

}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
   
   // Extract the day
   strftime(s_day, sizeof("00"), "%d", tick_time);
   
   // Extract the time
   s_current_time.hours = tick_time->tm_hour;
   s_current_time.hours = s_current_time.hours > 12 ? s_current_time.hours - 12 : s_current_time.hours;
   s_current_time.minutes = tick_time->tm_min;
   
   layer_mark_dirty(s_canvas_layer);
}

static void main_window_load(Window *window) {
   Layer *window_layer = window_get_root_layer(window);
   GRect window_bounds = layer_get_bounds(window_layer);
   
   // Create Layer
   s_canvas_layer = layer_create(GRect(0, 0, window_bounds.size.w, window_bounds.size.h));
   layer_add_child(window_layer, s_canvas_layer);
   
   // Center for the clock hands
   s_center = grect_center_point(&window_bounds);
   
   // Hands length
   s_radius = window_bounds.size.w < window_bounds.size.h ? window_bounds.size.w : window_bounds.size.h;
   s_radius /= 3;
   s_radius -= 3;
   
   // Set the update_proc
   layer_set_update_proc(s_canvas_layer, canvas_update_proc);
   
   // Get the last used values
   if (persist_read_bool(KEY_TICK_MARK)) {
      s_tick_mark = persist_read_bool(KEY_TICK_MARK);
   }
   if (persist_read_bool(KEY_NB_MARK)) {
      s_nb_mark = persist_read_bool(KEY_NB_MARK);
   }
   if (persist_read_bool(KEY_DAY_FRAME)) {
      s_day_frame = persist_read_bool(KEY_DAY_FRAME);
   }
      
   layer_mark_dirty(s_canvas_layer);

}

static void main_window_unload(Window *window) {
   // Destroy Layer
   layer_destroy(s_canvas_layer);
}

static void init(void) {
   s_main_window = window_create();
   window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = main_window_unload,
   });
   window_stack_push(s_main_window, true);
   window_set_background_color(s_main_window, GColorWhite);
   tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
   
   // Register message to receive preferences from the phone.
   app_message_register_inbox_received(inbox_received_handler);
   app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
   
}

static void deinit(void) {
   // Destroy main Window
   window_destroy(s_main_window);
}

int main(void) {
   init();
   app_event_loop();
   deinit();
}
