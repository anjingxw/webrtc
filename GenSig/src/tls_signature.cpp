#if defined(WIN32) || defined(WIN64)
#pragma warning(disable: 4819)			// file codec warning, that's boring!
#endif

#include "openssl/rsa.h"
#include "openssl/evp.h"
#include "openssl/objects.h"
#include "openssl/x509.h"
#include "openssl/err.h"
#include "openssl/pem.h"
#include "openssl/ssl.h"
#include "json/reader.h"
#include "base64.h"
#include "base64_url.h"
#include "tls_signature.h"
#include <stdio.h>
#include <zlib.h>
#include <sstream>
#if defined(WIN32) || defined(WIN64)
#pragma warning(disable: 4819)
#pragma warning(disable: 4267)
#pragma warning(disable: 4244)
#define snprintf(buf, buf_cnt, fmt, ...) _snprintf_s(buf, buf_cnt,  _TRUNCATE, fmt, ##__VA_ARGS__)
#define sscanf(content, fmt, ...) sscanf_s(content, fmt, ##__VA_ARGS__)
#endif

#define JSON_STRING \
	"{ \
    \"TLS.account_type\": \"%d\",\
    \"TLS.identifier\": \"%s\",\
    \"TLS.appid_at_3rd\": \"%s\",\
    \"TLS.sdk_appid\": \"%d\",\
    \"TLS.expire_after\": \"%d\",\
    \"TLS.version\": \"%s\"\
    }"

#define DEFAULT_EXPIRE  (24*3600*180)        // 默认有效期，180 天
#define TIME_UT_FORM 1
#define BASE_TYPE "account_type,identifier,sdk_appid,time,expire_after"
/*
   get_private_key 从pPriKey指向的私钥字符串中获取EVP_PKEY结构的私钥，成功则返回指向私钥的结构体指针（使用后需要调用EVP_PKEY_free (pkey)释放内存）, 失败返回NULL
   参数1：pPriKey指向私钥的指针
   参数2：uPriKeyLen为私钥的长度
 */
EVP_PKEY*  get_private_key(char* pPriKey, uint32_t uPriKeyLen)
{
	BIO *pBio = NULL;
	EVP_PKEY *pstPriKey = NULL;
	if ( NULL == (pBio = BIO_new_mem_buf(pPriKey, uPriKeyLen)) )
	{
		printf("BIO_new_mem_buf failed\n");
		return NULL;
	}
	pstPriKey =  PEM_read_bio_PrivateKey(pBio, NULL, NULL, NULL);//从BIO结构中获取EVP_PKEY结构的key
	BIO_free_all(pBio);
	if ( NULL == pstPriKey )
	{
		printf("PEM_read_bio_PrivateKey failed\n");
		return NULL;
	}
	return pstPriKey;
}
/*
   get_public_key 从pPubKey指向的公钥字符串中获取EVP_PKEY结构的公钥，成功返回存放公钥的结构体指针（使用后需要调用EVP_PKEY_free (pkey)释放内存）, 失败返回NULL
   参数1：pPubKey指向公钥的指针
   参数2：uPubKeyLen为公钥的长度
 */
EVP_PKEY*  get_public_key(char* pPubKey, uint32_t uPubKeyLen)
{
	BIO *pBio = NULL;
	EVP_PKEY *pstPubKey = NULL;
	if ( NULL == (pBio = BIO_new_mem_buf(pPubKey, uPubKeyLen)) )
	{
		return NULL;
	}
	pstPubKey =  PEM_read_bio_PUBKEY(pBio, NULL, NULL, NULL);//从BIO结构中获取EVP_PKEY结构的key
	BIO_free_all(pBio);
	if ( NULL == pstPubKey )
	{
		return NULL;
	}
	return pstPubKey;
}

