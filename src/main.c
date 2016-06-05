#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_hour_layer;
static TextLayer *s_minutes_layer;
static GFont s_hour_font;
static GFont s_minutes_font;
static BitmapLayer *logo_layer;
static GBitmap *logo_bitmap;

static void update_time() {
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	static char buffer[] = "00";
	static char buffer2[] = "00";
	
	if (clock_is_24h_style() == true) {
		//24-hour time
		strftime(buffer, sizeof("00"), "%H", tick_time);
		strftime(buffer2, sizeof("00"), "%M", tick_time);
	} else {
		//12-hour time
		strftime(buffer, sizeof("00"), "%I", tick_time);
		strftime(buffer2, sizeof("00"), "%M", tick_time);
	}
	
	//Display hours on hour layer
	text_layer_set_text(s_hour_layer, buffer);
	//Display minutes on minutes layer
	text_layer_set_text(s_minutes_layer, buffer2);
}

static void main_window_load(Window *window) {
	
	window_set_background_color(s_main_window, GColorWhite);
  
  // Get the Window's root layer and the bounds
  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  
  // Load the image
logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO);

// Create a BitmapLayer
logo_layer = bitmap_layer_create(bounds);

// Set the bitmap and center it
bitmap_layer_set_bitmap(logo_layer, logo_bitmap);
bitmap_layer_set_alignment(logo_layer, GAlignCenter);

// Add to the Window
layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(logo_layer));
	
	//Create hour TextLayer
	s_hour_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SYNC_BOLD_50));
	//Create minutes TextLayer
	s_minutes_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SYNC_BOLD_50));
	
	//Hour Layer
  #if defined(PBL_RECT) 
	  s_hour_layer = text_layer_create(GRect (0, 30, 144, 54));
  #elif defined(PBL_ROUND)
    s_hour_layer = text_layer_create(GRect (0, 35, 180, 54));
  #endif
	text_layer_set_background_color(s_hour_layer, GColorClear);
	text_layer_set_text_color(s_hour_layer, GColorBlack);
	text_layer_set_text(s_hour_layer, "00");
	
	//Minute Layer
	#if defined(PBL_RECT) 
    s_minutes_layer = text_layer_create(GRect (0, 74, 144, 54));
  #elif defined(PBL_ROUND) 
    s_minutes_layer = text_layer_create(GRect (0, 79, 180, 54));
  #endif
	text_layer_set_background_color(s_minutes_layer, GColorClear);
	text_layer_set_text_color(s_minutes_layer, GColorBlack);
	text_layer_set_text(s_minutes_layer, "00");
	
	//Make it look more like a watch
	text_layer_set_font(s_hour_layer, s_hour_font);
	text_layer_set_font(s_minutes_layer, s_minutes_font);
	text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
	text_layer_set_text_alignment(s_minutes_layer, GTextAlignmentCenter);
	
	//Add hour and miuntes layers as child windows to the Window's root layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hour_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_minutes_layer));
	
	
}

static void main_window_unload(Window *window) {
	
	//Unload GFonts
	fonts_unload_custom_font(s_hour_font);
	fonts_unload_custom_font(s_minutes_font);
	
	//Destroy hour layer
	text_layer_destroy(s_hour_layer);
	//Destroy minutes layer
	text_layer_destroy(s_minutes_layer);
	
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
 update_time();
}

static void init() {
	
	tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
	s_main_window = window_create();
	window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
	window_stack_push(s_main_window, true);
	update_time();
}

static void deinit() {
	//Destroy Window
	window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}