//-------------------------------------------------------------------------
//
// The MIT License (MIT)
//
// Copyright (c) 2013 Andrew Duncan
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
//-------------------------------------------------------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "life.h"

//-------------------------------------------------------------------------

#ifndef ALIGN_TO_16
#define ALIGN_TO_16(x)  ((x + 15) & ~15)
#endif

//-------------------------------------------------------------------------

#define LIVE 0xFFFF
#define LIVE2 0x1FFFE
#define LIVE3 0x2FFFD
#define DEAD 0x0

//-------------------------------------------------------------------------

void
newLife(
    LIFE_T *life,
    int32_t size)
{
    life->width = size;
    life->height = size;
    life->alignedWidth = ALIGN_TO_16(life->width);
    life->alignedHeight = ALIGN_TO_16(life->height);
    life->pitch = ALIGN_TO_16(life->width) * 2;

    life->buffer = calloc(1, life->pitch * life->alignedHeight);

    if (life->buffer == NULL)
    {
        fprintf(stderr, "life: memory exhausted\n");
        exit(EXIT_FAILURE);
    }

    life->last = calloc(1, life->pitch * life->alignedHeight);

    if (life->last == NULL)
    {
        fprintf(stderr, "life: memory exhausted\n");
        exit(EXIT_FAILURE);
    }

    struct timeval tv;
    gettimeofday(&tv, NULL);
    srand(tv.tv_usec);

    int32_t row = 0;
    for (row = 0 ; row < life->height ; row++)
    {
        int32_t col = 0;
        for (col = 0 ; col < life->width ; col++)
        {
            if (rand() > (RAND_MAX / 2))
            {
                life->buffer[col + (row * life->alignedWidth)] = LIVE;
            }
            else
            {
                life->buffer[col + (row * life->alignedWidth)] = DEAD;
            }
        }
    }
}

//-------------------------------------------------------------------------

