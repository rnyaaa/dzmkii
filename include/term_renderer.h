#include <string>
#include "geometry.h"

#ifndef _TERM_RENDERER_H
#define _TERM_RENDERER_H

enum class TermColor
{
    WHITE,
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE
};

#define MAX_OUTPUT_CHAR_WIDTH 8

class DZTermRenderer
{
public:
    DZTermRenderer(int width, int height);

    void setDimensions(int width, int height);
    v2i getDimensions();

    void setCursor(int x, int y);
    void moveCursor(int x, int y);

    void clear();

    void setText(TermColor fg, TermColor bg, char c);
    void setStroke(TermColor fg, TermColor bg, char c);
    void setFill(TermColor fg, TermColor bg, char c);

    void write(char *str);
    void rect(int width, int height);
    void putChar(char c, int x, int y);
    void display();
private:

    int width, height;
    struct { TermColor bg, fg; char c; } fill, stroke;
    struct { TermColor bg, fg; } text;
    struct { int x, y; } cursor;
    struct {
        TermColor *fg, *bg;
        char *display;
        char *output;
    } buf;
};

#endif // _TERM_RENDERER_H