int get_licence(char* pLicen, uint32_t* pLicenLen, const char* pData, uint32_t uDataLen, char* pPriKey, uint32_t uPriKeyLen)
{
	int iRet = 0;
	EVP_MD_CTX  stMdCtx;//描述散列算法上下文环境的结构
	EVP_PKEY*   pstPriKey = NULL;//私钥结构体指针
	if ( NULL == (pstPriKey = get_private_key(pPriKey, uPriKeyLen)) )//私钥获取失败
	{
		return -1;
	}
	EVP_SignInit(&stMdCtx, EVP_sha256());//使用sha256
	EVP_SignUpdate (&stMdCtx, pData, uDataLen);
	iRet = EVP_SignFinal (&stMdCtx, (unsigned char*)pLicen, pLicenLen, pstPriKey);//获取摘要
	EVP_MD_CTX_cleanup(&stMdCtx);
	EVP_PKEY_free(pstPriKey);//释放内存
	pstPriKey = NULL;
	if ( 1 != iRet )
	{
		return -2;
	}
	return 0;
}

int check_licence(const char* pLicen, uint32_t uLicenLen, const char* pData, uint32_t uDataLen, char* pPubKey, uint32_t uPubKeyLen)
{
	int iRet = 0;
	EVP_MD_CTX  stMdCtx;//描述散列算法上下文环境的结构
	EVP_PKEY*   pstPubKey = NULL;//公钥结构体指针
	if ( NULL == (pstPubKey = get_public_key(pPubKey, uPubKeyLen)) )//公钥获取失败
	{
		return -1;
	}
	EVP_VerifyInit(&stMdCtx, EVP_sha256());//使用sha256
	EVP_VerifyUpdate (&stMdCtx, pData, uDataLen);
	iRet = EVP_VerifyFinal (&stMdCtx, (unsigned char*)pLicen, uLicenLen, pstPubKey);
	EVP_MD_CTX_cleanup(&stMdCtx);
	EVP_PKEY_free(pstPubKey);//释放内存
	pstPubKey = NULL;
	if ( 1 != iRet )
	{
		return -2;
	}
	return 0;
}


int json_to_serial(const string& strJson,string& strSerail,string strSigTime)
{
	Json::Reader reader;
	Json::Value response;
	string appid_at_3rd;
	string identifier;
	string account_type;
	string sdk_appid;
	string strTime;
	string expire_after;
	if (reader.parse(strJson,response)) 
	{

		if (response.size() == 0) return -2;
		if (response.type() != Json::objectValue) return -3;


        

		bool isMember = response.isMember("TLS.appid_at_3rd");
		if (isMember) 
		{
            if (!response["TLS.appid_at_3rd"].isString()) return -4;
			appid_at_3rd = response["TLS.appid_at_3rd"].asString();
			strSerail += "TLS.appid_at_3rd:";
			strSerail += appid_at_3rd + "\n";
		}
        if (!response["TLS.account_type"].isString()) return -5;
		account_type = response["TLS.account_type"].asString();
		strSerail += "TLS.account_type:"; 
		strSerail += account_type + "\n";

        if (!response["TLS.identifier"].isString()) return -6;
		identifier = response["TLS.identifier"].asString();
		strSerail += "TLS.identifier:";
		strSerail += identifier + "\n";

        if (!response["TLS.sdk_appid"].isString()) return -7;
		sdk_appid = response["TLS.sdk_appid"].asString();
		strSerail += "TLS.sdk_appid:";
		strSerail += sdk_appid + "\n";

		if (strSigTime.empty()) {
            if (!response["TLS.time"].isString()) return -8;
			strTime = response["TLS.time"].asString();
			strSerail += "TLS.time:";
			strSerail += strTime + "\n";
		} 
		else 
		{
			strSerail += "TLS.time:";
			strSerail += strSigTime + "\n";
		}

        if (!response["TLS.expire_after"].isString()) return -9;
		expire_after = response["TLS.expire_after"].asString();
		strSerail += "TLS.expire_after:";
		strSerail += expire_after + "\n";

	} 
	else 
	{
		return -1;
	}

	return 0;
}




