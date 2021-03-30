#ifndef font_h
#define font_h

#include <inttypes.h>

// Class C is an uint with height number of bits
// Class T is a uint to be the X coordinate
// Int S is the number of glyphs
template <class C, class T, int S>
class LayerFont {
public:
  // Actual graphics:
  // bitmap with LSbit on the top, MSbit on the bottom
  // first column is leftmost, last column is rightmost
  // A 0 declares the end of the glyph, and is not drawn
  // A 1 declares an empty column
  C const *columns;
  // Position caching of where breaks are.
  // after subtracting indexOffset, 
  // Item 0 goes from column 0 to position[0]-1
  // Item x goes from column position[x-1] to position[x]-1
  T positions[S];
  int indexOffset; //Value of first index

  LayerFont(int offset = 0, C const *cols = NULL) : columns(cols), indexOffset(offset) {
    calcPositions();
  }

  void calcPositions() {
    if (columns == NULL) {
      return;
    }
    uint16_t pos;
    T i = 0;
    for (pos = 0; pos < S; pos++) {
      while (columns[i]) {
        i++;
      }
      positions[pos] = i;
      i++;
    }
  }

  T start(uint16_t index) {
    if (index <= indexOffset) {
      return 0; // If lower, is an error. If equal, is the start
    }
    index = index - indexOffset;
    if (index >= S) {
      return 0;
    }
    return positions[index - 1];
  }

  T end(uint16_t index) {
    if (index < indexOffset) {
      return 0; // If lower, is an error
    }
    index = index - indexOffset;
    if (index >= S) {
      return 0;
    }
    return positions[index] - 1;
  }

  T charWidth(uint16_t index) {
    return end(index) - start(index);
  }

  uint8_t valueAt(T x, uint8_t y) {
    C col = columns[x];
    if (col == 1) { //The exception is the nonterminating empty
      return 0;
    }
    uint8_t result = (col >> y) & 0x01;
    return result;
  }

  C columnAt(T x) {
    C col = columns[x];
    if (col == 1) { //The exception is the nonterminating empty
      return 0;
    }
    return col;
  }
};

#endif