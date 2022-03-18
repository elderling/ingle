#ifndef MOTION_H
#define MOTION_H

extern void adjust_xpos_to_line_length (void);
extern void move_down            (void);
extern void move_up              (void);
extern void move_left            (void);
extern void move_right           (void);
extern void move_line_start      (void);
extern void move_line_end        (void);
extern void move_line_begin      (void);
extern void move_line_last_char  (void);
extern void move_last_line       (void);

#endif // #ifndef MOTION_H