int tls_check_signature(const string& strJsonWithSig, char* pPubKey,uint32_t uPubKeyLen,string& strErrMsg,uint32_t dwFlag)
{
	Json::Reader reader;
	Json::Value response;

	char buf[1024]; int iLen = sizeof(buf);
	if (reader.parse(strJsonWithSig,response)) 
	{
		if(response.size() == 0){
			snprintf(buf,sizeof(buf)," json parse filed");
			strErrMsg = buf;
			return CHECK_ERR4;
		}

		if (response.type() != Json::objectValue) {
			snprintf(buf,sizeof(buf)," response.type:%d is not Json::objectValue\n",response.type());
			strErrMsg = buf;
			return CHECK_ERR5;
		}
        if (!response["TLS.sig"].isString()) return CHECK_ERR7;
		string strSig = response["TLS.sig"].asString();

		int iRet = base64_decode(strSig.c_str(),strSig.length(),buf,&iLen);
		if (iRet != 0) 
		{
			snprintf(buf,sizeof(buf)," base64_decode failed iRet:%d",iRet);
			strErrMsg = buf;
			return CHECK_ERR6;
		}
		strSig.assign(buf,iLen);

		string strSerial;
		iRet = json_to_serial(strJsonWithSig,strSerial,"");
		if (iRet != 0) 
		{
			snprintf(buf,sizeof(buf)," json_to_serial failed iRet:%d",iRet);
			strErrMsg = buf;
			return CHECK_ERR7;
		}
		iRet = check_licence(strSig.c_str(), strSig.length(), strSerial.c_str(), strSerial.length(), pPubKey,  uPubKeyLen);
		if (iRet != 0) 
		{
			snprintf(buf,sizeof(buf)," decrypt sig failed failed iRet:%d",iRet);
			strErrMsg = buf;
			return CHECK_ERR8;
		}
        if (!response["TLS.time"].isString()) return CHECK_ERR7;
		string strTime = response["TLS.time"].asString();
		struct tm sttm; memset(&sttm,0,sizeof(struct tm));
		uint32_t dwSigTime;
		if (dwFlag != TIME_UT_FORM) {
			sscanf(strTime.c_str(),"%04d-%02d-%02dT%02d:%02d:%02d-08:00"
					,&sttm.tm_year,&sttm.tm_mon,&sttm.tm_mday,&sttm.tm_hour,&sttm.tm_min,&sttm.tm_sec);
			sttm.tm_year = sttm.tm_year - 1900;
			sttm.tm_mon = sttm.tm_mon - 1;
			dwSigTime = mktime(&sttm);
		} else {
			dwSigTime = strtoul(strTime.c_str(),NULL,10);
		}
        if (!response["TLS.expire_after"].isString()) return CHECK_ERR7;
		string strExpiry = response["TLS.expire_after"].asString();
		uint32_t dwExpiry = strtoul(strExpiry.c_str(),NULL,10);

		if (dwSigTime + dwExpiry < (uint32_t)time(NULL)) 
		{
			snprintf(buf,sizeof(buf)," expire.sig init time:%u,expire time:%u",dwSigTime,dwExpiry);
			strErrMsg = buf;
			return CHECK_ERR9;
		}
	} 
	else 
	{
		snprintf(buf,sizeof(buf)," reader parse failed");
		strErrMsg = buf;
		return CHECK_ERR10;
	}
	return 0;
}


