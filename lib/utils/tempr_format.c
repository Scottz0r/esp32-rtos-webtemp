#include "tempr_format.h"

static void tempr_format_2_dec(int32_t num, char* buffer);

static void reverse(char* buffer, int slen);

/**
 * Assumes buffer is TEMPER_FORMAT_SIZE
*/
void tempr_format(int32_t value, char* buffer)
{
    if (!buffer)
    {
        return;
    }

    // Do not try to convert crazy temperatures.
    if (value < -99999 || value > 99999)
    {
        // Default initialize.
        buffer[0] = '-';
        buffer[1] = '.';
        buffer[2] = '-';
        buffer[3] = '-';
        buffer[4] = 0;
    }
    else
    {
        tempr_format_2_dec(value, buffer);
    }
}

static void tempr_format_2_dec(int32_t num, char* buffer)
{
    char* pb = buffer;

    int32_t sign = num;
    if (num < 0)
    {
        num = -num;
    }

    // Decimal places. Zero fills these so there is always TPS_NUM_DEC_PLACES decimal places.
    for(int i = 0; i < 2; ++i)
    {
        *pb = '0' + (char)(num % 10);
        ++pb;
        num /= 10;
    }

    *pb = '.';
    ++pb;

    // Do/while so there is always a zero in the whole places.
    do
    {
        *pb = '0' + (char)(num % 10);
        ++pb;
        num /= 10;
    }
    while(num > 0);

    if (sign < 0)
    {
        *pb = '-';
        ++pb;
    }

    // Null terminate!
    *pb = 0;

    // Reverse the buffer to display in the correct order.
    int slen = pb - buffer;
    reverse(buffer, slen);
}

static void reverse(char* buffer, int slen)
{
    char tmp;

    if (slen < 0)
    {
        return;
    }

    for(int i = 0, j = slen - 1; i < j; ++i, --j)
    {
        tmp = buffer[i];
        buffer[i] = buffer[j];
        buffer[j] = tmp;
    }
}
