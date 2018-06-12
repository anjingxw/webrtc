#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "base64.h"

static const char base64_table[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
    'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
    'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/', '\0'
};

static const char base64_pad = '=';

static const short base64_reverse_table[256] = {
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
    15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
    -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
    41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1
};

int
OI_base64_encode(const unsigned char *in_str, int length, char *out_str, int *ret_length)
{
    const unsigned char *current = in_str;
    unsigned char *p;

    if((length + 2) < 0 || ((length + 2) / 3) >= (1 << (sizeof(int) * 8 - 2)))
        return -1;
    if(*ret_length < (((length + 2) / 3) * 4))
        return -1;

    p = (unsigned char *) out_str;

    while(length > 2)
    {   /* keep going until we have less than 24 bits */
        *p++ = base64_table[current[0] >> 2];
        *p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
        *p++ = base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
        *p++ = base64_table[current[2] & 0x3f];

        current += 3;
        length -= 3;    /* we just handle 3 octets of data */
    }

    /* now deal with the tail end of things */
    if(length != 0)
    {
        *p++ = base64_table[current[0] >> 2];
        if(length > 1)
        {
            *p++ = base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
            *p++ = base64_table[(current[1] & 0x0f) << 2];
            *p++ = base64_pad;
        }
        else
        {
            *p++ = base64_table[(current[0] & 0x03) << 4];
            *p++ = base64_pad;
            *p++ = base64_pad;
        }
    }
    if(ret_length != NULL)
    {
        *ret_length = (int) (p - (unsigned char *) out_str);
    }
    *p = '\0';
    return 0;
}

int
OI_base64_decode(const char *in_str, int length, char *out_str, int *ret_length)
{
    const unsigned char *current = (const unsigned char *)in_str;
    int ch, i = 0, j = 0, k;

    /* this sucks for threaded environments */
    unsigned char *result = (unsigned char *) out_str;

    if(*ret_length < length + 1)
        return -1;

    /* run through the whole string, converting as we go */
    while((ch = *current++) != '\0' && length-- > 0)
    {
        if(ch == base64_pad)
            break;

        /* When Base64 gets POSTed, all pluses are interpreted as spaces.
           This line changes them back.  It's not exactly the Base64 spec,
           but it is completely compatible with it (the spec says that
           spaces are invalid).  This will also save many people considerable
           headache.  - Turadg Aleahmad <turadg@wise.berkeley.edu>
         */

        if(ch == ' ')
            ch = '+';

        ch = base64_reverse_table[ch];
        if(ch < 0)
            continue;

        switch (i % 4)
        {
            case 0:
                result[j] = ch << 2;
                break;
            case 1:
                result[j++] |= ch >> 4;
                result[j] = (ch & 0x0f) << 4;
                break;
            case 2:
                result[j++] |= ch >> 2;
                result[j] = (ch & 0x03) << 6;
                break;
            case 3:
                result[j++] |= ch;
                break;
        }
        i++;
    }

    k = j;
    /* mop things up if we ended on a boundary */
    if(ch == base64_pad)
    {
        switch (i % 4)
        {
            case 0:
            case 1:
                return -1;
            case 2:
                k++;
            case 3:
                result[k++] = 0;
        }
    }
    if(ret_length)
    {
        *ret_length = j;
    }
    result[j] = '\0';
    return 0;
}

#ifdef BASE64_DEBUG
int
main(int argc, char *argv[])
{
    char buffer[50];
    int length = 50, ret;

    if(argc < 2)
        return 0;
    ret = OI_base64_encode(argv[1], strlen(argv[1]), buffer, &length);
    if(ret != 0)
        printf("can't encode!\n");
    else
    {
        char outbuffer[50];

        printf("before:[%d]%s\nencode:[%d]%s\n", strlen(argv[1]), argv[1], strlen(buffer), buffer);
        length = 50;
        ret = OI_base64_decode(buffer, strlen(buffer), outbuffer, &length);
        printf("before:[%d]%s\ndecode:[%d]%s\n", strlen(buffer), buffer, strlen(outbuffer), outbuffer);
    }

    int a =1234 ;
	length = 50 ;
    ret = OI_base64_encode((char *)&a, 4, buffer, &length);
	printf("a=%d encode:[%d]%s\n",a,strlen(buffer), buffer);
    return 0;
}

#endif