int tls_gen_signature(const string& strJson,string& strSig,char* pPriKey,uint32_t uPriKeyLen,string& strErrMsg,uint32_t dwFlag)
{
	char pLicen[1024]; uint32_t LicenLen = sizeof(pLicen);
	string strSerial;
	time_t Time = time(NULL);
#if defined(WIN32) || defined(WIN64)
	struct tm stTm;
    localtime_s(&stTm,&Time);
	struct tm* pstTm = &stTm;
#else
	struct tm* pstTm = localtime(&Time);
#endif
	char TimeBuf[256] = {0};
	if (dwFlag == TIME_UT_FORM) {
		snprintf(TimeBuf,sizeof(TimeBuf), "%u",(uint32_t)Time);
	} else {
		snprintf(TimeBuf,sizeof(TimeBuf),"%04d-%02d-%02dT%02d:%02d:%02d-08:00"
				,pstTm->tm_year + 1900,pstTm->tm_mon+1,pstTm->tm_mday,pstTm->tm_hour,pstTm->tm_min,pstTm->tm_sec);
	}	
	int iRet = json_to_serial(strJson,strSerial,string(TimeBuf));
	if (iRet != 0)
	{
		snprintf(pLicen,sizeof(pLicen)," json decode failed, err code:%d\n",iRet);
		strErrMsg = strJson + pLicen;
		return -1;
	}
	Json::Reader reader;
	Json::Value response;

	if (reader.parse(strJson,response)) 
	{
		if (response.size() == 0) { 
			snprintf(pLicen,sizeof(pLicen)," json decode failed, json size:%d \n",response.size());
			strErrMsg = strJson + pLicen;
			return -2;
		}	
		if (response.type() != Json::objectValue) {
			snprintf(pLicen,sizeof(pLicen)," json decode failed, type:%d is not json objectValue\n",response.type());
			strErrMsg = strJson + pLicen;
			return -1;
		}
	}
	string strPK ; 
	strPK.assign(pPriKey,uPriKeyLen);
	iRet = get_licence(pLicen,&LicenLen,strSerial.c_str(),strSerial.length(),pPriKey,uPriKeyLen);
	if (iRet != 0) 
	{
		snprintf(pLicen,sizeof(pLicen),"get_licence err code:%d\n",iRet);
		strErrMsg = strSerial + pLicen;
		return -2;
	}

	char pBase64Licen[1024]; int pBase64LicenLen = sizeof(pBase64Licen);

	iRet = base64_encode((const unsigned char*)pLicen,(int32_t)LicenLen,pBase64Licen,&pBase64LicenLen);
	if (iRet != 0) 
	{
		snprintf(pLicen,sizeof(pLicen)," base64_encode failed %d\n",iRet);
		strErrMsg = pLicen;
		return -3;
	}

	string sSig; 
	sSig.assign(pBase64Licen,pBase64LicenLen);

	response["TLS.sig"] = sSig;
	response["TLS.time"] = string(TimeBuf);
	strSig = response.toStyledString();

	if (dwFlag == TIME_UT_FORM) {	
		uLongf uLen = sizeof(pBase64Licen);
		iRet = compress((Bytef*)pBase64Licen,&uLen,(const Bytef*)strSig.c_str(),strSig.length());
		if (iRet != Z_OK)
		{
			snprintf(pLicen,sizeof(pLicen)," compress failed %d\n",iRet);
			strErrMsg = pLicen;
			return -3;
		}
		strSig.assign(pBase64Licen,uLen);
		pBase64LicenLen = sizeof(pBase64Licen);
		iRet = base64_encode_url((const unsigned char*)strSig.c_str(),strSig.length(),pBase64Licen,&pBase64LicenLen);
		if (iRet != 0) 
		{
			snprintf(pLicen,sizeof(pLicen)," base64_encode failed %d\n",iRet);
			strErrMsg = pLicen;
			return -3;
		}
		strSig.assign(pBase64Licen,pBase64LicenLen);
	}
	return 0;
}

int json_check_section(const string& strSection,const string& strSigSection,string& strErrMsg)
{
	char buf[256] = {0};
	if (strSection == strSigSection) return 0; 
	snprintf(buf,sizeof(buf),"sig(%s) != req(%s)",
			strSection.c_str(),strSigSection.c_str());
	strErrMsg = buf;
	return 1;
}

