#include "term_renderer.h"
#include <algorithm>

DZTermRenderer::DZTermRenderer(int width, int height)
    : width(width)
    , height(height)
    , fill { TermColor::BLACK, TermColor::WHITE, '#'}
    , stroke { TermColor::BLACK, TermColor::WHITE, '#' }
    , text { TermColor::BLACK, TermColor::WHITE }
    , cursor { 0,0 }
{ 
    memset(&this->buf, 0, sizeof(this->buf));
    this->setDimensions(width, height);
}

v2i DZTermRenderer::getDimensions()
{
    return v2i { this->width, this->height };
}

void DZTermRenderer::setDimensions(int width, int height)
{
    if (this->buf.display) free(this->buf.display);
    if (this->buf.output)  free(this->buf.output);
    if (this->buf.fg)      free(this->buf.fg);
    if (this->buf.bg)      free(this->buf.bg);

    this->buf.display = (char *)  malloc(width * height);
    this->buf.fg      = (TermColor *) malloc(sizeof(TermColor) * width * height);
    this->buf.bg      = (TermColor *) malloc(sizeof(TermColor) * width * height);
    this->buf.output  = (char *)  malloc(
            MAX_OUTPUT_CHAR_WIDTH * (width + 1) * height);
}

void DZTermRenderer::setCursor(int x, int y)
{
    this->cursor.x = std::min(std::max(x, 0), this->width);
    this->cursor.y = std::min(std::max(y, 0), this->height);
}

void DZTermRenderer::moveCursor(int dx, int dy)
{
    this->setCursor(this->cursor.x + dx, this->cursor.y + dy);
}

void DZTermRenderer::setStroke(TermColor fg, TermColor bg, char c)
{
    this->stroke.fg = fg;
    this->stroke.bg = bg;
    this->stroke.c  = c;
}

void DZTermRenderer::setFill(TermColor fg, TermColor bg, char c)
{
    this->fill.fg = fg;
    this->fill.bg = bg;
    this->fill.c  = c;
}

void DZTermRenderer::putChar(char c, int x, int y)
{
    if (x >= width || y >= height || x < 0 || y < 0)
        return;
    this->buf.display[y * width + x] = c;
    this->buf.fg[y * width + x] = this->text.fg;
    this->buf.bg[y * width + x] = this->text.bg;
}

void DZTermRenderer::write(char *str)
{
    struct { int x, y; } original_cursor;

    original_cursor.x = this->cursor.x;
    original_cursor.y = this->cursor.y;

    int len = strlen(str);
    char *curr = str, *end = str + len;

    for (; curr < end; curr++)
    {
        if (*curr == '\n')
        {
            this->cursor.x = original_cursor.x;
            this->cursor.y += 1;
        }
        else
        {
            this->putChar(*curr, cursor.x, cursor.y);
            this->moveCursor(1, 0);
        }
    }
}

void DZTermRenderer::rect(int w, int h)
{
    for (int i = cursor.x; i < cursor.x + w && i < width; i++)
    {
        putChar(stroke.c, i, cursor.y);
        putChar(stroke.c, i, cursor.y + h - 1);
    }

    for (int i = cursor.y; i < cursor.y + h && i < height; i++)
    {
        putChar(stroke.c, cursor.x, i);
        putChar(stroke.c, cursor.x + w - 1, i);
    }

    for (int i = cursor.x + 1; i < cursor.x + w - 1; i++)
    {
        for (int j = cursor.y + 1; j < cursor.y + h - 1; j++)
        {
            putChar(fill.c, i, j);
        }
    }
}

void DZTermRenderer::clear()
{
    memset(this->buf.display, ' ', width * height);
    // TODO
    memset(this->buf.fg, 0, sizeof(TermColor) * width * height);
    memset(this->buf.bg, 0, sizeof(TermColor) * width * height);
}

void DZTermRenderer::display()
{
    char *curr = buf.output;
    memset(buf.output, 0, MAX_OUTPUT_CHAR_WIDTH * (width + 1) * height);

    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            char c = buf.display[j * width + i];
            TermColor fg = buf.fg[j * width + i];
            TermColor bg = buf.bg[j * width + i];

            // TODO: TermColor
            int n = sprintf(curr, "%c", c);
            curr += n;
        }
        int n = sprintf(curr, "\n");
        curr += n;
    }
    printf("%s", buf.output);
}




