void
iterateLife(
    LIFE_T *life)
{
    // swap buffer;

    uint16_t *lastBuffer = life->buffer;
    uint16_t *imageBuffer = life->last;

    life->last = lastBuffer;
    life->buffer = imageBuffer;

    //
    // +---+---+---+
    // | 0 | 1 | 2 |
    // +---+---+---+
    // | 3 |   | 4 |
    // +---+---+---+
    // | 5 | 6 | 7 |
    // +---+---+---+
    //

    int32_t nOff0;
    int32_t nOff1;
    int32_t nOff2;
    int32_t nOff3;
    int32_t nOff4;
    int32_t nOff5;
    int32_t nOff6;
    int32_t nOff7;

    int32_t nYoff0;
    int32_t nYoff1;
    int32_t nYoff2;
    int32_t nYoff3;
    int32_t nYoff4;
    int32_t nYoff5;
    int32_t nYoff6;
    int32_t nYoff7;

    int32_t off = 0;
    int32_t yOff = 0;

    uint32_t n = 0;
    int32_t alignedWidth = life->alignedWidth;

    int32_t row = 0;
    int32_t col = 0;

    //---------------------------------------------------------------------
    // first row

    nYoff0 = nYoff1 = nYoff2 = (life->height - 1) * alignedWidth;
    nYoff3 = nYoff4 = 0;
    nYoff5 = nYoff6 = nYoff7 = alignedWidth;

    yOff = 0;

    {
        //-----------------------------------------------------------------
        // first column

        nOff0 = nYoff0 + life->width - 1;
        nOff1 = nYoff1 + 0;
        nOff2 = nYoff2 + 1;
        nOff3 = nYoff3 + life->width - 1;
        nOff4 = nYoff4 + 1;
        nOff5 = nYoff5 + life->width - 1;
        nOff6 = nYoff6 + 0;
        nOff7 = nYoff7 + 1;

        off = yOff;

        n = lastBuffer[nOff0]
          + lastBuffer[nOff1]
          + lastBuffer[nOff2]
          + lastBuffer[nOff3]
          + lastBuffer[nOff4]
          + lastBuffer[nOff5]
          + lastBuffer[nOff6]
          + lastBuffer[nOff7];

        if (((lastBuffer[off] == LIVE) && (n == LIVE2)) || (n == LIVE3))
        {
            imageBuffer[off] = LIVE;
        }
        else
        {
            imageBuffer[off] = DEAD;
        }

        //-----------------------------------------------------------------
        // other columns

        nOff0 = nYoff0 + 0;
        nOff1 = nYoff1 + 1;
        nOff2 = nYoff2 + 2;
        nOff3 = nYoff3 + 0;
        nOff4 = nYoff4 + 2;
        nOff5 = nYoff5 + 0;
        nOff6 = nYoff6 + 1;
        nOff7 = nYoff7 + 2;

        off = yOff + 1;

        for (col = 1 ; col < (life->width - 1) ; col++)
        {
            n = lastBuffer[nOff0]
              + lastBuffer[nOff1]
              + lastBuffer[nOff2]
              + lastBuffer[nOff3]
              + lastBuffer[nOff4]
              + lastBuffer[nOff5]
              + lastBuffer[nOff6]
              + lastBuffer[nOff7];

            if (((lastBuffer[off] == LIVE) && (n == LIVE2)) || (n == LIVE3))
            {
                imageBuffer[off] = LIVE;
            }
            else
            {
                imageBuffer[off] = DEAD;
            }

            nOff0 += 1;
            nOff1 += 1;
            nOff2 += 1;
            nOff3 += 1;
            nOff4 += 1;
            nOff5 += 1;
            nOff6 += 1;
            nOff7 += 1;

            off += 1;
        }

        //-----------------------------------------------------------------
        // last column

        nOff0 = nYoff0 + life->width - 2;
        nOff1 = nYoff1 + life->width - 1;
        nOff2 = nYoff2 + 0;
        nOff3 = nYoff3 + life->width - 2;
        nOff4 = nYoff4 + 0;
        nOff5 = nYoff5 + life->width - 2;
        nOff6 = nYoff6 + life->width - 1;
        nOff7 = nYoff7 + 0;

        off = yOff + life->width - 1;

        n = lastBuffer[nOff0]
          + lastBuffer[nOff1]
          + lastBuffer[nOff2]
          + lastBuffer[nOff3]
          + lastBuffer[nOff4]
          + lastBuffer[nOff5]
          + lastBuffer[nOff6]
          + lastBuffer[nOff7];

        if (((lastBuffer[off] == LIVE) && (n == LIVE2)) || (n == LIVE3))
        {
            imageBuffer[off] = LIVE;
        }
        else
        {
            imageBuffer[off] = DEAD;
        }
    }

    //---------------------------------------------------------------------
    // other rows

    nYoff0 = nYoff1 = nYoff2 = 0;
    nYoff3 = nYoff4 = alignedWidth;
    nYoff5 = nYoff6 = nYoff7 = 2 * alignedWidth;

    yOff = alignedWidth;

    for (row = 1 ; row < (life->height - 1) ; row++)
    {
        //-----------------------------------------------------------------
        // first column

        nOff0 = nYoff0 + life->width - 1;
        nOff1 = nYoff1 + 0;
        nOff2 = nYoff2 + 1;
        nOff3 = nYoff3 + life->width - 1;
        nOff4 = nYoff4 + 1;
        nOff5 = nYoff5 + life->width - 1;
        nOff6 = nYoff6 + 0;
        nOff7 = nYoff7 + 1;

        off = yOff + 0;

        n = lastBuffer[nOff0]
          + lastBuffer[nOff1]
          + lastBuffer[nOff2]
          + lastBuffer[nOff3]
          + lastBuffer[nOff4]
          + lastBuffer[nOff5]
          + lastBuffer[nOff6]
          + lastBuffer[nOff7];

        if (((lastBuffer[off] == LIVE) && (n == LIVE2)) || (n == LIVE3))
        {
            imageBuffer[off] = LIVE;
        }
        else
        {
            imageBuffer[off] = DEAD;
        }

        //-----------------------------------------------------------------
        // other columns

        nOff0 = nYoff0 + 0;
        nOff1 = nYoff1 + 1;
        nOff2 = nYoff2 + 2;
        nOff3 = nYoff3 + 0;
        nOff4 = nYoff4 + 2;
        nOff5 = nYoff5 + 0;
        nOff6 = nYoff6 + 1;
        nOff7 = nYoff7 + 2;

        off = yOff + 1;

        for (col = 1 ; col < (life->width - 1) ; col++)
        {
            n = lastBuffer[nOff0]
              + lastBuffer[nOff1]
              + lastBuffer[nOff2]
              + lastBuffer[nOff3]
              + lastBuffer[nOff4]
              + lastBuffer[nOff5]
              + lastBuffer[nOff6]
              + lastBuffer[nOff7];

            if (((lastBuffer[off] == LIVE) && (n == LIVE2)) || (n == LIVE3))
            {
                imageBuffer[off] = LIVE;
            }
            else
            {
                imageBuffer[off] = DEAD;
            }

            nOff0 += 1;
            nOff1 += 1;
            nOff2 += 1;
            nOff3 += 1;
            nOff4 += 1;
            nOff5 += 1;
            nOff6 += 1;
            nOff7 += 1;

            off += 1;
        }

        //-----------------------------------------------------------------
        // last column

        nOff0 = nYoff0 + life->width - 2;
        nOff1 = nYoff1 + life->width - 1;
        nOff2 = nYoff2 + 0;
        nOff3 = nYoff3 + life->width - 2;
        nOff4 = nYoff4 + 0;
        nOff5 = nYoff5 + life->width - 2;
        nOff6 = nYoff6 + life->width - 1;
        nOff7 = nYoff7 + 0;

        off = yOff + life->width - 1;

        n = lastBuffer[nOff0]
          + lastBuffer[nOff1]
          + lastBuffer[nOff2]
          + lastBuffer[nOff3]
          + lastBuffer[nOff4]
          + lastBuffer[nOff5]
          + lastBuffer[nOff6]
          + lastBuffer[nOff7];

        if (((lastBuffer[off] == LIVE) && (n == LIVE2)) || (n == LIVE3))
        {
            imageBuffer[off] = LIVE;
        }
        else
        {
            imageBuffer[off] = DEAD;
        }

        //-----------------------------------------------------------------

        nYoff0 += alignedWidth;
        nYoff1 += alignedWidth;
        nYoff2 += alignedWidth;
        nYoff3 += alignedWidth;
        nYoff4 += alignedWidth;
        nYoff5 += alignedWidth;
        nYoff6 += alignedWidth;
        nYoff7 += alignedWidth;

        yOff += alignedWidth;
    }

    //---------------------------------------------------------------------
    // last row

    nYoff0 = nYoff1 = nYoff2 = (life->height - 2) * alignedWidth;
    nYoff3 = nYoff4 = (life->height - 1) * alignedWidth;
    nYoff5 = nYoff6 = nYoff7 = 0;

    yOff = (life->height - 1) * alignedWidth;

    {
        //-----------------------------------------------------------------
        // first column

        nOff0 = nYoff0 + life->width - 1;
        nOff1 = nYoff1 + 0;
        nOff2 = nYoff2 + 1;
        nOff3 = nYoff3 + life->width - 1;
        nOff4 = nYoff4 + 1;
        nOff5 = nYoff5 + life->width - 1;
        nOff6 = nYoff6 + 0;
        nOff7 = nYoff7 + 1;

        off = yOff + 0;

        n = lastBuffer[nOff0]
          + lastBuffer[nOff1]
          + lastBuffer[nOff2]
          + lastBuffer[nOff3]
          + lastBuffer[nOff4]
          + lastBuffer[nOff5]
          + lastBuffer[nOff6]
          + lastBuffer[nOff7];

        if (((lastBuffer[off] == LIVE) && (n == LIVE2)) || (n == LIVE3))
        {
            imageBuffer[off] = LIVE;
        }
        else
        {
            imageBuffer[off] = DEAD;
        }

        //-----------------------------------------------------------------
        // other columns

        nOff0 = nYoff0 + 0;
        nOff1 = nYoff1 + 1;
        nOff2 = nYoff2 + 2;
        nOff3 = nYoff3 + 0;
        nOff4 = nYoff4 + 2;
        nOff5 = nYoff5 + 0;
        nOff6 = nYoff6 + 1;
        nOff7 = nYoff7 + 2;

        off += 1;

        for (col = 1 ; col < (life->width - 1) ; col++)
        {
            n = lastBuffer[nOff0]
              + lastBuffer[nOff1]
              + lastBuffer[nOff2]
              + lastBuffer[nOff3]
              + lastBuffer[nOff4]
              + lastBuffer[nOff5]
              + lastBuffer[nOff6]
              + lastBuffer[nOff7];

            if (((lastBuffer[off] == LIVE) && (n == LIVE2)) || (n == LIVE3))
            {
                imageBuffer[off] = LIVE;
            }
            else
            {
                imageBuffer[off] = DEAD;
            }

            nOff0 += 1;
            nOff1 += 1;
            nOff2 += 1;
            nOff3 += 1;
            nOff4 += 1;
            nOff5 += 1;
            nOff6 += 1;
            nOff7 += 1;

            off += 1;
        }

        //-----------------------------------------------------------------
        // last column

        nOff0 = nYoff0 + life->width - 2;
        nOff1 = nYoff1 + life->width - 1;
        nOff2 = nYoff2 + 0;
        nOff3 = nYoff3 + life->width - 2;
        nOff4 = nYoff4 + 0;
        nOff5 = nYoff5 + life->width - 2;
        nOff6 = nYoff6 + life->width - 1;
        nOff7 = nYoff7 + 0;

        off = yOff + life->width - 1;

        n = lastBuffer[nOff0]
          + lastBuffer[nOff1]
          + lastBuffer[nOff2]
          + lastBuffer[nOff3]
          + lastBuffer[nOff4]
          + lastBuffer[nOff5]
          + lastBuffer[nOff6]
          + lastBuffer[nOff7];

        if (((lastBuffer[off] == LIVE) && (n == LIVE2)) || (n == LIVE3))
        {
            imageBuffer[off] = LIVE;
        }
        else
        {
            imageBuffer[off] = DEAD;
        }
    }
}

//-------------------------------------------------------------------------

void
destroyLife(
    LIFE_T* life)
{
    if (life->buffer)
    {
        free(life->buffer);
    }

    if (life->last)
    {
        free(life->last);
    }

    life->width = 0;
    life->alignedWidth = 0;
    life->height = 0;
    life->pitch = 0;
}
