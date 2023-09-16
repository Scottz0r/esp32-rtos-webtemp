#ifndef _WA_CIRCULAR_ARRAY_H_INCLUDE_GUARD
#define _WA_CIRCULAR_ARRAY_H_INCLUDE_GUARD

/**
 * Get the next index of a circular array.
*/
inline int CA_NEXT_IDX(int curr, int max)
{
    return (curr + 1) % max;
}

/**
 * Get the previous index of a circular array.
*/
inline int CA_PREV_IDX(int curr, int max)
{
    --curr;
    if (curr < 0)
    {
        return max - 1;
    }

    return curr;
}

#endif // _WA_CIRCULAR_ARRAY_H_INCLUDE_GUARD
