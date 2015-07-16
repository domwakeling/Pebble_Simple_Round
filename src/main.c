#include <pebble.h>

#define W_WIDTH 144
#define W_HEIGHT 168
//#define BG_COLOUR GColorBlack
//#define FG_COLOUR GColorWhite
#define ROT_X 71
#define ROT_Y 83
#define OR_X 0
#define OR_Y 0
//#define FG_DIA 65
#define HR_LENGTH 40
#define HR_STROKE 3
#define HR_COLOUR GColorBlack
#define MN_LENGTH 60
#define MN_STROKE 3
#define MN_COLOUR GColorBlack
#define SEC_LENGTH 60
#define SEC_STROKE 1
#define SEC_COLOUR GColorRed
#define MAX_HR 12
#define MAX_MN 60
#define MAX_SEC 60
#define MAX_DEG 360
#define MIN_REF 3								// how many times/minute the minute hand refreshes
#define POM_RAD 3
//#define TEXT_W 25
//#define TEXT_H 20
//#define TEXT_RAD 58

int hours, minutes, seconds;	

Window *main_window;
//Layer *background, *hour_l, *minute_l, *second_l, *pommel_l; 
Layer *hour_l, *minute_l, *second_l, *pommel_l; 
GPath *hour_p, *minute_p, *second_p;
//bool drawing_hands = false;
bool drawing_all = false;
//TextLayer *text_layers[12];
//GFont eurostile_font;
GBitmap *dial_map;
BitmapLayer *dial_layer;

GPathInfo hour_hand_info = {
	.num_points = 2,
	.points = (GPoint[]) { {OR_X, OR_Y - HR_LENGTH} , {OR_X, OR_Y} }
};

GPathInfo minute_hand_info = {
	.num_points = 2,
	.points = (GPoint[]) { {OR_X, OR_Y - MN_LENGTH} , {OR_X, OR_Y} }
};

GPathInfo second_hand_info = {
	.num_points = 2,
	.points = (GPoint[]) { {OR_X , OR_Y - SEC_LENGTH} , {OR_X, OR_Y} }
};

/* void background_update_proc(Layer *l, GContext *ctx) {
	graphics_context_set_fill_color(ctx, BG_COLOUR);
	graphics_fill_rect(ctx, GRect(0,0,W_WIDTH,W_HEIGHT), 0, GCornerNone);
	graphics_context_set_fill_color(ctx, FG_COLOUR);
	graphics_context_set_stroke_color(ctx, FG_COLOUR);
	graphics_fill_circle(ctx, GPoint(ROT_X,ROT_Y), FG_DIA);
	graphics_draw_circle(ctx, GPoint(ROT_X,ROT_Y), FG_DIA);
} */

void gpath_rotate_and_move(GPath *p, int degs) {
	// store the angle
	int32_t angle = TRIG_MAX_ANGLE * degs / MAX_DEG;
	
	// rotate the path to new angle
	gpath_rotate_to(p, angle);
	
	// move the path from origin to rotation point
	int x = ROT_X - OR_X;
	int y = ROT_Y - OR_Y;
	gpath_move_to(p, GPoint(x,y) );
}

void hour_update_proc(Layer *l, GContext *ctx) {
	//if(drawing_hands) {
		int deg = hours * MAX_DEG / MAX_HR + minutes * (MAX_DEG / MAX_HR) / MAX_MN;
		gpath_rotate_and_move(hour_p, deg);
		graphics_context_set_stroke_color(ctx, HR_COLOUR);
		graphics_context_set_stroke_width(ctx, HR_STROKE);
		gpath_draw_outline_open(ctx, hour_p);
	//}
}

void minute_update_proc(Layer *l, GContext *ctx) {
	//if(drawing_hands) {
		int deg = minutes * MAX_DEG / MAX_MN + seconds * (MAX_DEG / MAX_MN) / MAX_SEC;
		gpath_rotate_and_move(minute_p, deg);
		graphics_context_set_stroke_color(ctx, MN_COLOUR);
		graphics_context_set_stroke_width(ctx, MN_STROKE);
		gpath_draw_outline_open(ctx, minute_p);
	//}
}

void second_update_proc(Layer *l, GContext *ctx) {
	//if(drawing_hands) {
		gpath_rotate_and_move(second_p, seconds * MAX_DEG / MAX_SEC);
		graphics_context_set_stroke_color(ctx, SEC_COLOUR);
		graphics_context_set_stroke_width(ctx, SEC_STROKE);
		gpath_draw_outline_open(ctx, second_p);
	//}
}

void pommel_update_proc(Layer *l, GContext *ctx) {
	graphics_context_set_stroke_color(ctx, SEC_COLOUR);
	graphics_context_set_fill_color(ctx, SEC_COLOUR);
	graphics_fill_circle(ctx, GPoint(ROT_X, ROT_Y), POM_RAD);
	graphics_draw_circle(ctx, GPoint(ROT_X, ROT_Y), POM_RAD);
}

void update_time() {

	//drawing_hands = true;
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	// store the information
	hours = (int)tick_time->tm_hour;
	minutes = (int)tick_time->tm_min;
	seconds = (int)tick_time->tm_sec;
	
	if(hours > 11) hours -= MAX_HR;
	
	layer_mark_dirty(second_l);
	if(seconds % (MAX_SEC/MIN_REF) == 0 || drawing_all == false) layer_mark_dirty(minute_l);
	if((seconds == 0 && minutes%2 == 0 ) || drawing_all == false ) layer_mark_dirty(hour_l);
	
	drawing_all = true;
}

