#include "pebble.h"
#include "feature_accel_discs.h"
#include "round_math.h"

double square(double num) {
  return num * num;
}

float get_sqrt( float num ) {
  // Uses Babylonian sequence to approximate root of num
  float approx, product, b_seq;
  float tolerance = 0.001;

  approx = num;
  product = square(approx);
  // Check if the square of our approximation is within the error tolerance of num
  while(product - num >= tolerance) {
    b_seq = ( approx + ( num / approx ) ) / 2;
    approx = b_seq;
    product = square(approx);
  }
  return approx;
}

Vec2d multiply(Vec2d vec, double scale) {
  return (Vec2d) { .x = vec.x * scale,
                   .y = vec.y * scale };
}

Vec2d add(Vec2d a, Vec2d b) {
  return (Vec2d) { .x = a.x + b.x,
                   .y = a.y + b.y };
}

Vec2d subtract(Vec2d a, Vec2d b) {
  return (Vec2d) { .x = a.x - b.x,
                   .y = a.y - b.y };
}

double get_length(Vec2d vec) {
  return get_sqrt(square(vec.x) + square(vec.y));
}

Vec2d set_length(Vec2d vec, double new_length, double old_length) {
  return (Vec2d) { .x = vec.x * new_length / old_length,
                   .y = vec.y * new_length / old_length};
}

double dot(Vec2d a, Vec2d b) {
  return a.x * b.x + a.y * b.y;
}

Vec2d normalize(Vec2d vec) {
  double length = get_length(vec);

  if (length != 0) {
    return (Vec2d) { .x = vec.x / length,
                     .y = vec.y / length };
  } else {
    return (Vec2d) { .x = vec.x,
                     .y = vec.y };
  }
}

Vec2d find_reflection_velocity(Vec2d bounds, Disc *disc) {
  Vec2d normal = normalize(subtract(disc->pos, bounds));

  // Solve for parallel and perpendicular components of the discs velocity vector
  Vec2d perpendicular = multiply(normal, dot(disc->vel, normal));
  Vec2d parallel = subtract(disc->vel, perpendicular);
  
  float friction = 1;
  float elasticity = 1;
  // Deflection vector off wall
  Vec2d vec_incident = subtract(multiply(parallel, friction), multiply(perpendicular, elasticity));

  return vec_incident;
}