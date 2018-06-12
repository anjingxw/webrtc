#ifndef TC_BASE64__URL_H
#define TC_BASE64__URL_H

#ifdef __cplusplus
extern "C" {
#endif

//int base64_encode(const unsigned char *in_str, int length, char *out_str,int *ret_length);
//int base64_decode(const unsigned char *in_str, int length, char *out_str, int *ret_length);

//+ => *; / => - ; = => _
int base64_encode_url(const unsigned char *in_str, int length, char *out_str,int *ret_length);
int base64_decode_url(const unsigned char *in_str, int length, char *out_str, int *ret_length);

#ifdef __cplusplus
}
#endif

#endif

