#ifndef MAKEUP_DEF_H
#define MAKEUP_DEF_H

#include <string>

///@brief 美颜参数类型
typedef enum
{
	ZW_BEAUTIFY_NONE			= 0,	/// 无
	ZW_BEAUTIFY_SMOOTH          = 1,	/// 磨皮强度, [0,1.0], 默认值0.74
	ZW_BEAUTIFY_SOFT            = 2,	/// 柔嫩强度, [0,1.0], 默认值0.5
	ZW_BEAUTIFY_SATURATE        = 3,	/// 饱和程度, [0,1.0], 默认值0.2
	ZW_BEAUTIFY_CONTRAST        = 4,	/// 对比程度, [0,1.0], 默认值0.5
	ZW_BEAUTIFY_SHARP           = 5,	/// 锐化程度, [0,1.0], 默认值0.5

	ZW_BEAUTIFY_ENLARGE_EYE     = 6,	/// 大眼比例 [-1.0,1.0], 默认值0.0
	ZW_BEAUTIFY_SHRINK_FACE     = 7, 	/// 瘦脸比例 [-1.0,1.0], 默认值0.0
	ZW_BEAUTIFY_SMALL_FACE      = 8,    /// 小脸比例 [-1.0,1.0], 默认值0.0
	ZW_BEAUTIFY_MORPH           = 9,    /// 人脸拉伸比例， [0, 1.0], 默认值0.10
	ZW_BEAUTIFY_MASK            = 10,   /// 面具融合比例， [0, 1.0], 默认值0.10

	ZW_BEAUTIFY_BLING           = 11,    /// 闪光效果

    ZW_BEAUTIFY_MOUTH           = 14,    /// 嘴型 [-1.0,1.0], 默认值0.0
    ZW_BEAUTIFY_NOSE            = 15,    /// 鼻型 [-1.0,1.0], 默认值0.0
    ZW_BEAUTIFY_HEAD            = 16,    /// 瘦头 [-1.0,1.0], 默认值0.0
    ZW_BEAUTIFY_FOREHEAD        = 17,    /// 额头 [-1.0,1.0], 默认值0.0
    ZW_BEAUTIFY_JAW             = 18,    /// 下巴 [-1.0,1.0], 默认值0.0
    ZW_BEAUTIFY_MOVE_EYE        = 19,    /// 眼距 [-1.0,1.0], 默认值0.0
    ZW_BEAUTIFY_ROTATE_EYE      = 20,    /// 转眼 [-1.0,1.0], 默认值0.0
    ZW_BEAUTIFY_ROTATE_EYEBROW  = 21,    /// 转眉毛 [-1.0,1.0], 默认值0.0

} zw_beautify_type;


///@brief 微整形参数值
class PLASTIC_PARAM
{
public:
    int   nType;           ///< 参数类型,参见zw_beautify_type
    float fXRatio;              ///< x_ratio
    float fYRatio;              ///< y_ratio
    float fXScale;              ///< x_scale
    float fYScale;              ///< y_scale
    float fVertHorzRatio;       ///< 椭圆的短轴与长轴的比值
    std::string strLutFileName; ///< 查找表文件名
};

namespace FaceMakeup
{
    ///@brief 微整形参数值
    class MAKEUP_DATA
    {
    public:
        MAKEUP_DATA()
                :fXRatio(0.0f)
                ,fYRatio(0.0f)
                ,fVertHorzRatio(1.0f)
                ,fXScale(0.0f)
                ,fYScale(0.0f)
        {}

    public:
        float fXRatio;  ///< X compress ratio
        float fYRatio;  ///< Y compress ratio
        float fVertHorzRatio;   ///< vertical horizontal ratio for eclipse
        float fXScale;  ///< X compress scale
        float fYScale;  ///< Y compress scale
        std::string strLutFilePath; ///look up table file path
    };
}

#endif
