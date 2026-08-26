#ifndef MISC_H
#define MISC_H

#define TABLE_BLEND (Color){ 0, 0, 0, 64 }
#define POKAJAN_DARK_BLUE (Color){ 32, 152, 255, 255 }
#define POKAJAN_LIGHT_BLUE (Color){ 199, 241, 255, 255 }

#define SCREEN_W 1920
#define SCREEN_H 1080
#define LN_70 4.24

#define F_SKIP_TO_GAME 1
#define F_DEBUG 1

// anchor macros, refer to numpad

#define ANCHOR_7 (Vector2){ 0, 0 }
#define ANCHOR_8(x_, y_, scale_) (Vector2){ x_ / 2.0f * scale_, 0.0f }
#define ANCHOR_9(x_, y_, scale_) (Vector2){ x_ * scale_, 0.0f }

#define ANCHOR_4(x_, y_, scale_) (Vector2){ 0.0f, y_ / 2.0f * scale_ }
#define ANCHOR_5(x_, y_, scale_) (Vector2){ x_ / 2.0f * scale_, y_ / 2.0f * scale_ }
#define ANCHOR_6(x_, y_, scale_) (Vector2){ x_ * scale_, y_ / 2.0f * scale_ }

#define ANCHOR_1(x_, y_, scale_) (Vector2){ 0.0f, y_ * scale_ }
#define ANCHOR_2(x_, y_, scale_) (Vector2){ x_ / 2.0f * scale_, y_ * scale_ }
#define ANCHOR_3(x_, y_, scale_) (Vector2){ x_ * scale_, y_ * scale_ }

#endif