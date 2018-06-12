//
//  infobird_sig.c
//  infobirdimsig
//
//  Created by anjingxw@126.com on 2017/3/22.
//  Copyright © 2017年 xunniao. All rights reserved.
//

#import "infobird_sig.h"
#import "tls_signature.h"

@interface GenSigHelper ()

@property (nonatomic, assign) uint32_t appType;
@property (nonatomic, strong) NSString* priKey;

@end

@implementation GenSigHelper

-(instancetype)init:(uint32_t)appType priKey:(NSString*)priKey;{
    self = [super init];
    if (self) {
        self.appType = appType;
        self.priKey = priKey;
    }
    return self;
}

-(void)GenSig:(NSString*)identifier result:(void(^)(int result ,NSString *sig, NSString *error))reusltHandler{
    std::string strError;
    std::string strSig;
    std::string strIdentifier([identifier cStringUsingEncoding:NSASCIIStringEncoding]);
    //std::string strPriKey([self.priKey cStringUsingEncoding:NSASCIIStringEncoding]);
    
//    static const char kKeyPEM[] =
//    "-----BEGIN PRIVATE KEY-----\n"
//    "MIGHAgEAMBMGByqGSM49AgEGCCqGSM49AwEHBG0wawIBAQQgBw8IcnrUoEqc3VnJ\n"
//    "TYlodwi1b8ldMHcO6NHJzgqLtGqhRANCAATmK2niv2Wfl74vHg2UikzVl2u3qR4N\n"
//    "Rvvdqakendy6WgHn1peoChj5w8SjHlbifINI2xYaHPUdfvGULUvPciLB\n"
//    "-----END PRIVATE KEY-----\n";
    
      static const char kKeyPEM[] = "-----BEGIN PRIVATE KEY-----\n"
    "MIGEAgEAMBAGByqGSM49AgEGBSuBBAAKBG0wawIBAQQg8GZwgg7t9PVj7WFh/KMS\n"
    "f9PKtofHbwmHYbDEQOIHWK2hRANCAARyPgsURfruNjTjKdIsrPmHhRhu6Aulda9O\n"
    "2/YPT+m3jWNOICHU1eM5bq6jO9DJcnFmtLISiKAxuDsYY1uzHsTV\n"
    "-----END PRIVATE KEY-----\n";
    
    std::string strPriKey(kKeyPEM);
    
    int ret = tls_gen_signature_ex2(self.appType, strIdentifier, strSig, strPriKey , strError);
    NSString *sig = [NSMutableString stringWithCString:strSig.c_str() encoding:NSASCIIStringEncoding];
    NSString *errorMsg = [NSMutableString stringWithCString:strError.c_str() encoding:NSASCIIStringEncoding];
    
    reusltHandler(ret, sig, errorMsg);
}
@end

