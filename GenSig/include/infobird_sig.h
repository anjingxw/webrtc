#import  <Foundation/Foundation.h>


@interface GenSigHelper : NSObject

-(instancetype)init:(uint32_t)appType priKey:(NSString*)priKey;

-(void)GenSig:(NSString*)identifier result:(void(^)(int result ,NSString *sig, NSString *error))reuslt;

@end
