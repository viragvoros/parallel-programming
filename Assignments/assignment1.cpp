#include <cstring>
#include <iostream>
#include <cmath>

#include "vv-aes.h"

uint8_t subs_dict[UNIQUE_CHARACTERS];

void create_map(){
    for(int i =0; i < UNIQUE_CHARACTERS; i++){
        subs_dict[originalCharacter[i]] = substitutedCharacter[i];
    }
}

/**
 * This function takes the characters stored in the 4x4 message array and substitutes each character for the
 * corresponding replacement as specified in the originalCharacter and substitutedCharacter array.
 * This corresponds to step 2.1 in the VV-AES explanation.
 */

void substitute_bytes() {
    // For each byte in the message
    for (int column = 0; column < MESSAGE_BLOCK_WIDTH; column++) {
        for (int row = 0; row < MESSAGE_BLOCK_WIDTH; row++) {
            // Search for the byte in the original character list
            // and replace it with corresponding the element in the substituted character list

            message[row][column] = subs_dict[message[row][column]];
        }
    }
}



/*
 * This function shifts each row by the number of places it is meant to be shifted according to the AES specification.
 * Row zero is shifted by zero places. Row one by one, etc.
 * This corresponds to step 2.2 in the VV-AES explanation.
 */
void shift_rows() {
    // Shift each row, where the row index corresponds to how many columns the data is shifted.
    for (int row = 0; row < MESSAGE_BLOCK_WIDTH; ++row) {
        for (int shifts = 0; shifts < row; ++shifts) {

            auto temp = message[row][0];
            message[row][0] = message[row][1];
            message[row][1] = message[row][2];
            message[row][2] = message[row][3];
            message[row][3] = message[row][4];
            message[row][4] = message[row][5];
            message[row][5] = message[row][6];
            message[row][6] = message[row][7];
            message[row][7] = temp;

            
        }
    }
}

/*
 * This function calculates x^n for polynomial evaluation.
 */

int power(int x, int n) { //This needs to be replaced by standard library function.
    // Calculates x^n
    if (n == 0) {
        return 1;
    }
    return x * power(x, n - 1); 
}


/*
 * This function evaluates four different polynomials, one for each row in the column.
 * Each polynomial evaluated is of the form
 * m'[row, column] = c[r][3] m[3][column]^4 + c[r][2] m[2][column]^3 + c[r][1] m[1][column]^2 + c[r][0]m[0][column]^1
 * where m' is the new message value, c[r] is an array of polynomial coefficients for the current result row (each
 * result row gets a different polynomial), and m is the current message value.
 *
 */
void multiply_with_polynomial(int column) {
    for (int row = 0; row < MESSAGE_BLOCK_WIDTH; ++row) {
        int result = 0;
        for (int degree = 0; degree < MESSAGE_BLOCK_WIDTH; degree++) {
            result += polynomialCoefficients[row][degree] * power(message[degree][column], degree + 1);
        }
        message[row][column] = result;
    }
}

/*
 * For each column, mix the values by evaluating them as parameters of multiple polynomials.
 * This corresponds to step 2.3 in the VV-AES explanation.
 */
void mix_columns() {
    for (int row = 0; row < MESSAGE_BLOCK_WIDTH; ++row) {

        for (int column = 0; column < MESSAGE_BLOCK_WIDTH; ++column) {
        int result = 0;

        result += polynomialCoefficients[row][0] * message[0][column];
        result += polynomialCoefficients[row][1] * power(message[1][column], 2);
        result += polynomialCoefficients[row][2] * power(message[2][column], 3);
        result += polynomialCoefficients[row][3] * power(message[3][column], 4);
        result += polynomialCoefficients[row][4] * power(message[4][column], 5);
        result += polynomialCoefficients[row][5] * power(message[5][column], 6);
        result += polynomialCoefficients[row][6] * power(message[6][column], 7);
        result += polynomialCoefficients[row][7] * power(message[7][column], 8);

        message[row][column] = result;
    }
    }
}

/*
 * Add the current key to the message using the XOR operation.
 */
void add_key() {
    for (int row = 0; row < MESSAGE_BLOCK_WIDTH; row++) {
        for (int column = 0; column < MESSAGE_BLOCK_WIDTH; ++column) {
            // ^ == XOR
            message[row][column] = message[row][column] ^ key[row][column];
        }
    }
}

/*
 * Your main encryption routine.
 */
int main() {
    // Receive the problem from the system.
    readInput();
    create_map();
    // For extra security (and because Varys wasn't able to find enough test messages to keep you occupied) each message
    // is put through VV-AES lots of times. If we can't stop the adverse Maesters from decrypting our highly secure
    // encryption scheme, we can at least slow them down.
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        // For each message, we use a predetermined key (e.g. the password). In our case, its just pseudo random.
        set_next_key();

        // First, we add the key to the message once:
        add_key();

        // We do 9+1 rounds for 128 bit keys
        for (int round = 0; round < NUM_ROUNDS; round++) {
            //In each round, we use a different key derived from the original (refer to the key schedule).
            set_next_key();

            // These are the four steps described in the slides.
            substitute_bytes();
            shift_rows();
            mix_columns();
            add_key();
        }
        // Final round
        substitute_bytes();
        shift_rows();
        add_key();
    }

    // Submit our solution back to the system.
    writeOutput();
    return 0;
}
