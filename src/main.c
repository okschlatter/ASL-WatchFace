#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;  
static TextLayer *s_weather_layer;
static GFont s_time_font;
static GFont s_weather_font;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
bool invert;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  //TEST:
   //Use 12 hour format
//strftime(buffer, sizeof("00:00"),"%H*%M", tick_time);
//PRODUCTION:
   // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    // Use 24 hour format
	  strftime(buffer, sizeof("00:00:00"), "%H:%M", tick_time);
   } else {
     // Use 12 hour format
     strftime(buffer, sizeof("00:00:00"),"%I:%M", tick_time);
   }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void tap_handler(AccelAxisType axis, int32_t direction) {
 APP_LOG(APP_LOG_LEVEL_INFO, "Detected Tap/Twist");
	
	if (invert == 1){
		invert = 0;
		APP_LOG(APP_LOG_LEVEL_INFO, "Set bool invert as 0");
		
		//Change the colors
	text_layer_set_background_color(s_weather_layer, GColorBlack);
	text_layer_set_text_color(s_weather_layer, GColorWhite);
	text_layer_set_background_color(s_time_layer, GColorWhite);
	text_layer_set_text_color(s_time_layer, GColorBlack);
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SPARK_ICON_LIGHT);
	s_background_layer = bitmap_layer_create(GRect(0, 1, 144, 180));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	} 
	else {
		invert = 1;
		APP_LOG(APP_LOG_LEVEL_INFO, "Set bool invert as 1");
		
		//Change the colors
	text_layer_set_background_color(s_weather_layer, GColorWhite);
	text_layer_set_text_color(s_weather_layer, GColorBlack);
	text_layer_set_background_color(s_time_layer, GColorBlack);
	text_layer_set_text_color(s_time_layer, GColorWhite);
	s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SPARK_ICON);
	s_background_layer = bitmap_layer_create(GRect(0, 1, 144, 180));
	bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
	}
}

static void main_window_load(Window *window) {
  // Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_SPARK_ICON);
  s_background_layer = bitmap_layer_create(GRect(0, 1, 144, 180));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create temperature Layer
  s_weather_layer = text_layer_create(GRect(0, 0, 144, 15));
  //A file called "Line 66" would go here
  text_layer_set_background_color(s_weather_layer, GColorWhite);
  text_layer_set_text_color(s_weather_layer, GColorBlack);
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentCenter);
  text_layer_set_text(s_weather_layer, "Loading...");
  
  // Create time TextLayer
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SYNC_BOLD_24));
  // Create second custom font, apply it and add to Window
  s_weather_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SYNC_14));

  //Weather layer
  text_layer_set_font(s_weather_layer, s_weather_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_weather_layer));

  //time layer
  s_time_layer = text_layer_create(GRect(0, 144, 144, 24));
  text_layer_set_background_color(s_time_layer, GColorBlack);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");

  // Improve the layout to be more like a watchface
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
	
	//Subscribe to Accelerometer tap service
	accel_tap_service_subscribe(tap_handler);
}

static void main_window_unload(Window *window) {
    
    // Unload GFont
    fonts_unload_custom_font(s_time_font);
    // Destroy time layer
    text_layer_destroy(s_time_layer);
    // Destroy weather layer
    text_layer_destroy(s_weather_layer);
    // destroy custom font
    fonts_unload_custom_font(s_weather_font);
    // how to destroy the background image?
}

//register all events
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
 update_time();
  // Get weather update every 30 minutes
  if(tick_time->tm_min % 1 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);
  
    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);
  
    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char temperature_buffer[8];
  static char conditions_buffer[32];
  static char weather_layer_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_TEMPERATURE:
      snprintf(temperature_buffer, sizeof(temperature_buffer), "%dF", (int)t->value->int32);
      break;
    case KEY_CONDITIONS:
      snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_weather_layer, weather_layer_buffer);
}



static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}


static void init() {
 
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Make sure the time is displayed from the start
  update_time();
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}