int json_to_check(const string& strJson,const SigInfo& stSigInfo,uint32_t& dwExpireTime,uint32_t& dwInitTime,int iFlag,string& strErrMsg)
{
	Json::Reader reader;
	Json::Value response;
	string identifier;
	string sdk_appid;

	if (reader.parse(strJson,response)) 
	{
		if (response.size() == 0)
        {
			strErrMsg += "response empty";
			return CHECK_ERR4;
		}	

        if (!response["TLS.identifier"].isString()) return CHECK_ERR13;
		identifier = response["TLS.identifier"].asString();
		if (json_check_section(identifier, stSigInfo.strIdentify, strErrMsg))
		{
			strErrMsg += "(identifier not match)";
			return CHECK_ERR13;
		}

        if (!response["TLS.sdk_appid"].isString())
            return CHECK_ERR14;
		sdk_appid = response["TLS.sdk_appid"].asString();
		if (json_check_section(sdk_appid,stSigInfo.strAppid,strErrMsg))
		{
			strErrMsg += "(sdk_appid not match)";
			return CHECK_ERR14;
		}
		struct tm sttm; memset(&sttm,0,sizeof(struct tm));
        if (!response["TLS.time"].isString()) return CHECK_ERR10; 
		string strTime = response["TLS.time"].asString();	
		if (iFlag != TIME_UT_FORM) {
			sscanf(strTime.c_str(),"%04d-%02d-%02dT%02d:%02d:%02d-08:00"
					,&sttm.tm_year,&sttm.tm_mon,&sttm.tm_mday,&sttm.tm_hour,&sttm.tm_min,&sttm.tm_sec);
			sttm.tm_year = sttm.tm_year - 1900;
			sttm.tm_mon = sttm.tm_mon - 1;
			dwInitTime = mktime(&sttm);
		} else {
			dwInitTime = strtoul(strTime.c_str(),NULL,10);
		}
        if (!response["TLS.expire_after"].isString()) return CHECK_ERR10; 
		dwExpireTime = strtoul(response["TLS.expire_after"].asString().c_str(),NULL,10);
	}
	else
	{
		strErrMsg = strJson + "---parse failed";
		return CHECK_ERR10;
	}

	return 0;
}


int tls_check_signature_ex(
    const string& strSig,
    char* pPubKey,
    uint32_t uPubKeyLen,
    const SigInfo& stSigInfo,
    uint32_t& dwExpireTime,
    uint32_t& dwInitTime,
    string& strErrMsg)
{
	Json::Reader reader;
	Json::Value response;

	if (strSig.empty()) {
		strErrMsg = "strSig empty";
		return CHECK_ERR1;
	}

	if (reader.parse(strSig,response)) {
		int ret = 0;
		if ((ret = tls_check_signature(strSig,pPubKey,uPubKeyLen,strErrMsg,0)) != 0) return ret;
		return json_to_check(strSig,stSigInfo,dwExpireTime,dwInitTime,0,strErrMsg);		
	} else {
		char buf[1024]; int iLen = sizeof(buf);
		int iRet = base64_decode_url((const unsigned char*)strSig.c_str(),strSig.length(),buf,&iLen);
		if (iRet != 0) {
			snprintf(buf,sizeof(buf)," base64_decode failed, data len:%zu,iRet:%d\n",strSig.length(),iRet);
			strErrMsg = buf;
			return CHECK_ERR2;
		}
		char buf2[1024];
		uLongf uLen = sizeof(buf2); //string tmp;
		iRet = uncompress((Bytef*)buf2, &uLen,(const Bytef*)buf,iLen);
        //tmp.assign((char*)buf2,uLen);  tmp += "try:";
		if (iRet !=  Z_OK) {
			snprintf(buf,sizeof(buf), "uncompress failed iRet:%d",iRet);
			strErrMsg = buf;
			return CHECK_ERR3;
		}
		int iret = 0;
		string strJsonSig ;strJsonSig.assign(buf2,uLen);
		if ((iret = tls_check_signature(strJsonSig,pPubKey,uPubKeyLen,strErrMsg,TIME_UT_FORM)) != 0)  { 
            //strErrMsg += tmp;    
            return iret;
        }       
		iret = json_to_check(strJsonSig,stSigInfo,dwExpireTime,dwInitTime,TIME_UT_FORM,strErrMsg);		
        return iret;
    }
	return 0;
}

