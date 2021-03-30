#ifndef __displaylines_hpp
#define __displaylines_hpp

#include <Arduino.h>
#include <inttypes.h>

class DisplayLines { //Each line is a byte height
    uint16_t width;
    uint8_t height;
    uint8_t *ringBuffer;
    uint32_t startingLine;
    uint32_t endingLine;
    uint16_t cursorX;
    uint32_t cursorLine;
    inline uint8_t *internalLineAtIndex(uint32_t index) {
        return ringBuffer + width * (index % height);
    }
    inline void internalClearLine(uint32_t index) {
        memset(internalLineAtIndex(index), 0, width);
    }
    void printCharacter(char character); //newlines call newLine
    inline uint8_t *byteAtPosLine(uint16_t x, uint32_t line) {
      return internalLineAtIndex(line) + x;
    }

public:
    DisplayLines(uint16_t w, uint8_t h);
    void clearScreen();
    inline void clearLine(uint32_t index) {
        if ((index < startingLine) || (index > endingLine)) {
            return;
        }
        internalClearLine(index);
    }
    inline uint8_t * lineAtIndex(uint32_t index) {
        if ((index < startingLine) || (index > endingLine)) {
            return NULL;
        }
        return internalLineAtIndex(index);
    }
    inline uint16_t getWidth() { return width; }
    inline uint32_t getStarting() { return startingLine; }
    inline uint32_t getEnding() { return endingLine; }
    inline uint16_t getCursorX() { return cursorX; }
    inline uint32_t getCursorLine() { return cursorLine; }
    inline void moveCursor(uint16_t x, uint32_t y) {
      if ((y < startingLine) || (y > endingLine)) {
        return;
      }
      if (x >= width) {
        x = width - 1;
      }
      cursorX = x;
      cursorLine = y;
    }
    void printString(char *string);
    void printImage(uint8_t *image, uint32_t length) {
      if (cursorX + length >= width) {
        length = width - cursorX - 1;
      }
      memcpy(byteAtPosLine(cursorLine, cursorX), image, length);
    }
    void newLine();
};

#endif
