//
//  QHVCEditTestMacroDefs.h
//  QHVCEditKitTests
//
//  Created by liyue-g on 2018/12/5.
//  Copyright © 2018 liyue-g. All rights reserved.
//

#ifndef QHVCEditTestMacroDefs_h
#define QHVCEditTestMacroDefs_h

#define QHVCEDIT_TEST_RETURN_VALUE(expression) {\
if(expression != 0) \
return -1; \
}

#define QHVCEDIT_TEST_OBJECT(expression) {\
if(!expression) \
return -1; \
}

#endif /* QHVCEditTestMacroDefs_h */
