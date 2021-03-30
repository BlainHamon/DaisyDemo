#include "displayLines.hpp"
#include "daisyfont.h"

DisplayLines::DisplayLines(uint16_t w, uint8_t h) : width(w), height(h),
    startingLine(0), endingLine(0)
{
    ringBuffer = (uint8_t*) malloc(w * h * sizeof(uint8_t));
    clearScreen();
}

void DisplayLines::printCharacter(char character) {
  if (character == '\n') {
    newLine();
    return;
  }
  uint16_t start = daisyFont.start(character);
  uint16_t end = daisyFont.end(character);
  uint8_t *row = internalLineAtIndex(cursorLine);
  for (uint16_t i = start; i < end; i++) {
    row[cursorX] = daisyFont.columnAt(i);
    cursorX++;
  }
}

void DisplayLines::printString(char *string) {
  char *current = string;
  while (*current) {
    char thisChar = *current;
    if (daisyFont.charWidth(thisChar) + cursorX >= width) {
      newLine();
    }
    printCharacter(thisChar);
    current++;
  };
}

void DisplayLines::newLine() {
    cursorLine ++; cursorX = 0;
    if (cursorLine > endingLine) {
        endingLine = cursorLine;
    }
    if (startingLine + height <= endingLine) {
        startingLine = endingLine - height + 1;
    }
    internalClearLine(cursorLine);
}

void DisplayLines::clearScreen() {
    startingLine = 0;
    endingLine = height - 1;
    cursorX = 0;
    cursorLine = 0;
    memset(ringBuffer, 0, width * height);
}
