#include "pebble.h"
#include "feature_accel_discs.h"
#include "round_math.h"

#define MATH_PI 3.141592653589793238462
#define NUM_DISCS 20
#define DISC_DENSITY 0.25
#define ACCEL_RATIO 0.05
#define ACCEL_STEP_MS 50

static Window *s_main_window;
static Layer *s_disc_layer;

static Disc disc_array[NUM_DISCS];
static GRect window_frame;

static double disc_calc_mass(Disc *disc) {
  return MATH_PI * disc->radius * disc->radius * DISC_DENSITY;
}

static void disc_init(Disc *disc) {
  static double next_radius = 3;

  GRect frame = window_frame;
  disc->pos.x = frame.size.w / 2;
  disc->pos.y = frame.size.h / 2;
  disc->vel.x = 0;
  disc->vel.y = 0;
  disc->radius = next_radius;
  disc->mass = disc_calc_mass(disc);
#ifdef PBL_COLOR
  disc->color = GColorFromRGB(rand() % 255, rand() % 255, rand() % 255);
#endif
  next_radius += 0.5;
}

static void disc_apply_force(Disc *disc, Vec2d force) {
  disc->vel.x += force.x / disc->mass;
  disc->vel.y += force.y / disc->mass;
}

static void disc_apply_accel(Disc *disc, AccelData accel) {
  disc_apply_force(disc, (Vec2d) {
    .x = accel.x * ACCEL_RATIO,
    .y = -accel.y * ACCEL_RATIO
  });
}

static void disc_update(Disc *disc) {
  double e = PBL_IF_ROUND_ELSE(0.7, 0.5);

  // update disc position
  disc->pos.x += disc->vel.x;
  disc->pos.y += disc->vel.y;

#ifdef PBL_ROUND
  // -1 accounts for how pixels are drawn onto the screen. Pebble round has a 180x180 pixel screen.
  // Since this is an even number, the centre of the screen is a line separating two side by side
  // pixels. Thus, if you were to draw a pixel at (90, 90), it would show up on the bottom right
  // pixel from the center point of the screen.
  Vec2d circle_center = (Vec2d) { .x = window_frame.size.w / 2 - 1,
                                  .y = window_frame.size.h / 2 - 1 };

  if ((square(circle_center.x - disc->pos.x) + square(circle_center.y - disc->pos.y)) > square(circle_center.x - disc->radius)) {
    // Check to see whether disc is within screen radius
    Vec2d norm = subtract(disc->pos, circle_center);
    if (get_length(norm) > (circle_center.x - disc->radius)) {
      norm = set_length(norm, (circle_center.x - disc->radius), get_length(norm));
      disc->pos = add(circle_center, norm);
    }
    disc->vel = multiply(find_reflection_velocity(circle_center, disc), e);
  }
#else
  if ((disc->pos.x - disc->radius < 0 && disc->vel.x < 0)
    || (disc->pos.x + disc->radius > window_frame.size.w && disc->vel.x > 0)) {
    disc->vel.x = -disc->vel.x * e;
  }

  if ((disc->pos.y - disc->radius < 0 && disc->vel.y < 0)
    || (disc->pos.y + disc->radius > window_frame.size.h && disc->vel.y > 0)) {
    disc->vel.y = -disc->vel.y * e;
  }
#endif
}

static void disc_draw(GContext *ctx, Disc *disc) {
  graphics_context_set_fill_color(ctx, PBL_IF_COLOR_ELSE(disc->color, GColorWhite));

  graphics_fill_circle(ctx, GPoint(disc->pos.x, disc->pos.y), disc->radius);
}

static void disc_layer_update_callback(Layer *me, GContext *ctx) {
  for (int i = 0; i < NUM_DISCS; i++) {
    disc_draw(ctx, &disc_array[i]);
  }
}

static void timer_callback(void *data) {
  AccelData accel = (AccelData) { .x = 0, .y = 0, .z = 0 };
  accel_service_peek(&accel);

  for (int i = 0; i < NUM_DISCS; i++) {
    Disc *disc = &disc_array[i];
    disc_apply_accel(disc, accel);
    disc_update(disc);
  }

  layer_mark_dirty(s_disc_layer);

  app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect frame = window_frame = layer_get_frame(window_layer);

  s_disc_layer = layer_create(frame);
  layer_set_update_proc(s_disc_layer, disc_layer_update_callback);
  layer_add_child(window_layer, s_disc_layer);

  for (int i = 0; i < NUM_DISCS; i++) {
    disc_init(&disc_array[i]);
  }
}

static void main_window_unload(Window *window) {
  layer_destroy(s_disc_layer);
}

static void init(void) {
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);

  accel_data_service_subscribe(0, NULL);

  app_timer_register(ACCEL_STEP_MS, timer_callback, NULL);
}

static void deinit(void) {
  accel_data_service_unsubscribe();

  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
