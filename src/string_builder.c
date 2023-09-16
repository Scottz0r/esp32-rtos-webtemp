#include "string_builder.h"

static void make_null_terminated(strbld_t* sb);

/**
 * Initialize a string builder. The builder will use the given buffer, so the buffer must have a larger lifetime than
 * the string builder.
 */
int strbld_init(strbld_t* sb, char* buffer, size_t size)
{
    if (!sb || !buffer || size == STRBLD_NPOS || size == 0)
    {
        return STRBLD_FAIL;
    }

    sb->buffer = buffer;
    sb->capacity = size;
    sb->size = 0;

    // Start out null terminated to guarantee stable state.
    sb->buffer[0] = 0;

    return STRBLD_OK;
}

/**
 * Append the given null terminated string to the string builder.
 *
 * STRBLD_OK will be returned if the string was completely written.
 * STRBLD_TRUNCATED will be returned if the string was truncated.
 */
int strbld_append(strbld_t* sb, const char* value)
{
    if (!sb || !value)
    {
        return STRBLD_FAIL;
    }

    int retcode = STRBLD_OK;
    const char* pv = value;

    // Start writing at the end of the string builder buffer.
    char* pbuff = sb->buffer + sb->size;

    // Use one less to account for requiring a null terminator in the string builder.
    char* buff_end = sb->buffer + sb->capacity - 1;

    while (*pv != 0 && pbuff < buff_end)
    {
        *(pbuff++) = *(pv++);
    }

    // If the last byte read was not a null terminated, then the input was truncated.
    if (*pv != 0)
    {
        retcode = STRBLD_TRUNCATED;
    }

    // Update the size on the string builder. More efficient to do in one go rather than incrementing size for each
    // character written.
    sb->size = pbuff - sb->buffer;

    // Always null terminate the buffer, but do not increase the size. If size is increased then a null value will be
    // in the middle of the string.
    make_null_terminated(sb);

    return retcode;
}

/**
 * Append a single character to the string builder. More efficient than strbld_append for single characters.
*/
int strbld_append_char(strbld_t* sb, char value)
{
    if (!sb || !value)
    {
        return STRBLD_FAIL;
    }

    int rc = STRBLD_OK;

    // If there is enough space in the buffer, then it's a simple assignment. Otherwise, the string builder if full
    // and the input was truncated.
    if (sb->size < sb->capacity - 1)
    {
        sb->buffer[sb->size] = value;
        ++(sb->size);
        make_null_terminated(sb);
    }
    else
    {
        rc = STRBLD_TRUNCATED;
    }

    return rc;
}

/**
 * Helper method to append a string followed by a new line.
 */
int strbld_append_line(strbld_t* sb, const char* value)
{
    int rc = strbld_append(sb, value);

    if (rc == STRBLD_OK)
    {
        rc = strbld_append(sb, STRBLD_NEWLINE);
    }

    return rc;
}

/**
 * Helper method to append a string enclosed in an HTML tag. Ex: "<html_tag>value</html_tag>"
*/
int strbld_append_html(strbld_t* sb, const char* value, const char* html_tag)
{
    int rc = STRBLD_OK;

    rc = strbld_append_char(sb, '<');
    if (rc != STRBLD_OK)
    {
        return rc;
    }

    rc = strbld_append(sb, html_tag);
    if (rc != STRBLD_OK)
    {
        return rc;
    }

    rc = strbld_append_char(sb, '>');
    if (rc != STRBLD_OK)
    {
        return rc;
    }

    rc = strbld_append(sb, value);
    if (rc != STRBLD_OK)
    {
        return rc;
    }

    rc = strbld_append_char(sb, '<');
    if (rc != STRBLD_OK)
    {
        return rc;
    }

    rc = strbld_append_char(sb, '/');
    if (rc != STRBLD_OK)
    {
        return rc;
    }

    rc = strbld_append(sb, html_tag);
    if (rc != STRBLD_OK)
    {
        return rc;
    }

    rc = strbld_append_char(sb, '>');
    if (rc != STRBLD_OK)
    {
        return rc;
    }

    return rc;
}

/**
 * Get the built string from the string builder. This method should be used instead of accessing the string builder's
 * fields.
 */
const char* strbld_get(strbld_t* sb, size_t* strlen)
{
    if (!sb)
    {
        return NULL;
    }

    // Ensure null termination behavior. This is a cheap operation to ensure the pointer returned is a null terminated
    // string.
    make_null_terminated(sb);

    // Optional string length output.
    if (strlen != NULL)
    {
        *strlen = sb->size;
    }

    return sb->buffer;
}

/**
 * Makes the string builder null terminated. Does not update the size! This will allow subsequent appends to overwrite
 * null terminators (unless the buffer is full).
*/
static void make_null_terminated(strbld_t* sb)
{
    if (sb)
    {
        if (sb->size < sb->capacity)
        {
            sb->buffer[sb->size] = 0;
        }
        else
        {
            size_t last_byte = sb->capacity - 1;
            sb->buffer[last_byte] = 0;
        }
    }
}
