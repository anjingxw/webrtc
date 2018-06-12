#ifndef TC_BASE64_H
#define TC_BASE64_H

#ifdef __cplusplus
extern "C"
{
#endif

#define base64_encode(in_str,length,out_str,ret_length) OI_base64_encode(in_str,length,out_str,ret_length)
#define base64_decode(in_str,length,out_str,ret_length) OI_base64_decode(in_str,length,out_str,ret_length)

    // �ı�ԭ��  ת��  BASE64 ����
    int OI_base64_encode(const unsigned char *in_str, int length, char *out_str, int *ret_length);

    // BASE64 ����  ת�� �ı�ԭ��
    int OI_base64_decode(const char *in_str, int length, char *out_str, int *ret_length);

#ifdef __cplusplus
}
#endif

#endif
