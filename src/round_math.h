#pragma once

double square(double num);
float get_sqrt( float num );
Vec2d multiply(Vec2d vec, double scale);
Vec2d add(Vec2d a, Vec2d b);
Vec2d subtract(Vec2d a, Vec2d b);
double get_length(Vec2d vec);
Vec2d set_length(Vec2d vec, double new_length, double old_length);
double dot(Vec2d a, Vec2d b);
Vec2d normalize(Vec2d vec);
Vec2d find_reflection_velocity(Vec2d bounds, Disc *disc);