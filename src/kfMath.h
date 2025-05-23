#ifndef KF_MATH_H
#define KF_MATH_H

#include "kfBios.h"

#define ISIZE_SIZE     sizeof(isize)
#define BYTE_CELL_SIZE ISIZE_SIZE * 2



// Used to interface with the forth stack
typedef struct {
    isize low;
    isize high;
} TwoCell;

// Used to actually do the math
typedef struct {
    // Stored in little endian
    uint8_t bytes[BYTE_CELL_SIZE];
} ByteCell;



ByteCell TwoCellToByteCell(TwoCell input) {
    ByteCell output;
    for (int i = 0; i < ISIZE_SIZE; i++) {
        output.bytes[i] = input.low & 0xFF;
        input.low >>= 8;
    }
    for (int i = ISIZE_SIZE; i < BYTE_CELL_SIZE; i++) {
        output.bytes[i] = input.high & 0xFF;
        input.high >>= 8;
    }
    return output;
}

TwoCell ByteCellToTwoCell(ByteCell input) {
    TwoCell output;
    for (int i = ISIZE_SIZE - 1; i >= 0; i--) {
        output.low <<= 8;
        output.low |= input.bytes[i];
    }
    for (int i = BYTE_CELL_SIZE - 1; i >= ISIZE_SIZE; i--) {
        output.high <<= 8;
        output.high |= input.bytes[i];
    }
    return output;
}



ByteCell ByteCellsAdd(ByteCell a, ByteCell b) {
    uint8_t carry = 0;
    for (int i = 0; i < BYTE_CELL_SIZE; i++) {
        uint16_t acc = a.bytes[i];
        acc += b.bytes[i];
        acc += carry;
        a.bytes[i] = acc & 0xFF;
        carry = acc >> 8;
    }
    return a;
}

ByteCell ByteCellShift8Left(ByteCell input) {
    for (int i = BYTE_CELL_SIZE - 1; i > 0; i--) {
        input.bytes[i] = input.bytes[i - 1];
    }
    input.bytes[0] = 0;
    return input;
}

ByteCell ByteCellShift8Right(ByteCell input) {
    for (int i = 0; i < BYTE_CELL_SIZE - 1; i++) {
        input.bytes[i] = input.bytes[i + 1];
    }
    input.bytes[BYTE_CELL_SIZE - 1] = 0;
    return input;
}

ByteCell ByteCellMultiply(ByteCell a, uint8_t b) {
    uint8_t carry = 0;
    for (int i = 0; i < BYTE_CELL_SIZE; i++) {
        uint16_t acc = a.bytes[i];
        acc *= b;
        acc += carry;
        a.bytes[i] = acc & 0xFF;
        carry = acc >> 8;
    }
    return a;
}

ByteCell ByteCellsMultiply(ByteCell a, ByteCell b) {
    TwoCell z;
    z.high = 0;
    z.low = 0;
    ByteCell acc = TwoCellToByteCell(z);
    for (int i = 0; i < BYTE_CELL_SIZE; i++) {
        ByteCell tmp = ByteCellMultiply(a, b.bytes[i]);
        acc = ByteCellsAdd(acc, tmp);
        a = ByteCellShift8Left(a);
    }
    return acc;
}



/*

ByteCell ByteCellDivide(ByteCell a, uint8_t b) {
    // TODO actually write this
}
void TwoCellPrint(TwoCell input) {
    printf("%d %d\n", input.low, input.high);
}
void ByteCellPrint(ByteCell input) {
    for (int i = 0; i < BYTE_CELL_SIZE; i++) {
        if (i == ISIZE_SIZE)
            printf("  ");
        printf("%d ", input.bytes[i]);
    }
    printf("\n");
}
void TwoCellTest() {
    TwoCell input;
    input.high = 0;
    input.low = -2;
    printf("input = ");
    TwoCellPrint(input);
    printf("\noutput = input\n");
    ByteCell output = TwoCellToByteCell(input);
    ByteCellPrint(output); TwoCellPrint(ByteCellToTwoCell(output));
    printf("\noutput2 = output + output\n");
    ByteCell output2 = ByteCellsAdd(output, output);
    ByteCellPrint(output2); TwoCellPrint(ByteCellToTwoCell(output2));
    printf("\noutput3 = output2 + output2\n");
    ByteCell output3 = ByteCellsAdd(output2, output2);
    ByteCellPrint(output3); TwoCellPrint(ByteCellToTwoCell(output3));
    printf("\noutput4 = output3 << 8\n");
    ByteCell output4 = ByteCellShift8Left(output3);
    ByteCellPrint(output4); TwoCellPrint(ByteCellToTwoCell(output4));
    printf("\noutput5 = output4 >> 8\n");
    ByteCell output5 = ByteCellShift8Right(output4);
    ByteCellPrint(output5); TwoCellPrint(ByteCellToTwoCell(output5));
    printf("\noutput6 = output * 3\n");
    ByteCell output6 = ByteCellMultiply(output, 3);
    ByteCellPrint(output6); TwoCellPrint(ByteCellToTwoCell(output6));
    printf("\noutput7 = output * output2\n");
    ByteCell output7 = ByteCellsMultiply(output, output2);
    ByteCellPrint(output7); TwoCellPrint(ByteCellToTwoCell(output7));
    printf("\noutput8 = output4 / 128\n");
    ByteCell output8 = ByteCellDivide(output4, 128);
    ByteCellPrint(output8); TwoCellPrint(ByteCellToTwoCell(output8));
}
// */

#endif // KF_MATH_H