TLS_API int tls_check_signature_ex2(
    const string& strSig,
    const string& strPubKey,
    uint32_t dwSdkAppid,
    const string& strIdentifier,
    uint32_t& dwExpireTime,
    uint32_t& dwInitTime,
    string& strErrMsg)
{
	Json::Reader reader;
	Json::Value response;

	if (strSig.empty())
    {
		strErrMsg = "strSig empty";
		return CHECK_ERR1;
	}
    
    SigInfo sigInfo;
    sigInfo.strAccountType = "0";
    sigInfo.strAppid3Rd = "0";
    sigInfo.strIdentify = strIdentifier;
    std::stringstream ss;
    ss << dwSdkAppid;
    sigInfo.strAppid = ss.str();

    // 为了兼容旧的格式，首先使用 json 格式尝试解析
	if (reader.parse(strSig,response))
    {
        // 并且时间格式采用的是字符串
		int ret = tls_check_signature(strSig, const_cast<char *>(strPubKey.c_str()), strPubKey.size(), strErrMsg, 0);
        if (0 != ret)
        {
            return ret;
        }        
		return json_to_check(strSig, sigInfo, dwExpireTime, dwInitTime, 0, strErrMsg);
	}
    else
    {
        // 使用了 base64 编码并且压缩的版本
		char buf[1024];
        int iLen = sizeof(buf);
		int iRet = base64_decode_url((unsigned char *)(strSig.c_str()), strSig.size(), buf, &iLen);
		if (iRet != 0) {
			snprintf(buf,sizeof(buf)," base64_decode failed, data len:%zu,iRet:%d\n",strSig.length(),iRet);
			strErrMsg = buf;
			return CHECK_ERR2;
		}
		char buf2[1024];
		uLongf uLen = sizeof(buf2);
		iRet = uncompress((Bytef*)buf2, &uLen,(const Bytef*)buf,iLen);
		if (iRet !=  Z_OK)
        {
            // 加压缩失败
			snprintf(buf,sizeof(buf), "uncompress failed iRet:%d",iRet);
			strErrMsg = buf;
			return CHECK_ERR3;
		}

		string strJsonSig;
        strJsonSig.assign(buf2,uLen);
        iRet = tls_check_signature(
            strJsonSig, const_cast<char *>(strPubKey.c_str()),
            strPubKey.size(), strErrMsg, TIME_UT_FORM);
		if (0 != iRet)
        {
            return iRet;
        }
        
		return json_to_check(strJsonSig, sigInfo, dwExpireTime, dwInitTime, TIME_UT_FORM, strErrMsg);
    }
	return 0;
}

int tls_gen_signature_ex(
    uint32_t dwExpire,
    const string& strAppid3rd,
    uint32_t dwSdkAppid,
    const string& strIdentifier,
    uint32_t dwAccountType,
    string& strSig,
    char* pPriKey,
    uint32_t uPriKeyLen,
    string& strErrMsg)
{
	char buff[512];
	int iLen = snprintf(buff,sizeof(buff),JSON_STRING
			,dwAccountType,strIdentifier.c_str()
			,strAppid3rd.c_str(),dwSdkAppid,dwExpire
			,API_VERSION);
	if (iLen >= (int)(sizeof(buff) - 1)) {
		snprintf(buff,sizeof(buff),"gen sig buf is empty iLen:%d",iLen);
		strErrMsg = buff;
		return -1;
	}
	string strJson = buff; 
	return tls_gen_signature(strJson,strSig,pPriKey,uPriKeyLen,strErrMsg,TIME_UT_FORM);
}

// 带有效期生成 sig 的接口
TLS_API int tls_gen_signature_ex2_with_expire(
    uint32_t dwSdkAppid,
    const string& strIdentifier,
    uint32_t dwExpire,
    string& strSig,
    string& strPriKey,
    string& strErrMsg)
{
	char buff[512];
	int iLen = snprintf(buff, sizeof(buff), JSON_STRING, 0, strIdentifier.c_str(), "0", dwSdkAppid, dwExpire, API_VERSION);
	if (iLen >= (int)(sizeof(buff) - 1)) {
		snprintf(buff,sizeof(buff),"gen sig buf is empty iLen:%d",iLen);
		strErrMsg = buff;
		return -1;
	}
	string strJson = buff; 
    int ret = tls_gen_signature(
        strJson, strSig, const_cast<char *>(strPriKey.c_str()),
        strPriKey.size(), strErrMsg, TIME_UT_FORM);
	return ret;
}

// 简化版生成 sig 的接口
TLS_API int tls_gen_signature_ex2(
    uint32_t dwSdkAppid,
    const string& strIdentifier,
    string& strSig,
    string& strPriKey,
    string& strErrMsg)
{
    return tls_gen_signature_ex2_with_expire(
        dwSdkAppid, strIdentifier, DEFAULT_EXPIRE, strSig, strPriKey, strErrMsg);
}
