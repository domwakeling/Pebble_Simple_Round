#include <pebble.h>

#define W_WIDTH 144
#define W_HEIGHT 168
#define ROT_X 71
#define ROT_Y 83
#define OR_X 0
#define OR_Y 0
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
#define MIN_REF 3								// how many times per minute the minute hand refreshes
#define POM_RAD 3
#define TEXT_Y 145
#define TEXT_X 4
#define TEXT_COLOUR GColorWhite

int hours_angle = -1;
int minutes_angle = -1;
int seconds;

Window *main_window;
Layer *hour_l, *minute_l, *second_l, *pommel_l;
TextLayer *date_text, *month_text;
GPath *hour_p, *minute_p, *second_p;
GBitmap *dial_map;
GFont *helvetica;
BitmapLayer *dial_layer;
char date_buffer[] = "00";
char month_buffer[] = "JUN";

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
	gpath_rotate_and_move(hour_p, hours_angle);
	graphics_context_set_stroke_color(ctx, HR_COLOUR);
	graphics_context_set_stroke_width(ctx, HR_STROKE);
	gpath_draw_outline_open(ctx, hour_p);
}

void minute_update_proc(Layer *l, GContext *ctx) {
	gpath_rotate_and_move(minute_p, minutes_angle);
	graphics_context_set_stroke_color(ctx, MN_COLOUR);
	graphics_context_set_stroke_width(ctx, MN_STROKE);
	gpath_draw_outline_open(ctx, minute_p);
}

void second_update_proc(Layer *l, GContext *ctx) {
	gpath_rotate_and_move(second_p, seconds * MAX_DEG / MAX_SEC);
	graphics_context_set_stroke_color(ctx, SEC_COLOUR);
	graphics_context_set_stroke_width(ctx, SEC_STROKE);
	gpath_draw_outline_open(ctx, second_p);
}

void pommel_update_proc(Layer *l, GContext *ctx) {
	graphics_context_set_stroke_color(ctx, SEC_COLOUR);
	graphics_context_set_fill_color(ctx, SEC_COLOUR);
	graphics_fill_circle(ctx, GPoint(ROT_X, ROT_Y), POM_RAD);
	graphics_draw_circle(ctx, GPoint(ROT_X, ROT_Y), POM_RAD);
}

void update_time() {

	// get a temp struct containing the current time
	time_t temp = time(NULL);
	struct tm *tick_time = localtime(&temp);
	
	// store the information
	int hours = (int)tick_time->tm_hour;
	int minutes = (int)tick_time->tm_min;
	seconds = (int)tick_time->tm_sec;
	
	if( (seconds == 0 && hours == 0 && minutes == 0 ) || hours_angle == -1) {
		// get the date into the buffers
		strftime(date_buffer, sizeof(date_buffer), "%d", tick_time);
		strftime(month_buffer, sizeof(month_buffer), "%b", tick_time);
		
		// convert month to upper case
		u_int i = 0;
		for (i = 0; i < strlen(month_buffer) ; i++) {
			if( (month_buffer[i] >= 97) && (month_buffer[i] <= 122) ) {
				month_buffer[i] = month_buffer[i] - 32;
			} 
		}
		
		// set the layers
		text_layer_set_text(date_text, date_buffer);
		text_layer_set_text(month_text, month_buffer);		
	}
	
	// draw the hours hand if necessary
	if(hours > 11) hours -= MAX_HR;
	if( (seconds == 0 && minutes%2 == 0 ) || hours_angle == -1 ) {
		hours_angle = hours * MAX_DEG / MAX_HR + minutes * (MAX_DEG / MAX_HR) / MAX_MN;
		layer_mark_dirty(hour_l);
	}

	// draw the minutes hand if necessary
	if( (seconds % (MAX_SEC/MIN_REF)) == 0 || minutes_angle == -1) {
		minutes_angle = minutes * MAX_DEG / MAX_MN + seconds * (MAX_DEG / MAX_MN) / MAX_SEC;
		layer_mark_dirty(minute_l);
	}
	
	// draw the seconds hand
	layer_mark_dirty(second_l);	
}

void tick_handler(struct tm *tick_time, TimeUnits unit_changed) {
	update_time();
}

void main_window_load(Window *w) {
	// create the layers
	GRect r = GRect(0,0,W_WIDTH, W_HEIGHT);
	dial_layer = bitmap_layer_create(r);
	hour_l = layer_create(r);
	minute_l = layer_create(r);
	second_l = layer_create(r);
	pommel_l = layer_create(r);
	
	// load custom font
	helvetica = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_HELVETICA_18));
	
	// create text layers
	GRect fr = GRect(TEXT_X, TEXT_Y, W_WIDTH - 2*TEXT_X, W_HEIGHT - TEXT_Y);
	date_text = text_layer_create(fr);
	month_text = text_layer_create(fr);
	
	// format text layers
	text_layer_set_text_color(date_text, TEXT_COLOUR);
	text_layer_set_text_color(month_text, TEXT_COLOUR);
	text_layer_set_background_color(date_text, GColorClear);
	text_layer_set_background_color(month_text, GColorClear);
	text_layer_set_font(date_text, helvetica);
	text_layer_set_font(month_text, helvetica);
	text_layer_set_text_alignment(date_text, GTextAlignmentLeft);
	text_layer_set_text_alignment(month_text, GTextAlignmentRight);
	
	// create the paths before update procs can call them
	hour_p = gpath_create(&hour_hand_info);
	minute_p = gpath_create(&minute_hand_info);
	second_p = gpath_create(&second_hand_info);
	
	// set the layer update procs
	layer_set_update_proc(hour_l, hour_update_proc);
	layer_set_update_proc(minute_l, minute_update_proc);
	layer_set_update_proc(second_l, second_update_proc);
	layer_set_update_proc(pommel_l, pommel_update_proc);
	
	// create the bitmap and set the bitmap layer
	dial_map = gbitmap_create_with_resource(RESOURCE_ID_DIAL);
	bitmap_layer_set_bitmap(dial_layer, dial_map);
	
	// add them to the window
	Layer *w_layer = window_get_root_layer(w);
	layer_add_child(w_layer, (Layer *)dial_layer);
	layer_add_child(w_layer, hour_l);
	layer_add_child(w_layer, minute_l);
	layer_add_child(w_layer, second_l);
	layer_add_child(w_layer, pommel_l);
	layer_add_child(w_layer, (Layer *)date_text);
	layer_add_child(w_layer, (Layer *)month_text);
	
	update_time();		// ensure we're showing the correct time
} 

void main_window_unload(Window *w) {
	// destroy reources
	gbitmap_destroy(dial_map);
	gpath_destroy(hour_p);
	gpath_destroy(minute_p);
	gpath_destroy(second_p);
	fonts_unload_custom_font(helvetica);
	
	// destroy the layers
	bitmap_layer_destroy(dial_layer);
	layer_destroy(hour_l);
	layer_destroy(minute_l);
	layer_destroy(second_l);
	layer_destroy(pommel_l);
	text_layer_destroy(date_text);
	text_layer_destroy(month_text);
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