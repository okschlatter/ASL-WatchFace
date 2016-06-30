#include <pebble.h>
#include <pebble_fonts.h>
#include <pebble-events/pebble-events.h>
#include <pebble-owm-weather/owm-weather.h>


#define BackroundColor     
#define TempUnit           

static Window *s_main_window;
static TextLayer *s_hour_layer;
static TextLayer *s_minutes_layer;
static TextLayer *s_temp_layer;
static TextLayer *s_short_layer;
static GFont s_hour_font;
static GFont s_minutes_font;
static BitmapLayer *logo_layer;
static OWMWeatherInfo *s_info;
static OWMWeatherCallback *s_callback;
static OWMWeatherStatus s_status;
static GBitmap *logo_bitmap;
static char temp_unit[2];
static char temp_buffer[128];

 // Replace this with your own API key from OpenWeatherMap.org
static char *api_key = "50ef49bbe9fe20384c1756a17338d49c";

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Inbox called!");
  
  // Get color
  Tuple *bg_color_t = dict_find(iter, MESSAGE_KEY_BackgroundColor);
  if(bg_color_t){
    GColor bg_color = GColorFromHEX(bg_color_t->value->int32);
    window_set_background_color(s_main_window, bg_color);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Set new Background Color");
  }
  
  Tuple *temp_unit_t = dict_find(iter, MESSAGE_KEY_TempUnit);
  if(temp_unit_t){
    char *temp_unit = temp_unit_t->value->cstring;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Set new Temp Unit");
  }
  
  Tuple *reply_tuple = dict_find(iter, MESSAGE_KEY_Reply);
  if(reply_tuple) {

    Tuple *desc_short_tuple = dict_find(iter, MESSAGE_KEY_DescriptionShort);
    strncpy(s_info->description_short, desc_short_tuple->value->cstring, OWM_WEATHER_BUFFER_SIZE);

    Tuple *temp_tuple = dict_find(iter, MESSAGE_KEY_TempK);
    s_info->temp_k = temp_tuple->value->int32;
    s_info->temp_c = s_info->temp_k - 273;
    s_info->temp_f = ((s_info->temp_c) * 1.8) + 32;

    s_status = OWMWeatherStatusAvailable;
    app_message_deregister_callbacks();
    s_callback(s_info, s_status);
    
      if(*temp_unit == 'F'){
        snprintf(temp_buffer, sizeof(temp_buffer),         
        (*temp_f) + " \u00B0" + 'F');
      } else if(*temp_unit == 'C'){
        snprintf(*temp_buffer, sizeof(temp_buffer),         
        (*temp_c) + " \u00B0" + 'C');
      } else {
        snprintf(temp_buffer, sizeof(temp_buffer),         
        (*temp_k) + " \u00B0" + 'K');
      };
      text_layer_set_text(s_temp_layer, temp_buffer);

      APP_LOG(APP_LOG_LEVEL_DEBUG, temp_buffer);
      static char short_buffer[128];
      snprintf(short_buffer, sizeof(short_buffer),
      description_short);
              text_layer_set_text(s_short_layer, short_buffer);
  }

  Tuple *err_tuple = dict_find(iter, MESSAGE_KEY_BadKey);
  if(err_tuple) {
    s_status = OWMWeatherStatusBadKey;
    s_callback(s_info, s_status);
    text_layer_set_text(s_temp_layer, "Bad Key!");
    text_layer_set_text(s_short_layer, "\U0001F633");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Bad Key!");
  }

  err_tuple = dict_find(iter, MESSAGE_KEY_LocationUnavailable);
  if(err_tuple) {
    s_status = OWMWeatherStatusLocationUnavailable;
    s_callback(s_info, s_status);
    text_layer_set_text(s_temp_layer, "Location Unavailable");
    text_layer_set_text(s_short_layer, "\U0001F608");
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Location Unavailable");
  }

}

// static void fail_and_callback() {
//   APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to send request!");
//   s_status = OWMWeatherStatusFailed;
//   s_callback(s_info, s_status);
// }

// static bool fetch() {
//   DictionaryIterator *out;
//   AppMessageResult result = app_message_outbox_begin(&out);
//   if(result != APP_MSG_OK) {
//     fail_and_callback();
//     return false;
//   }

//   dict_write_cstring(out, MESSAGE_KEY_Request, s_api_key);

//   result = app_message_outbox_send();
//   if(result != APP_MSG_OK) {
//     fail_and_callback();
//     return false;
//   }

//   s_status = OWMWeatherStatusPending;
//   s_callback(s_info, s_status);
//   return true;
// }

// void owm_weather_init(char *api_key) {
//   if(s_info) {
//     free(s_info);
//   }