void tick_handler(struct tm *tick_time, TimeUnits unit_changed) {
	update_time();
}

void main_window_load(Window *w) {
	// create the layers
	GRect r = GRect(0,0,W_WIDTH, W_HEIGHT);
	//background = layer_create(r);
	dial_layer = bitmap_layer_create(r);
	hour_l = layer_create(r);
	minute_l = layer_create(r);
	second_l = layer_create(r);
	pommel_l = layer_create(r);
	
	// create the paths before update procs can call them
	hour_p = gpath_create(&hour_hand_info);
	minute_p = gpath_create(&minute_hand_info);
	second_p = gpath_create(&second_hand_info);
	
	// set the layer update procs
	//layer_set_update_proc(background, background_update_proc);
	layer_set_update_proc(hour_l, hour_update_proc);
	layer_set_update_proc(minute_l, minute_update_proc);
	layer_set_update_proc(second_l, second_update_proc);
	layer_set_update_proc(pommel_l, pommel_update_proc);
	
	// set the bitmap layer
	dial_map = gbitmap_create_with_resource(RESOURCE_ID_DIAL);
	bitmap_layer_set_bitmap(dial_layer, dial_map);
	
	// add them to the window
	Layer *w_layer = window_get_root_layer(w);
	//layer_add_child(w_layer, background);
	layer_add_child(w_layer, (Layer *)dial_layer);
	layer_add_child(w_layer, hour_l);
	layer_add_child(w_layer, minute_l);
	layer_add_child(w_layer, second_l);
	layer_add_child(w_layer, pommel_l);
	
	// load font
	//eurostile_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_EUROSTILE_13));
	
	// create the text layers
	/*for(int i = 0; i < 12; i++) {
		
		// calculate the origin for each layer and set rectangle
		int32_t angle = TRIG_MAX_ANGLE * i / MAX_HR;
		float x_pos = ROT_X - TEXT_RAD * sin_lookup(angle) / TRIG_MAX_RATIO - TEXT_W / 2 ;
		float y_pos = ROT_Y - TEXT_RAD * cos_lookup(angle) / TRIG_MAX_RATIO - TEXT_H / 2;
		if(i > 0 && i < 6) x_pos += 1;			// tweak the left hand numbers to right
		if(i == 2) x_pos += 1;							// 10 needs an additional push
		if(i >= 4 && i <= 8) y_pos -= 1;		// tweak bottom numbers up
		if(i >= 5 && i <= 7) y_pos -= 1;		// and again for the bottom three
		GRect r = GRect(x_pos,y_pos,TEXT_W,TEXT_H);
		text_layers[i] = text_layer_create(r);
		
		// set format and add to background layer
		text_layer_set_background_color(text_layers[i], GColorClear);
		text_layer_set_text_color(text_layers[i], GColorBlack);
		//text_layer_set_font(text_layers[i], eurostile_font);
		text_layer_set_font(text_layers[i], fonts_get_system_font(FONT_KEY_GOTHIC_18));
		text_layer_set_text_alignment(text_layers[i], GTextAlignmentCenter);
		layer_add_child(background, text_layer_get_layer(text_layers[i]));
	}
	
	// once they're all created, set the text contents
	text_layer_set_text(text_layers[11], "1");
	text_layer_set_text(text_layers[10], "2");
	text_layer_set_text(text_layers[9], "3");
	text_layer_set_text(text_layers[8], "4");
	text_layer_set_text(text_layers[7], "5");
	text_layer_set_text(text_layers[6], "6");
	text_layer_set_text(text_layers[5], "7");
	text_layer_set_text(text_layers[4], "8");
	text_layer_set_text(text_layers[3], "9");
	text_layer_set_text(text_layers[2], "10");
	text_layer_set_text(text_layers[1], "11");
	text_layer_set_text(text_layers[0], "12"); */
	
	update_time();
} 

void main_window_unload(Window *w) {
	// destroy the font
	//fonts_unload_custom_font(eurostile_font);
	
	// destroy the text layers
	/* for(int i = 0; i < 12; i++) {
		text_layer_destroy(text_layers[i]);
	} */
	
	gbitmap_destroy(dial_map);
	gpath_destroy(hour_p);
	gpath_destroy(minute_p);
	gpath_destroy(second_p);
	
	// destroy the layers
	//layer_destroy(background);
	bitmap_layer_destroy(dial_layer);
	layer_destroy(hour_l);
	layer_destroy(minute_l);
	layer_destroy(second_l);
	layer_destroy(pommel_l);
	
}

void init() {
	// create the window
	main_window = window_create();
	
	// set it's handlers
	window_set_window_handlers(main_window, (WindowHandlers) {
		.load = main_window_load,
		.unload = main_window_unload
	});
	
	// subscribe to tickhandler & set handler
	tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
	
	// push the window onto the stack
	window_stack_push(main_window, true);
}


void deinit() {
	window_destroy(main_window);
}


int main() {
	init();
	app_event_loop();
	deinit();
}