//   if(!api_key) {
//     APP_LOG(APP_LOG_LEVEL_ERROR, "API key was NULL!");
//     return;
//   }

//   strncpy(s_api_key, api_key, sizeof(s_api_key));

//   s_info = (OWMWeatherInfo*)malloc(sizeof(OWMWeatherInfo));
//   s_status = OWMWeatherStatusNotYetFetched;
//   events_app_message_request_inbox_size(2026);
//   events_app_message_request_outbox_size(656);
//   events_app_message_register_inbox_received(inbox_received_handler, NULL);
// }

// bool owm_weather_fetch(OWMWeatherCallback *callback) {
//   if(!s_info) {
//     APP_LOG(APP_LOG_LEVEL_ERROR, "OWM Weather library is not initialized!");
//     return false;
//   }

//   if(!callback) {
//     APP_LOG(APP_LOG_LEVEL_ERROR, "OWMWeatherCallback was NULL!");
//     return false;
//   }

//   s_callback = callback;

//   if(!bluetooth_connection_service_peek()) {
//     s_status = OWMWeatherStatusBluetoothDisconnected;
//     s_callback(s_info, s_status);
//     return false;
//   }

//   return fetch();
// }

// void owm_weather_deinit() {
//   if(s_info) {
//     free(s_info);
//     s_info = NULL;
//     s_callback = NULL;
//   }
// }

// OWMWeatherInfo* owm_weather_peek() {
//   if(!s_info) {
//     APP_LOG(APP_LOG_LEVEL_ERROR, "OWM Weather library is not initialized!");
//     return NULL;
//   }

//   return s_info;
// }

static void js_ready_handler(void *context) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "js_ready_handler called!");
  owm_weather_fetch(weather_callback);
}

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
	
#if defined(PBL_BW)
    window_set_background_color(s_main_window, GColorWhite);
#elif defined(PBLE_COLOR)
  int red = persist_read_int(KEY_COLOR_RED);
  int green = persist_read_int(KEY_COLOR_GREEN);
  int blue = persist_read_int(KEY_COLOR_BLUE);
  
  GColor bg_color = GColorFromRGB(red, green, blue);
  window_set_background_color(s_main_window, bg_color);
#endif
  
  // Get the Window's root layer and the bounds
  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  
  // Load the image
  #ifdef PBL_COLOR
    logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO);
  #else
    logo_bitmap = gbitmap_create_with_resource(RESOURCE_ID_LOGO_BW);
  #endif

// Create a BitmapLayer
logo_layer = bitmap_layer_create(bounds);

// Set the bitmap and center it
bitmap_layer_set_bitmap(logo_layer, logo_bitmap);
bitmap_layer_set_alignment(logo_layer, GAlignCenter);
bitmap_layer_set_compositing_mode(logo_layer, GCompOpSet);

// Add to the Window
layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(logo_layer));
	
	// Set font hour TextLayer
	s_hour_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_SYNC_BOLD_50));
	// Set font minutes TextLayer
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
	
  //Temp layer
  #if defined(PBL_RECT)
    s_temp_layer = text_layer_create(GRect (0, 0, 144, 45));
  #elif defined(PBL_ROUND)
    s_temp_layer = text_layer_create(GRect (0, 0, 180, 45));
  #endif
  text_layer_set_background_color(s_temp_layer, GColorClear);
  text_layer_set_text_color(s_temp_layer, GColorBlack);
  text_layer_set_text(s_temp_layer, "Ready!");
  
  //Short Layer
  #if defined(PBL_RECT)
    s_short_layer = text_layer_create(GRect (0, 130, 144, 18));
  #elif defined(PBL_ROUND)
    s_short_layer = text_layer_create(GRect (0, 145, 180, 30));
  #endif
  text_layer_set_background_color(s_short_layer, GColorClear);
  text_layer_set_text_color(s_short_layer, GColorBlack);
  text_layer_set_text(s_short_layer, "\U0001F61C");
  
	//Make it look more like a watch
	text_layer_set_font(s_hour_layer, s_hour_font);
	text_layer_set_font(s_minutes_layer, s_minutes_font);
  text_layer_set_font(s_temp_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_font(s_short_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
	text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
	text_layer_set_text_alignment(s_minutes_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_temp_layer, GTextAlignmentCenter);
  text_layer_set_text_alignment(s_short_layer, GTextAlignmentCenter);
	
	//Add hour, miuntes, and weather layers as child windows to the Window's root layer
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_hour_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_minutes_layer));
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_temp_layer));
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_short_layer));
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

  owm_weather_init(api_key);
  events_app_message_open();

  app_timer_register(3000, js_ready_handler, NULL);
  
  app_message_register_inbox_received(inbox_received_handler);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
	//Destroy Window
	window_destroy(s_main_window);
  owm_weather_deinit();
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}